const Router = require('./router')
const ID3Writer = require('browser-id3-writer');

addEventListener('fetch', event => {
    event.respondWith(handleRequest(event.request))
})

const format_string_to_num = {
    aac_96: '8',
    64: '10',
    128: '1',
    320: '3',
    flac: '9',
    mp4_ra1: '13',
    mp4_ra2: '14',
    mp4_ra3: '15',
    mhm1_ra1: '16',
    mhm1_ra2: '17',
    mhm1_ra3: '18',
    sbc_256: '12',
    misc: '0',
}

const format_string_to_gw = {
    aac_96: 'AAC_96',
    64: 'MP3_64',
    128: 'MP3_128',
    320: 'MP3_320',
    flac: 'FLAC',
    mp4_ra1: 'MP4_RA1',
    mp4_ra2: 'MP4_RA2',
    mp4_ra3: 'MP4_RA3',
    mhm1_ra1: 'MHM1_RA1',
    mhm1_ra2: 'MHM1_RA2',
    mhm1_ra3: 'MHM1_RA3',
    sbc_256: 'SBC_256',
    misc: 'MP3_MISC',
}

async function gw_api_call(method, params) {
    if (method === 'deezer.getUserData') {
        checkForm = ''
    }

    if (!params) {
        params = {}
    }

    let cookies = `arl=${ARL}`
    if (sid) {
        cookies += `; sid=${sid}`
    }

    const headers = new Headers({ 'cookie': cookies })

    const init = {
        method: 'POST',
        headers: headers,
        body: JSON.stringify(params),
    }

    const response = await fetch(`https://www.deezer.com/ajax/gw-light.php?method=${method}&input=3&api_version=1.0&api_token=${encodeURIComponent(checkForm)}&cid=${Math.floor(Math.random() * 1e9)}`, init)
    const json = await response.json()

    if (json.error.length !== 0) {
        return new Response(JSON.stringify(json.error), { status: 500, headers: { 'content-type': 'application/json' } })
    }

    if (method === 'deezer.getUserData') {
        checkForm = json.results.checkForm
        await KV.put('checkForm', checkForm)

        sid = response.headers.get('set-cookie').split(',').map(v => v.trimStart())[0]
        sid = sid.match(/^sid=(fr[\da-f]+)/)[1]
        await KV.put('sid', sid)
    }

    return json.results
}

async function handler(type, request) {
    license_token = await KV.get('license_token')
    checkForm = await KV.get('checkForm')
    sid = await KV.get('sid')

    if (license_token === null) {
        const user_data = await gw_api_call('deezer.getUserData')
        if (user_data.constructor.name === 'Response') {
            return user_data
        }

        if (user_data.USER.USER_ID === 0) {
            return new Response('Invalid arl', { status: 500, headers: { 'content-type': 'text/plain' } })
        }

        license_token = user_data.USER.OPTIONS.license_token
        const expiration = user_data.USER.OPTIONS.expiration_timestamp

        await KV.put('license_token', license_token, { expiration: expiration })
    }

    const url = new URL(request.url)
    const id = url.pathname.split('/')[2]

    format = url.searchParams.get('f')
    if (format === null) {
        format = '320'
    } else {
        format = format.toLowerCase()
        if (format_string_to_num[format] === undefined) {
            index = Object.values(format_string_to_num).indexOf(format)
            if (index === -1) {
                return new Response('Invalid format', { status: 400, headers: { 'content-type': 'text/plain' } })
            }
            format = Object.keys(format_string_to_num)[index]
        }
    }

    let tagging = url.searchParams.get('t')
    tagging = (tagging === 'true' || tagging === '1') && ['misc', '128', '320'].includes(format)

    switch (type) {
        case 'track':
            return await track(id, format, tagging)
        case 'album':
        case 'playlist':
            return await m3u8(type, id, format, tagging, url.host)
    }
}

