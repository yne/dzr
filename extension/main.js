/**@type import('vscode') */
const vscode = require("vscode");
const crypto = require('crypto');
const https = require('https');
const conf = vscode.workspace.getConfiguration("dzr");
const location = vscode.ProgressLocation.Notification;
const type2icon = {
	track: '$(play-circle) ',
	artist: '$(person) ',
	album: '$(issues) ',
	playlist: '$(list-unordered)',
	radio: '$(broadcast) ',
	genre: '$(telescope) ',
	user: '$(account) ',
};

// still no fetch() in 2023 ?
const fetch = (url, opt, data) => new Promise((resolve, reject) => {
	//console.debug(url, opt, data);
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
const menus = {
	_: [
		{ path: 'search/track?q=', label: '$(play-circle) track search' },
		{ path: 'search/artist?q=', label: '$(person) artist search' },
		{ path: 'search/album?q=', label: '$(issues) album search' },
		{ path: 'search/playlist?q=', label: '$(list-unordered) playlist search' },
		{ path: 'search/user?q=', label: '$(account) user search' },
		{ path: 'search/radio?q=', label: '$(broadcast) radio search' },
		{ path: 'genre', label: '$(telescope) explore' },
		{ path: 'radio', label: '$(broadcast) radios list' },
		{ path: 'user/0', label: '$(account) user id' },
	],
	_artist_0: [
		{ path: '/top?limit=50', label: '$(play-circle) Top Tracks' },
		{ path: '/albums', label: '$(issues) Albums' },
		{ path: '/related', label: '$(person) Similar Artists' },
		{ path: '/radio', label: '$(broadcast) Flow' },
		{ path: '/playlists', label: '$(list-unordered) Playlists' }
	],
	_user_0: [
		{ path: '/playlists', label: '$(list-unordered) Playlists' },
		{ path: '/tracks', label: '$(play-circle) Favorite Tracks' },
		{ path: '/albums', label: '$(issues) Favorite Albums' },
		{ path: '/artists', label: '$(person) Favorite Artists' },
		{ path: '/flow', label: '$(broadcast) Flow' },
		{ path: '/charts', label: '$(play-circle) Charts' },
	],
	_genre_0: [{ label: '/radios' }, { label: '/artists' }],
	_radio_0: [{ label: '/tracks' }],
	_album_0: [{ label: '/tracks' }],
}
// browse can be : user query / list(static) / list(fetch)
async function browse(url_or_event) { // sometimes 
	const url = typeof(url_or_event) == "string" ? url_or_event : '/';
	const id = url.replace(/\d+/g, '0').replace(/[^\w]/g, '_');
	if (url.endsWith('=') || url.endsWith('/0')) { // query step
		const input = await vscode.window.showInputBox();
		if (!input) return;
		return await browse(url.replace(/0$/, '') + input);
	} else if (menus[id]) { // menu step
		const pick = menus[id].length > 1 ? await vscode.window.showQuickPick(menus[id], { title: url }) : menus[id][0];
		if (!pick) return;
		return await browse(url + (pick.path || pick.label));
	} else { // fetch step
		const json = JSON.parse(await fetch("https://api.deezer.com" + url)); // todo: json.next?
		const data = json.data?.tracks || json.data || []; // type==playlist use deeper list
		const picked = url.match(/\/(playlist|album)\//);
		const canPickMany = data.find(item => item.type == "track");
		try {
			const choices = data.map(entry => ({
				...entry, picked,
				label: (type2icon[entry.type] || '') + (entry.title_short || entry.name),
				description: [entry.artist?.name,entry.title_version].join(' '),
				path: `/${entry.type}/${entry.id}`,
			}));
			const picks = await vscode.window.showQuickPick(choices, { title: url, canPickMany });
			if (!picks) return;
			return canPickMany ? picks : await browse(picks.path);
		} catch (e) { console.error(e) }
	}
}

const with_url = async (songs) => songs?.length ? await vscode.window.withProgress({title: 'Fetching Song Info...', location }, async (progress) => {
	const base = "https://www.deezer.com/ajax/gw-light.php?input=3&api_version=1.0";
	const increment = 100/4;
	const gw = async(method, sid, api_token = "", opt = {}, data) => JSON.parse(await fetch(`${base}&method=${method}&api_token=${api_token}`,
			{ ...opt, headers: { Cookie: `sid=${sid}`, ...opt?.headers } }, data)).results;
	progress.report({increment})
	const DZR_PNG = await gw('deezer.ping');
	progress.report({increment})
	const USR_NFO = await gw('deezer.getUserData', DZR_PNG.SESSION);
	progress.report({increment})
	const SNG_NFO = await gw('song.getListData', DZR_PNG.SESSION, USR_NFO.checkForm, { method: 'POST' }, JSON.stringify({ sng_ids: songs.map(s => s.id) }));
	progress.report({increment})
	const URL_NFO = JSON.parse(await fetch('https://media.deezer.com/v1/get_url', { method: 'POST' }, JSON.stringify({
		track_tokens: SNG_NFO.data.map(d => d.TRACK_TOKEN),
		license_token: USR_NFO.USER.OPTIONS.license_token,
		media: [{ type: "FULL", formats: [{ cipher: "BF_CBC_STRIPE", format: "MP3_128" }] }]
	})));
	return songs.map(({id,title_short,title_version,artist,contributors,duration},i) => ({
		id,title:title_short,version:title_version,duration,
		artists:(contributors||[artist])?.map(({id,name})=>({id,name})),
		size:+SNG_NFO.data[i].FILESIZE,
		expire:SNG_NFO.data[i].TRACK_TOKEN_EXPIRE,
		url: URL_NFO.data[i].media[0].sources[0].url
	}));
}):[];

class DzrWebView { // can't Audio() in VSCode, we need a webview
	statuses = ['dzr.play','dzr.show','dzr.next'].map((command) => {
		const item = vscode.window.createStatusBarItem(command, vscode.StatusBarAlignment.Left, 10000);
		item.color = new vscode.ThemeColor('statusBarItem.prominentBackground');
		item.backgroundColor = new vscode.ThemeColor('statusBarItem.errorBackground');
		item.command = command;
		item.text = command;
		item.show();
		return item;
	});
	panel = null;
	#state = {index:-1,playing:false,ready:false,label:"artist - song",looping:conf.get('loop'),queue:conf.get('queue')};
	state = new Proxy(this.#state, {set: (target, key, value) => {
		target[key] = value;
		if (['queue', 'looping'].includes(key)) { // persist those values across reboot
			conf.update(key, value, vscode.ConfigurationTarget.Global);
		}
		vscode.commands.executeCommand('setContext', `dzr.${key}`, value);
		this.post('state', target, [key]);
		this.renderStatus();
		return true;
	}});

	constructor() {
		this.initAckSemaphore();
		this.renderStatus();
	}

	renderStatus() {
		this.statuses[0].command = this.state.playing ? 'dzr.pause' : 'dzr.play';
		this.statuses[0].text = this.state.ready && (this.state.playing ? "$(debug-pause)" : "$(play)");
		this.statuses[1].tooltip = this.state.ready ? this.state.label : "Initiate interaction first";
		this.statuses[1].text = this.state.ready ? this.state.label.length<20?this.state.label:(this.state.label.slice(0,20)+'…') : "$(play)"
		this.statuses[2].text = this.state.ready && this.state.queue.length ? `${this.state.index+1}/${this.state.queue.length} $(chevron-right)`:null;//debug-step-over
	}
	async show(htmlUri) {
		if(this.panel)return this.panel.reveal(vscode.ViewColumn.One);
		this.panel = vscode.window.createWebviewPanel('dzr.player','Player',vscode.ViewColumn.One, {
			enableScripts: true,
			enableCommandUris: true,
			enableFindWidget: true,
			retainContextWhenHidden: true
		});
		this.panel.webview.html = (await vscode.workspace.fs.readFile(htmlUri)).toString();
		this.panel.webview.onDidReceiveMessage((action, ...args) => this[action] ? this[action](...args) : this.badAction(action));
		this.panel.onDidDispose(() => this.state.ready = this.panel = null);
		this.post('state', this.state, Object.keys(this.state));
	}
	initAckSemaphore() { this.postAck = new Promise((then) => this.waitAckSemaphore = then); }
	post = (action, ...arg) => this.panel?.webview.postMessage([action, ...arg ]);
	// event from webview player
	player_bufferized() {
		this.waitAckSemaphore();
		this.initAckSemaphore();
	}
	player_playing() { this.state.ready = this.state.playing = true; }
	player_pause() { this.state.playing = false; }
	player_ended() { vscode.commands.executeCommand('dzr.next'); }
	user_interact() { this.state.ready = true; }
	error(msg) { vscode.window.showErrorMessage(msg); }
	badAction(action) { console.error(`unHandled action "${action}" from webview`); }
}
exports.activate = async function (/**@type {vscode.ExtensionContext}*/ context) {
	// deezer didn't DMCA'd dzr so let's follow the same path here
	conf.get('cbc') || vscode.window.withProgress({title: 'Extracting CBC key...', location}, async () => {
		const html_url = 'https://www.deezer.com/en/channels/explore';
		const html = (await fetch(html_url)).toString('utf-8');
		const js_url = html.match(/src="(http[^"]+app-web\.[^"]+\.js)"/)?.[1];
		if (!js_url) return await vscode.window.showErrorMessage('CBC Extract: No JS WebApp found');
		const keys = (await fetch(js_url)).toString('utf-8').match(/%5B0x..%2C.{39}%2C0x..%5D/g);
		const [a, b] = keys.map(part => part.slice(3, -3).split('%2C').map(i => String.fromCharCode(parseInt(i))).reverse());
		const cbc = a.map((a, i) => `${a}${b[i]}`).join('');// zip a+b
		const sha = crypto.createHash('sha1').update(cbc).digest('hex').slice(0,8);
		if (sha != '3ad58d92') return await vscode.window.showErrorMessage('Bad extracted key');
		conf.update('cbc', cbc, vscode.ConfigurationTarget.Global);
	});
	const dzr = new DzrWebView();
	const htmlUri = vscode.Uri.joinPath(context.extensionUri, 'webview.html');
	context.subscriptions.push(
		...dzr.statuses,
		vscode.commands.registerCommand('dzr.show', () => dzr.show(htmlUri)),
		vscode.commands.registerCommand("dzr.play", () => dzr.post('play')),
		vscode.commands.registerCommand("dzr.pause", () => dzr.post('pause')),
		vscode.commands.registerCommand("dzr.loopAll", () => dzr.looping = true),
		vscode.commands.registerCommand("dzr.loopOff", () => dzr.looping = false),
		vscode.commands.registerCommand("dzr.add", async (path) => dzr.state.queue = [...dzr.state.queue, ...await with_url(await browse(path))||[]]),
		vscode.commands.registerCommand("dzr.pop", async (index) => dzr.state.queue = [...dzr.state.queue.slice(0,index),...dzr.state.queue.slice(index+1)]),
		vscode.commands.registerCommand("dzr.clear", async () => dzr.state.queue = []),
		vscode.commands.registerCommand("dzr.shuffle", async () => {
			const shuffle = [...dzr.state.queue];
			for (let i = shuffle.length - 1; i > 0; i--) {
				const j = Math.floor(Math.random() * (i + 1));
				[shuffle[i], shuffle[j]] = [shuffle[j], shuffle[i]];
			}
			dzr.state.queue = shuffle;
		}),
		vscode.commands.registerCommand("dzr.next", async (pos=dzr.state.index + 1) => {
			dzr.state.index = (pos >= dzr.state.queue.length) ? 0 : pos;
			const item = dzr.state.queue[dzr.state.index];
			item && vscode.commands.executeCommand('dzr.load', item.url, item.id, `${item.title} - ${item.artists?.map(a=>a.name).join()}`);
		}),
		vscode.commands.registerCommand("dzr.load", async (url,id,label) => {
			dzr.state.label = label;
			const hex = (str) => str.split('').map(c => c.charCodeAt(0))
			const md5 = hex(crypto.createHash('md5').update(`${id}`).digest('hex'));
			const key = Buffer.from(hex(conf.get('cbc')).map((c, i) => c ^ md5[i] ^ md5[i + 16]));
			const iv = Buffer.from([0, 1, 2, 3, 4, 5, 6, 7]);
			const stripe = 2048;//TODO:use .pipe() API https://codereview.stackexchange.com/questions/57492/
			dzr.post('open', {id});
			const buf_enc = await fetch(url);
			for(let pos = 0; pos < buf_enc.length ; pos+=stripe) {
				if((pos>>11)%3)continue;
				const ciph = crypto.createDecipheriv('bf-cbc', key, iv).setAutoPadding(false)
				const deco = ciph.update(buf_enc.subarray(pos, pos+stripe));
				buf_enc.set(deco, pos);
			}
			dzr.post('append', Uint8Array.from(buf_enc));
			await dzr.postAck;
			dzr.post('close');
		}),
	)
}
