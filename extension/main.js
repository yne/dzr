/**@type import('vscode') */
const vscode = require("vscode");
const crypto = require('crypto');
const https = require('https');
const conf = vscode.workspace.getConfiguration("dzr");
const location = vscode.ProgressLocation.Notification;
const hhmmss = (s) => (new Date(s * 1000)).toISOString().slice(11, 19).replace(/^00:/, '');
const wait = (ms = 1000) => new Promise(resolve => setTimeout(resolve, ms));
// still no fetch() in 2023 ?
const fetch = (url, opt, data) => new Promise((resolve, reject) => {
	const chunks = [], req = https.request(url, opt, res => {
		res.on('data', chunk => chunks.push(chunk));
		res.on('end', () => resolve(Buffer.concat(chunks)));
	}).on('error', reject);
	if (data) req.write(data);
	req.end();
});

// deezer API wall of shame:
// - not restful, so we can't infer it structure
// - /track/:id gives contributors but /search/track?q= don't
// - inconsistent listing structure (/playlist/:id => tracks.data, sometimes=>data, sometimes data.tracks)
// browse can be called from: user query / self list(from static menu) / self list(from fetch result)
async function browse(url_or_event, label) {
	console.log(url_or_event);
	try {
		const url = typeof (url_or_event) == "string" ? url_or_event : '/';
		const id = url.replace(/\d+/g, '0').replace(/[^\w]/g, '_');
		const menus = conf.get('menus');
		const title = (label || '').replace(/\$\(.+?\)/g, '');
		if (url.endsWith('=') || url.endsWith('/0')) { // query step
			const input = await vscode.window.showInputBox({ title });
			if (!input) return;
			return await browse(url.replace(/0$/, '') + input, `${label}: ${input}`);
		} else if (menus[id]) { // menu step
			const pick = menus[id].length > 1 ? await vscode.window.showQuickPick(menus[id], { title: title || url }) : menus[id][0];
			if (!pick) return;
			return await browse(url + pick.path, pick.label);
		} else { // fetch step
			const json = JSON.parse(await fetch("https://api.deezer.com" + url)); // todo: json.next?
			console.debug(json);
			const data = json.data?.tracks || json.data || json.tracks?.data;
			const picked = url.match(/\/(playlist|album)\//);
			const canPickMany = data.find(item => item.type == "track");
			const type2icon = conf.get('type2icon');
			const choices = data.map(entry => ({
				...entry, picked,
				label: (type2icon[entry.type] || '') + (entry.title_short || entry.name || entry.title),
				description: [entry.artist?.name, entry.title_version, entry.nb_tracks].join(' '),
				path: `/${entry.type}/${entry.id}`,
			}));
			const picks = await vscode.window.showQuickPick(choices, { title: title || url, canPickMany });
			if (!picks) return;
			return canPickMany ? picks : await browse(picks.path, picks.label);
		}
	} catch (e) { console.error(e) }
}
/* songs may came from API (full info) or storage (light info) */
const with_url = async (songs) => songs?.length ? await vscode.window.withProgress({ title: 'Fetching Song Info...', location }, async (progress) => {
	try { // take 7s (with, or without agent)
		const next = (val) => (progress.report({ increment: 100 / 4 }), val);
		const gw = async (method, sid, api_token = "", opt = {}, data) => JSON.parse(await fetch(`${base}&method=${method}&api_token=${api_token}`,
			{ ...opt, headers: { Cookie: `sid=${sid}`, ...opt?.headers } }, data)).results;
		const base = next("https://www.deezer.com/ajax/gw-light.php?input=3&api_version=1.0");
		const DZR_PNG = next(await gw('deezer.ping'));
		const USR_NFO = next(await gw('deezer.getUserData', DZR_PNG.SESSION));
		const SNG_NFO = next(await gw('song.getListData', DZR_PNG.SESSION, USR_NFO.checkForm, { method: 'POST' }, JSON.stringify({ sng_ids: songs.map(s => s.id) })));
		const URL_NFO = next(JSON.parse(await fetch('https://media.deezer.com/v1/get_url', { method: 'POST' }, JSON.stringify({
			track_tokens: SNG_NFO.data.map(d => d.TRACK_TOKEN),
			license_token: USR_NFO.USER.OPTIONS.license_token,
			media: [{ type: "FULL", formats: [{ cipher: "BF_CBC_STRIPE", format: "MP3_128" }] }]
		}))));
		const errors = URL_NFO.data.map((nfo, i) => [nfo.errors, songs[i]]).filter(([err]) => err).map(([[err], sng]) => `${sng.title}: ${err.message} (${err.code})`).join('\n');
		if (errors) setTimeout(() => vscode.window.showWarningMessage(errors), 500); // can't warn while progress ?
		return songs.map(({/* api :*/ id, md5_image, duration, title_short, title_version, artist, contributors,
		                   /*cache:*/ title, version, artists }, i) => ({
			id, md5_image, duration,
			title: title_short?.replace(/ ?\(feat.*?\)/, '') || title,
			version: title_version || version,
			artists: artists ?? (contributors || [artist])?.map(({ id, name }) => ({ id, name })),
			size: +SNG_NFO.data[i].FILESIZE,
			expire: SNG_NFO.data[i].TRACK_TOKEN_EXPIRE,
			url: URL_NFO.data[i].media?.[0]?.sources?.[0]?.url
		})).filter(sng => sng.url);
	} catch (e) { console.error(e) }
}) : [];

class DzrWebView { // can't Audio() in VSCode, we need a webview
	statuses = ['dzr.play', 'dzr.show', 'dzr.load'].map((command) => {
		const item = vscode.window.createStatusBarItem(command, vscode.StatusBarAlignment.Left, 10000);
		item.color = new vscode.ThemeColor('statusBarItem.prominentBackground');
		item.backgroundColor = new vscode.ThemeColor('statusBarItem.errorBackground');
		item.command = command;
		item.text = command;
		item.show();
		return item;
	});
	panel = null;
	#state = {};
	state = new Proxy(this.#state, {
		set: (target, key, value) => {
			target[key] = value;
			if (['queue', 'looping'].includes(key)) { // persist those values across reboot
				conf.update(key, value, vscode.ConfigurationTarget.Global);
			}
			if (key == 'queue') this._onDidChangeTreeData.fire();
			vscode.commands.executeCommand('setContext', `dzr.${key}`, value);
			this.post('state', target, [key]);
			this.renderStatus();
			return true;
		}
	});

	constructor() {
		this.initAckSemaphore();
		this.state.queue = conf.get('queue'); // first is best
		this.state.looping = conf.get('looping');
	}
	renderStatus() {
		const index = this.state.queue?.indexOf(this.state.current);
		const label = this.state.current ? `${this.state.current.title} - ${this.state.current.artists?.map(a => a.name).join()}` : '';
		this.statuses[0].command = this.state.playing ? 'dzr.pause' : 'dzr.play';
		this.statuses[0].text = this.state.ready && (this.state.playing ? "$(debug-pause)" : "$(play)");
		this.statuses[1].tooltip = this.state.ready ? label : "Initiate interaction first";
		this.statuses[1].text = this.state.ready ? label.length < 20 ? label : (label.slice(0, 20) + '…') : "$(play)"
		this.statuses[2].text = this.state.ready && this.state.queue.length ? `${index + 1 || '?'}/${this.state.queue.length} $(chevron-right)` : null;//debug-step-over
		this.treeView.description = (this.state.queue?.length ? `${index + 1 || '?'}/${this.state.queue.length}` : '') + ` loop:${this.state.looping}`;
		this.treeView.message = this.state.queue?.length ? null : "Empty Queue. Add tracks to queue using '+'";
	}
	async show(htmlUri, iconPath) {
		if (this.panel) return this.panel.reveal(vscode.ViewColumn.One);
		this.panel = vscode.window.createWebviewPanel('dzr.player', 'Player', vscode.ViewColumn.One, {
			enableScripts: true,
			enableCommandUris: true,
			retainContextWhenHidden: true,
		});
		this.panel.iconPath = iconPath;
		this.panel.webview.html = (await vscode.workspace.fs.readFile(htmlUri)).toString();
		this.panel.webview.onDidReceiveMessage((action, ...args) => this[action] ? this[action](...args) : this.badAction(action));
		this.panel.onDidDispose(() => this.state.ready = this.panel = null);
		this.post('state', this.state, Object.keys(this.state));
	}
	initAckSemaphore() { this.postAck = new Promise((then) => this.waitAckSemaphore = then); }
	post = (action, ...arg) => this.panel?.webview.postMessage([action, ...arg]);
	// event from webview player
	player_bufferized() {
		this.waitAckSemaphore();
		this.initAckSemaphore();
	}
	player_playing() { this.state.ready = this.state.playing = true; }
	player_pause() { this.state.playing = false; }
	player_ended() { vscode.commands.executeCommand('dzr.load', null); }
	user_interact() { this.state.ready = true; }
	user_next() { vscode.commands.executeCommand('dzr.load'); }
	error(msg) { vscode.window.showErrorMessage(msg); }
	badAction(action) { console.error(`unHandled action "${action}" from webview`); }
	// tree
	dropMimeTypes = ['application/vnd.code.tree.dzrQueue'];
	dragMimeTypes = ['text/uri-list'];
	_onDidChangeTreeData = new vscode.EventEmitter();
	onDidChangeTreeData = this._onDidChangeTreeData.event;
	treeView = vscode.window.createTreeView('dzr.queue', { treeDataProvider: this, dragAndDropController: this, canSelectMany: true });

	/**@returns {vscode.TreeItem} */
	getTreeItem = (item) => ({
		iconPath: vscode.ThemeIcon.File,
		label: item.title,
		description: item.artists.map(a => a.name).join(),
		contextValue: 'dzr.track',
		command: { title: 'Play', command: 'dzr.load', tooltip: 'Play', arguments: [this.state.queue.indexOf(item)] },
		tooltip: hhmmss(item.duration)//JSON.stringify(item, null, 2),
	})
	getChildren = () => this.state.queue
	async handleDrag(sources, treeDataTransfer, token) {
		treeDataTransfer.set(this.dropMimeTypes[0], new vscode.DataTransferItem(sources));
	}
	async handleDrop(onto, transfer, token) {
		const sources = transfer.get(this.dropMimeTypes[0])?.value;
		if (!sources || sources.includes(onto)) return; //don't move selection onto one of it members
		const striped = this.state.queue.filter(item => !sources.includes(item));
		const index = this.state.queue.indexOf(onto);
		this.state.queue = [...striped.slice(0, index), ...sources, ...striped.slice(index)];
	}
}
exports.activate = async function (/**@type {vscode.ExtensionContext}*/ context) {
	// deezer didn't DMCA'd dzr so let's follow the same path here
	conf.get('cbc') || vscode.window.withProgress({ title: 'Extracting CBC key...', location }, async () => {
		const html_url = 'https://www.deezer.com/en/channels/explore';
		const html = (await fetch(html_url)).toString('utf-8');
		const js_url = html.match(/src="(http[^"]+app-web\.[^"]+\.js)"/)?.[1];
		if (!js_url) return await vscode.window.showErrorMessage('CBC Extract: No JS WebApp found');
		const keys = (await fetch(js_url)).toString('utf-8').match(/%5B0x..%2C.{39}%2C0x..%5D/g);
		const [a, b] = keys.map(part => part.slice(3, -3).split('%2C').map(i => String.fromCharCode(parseInt(i))).reverse());
		const cbc = a.map((a, i) => `${a}${b[i]}`).join('');// zip a+b
		const sha = crypto.createHash('sha1').update(cbc).digest('hex').slice(0, 8);
		if (sha != '3ad58d92') return await vscode.window.showErrorMessage('Bad extracted key');
		conf.update('cbc', cbc, vscode.ConfigurationTarget.Global);
	});
	const dzr = new DzrWebView();
	const htmlUri = vscode.Uri.joinPath(context.extensionUri, 'webview.html');
	const iconUri = vscode.Uri.joinPath(context.extensionUri, 'logo.svg'); //same for light+dark
	context.subscriptions.push(...dzr.statuses, dzr.treeView,
		vscode.commands.registerCommand('dzr.show', () => dzr.show(htmlUri, iconUri)),
		vscode.commands.registerCommand("dzr.play", () => dzr.post('play')),
		vscode.commands.registerCommand("dzr.pause", () => dzr.post('pause')),
		vscode.commands.registerCommand("dzr.loopQueue", () => dzr.state.looping = "queue"),
		vscode.commands.registerCommand("dzr.loopTrack", () => dzr.state.looping = "track"),
		vscode.commands.registerCommand("dzr.loopOff", () => dzr.state.looping = "off"),
		vscode.commands.registerCommand("dzr.add", async (path, label) => dzr.state.queue = [...dzr.state.queue, ...await with_url(await browse(path, label)) || []]),
		vscode.commands.registerCommand("dzr.remove", async (item, items) => (items || [item]).map(i => vscode.commands.executeCommand('dzr.removeAt', dzr.state.queue.indexOf(i)))),
		vscode.commands.registerCommand("dzr.removeAt", async (index) => index >= 0 && (dzr.state.queue = [...dzr.state.queue.slice(0, index), ...dzr.state.queue.slice(index + 1)])),
		vscode.commands.registerCommand("dzr.clear", async () => dzr.state.queue = []),
		vscode.commands.registerCommand("dzr.shuffle", async () => {
			const shuffle = [...dzr.state.queue];
			for (let i = shuffle.length - 1; i > 0; i--) {
				const j = Math.floor(Math.random() * (i + 1));
				[shuffle[i], shuffle[j]] = [shuffle[j], shuffle[i]];
			}
			dzr.state.queue = shuffle;
		}),
		vscode.commands.registerCommand("dzr.load", async (pos) => { //pos=null if player_end / pos=undefine if user click
			pos = pos ?? dzr.state.queue.indexOf(dzr.state.current) + (dzr.state.looping == 'track' ? 0 : 1);
			if (!dzr.state.queue[pos]) { // out of bound track
				if (dzr.state.looping == 'off') return; // don't loop if unwanted
				pos = 0; // loop position if looping
			}
			if (!dzr.state.ready) {
				vscode.commands.executeCommand('dzr.show');
				while (!dzr.state.ready) await wait();
			}
			dzr.state.current = dzr.state.queue[pos];
			if (dzr.state.current.expire < (new Date() / 1000)) {
				dzr.state.queue = await with_url(dzr.state.queue);//TODO: hope item is now up to date
			}
			const hex = (str) => str.split('').map(c => c.charCodeAt(0))
			const md5 = hex(crypto.createHash('md5').update(`${dzr.state.current.id}`).digest('hex'));
			const key = Buffer.from(hex(conf.get('cbc')).map((c, i) => c ^ md5[i] ^ md5[i + 16]));
			const iv = Buffer.from([0, 1, 2, 3, 4, 5, 6, 7]);
			const stripe = 2048;//TODO:use .pipe() API https://codereview.stackexchange.com/questions/57492/
			dzr.post('open', dzr.state.current);
			const buf_enc = await fetch(dzr.state.current.url);
			for (let pos = 0; pos < buf_enc.length; pos += stripe) {
				if ((pos >> 11) % 3) continue;
				const ciph = crypto.createDecipheriv('bf-cbc', key, iv).setAutoPadding(false)
				const deco = ciph.update(buf_enc.subarray(pos, pos + stripe));
				buf_enc.set(deco, pos);
			}
			dzr.post('append', Uint8Array.from(buf_enc));
			await dzr.postAck;
			dzr.post('close');
		}),
	)
}
