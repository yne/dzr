class DeezerAPI {

    constructor(arl, electron = false) {
        this.arl = arl;
        this.electron = electron;
        this.url = 'https: //www.deezer.com/ajax/gw-light.php';
    }
    //Get headers
    headers() {
        let cookie = `arl=${this.arl
            }`;
        if (this.sid) cookie += `; sid=${this.sid
            }`;
        return {
            "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/67.0.3396.99 Safari/537.36",
            "Content-Language": "en-US",
            "Cache-Control": "max-age=0",
            "Accept": "*/*",
            "Accept-Charset": "utf-8,ISO-8859-1;q=0.7,*;q=0.3",
            "Accept-Language": "en-US,en;q=0.9,en-US;q=0.8,en;q=0.7",
            "Connection": 'keep-alive',
            "Cookie": cookie
        }
    }
    //Wrapper for api calls, because axios doesn't work reliably with electron
    async callApi(method, args = {}, gatewayInput = null) {
        if (this.electron) return await this._callApiElectronNet(method, args, gatewayInput);
        return await this._callApiAxios(method, args, gatewayInput);
    }
    //gw_light api call using axios, unstable in electron
    async _callApiAxios(method, args = {}, gatewayInput = null) {
        let data = await axios({
            url: this.url,
            method: 'POST',
            headers: this.headers(),
            responseType: 'json',
            params: Object.assign({
                api_version: '1.0',
                api_token: this.token ? this.token : 'null',
                input: '3',
                method: method,
            },
                gatewayInput ? {
                    gateway_input: JSON.stringify(gatewayInput)
                } : null
            ),
            data: args
        });

        //Save SID cookie to not get token error
        if (method == 'deezer.getUserData') {
            let sidCookie = data.headers['set-cookie'
            ].filter((e) => e.startsWith('sid='));
            if (sidCookie.length > 0) {
                sidCookie = sidCookie[
                    0
                ].split(';')[
                    0
                ];
                this.sid = sidCookie.split('=')[
                    1
                ];
            }
        }
        //Invalid CSRF
        if (data.data.error && data.data.error.VALID_TOKEN_REQUIRED) {
            await this.authorize();
            return await this.callApi(method, args, gatewayInput);
        }

        return data.data;
    }
    //gw_light api call using electron net
    async _callApiElectronNet(method, args = {}, gatewayInput = null) {
        const net = require('electron').net;
        let data = await new Promise((resolve, reject) => {
            //Create request
            let req = net.request({
                method: 'POST',
                url: this.url + '?' + querystring.stringify(Object.assign({
                    api_version: '1.0',
                    api_token: this.token ? this.token : 'null',
                    input: '3',
                    method: method,
                },
                    gatewayInput ? {
                        gateway_input: JSON.stringify(gatewayInput)
                    } : null
                )),
            });

            req.on('response', (res) => {
                let data = Buffer.alloc(0);

                //Save SID cookie
                if (method == 'deezer.getUserData') {
                    let sidCookie = res.headers['set-cookie'
                    ].filter((e) => e.startsWith('sid='));
                    if (sidCookie.length > 0) {
                        sidCookie = sidCookie[
                            0
                        ].split(';')[
                            0
                        ];
                        this.sid = sidCookie.split('=')[
                            1
                        ];
                    }
                }
                //Response data
                res.on('data', (buffer) => {
                    data = Buffer.concat([data, buffer
                    ]);
                });
                res.on('end', () => {
                    resolve(data);
                })
            });
            req.on('error', (err) => {
                reject(err);
            });

            //Write headers
            let headers = this.headers();
            for (let key of Object.keys(headers)) {
                req.setHeader(key, headers[key
                ]);
            }
            req.write(JSON.stringify(args));
            req.end();
        });

        data = JSON.parse(data.toString('utf-8'));

        //Invalid CSRF
        if (data.error && data.error.VALID_TOKEN_REQUIRED) {
            await this.authorize();
            return await this.callApi(method, args, gatewayInput);
        }

        return data;
    }
    //true / false if success
    async authorize() {
        let data = await this.callApi('deezer.getUserData');
        this.token = data.results.checkForm;
        this.userId = data.results.USER.USER_ID;
        this.favoritesPlaylist = data.results.USER.LOVEDTRACKS_ID.toString();
        try {
            await this.authorizeAtv();
        } catch (e) {
            logger.warn(`Couldnt authorize ATV: ${e
                }`);
        }

        if (!this.userId || this.userId == 0 || !this.token) return false;
        return true;
    }
    //Authorize Android TV
    async authorizeAtv() {
        let res = await axios.post(
            "https://distribution-api.deezer.com/device/token",
            {
                "brand_name": "Hisense",
                "device_id": "7239e4071d8992c955ad",
                "model_name": "HE50A6109FUWTS",
                "country_code": "FRA"
            }
        );
        let deviceToken = res.data.device_token;
        // Get unauthorized token
        res = await axios.get('https: //connect.deezer.com/oauth/access_token.php?grant_type=client_credentials&client_id=447462&client_secret=a83bf7f38ad2f137e444727cfc3775cf&output=json');
        let accessToken = res.data.access_token;
        // Get smart login code
        res = await axios.post(`https: //connect.deezer.com/2.0/smartlogin/device?access_token=${accessToken}&device_token=${deviceToken}`);
        let smartLoginCode = res.data.data.smartLoginCode;
        // Associate
        await this.callApi('deezer.associateSmartLoginCodeWithUser',
            {
                'SMARTLOGIN_CODE': smartLoginCode
            });
        // Get authorized token
        res = await axios.get(`https: //connect.deezer.com/2.0/smartlogin/${smartLoginCode}?access_token=${accessToken}`);
        accessToken = res.data.data.accessToken;
        this.accessToken = accessToken;
    }
    //Wrapper because electron is piece of shit
    async callPublicApi(path, params) {
        if (this.electron) return await this._callPublicApiElectron(path, params);
        return await this._callPublicApiAxios(path, params);
    }

    async _callPublicApiElectron(path, params) {
        const net = require('electron').net;
        let data = await new Promise((resolve, reject) => {
            //Create request
            let req = net.request({
                method: 'GET',
                url: `https: //api.deezer.com/${encodeURIComponent(path)}/${encodeURIComponent(params)}`
            });

            req.on('response', (res) => {
                let data = Buffer.alloc(0);
                //Response data
                res.on('data', (buffer) => {
                    data = Buffer.concat([data, buffer
                    ]);
                });
                res.on('end', () => {
                    resolve(data);
                })
            });
            req.on('error', (err) => {
                reject(err);
            });
            req.end();
        });

        data = JSON.parse(data.toString('utf-8'));
        return data;
    }

    async _callPublicApiAxios(path, params) {
        let res = await axios({
            url: `https: //api.deezer.com/${encodeURIComponent(path)}/${encodeURIComponent(params)}`,
            responseType: 'json',
            method: 'GET'
        });
        return res.data;
    }
    //Get track URL
    static getUrl(trackId, md5origin, mediaVersion, quality = 3) {
        const magic = Buffer.from([
            0xa4
        ]);
        let step1 = Buffer.concat([
            Buffer.from(md5origin),
            magic,
            Buffer.from(quality.toString()),
            magic,
            Buffer.from(trackId),
            magic,
            Buffer.from(mediaVersion)
        ]);
        //MD5
        let md5sum = crypto.createHash('md5');
        md5sum.update(step1);
        let step1md5 = md5sum.digest('hex');

        let step2 = Buffer.concat([
            Buffer.from(step1md5),
            magic,
            step1,
            magic
        ]);
        //Padding
        while (step2.length % 16 > 0) {
            step2 = Buffer.concat([step2, Buffer.from('.')
            ]);
        }
        //AES
        let aesCipher = crypto.createCipheriv('aes-128-ecb', Buffer.from('jo6aey6haid2Teih'), Buffer.from(''));
        let step3 = Buffer.concat([aesCipher.update(step2, 'binary'), aesCipher.final()
        ]).toString('hex').toLowerCase();

        return `https: //e-cdns-proxy-${md5origin.substring(0, 1)}.dzcdn.net/mobile/1/${step3}`;
    }
    //Generate url with android tv support
    async generateUrl(trackId, md5origin, mediaVersion, quality = 3) {
        if (quality == 9 && this.accessToken) {
            try {
                let res = await axios({
                    method: 'GET',
                    url: `https: //api.deezer.com/platform/gcast/track/${trackId}/streamUrls`,
                    headers: {
                        "Authorization": "Bearer " + this.accessToken
                    },
                    responseType: 'json'
                });
                if (res.data.data.attributes.url_flac)
                    return {
                        encrypted: false, url: res.data.data.attributes.url_flac
                    };
            } catch (e) {
                console.warn(`Failed getting ATV URL! Using normal: ${e
                    }`)
            }
        }
        return {
            encrypted: true,
            url: DeezerAPI.getUrl(trackId, md5origin, mediaVersion, quality)
        };
    }


    async fallback(info, quality = 3) {
        let qualityInfo = Track.getUrlInfo(info);

        //User uploaded MP3s
        if (qualityInfo.trackId.startsWith('-')) {
            qualityInfo.quality = 3;
            return qualityInfo;
        }
        //Quality fallback
        let newQuality = await this.qualityFallback(qualityInfo, quality);
        if (newQuality != null) {
            return qualityInfo;
        }
        //ID Fallback
        let trackData = await this.callApi('deezer.pageTrack',
            {
                sng_id: qualityInfo.trackId
            });
        try {
            if (trackData.results.DATA.FALLBACK.SNG_ID.toString() != qualityInfo.trackId) {
                let newId = trackData.results.DATA.FALLBACK.SNG_ID.toString();
                let newTrackData = await this.callApi('deezer.pageTrack',
                    {
                        sng_id: newId
                    });
                let newTrack = new Track(newTrackData.results.DATA);
                return this.fallback(newTrack.streamUrl);
            }
        } catch (e) {
            logger.warn('TrackID Fallback failed: ' + e + ' Original ID: ' + qualityInfo.trackId);
        }
        //ISRC Fallback
        try {
            let publicTrack = this.callPublicApi('track', 'isrc:' + trackData.results.DATA.ISRC);
            let newId = publicTrack.id.toString();
            let newTrackData = await this.callApi('deezer.pageTrack',
                {
                    sng_id: newId
                });
            let newTrack = new Track(newTrackData.results.DATA);
            return this.fallback(newTrack.streamUrl);
        } catch (e) {
            logger.warn('ISRC Fallback failed: ' + e + ' Original ID: ' + qualityInfo.trackId);
        }
        return null;
    }
    //Fallback thru available qualities, -1 if none work
    async qualityFallback(info, quality = 3) {
        try {
            let urlGen = await this.generateUrl(info.trackId, info.md5origin, info.mediaVersion, quality);
            let res = await axios.head(urlGen.url);
            if (res.status > 400) throw new Error(`Status code: ${res.status
                }`);
            //Make sure it's an int
            info.quality = parseInt(quality.toString(),
                10);
            info.size = parseInt(res.headers['content-length'
            ],
                10);
            info.encrypted = urlGen.encrypted;
            if (!info.encrypted)
                info.direct = urlGen.url;
            return info;
        } catch (e) {
            logger.warn('Quality fallback: ' + e);
            //Fallback
            //9 - FLAC
            //3 - MP3 320
            //1 - MP3 128
            let nq = -1;
            if (quality == 3) nq = 1;
            if (quality == 9) nq = 3;
            if (quality == 1) return null;
            return this.qualityFallback(info, nq);
        }
    }
}