async function track(id, format, tagging) {
    const json = await gw_api_call('song.getData', { 'SNG_ID': id })
    if (json.constructor.name === 'Response') {
        return json
    }

    if (parseInt(json.SNG_ID) < 0) {   // user-uploaded track
        format = 'misc'
    }

    if (json['FILESIZE_' + format_string_to_gw[format]] == false) {
        return new Response('Format unavailable', { status: 403, headers: { 'content-type': 'text/plain' } })
    }

    const wasm = await import('./pkg')

    legacy_url = !['320', 'flac'].includes(format)
    if (!legacy_url) {     // server-side stream url
        // needed if track has fallback, like https://www.deezer.com/track/11835714
        let track_token
        if (json.FALLBACK !== undefined) {
            track_token = json.FALLBACK.TRACK_TOKEN
        } else {
            track_token = json.TRACK_TOKEN
        }

        const body = {
            license_token: license_token,
            media: [
                {
                    type: 'FULL',
                    formats: [
                        {
                            cipher: 'BF_CBC_STRIPE',
                            format: format_string_to_gw[format]
                        }
                    ]
                }
            ],
            track_tokens: [ track_token ]
        }

        const init = {
            method: 'POST',
            body: JSON.stringify(body)
        }

        const resp = await fetch('https://media.deezer.com/v1/get_url', init)
        if (resp.status !== 200) {
            return new Response("Couldn't get stream URL", { status: 403, headers: { 'content-type': 'text/plain' } })
        }

        const media_json = await resp.json()

        result = media_json.data[0].media[0].sources[0].url
    } else {    // legacy stream url
        result = await legacy_track_url(json, format, wasm.legacy_stream_url)
        if (typeof result === 'object') {
            return result
        }
    }

    const track = await fetch(result)
    if (track.status !== 200) {
        return new Response("Couldn't get track stream", { status: 403, headers: { 'content-type': 'text/plain' } })
    }

    let id3
    if (tagging) {
        id3 = new ID3Writer(Buffer.alloc(0));
        id3.padding = 0
        id3.setFrame('TIT2', json.SNG_TITLE)
            .setFrame('TALB', json.ALB_TITLE)
            .setFrame('TPE2', json.ART_NAME)

        if (json.ARTISTS !== undefined) {
            artist_list = [];
            for (const a of json.ARTISTS) {
                artist_list.push(a.ART_NAME)
            }

            id3.setFrame('TPE1', artist_list)
        }

        if (json.TRACK_NUMBER !== undefined) {
            id3.setFrame('TRCK', json.TRACK_NUMBER)
        }

        if (json.DISK_NUMBER !== undefined) {
            id3.setFrame('TPOS', json.DISK_NUMBER)
        }

        if (json.ISRC !== '') {
            id3.setFrame('TSRC', json.ISRC)
        }

        if (json.PHYSICAL_RELEASE_DATE !== undefined) {
            const split = json.PHYSICAL_RELEASE_DATE.split('-')
            id3.setFrame('TYER', split[0])
            id3.setFrame('TDAT', split[2] + split[1])
        }

        if (json.ALB_PICTURE !== '') {
            const url = `https://cdns-images.dzcdn.net/images/cover/${json.ALB_PICTURE}/1000x1000-000000-80-0-0.jpg`
            const cover = await fetch(url)
            const coverBuffer = await cover.arrayBuffer()

            id3.setFrame('APIC', {
                type: 3,
                data: coverBuffer,
                description: 'cover'
            });
        }

        id3.addTag();
    }

    let { readable, writable } = new TransformStream()
    const writer = writable.getWriter()

    if (tagging) {
        writer.write(id3.arrayBuffer)
    }

    // needed if track has fallback, like https://www.deezer.com/track/11835714
    if (json.FALLBACK) {
        id = json.FALLBACK.SNG_ID
    }

    const cipher = new wasm.Cipher(id)

    const length = parseInt(track.headers.get('Content-Length'))

    pipeDecryptedStream(writer, track.body, length, cipher)

    return new Response(readable, { status: 200, headers: { 'content-type': 'audio/mpeg' } })
}

async function pipeDecryptedStream(writer, body, length, cipher) {
    const reader = body.getReader({ mode: 'byob' })
    let byteCount = 0
    let end = false
    while (!end) {
        end = byteCount + 2048 > length
        let chunk
        if (!end) {
            chunk = new Int8Array(2048)
        } else {
            chunk = new Int8Array(length - byteCount)
        }

        // if read chunk isn't 2048 bytes, read until it is
        // cause of retarded readable streams not having an option to specify min bytes
        let tempLength = 0
        while (tempLength !== chunk.length) {
            let read = (await reader.read(new Int8Array(chunk.length - tempLength))).value
            chunk.set(read, tempLength)
            tempLength += read.length
        }

        if (byteCount % 6144 === 0 && !end) {
            // encrypted chunk
            cipher.decrypt_chunk(chunk)
        }

        await writer.write(chunk)
        byteCount += 2048
    }

    await reader.cancel()
    await writer.close()
}

function legacy_track_url(json, format, url_func) {
    // needed if track has fallback, like https://www.deezer.com/track/11835714
    if (json.FALLBACK) {
        json = json.FALLBACK
    }

    const id = json.SNG_ID.toString()
    const md5_origin = json.MD5_ORIGIN
    const media_version = json.MEDIA_VERSION

    format = format_string_to_num[format]

    return url_func(md5_origin, format, id, media_version)
}

async function m3u8(type, id, format, tagging, host) {
    const response = await fetch(`https://api.deezer.com/${type}/${id}?limit=-1`)
    const json = await response.json()
    if (json.error !== undefined) {
        return new Response(JSON.stringify(json.error), { status: 403, headers: { 'content-type': 'application/json' } })
    }

    let list = '#EXTM3U\n'

    for (const track of json.tracks.data) {
        if (track.id < 0) {   // user-uploaded track
            format = 'misc'
        }
        let result = `https://${host}/track/${track.id}?f=${format}&t=${+ tagging}`
        list += `#EXTINF:${track.duration},${track.title}\n${result}\n`
    }

    return new Response(list, { status: 200, headers: { 'content-type': 'audio/mpegurl' } })
}

async function indexHandler() {
    return new Response(require('./index.html'), { status: 200, headers: { 'content-type': 'text/html' } })
}

async function handleRequest(request) {
    const r = new Router()
    r.get('/', () => indexHandler())
    r.get('/track/-?\\d+', () => handler('track', request))
    r.get('/album/\\d+', () => handler('album', request))
    r.get('/playlist/\\d+', () => handler('playlist', request))

    const resp = await r.route(request)
    return resp
}
