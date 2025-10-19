// @ts-check
/// curl -Ok https://raw.githubusercontent.com/microsoft/vscode/main/src/vscode-dts/vscode.d.ts
/// <reference path="./vscode.d.ts" />
const vscode = require("vscode");
const crypto = require('crypto');
const https = require('https');
const conf = () => vscode.workspace.getConfiguration("dzr");
const location = vscode.ProgressLocation.Notification;
const hhmmss = (s) => (new Date(s * 1000)).toISOString().slice(11, 19).replace(/^00:/, '');
const wait = (ms = 1000) => new Promise(resolve => setTimeout(resolve, ms));
// still no fetch() in 2023 ?
const fetch = (url, opt, data) => new Promise((resolve, reject) => {
	console.log({ url, opt, data })
	const chunks = [], req = https.request(url, { rejectUnauthorized: conf().get('secure'), ...opt }, res => {
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
async function browse(url_or_event_or_ids, label) {
	try {
		if (Array.isArray(url_or_event_or_ids)) return url_or_event_or_ids.map(id => ({ id }));
		const ignoreFocusOut = true;
		const url = typeof (url_or_event_or_ids) == "string" ? url_or_event_or_ids : '/';
		const id = url.replace(/\d+/g, '0').replace(/[^\w]/g, '_');
		const menus = conf().get('menus');
		const title = (label || '').replace(/\$\(.+?\)/g, '');
		if (url.endsWith('=') || url.endsWith('/0')) { // query step
			const input = await vscode.window.showInputBox({ title, ignoreFocusOut });
			if (!input) return;
			return await browse(url.replace(/0$/, '') + input, `${label}: ${input}`);
		} else if (menus[id]) { // menu step
			const pick = menus[id].length > 1 ? await vscode.window.showQuickPick(menus[id], { title: title || url, ignoreFocusOut }) : menus[id][0];
			if (!pick) return;
			return await browse(url + pick.path, pick.label);
		} else { // fetch step
			/**@type {{type:string,title_short:string,artist:{name:string},title_version:string,title:string,name:string,nb_tracks:string,id:number}[]} */
			const data = [];
			for (let n = conf().get('nextCount'), json, nextUrl = "https://api.deezer.com" + url; --n && nextUrl && (json = JSON.parse(await fetch(nextUrl))); nextUrl = json.next) {
				data.push(...(json.data?.tracks || json.data || json.tracks?.data));
			}
			const picked = !!url.match(/\/(playlist|album)\//);
			const canPickMany = !!data.find(item => item.type == "track");
			const type2icon = conf().get('type2icon');
			const choices = data.map(entry => ({
				...entry, picked,
				label: (type2icon[entry.type] || '') + (entry.title_short || entry.name || entry.title),
				description: [entry.artist?.name, entry.title_version, entry.nb_tracks].join(' '),
				path: `/${entry.type}/${entry.id}`,
			}));
			const picks = await vscode.window.showQuickPick(choices, { title: title || url, canPickMany, ignoreFocusOut });
			if (!picks) return;
			return canPickMany ? picks : await browse(picks.path, picks.label);
		}
	} catch (e) { console.error(e) }
}
let DZR_PNG, USR_NFO;
const with_url = async (songs) => songs?.length ? await vscode.window.withProgress({ title: 'Fetching', location }, async (progress) => {
	try { // take 7s (with, or without agent)
		const next = (message, val) => (progress.report({ increment: 100 / 4, message }), val);
		const gw = async (method, arl, sid, api_token = "", opt = {}, data) => JSON.parse(await fetch(`${base}&method=${method}&api_token=${api_token}`,
			{ ...opt, headers: { Cookie: `sid=${sid}; arl=${arl}`, ...opt?.headers } }, data)).results;
		const base = "https://www.deezer.com/ajax/gw-light.php?input=3&api_version=1.0";
		let DZR_ARL = next("ARL", conf().get('arl'));
		const format = conf().get('format') || 'MP3_128';
		if (!DZR_ARL) {
			DZR_ARL = (await vscode.window.showInputBox({
				ignoreFocusOut: true,
				placeHolder: "deezer://autolog/xxxx",
				prompt: "Login on [deezer.com](https://www.deezer.com/), then copy the button address displayed on [this page](https://www.deezer.com/desktop/login/electron/callback)"
			}))?.match(/[0-9a-f]{192}/)?.[0];
			if (!DZR_ARL) return vscode.window.showErrorMessage("No ARL found");
			conf().update('arl', DZR_ARL, vscode.ConfigurationTarget.Global);
		}
		DZR_PNG = next("session", DZR_PNG || await gw('deezer.ping', DZR_ARL));
		USR_NFO = next("user right", USR_NFO || await gw('deezer.getUserData', DZR_ARL, DZR_PNG.SESSION));
		const SNG_NFO = next("song info", await gw('song.getListData', DZR_ARL, DZR_PNG.SESSION, USR_NFO.checkForm, { method: 'POST' }, JSON.stringify({ sng_ids: songs.map(s => s.id) }))).data.map(e => e.FALLBACK || e);
		const GET_URL = next("song stream", JSON.parse(await fetch('https://media.deezer.com/v1/get_url', { method: 'POST' }, JSON.stringify({
			track_tokens: SNG_NFO.map(d => d.TRACK_TOKEN),
			license_token: USR_NFO.USER.OPTIONS.license_token,
			media: [{ type: "FULL", formats: [{ cipher: "BF_CBC_STRIPE", format }] }]
		}))))?.data?.map((url, i) => Object.assign(SNG_NFO[i], url));
		const errors = SNG_NFO.filter(e => e.errors).map(e => `[${e.SNG_ID}] ${e.ART_NAME} - ${e.SNG_TITLE}:${JSON.stringify(e.errors)}`);
		const skiped = SNG_NFO.filter(s => !s.media?.[0]?.sources?.[0]?.url).map(e => `[${e.SNG_ID}] no MEDIA`);
		if (errors.length || skiped.length) {
			vscode.window.showWarningMessage([...errors, ...skiped].join('\n'), "Continue", "Flush ARL")
				.then(e => { if (e == "Flush ARL") conf().update('arl', undefined, vscode.ConfigurationTarget.Global) });
		}
		return SNG_NFO.map(nfo => ({
			id: nfo.SNG_ID,
			md5_image: nfo.ALB_PICTURE,
			duration: +nfo.DURATION,
			title: nfo.SNG_TITLE.replace(/ ?\(feat.*?\)/, ''),
			artists: (nfo.ARTISTS || []).map(a => ({ id: a.ART_ID, name: a.ART_NAME, md5: a.ART_PICTURE })),
			size: +nfo.FILESIZE,
			expire: nfo.TRACK_TOKEN_EXPIRE,
			url: nfo.media?.[0]?.sources?.[0]?.url
		})).filter(nfo => nfo.url);
	} catch (e) {
		console.error(e);
		vscode.window.showErrorMessage(e);
		return []
	}
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
	/**@type {vscode.WebviewPanel|null}*/
	panel = null;
	#state = {};
	state = new Proxy(this.#state, {
		set: (target, k, value) => {
			const key = String(k);
			target[key] = value;
			if (['queue', 'looping'].includes(key)) { // persist those values across reboot
				conf().update(key, value, vscode.ConfigurationTarget.Global);
			}
			if (key == 'queue') this._onDidChangeTreeData.fire(null);
			vscode.commands.executeCommand('setContext', `dzr.${key}`, value);
			this.post('state', target, [key]);
			this.renderStatus();
			return true;
		}
	});

	constructor() {
		this.initAckSemaphore();
		this.state.queue = conf().get('queue'); // first is best
		this.state.looping = conf().get('looping');
	}
	renderStatus() {
		const index = this.state.queue?.indexOf(this.state.current);
		const label = this.state.current ? `${this.state.current.title} - ${this.state.current.artists?.map(a => a.name).join()}` : '';
		this.statuses[0].command = this.state.playing ? 'dzr.pause' : 'dzr.play';
		this.statuses[0].text = this.state.ready && (this.state.playing ? "$(debug-pause)" : "$(play)");
		this.statuses[1].tooltip = this.state.ready ? label : "Initiate interaction first";
		this.statuses[1].text = this.state.ready ? label.length < 20 ? label : (label.slice(0, 20) + '…') : "$(play)"
		this.statuses[2].text = this.state.ready && this.state.queue.length ? `${index + 1 || '?'}/${this.state.queue.length} $(chevron-right)` : '';//debug-step-over
		this.treeView.title = (this.state.queue?.length ? `${index + 1 || '?'}/${this.state.queue.length}` : '') + ` loop:${this.state.looping}`;
		this.treeView.message = this.state.queue?.length ? "" : "Empty Queue. Add tracks from the '+' menu";
		this.treeView.badge = { tooltip: label, value: this.state.playing ? index + 1 : 0 }
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
		this.panel.webview.onDidReceiveMessage(([action, ...args] = []) => this[action] ? this[action](...args) : this.badAction(action));
		this.panel.onDidDispose(() => this.state.ready = this.panel = null);
		this.post('state', this.state, Object.keys(this.state));
	}
	initAckSemaphore() { this.postAck = new Promise((then) => this.waitAckSemaphore = then); }
	post = (action, ...arg) => this.panel?.webview.postMessage([action, ...arg]);
	// event from webview player
	player_bufferized() {
		this.waitAckSemaphore?.(0);
		this.initAckSemaphore();
	}
	player_volumechange({ volume }) { conf().update("volume", volume, vscode.ConfigurationTarget.Global); }
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
	/**@type {import('vscode').TreeView}*/
	treeView = vscode.window.createTreeView('dzr.queue', { treeDataProvider: this, dragAndDropController: this, canSelectMany: true });
	highlighted = (label, active) => ({ label, highlights:/**@type {[number, number][]}*/(active ? [[0, label.length]] : []) })
	/**@returns {vscode.TreeItem} */
	getTreeItem = (item) => ({
		iconPath: new vscode.ThemeIcon("music"),
		label: this.highlighted(item.title + ' - ' + item.artists.map(a => a.name).join(), item == this.state.current),
		description: hhmmss(item.duration || 0) + " " + (item.version || ''),
		contextValue: 'dzr.track',
		command: { title: 'Play', command: 'dzr.load', tooltip: 'Play', arguments: [this.state.queue.indexOf(item)] },
	})
	getParent = () => null
	getChildren = () => {

		return this.state.queue
	}
	async handleDrag(sources, treeDataTransfer) {
		treeDataTransfer.set(this.dropMimeTypes[0], new vscode.DataTransferItem(sources));
	}
	async handleDrop(onto, transfer) {
		const sources = transfer.get(this.dropMimeTypes[0])?.value;
		if (!sources || sources.includes(onto)) return; //don't move selection onto one of it members
		const striped = this.state.queue.filter(item => !sources.includes(item));
		const index = this.state.queue.indexOf(onto);
		this.state.queue = [...striped.slice(0, index), ...sources, ...striped.slice(index)];
	}
}
exports.activate = async function (/**@type {import('vscode').ExtensionContext}*/ context) {
	// deezer didn't DMCA'd dzr so let's follow the same path here
	conf().get('cbc') || vscode.window.withProgress({ title: 'Extracting CBC key...', location }, async () => {
		const html_url = 'https://www.deezer.com/en/channels/explore';
		const html = (await fetch(html_url)).toString('utf-8');
		const js_url = html.match(/src="(http[^"]+app-web\.[^"]+\.js)"/)?.[1];
		if (!js_url) return await vscode.window.showErrorMessage('CBC Extract: No JS WebApp found');
		const keys = (await fetch(js_url)).toString('utf-8').match(/%5B0x..%2C.{39}%2C0x..%5D/g);
		const [a, b] = keys.map(part => part.slice(3, -3).split('%2C').map(i => String.fromCharCode(parseInt(i))).reverse());
		const cbc = a.map((a, i) => `${a}${b[i]}`).join('');// zip a+b
		const sha = crypto.createHash('sha1').update(cbc).digest('hex').slice(0, 8);
		if (sha != '3ad58d92') return await vscode.window.showErrorMessage('Bad extracted key');
		conf().update('cbc', cbc, vscode.ConfigurationTarget.Global);
	});
	const dzr = new DzrWebView();
	const htmlUri = vscode.Uri.joinPath(context.extensionUri, 'webview.html');
	const iconUri = vscode.Uri.joinPath(context.extensionUri, 'logo.svg'); //same for light+dark

	context.subscriptions.push(...dzr.statuses, dzr.treeView,
		// catch vscode://yne.dzr/* urls
		vscode.window.registerUriHandler({ handleUri(uri) { (({ path, query }) => vscode.commands.executeCommand(`dzr.${path.slice(1)}`, ...(query ? JSON.parse(query) : [])))(uri); } }),
		vscode.commands.registerCommand('dzr.show', () => dzr.show(htmlUri, iconUri)),
		vscode.commands.registerCommand("dzr.play", () => dzr.post('play')),
		vscode.commands.registerCommand("dzr.pause", () => dzr.post('pause')),
		vscode.commands.registerCommand("dzr.href", (track) => vscode.env.openExternal(vscode.Uri.parse(`https://deezer.com/track/${track.id}`))),
		vscode.commands.registerCommand("dzr.loopQueue", () => dzr.state.looping = "queue"),
		vscode.commands.registerCommand("dzr.loopTrack", () => dzr.state.looping = "track"),
		vscode.commands.registerCommand("dzr.loopOff", () => dzr.state.looping = "off"),
		vscode.commands.registerCommand("dzr.add", async (path, label) => with_url(await browse(path, label)).then(tracks => dzr.state.queue = [...dzr.state.queue, ...tracks])),
		vscode.commands.registerCommand("dzr.remove", async (item, items) => (items || [item]).map(i => vscode.commands.executeCommand('dzr.removeAt', dzr.state.queue.indexOf(i)))),
		vscode.commands.registerCommand("dzr.removeAt", async (index) => index >= 0 && (dzr.state.queue = [...dzr.state.queue.slice(0, index), ...dzr.state.queue.slice(index + 1)])),
		vscode.commands.registerCommand("dzr.clear", async () => dzr.state.queue = []),
		vscode.commands.registerCommand("dzr.shareAll", async () => vscode.commands.executeCommand("dzr.share")),
		vscode.commands.registerCommand("dzr.share", async (track, tracks) => {
			const ids = JSON.stringify(track ? [(tracks || [track]).map(e => e.id || track.id)] : [dzr.state.queue.map(q => q.id)]);
			vscode.env.clipboard.writeText(vscode.Uri.from({ scheme: "vscode", authority: context.extension.id, path: '/add', query: ids }).toString())
		}),
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
			if ((dzr.state.queue[pos].expire || 0) < (+new Date() / 1000)) {
				dzr.state.queue = await with_url(dzr.state.queue);//TODO: hope item is now up to date
			}
			const prev = dzr.state?.current;
			dzr.state.current = dzr.state.queue[pos];
			dzr._onDidChangeTreeData.fire(prev);
			dzr._onDidChangeTreeData.fire(this.state?.current);
			const hex = (str) => str.split('').map(c => c.charCodeAt(0))
			const md5 = hex(crypto.createHash('md5').update(`${dzr.state.current.id}`).digest('hex'));
			const key = Buffer.from(hex(conf().get('cbc')).map((c, i) => c ^ md5[i] ^ md5[i + 16]));
			const iv = Buffer.from([0, 1, 2, 3, 4, 5, 6, 7]);
			const stripe = 2048;//TODO:use .pipe() API https://codereview.stackexchange.com/questions/57492/
			dzr.post('open', dzr.state.current, conf().get("volume"));
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