(function(){

    const API_DEEZER = "https://api.deezer.com";

    const CBC = "g4el58wc0zvf9na1";

    const root = document.getElementById("root");
    
    const searchKeys = [
      { name: "Track", path: "/search/track?q=" },
      { name: "Artist", path: "/search/artist?q=" },
      { name: "Album", path: "/search/album?q=" },
      { name: "Playlist", path: "/search/playlist?q=" },
      { name: "User", path: "/search/user?q=" },
      { name: "Radio", path: "/search/radio?q=" },
    ];

    const searchUser = [{ name: "User", path: "/user/0" }];

    const searchList = [
      { name: "Genre", path: "/genre" },
      { name: "Radio", path: "/radio" },
    ];

    const inputSearch = createElement('input', {
      type: 'text',
      placeholder: 'Search',
    });
    root.appendChild(inputSearch);

    const fieldset = createElement('fieldset', {
      style: 'margin: 10px 0;'
    });
    const legend = createElement('legend', {
      innerText: 'Search results'
    })
    fieldset.appendChild(legend);

    const back = createElement("button", {
      innerText: "Back",
      style: "display: none",
      onclick: () => {},
    });

    const next = createElement('button', {
      innerText: 'Next',
      style: 'display: none',
      onclick: () =>{}
    })

    const play = createElement("button", {
      innerText: "Play Tracks",
      style: "display: none",
      onclick: async () => {
        function gw({ method, api_token = '', sid = '', opt={}, data }) {
          const URL = `https://www.deezer.com/ajax/gw-light.php?input=3&api_version=1.0&method=${method}&api_token=${api_token}`;
          
          const headers = {
            Cookie: `sid=${sid}`,
          };

          if (opt && opt.method === "POST") {
            return post(URL, headers, data);
          } else {
            return get(URL, headers);
          }
        }
        async function _gw({ method, api_token, sid, opt, data }) {

          const response = await gw({ method, api_token, sid, opt, data });
          if (response) {
            if (response.error && response.error.length > 0) {
              alert(JSON.stringify(response.error));
              return {};
            }
            return response.results;
          }
          return {};
        }
        const DZR_PNG = await _gw({
          method: "deezer.ping",
          api_token: ""
        });

        const USR_NFO = await _gw({
          method: "deezer.getUserData",
          sid: DZR_PNG["SESSION"]
        });


        const SNG_NFO = await _gw({
          method: "song.getListData",
          sid: DZR_PNG["SESSION"],
          api_token: USR_NFO["checkForm"],
          opt: { method: "POST" },
          data: {
            sng_ids: Array.from(fieldset.children)
              .filter((el) => el instanceof HTMLLIElement)
              .map((el) => parseInt(el.id)),
          }
        });
        

        if(SNG_NFO && SNG_NFO.data){
          const URL_NFO = await post("https://media.deezer.com/v1/get_url", {}, {
            track_tokens: [...SNG_NFO.data.map((d) => d.TRACK_TOKEN)],
            license_token: USR_NFO["USER"]["OPTIONS"]["license_token"],
            media: [{
              type: "FULL",
              formats: [{
                cipher: "BF_CBC_STRIPE",
                format: "MP3_128",
              }],
            }],
          });

          const tracks = Array.from(fieldset.children).filter(
            (el) => el instanceof HTMLLIElement
          );
          const errors = URL_NFO.data
            .map((nfo, i) => [
              nfo.errors,tracks[i],
            ])
            .filter(([err]) => err)
            .map(([[err], sng]) => `${sng.title}: ${err.message} (${err.code})`)
            .join("\n");
          
            const songs = tracks.map((el, i) => {
              const {ALB_PICTURE, DURATION, TITLE, ARTISTS, FILESIZE, TRACK_TOCKEN_EXPIRE} = SNG_NFO.data[i];
              return {
                id: el.id,
                md5_image: ALB_PICTURE,
                duration: DURATION,
                title: TITLE,
                artists: ARTISTS.map(v => {
                  return { id: v.ART_ID, name: v.ART_NAME, md5: ALB_PICTURE };
                }),
                size: FILESIZE,
                expire: TRACK_TOCKEN_EXPIRE,
                url: URL_NFO['data'][i]['media'][0]['sources'][0]['url']
              };
            });

            
            const hex = (str) => str.split("").map((c) => c.charCodeAt(0));

            songs.forEach(async (v) => {
             
              const md5 = hex(
                MD5(new String(v.id))
              ); 
              
              const key = hex(CBC)
                .map((c, i) => c ^ md5[i] ^ md5[i + 16])
                .map(String.fromCharCode)
                .join("");

              // const iv = CryptoJS.enc.Utf8.parse("01234567");

              function b64EncodeUnicode(str) {
                // first we use encodeURIComponent to get percent-encoded UTF-8,
                // then we convert the percent encodings into raw bytes which
                // can be fed into btoa.
                return btoa(
                  encodeURIComponent(str).replace(
                    /%([0-9A-F]{2})/g,
                    function toSolidBytes(match, p1) {
                      return String.fromCharCode("0x" + p1);
                    }
                  )
                );
              }

              const stripe = 2048;

              const buf_enc = await get(v.url, {}, "text");
              const chunks = [];
              for(let pos = 0; pos < buf_enc.length; pos += stripe){
                if((pos >> 11) % 3)
                continue;
              
                const decrypt = blowfish.decrypt(
                  b64EncodeUnicode(buf_enc),
                  key,
                  {
                    cipherMode: 1,
                  }
                );
                chunks.push(decrypt);
              }
              const final = new TextEncoder().encode(chunks.join(""));

              // sendMessage(
              //   {
              //     action: "play",
              //     songs: [final],
              //   },
              //   function (a) {
              //   }
              // );
             
              function downloadByteArray(byteArray, fileName, fileType) {
                // Criar um Blob a partir do byteArray
                const blob = new Blob([byteArray], { type: fileType });

                // Criar um URL temporário para o Blob
                const url = URL.createObjectURL(blob);

                // Criar um link temporário
                const a = document.createElement("a");
                a.href = url;
                a.download = fileName; // Nome do arquivo que será baixado
                document.body.appendChild(a);

                // Simular um clique no link para iniciar o download
                a.click();

                // Remover o link da página
                document.body.removeChild(a);

                // Liberar o URL temporário
                URL.revokeObjectURL(url);
              }

              downloadByteArray(final, 'teste.mp3', "audio/mpeg");


            });



        }

      }
    });

    searchKeys.map((v) => {
        var populate = (json) => {
          fieldset.appendChild(legend);
          for (data of json.data) {
            var li = createElement('li', {
              id: data.id,
              style: 'cursor: pointer; padding: 5px 10px;',
              innerText : buildLabel(data),
              data: data,
              onclick: async function(){
                fieldset.replaceChildren([]);
                fieldset.appendChild(legend);
                next.style = "display: none";
                legend.innerText = 'Tracks List';
                if(this.data['type'] === 'track'){
                  fieldset.appendChild(
                    createElement("li", {
                      id: this.data.id,
                      style: "cursor: pointer; padding: 5px 10px;",
                      innerText: buildLabel(this.data),
                      data: this.data
                    })
                  );
                  play.style = 'display: block';
                }else{
                  const json = await get(
                    this.data.tracklist +
                      (this.data.nb_tracks
                        ? "&limit=" + this.data.nb_tracks
                        : "")
                  );

                  for (data of json.data) {
                    fieldset.appendChild(
                      createElement("li", {
                        id: data.id,
                        style: "cursor: pointer; padding: 5px 10px;",
                        innerText: buildLabel(data),
                        data: data,
                      })
                    );
                  }
                  play.style = "display: block";
                }
              }
            }) 
            fieldset.appendChild(li);
          }
          return json;
        };
        var button = document.createElement("button");
        button.style = 'margin: 5px 10px';
        button.innerText = v.name;
        button.addEventListener("click", async () => {
            fieldset.replaceChildren([])
            play.style = "display: none";
            if(inputSearch.value){
              var url = API_DEEZER + v.path + inputSearch.value;
              const response = await get(url);
              var json = populate(response);
              if(json.next){
                next.style = 'display: block';
                next.addEventListener("click", async () => {
                  const response = await get(json.next);
                  json = populate(response);
                  if (!json.next) {
                    next.style = "display: none";
                  }
                })
              }else{
                next.style = 'display: none';
              }
            }
        });
        return button;
    }).forEach(v => root.appendChild(v));
    root.appendChild(fieldset);
    root.appendChild(next);
    root.appendChild(play);
    
    function sendMessageContentScript(message) {
      return new Promise((resolve, reject)=>{
        chrome.tabs.query({active: true, currentWindow: true}).then(tabs =>{
          chrome.tabs.sendMessage(tabs[0].id, message).then(resolve).catch(reject);
        })
      });
    }

    function sendMessage(message, callback) {
      chrome.runtime.sendMessage(message, callback);
    }

    async function get(url, headers = {}, type = 'json') {
      const { origin } = new URL(url);

      // Solicita permissões de maneira assíncrona
      const granted = await new Promise((resolve) => {
        chrome.permissions.request(
          {
            permissions: ["tabs"],
            origins: [`${origin}/*`],
          },
          resolve
        );
      });

      if (!granted) {
        throw new Error("Permission denied");
      }

      try {
        const response = await fetch(url, {
          method: "GET",
          headers: Object.assign(
            {
              ...headers,
            },
            type == "json" ? { "Content-Type": "application/json" } : {}
          ),
        });

        if (!response.ok) {
          const errorData = await response.text();
          throw new Error(
            `Error: ${response.status} - ${
              errorData.message || "Request failed"
            }`
          );
        }
       
        return await response[type]();
      } catch (error) {
        throw new Error(`Fetch error: ${error.message}`);
      }
    }


    async function post(url, headers = {}, data = {}) {
      const { origin } = new URL(url);

      const granted = await new Promise((resolve) => {
        chrome.permissions.request(
          {
            permissions: ["tabs"],
            origins: [`${origin}/*`],
          },
          resolve
        );
      });

      if (!granted) {
        throw new Error("Permission denied");
      }

      try {
        const response = await fetch(url, {
          method: "POST",
          headers: {
            ...headers,
          },
          body: JSON.stringify(data),
        });

        if (!response.ok) {
          const text = await response.text(); // Lê a resposta como texto
          throw new Error(`Error: ${response.status} - ${text}`); // Lança o texto em vez de tentar analisar como JSON
        }

        const jsonResponse = await response.json(); // Agora tenta analisar como JSON
        return jsonResponse;
      } catch (error) {
        throw new Error(`Fetch error: ${error.message}`);
      }
    }

    function buildLabel(data){
      var label = [];
      if (data.title) {
        label.push(data.title);
      }
      if (data.name) {
        label.push(data.name);
      }
      if (data.artist && data.artist.name) {
        label.push(data.artist.name);
      }
      if (data.album && data.album.title) {
        label.push(data.album.title);
      }
      if (data["nb_tracks"]) {
        label.push(` (${data["nb_tracks"]})`);
      }
      return label.join(' - ')
    }
    
    function createElement(name, props){
      const el = document.createElement(name);
      for(i in props){
        el[i] = props[i];
      }
      return el;
    }
    var MD5 = function (d) {
      var r = M(V(Y(X(d), 8 * d.length)));
      return r.toLowerCase();
    };
    function M(d) {
      for (var _, m = "0123456789ABCDEF", f = "", r = 0; r < d.length; r++)
        (_ = d.charCodeAt(r)),
          (f += m.charAt((_ >>> 4) & 15) + m.charAt(15 & _));
      return f;
    }
    function X(d) {
      for (var _ = Array(d.length >> 2), m = 0; m < _.length; m++) _[m] = 0;
      for (m = 0; m < 8 * d.length; m += 8)
        _[m >> 5] |= (255 & d.charCodeAt(m / 8)) << m % 32;
      return _;
    }
    function V(d) {
      for (var _ = "", m = 0; m < 32 * d.length; m += 8)
        _ += String.fromCharCode((d[m >> 5] >>> m % 32) & 255);
      return _;
    }
    function Y(d, _) {
      (d[_ >> 5] |= 128 << _ % 32), (d[14 + (((_ + 64) >>> 9) << 4)] = _);
      for (
        var m = 1732584193,
          f = -271733879,
          r = -1732584194,
          i = 271733878,
          n = 0;
        n < d.length;
        n += 16
      ) {
        var h = m,
          t = f,
          g = r,
          e = i;
        (f = md5_ii(
          (f = md5_ii(
            (f = md5_ii(
              (f = md5_ii(
                (f = md5_hh(
                  (f = md5_hh(
                    (f = md5_hh(
                      (f = md5_hh(
                        (f = md5_gg(
                          (f = md5_gg(
                            (f = md5_gg(
                              (f = md5_gg(
                                (f = md5_ff(
                                  (f = md5_ff(
                                    (f = md5_ff(
                                      (f = md5_ff(
                                        f,
                                        (r = md5_ff(
                                          r,
                                          (i = md5_ff(
                                            i,
                                            (m = md5_ff(
                                              m,
                                              f,
                                              r,
                                              i,
                                              d[n + 0],
                                              7,
                                              -680876936
                                            )),
                                            f,
                                            r,
                                            d[n + 1],
                                            12,
                                            -389564586
                                          )),
                                          m,
                                          f,
                                          d[n + 2],
                                          17,
                                          606105819
                                        )),
                                        i,
                                        m,
                                        d[n + 3],
                                        22,
                                        -1044525330
                                      )),
                                      (r = md5_ff(
                                        r,
                                        (i = md5_ff(
                                          i,
                                          (m = md5_ff(
                                            m,
                                            f,
                                            r,
                                            i,
                                            d[n + 4],
                                            7,
                                            -176418897
                                          )),
                                          f,
                                          r,
                                          d[n + 5],
                                          12,
                                          1200080426
                                        )),
                                        m,
                                        f,
                                        d[n + 6],
                                        17,
                                        -1473231341
                                      )),
                                      i,
                                      m,
                                      d[n + 7],
                                      22,
                                      -45705983
                                    )),
                                    (r = md5_ff(
                                      r,
                                      (i = md5_ff(
                                        i,
                                        (m = md5_ff(
                                          m,
                                          f,
                                          r,
                                          i,
                                          d[n + 8],
                                          7,
                                          1770035416
                                        )),
                                        f,
                                        r,
                                        d[n + 9],
                                        12,
                                        -1958414417
                                      )),
                                      m,
                                      f,
                                      d[n + 10],
                                      17,
                                      -42063
                                    )),
                                    i,
                                    m,
                                    d[n + 11],
                                    22,
                                    -1990404162
                                  )),
                                  (r = md5_ff(
                                    r,
                                    (i = md5_ff(
                                      i,
                                      (m = md5_ff(
                                        m,
                                        f,
                                        r,
                                        i,
                                        d[n + 12],
                                        7,
                                        1804603682
                                      )),
                                      f,
                                      r,
                                      d[n + 13],
                                      12,
                                      -40341101
                                    )),
                                    m,
                                    f,
                                    d[n + 14],
                                    17,
                                    -1502002290
                                  )),
                                  i,
                                  m,
                                  d[n + 15],
                                  22,
                                  1236535329
                                )),
                                (r = md5_gg(
                                  r,
                                  (i = md5_gg(
                                    i,
                                    (m = md5_gg(
                                      m,
                                      f,
                                      r,
                                      i,
                                      d[n + 1],
                                      5,
                                      -165796510
                                    )),
                                    f,
                                    r,
                                    d[n + 6],
                                    9,
                                    -1069501632
                                  )),
                                  m,
                                  f,
                                  d[n + 11],
                                  14,
                                  643717713
                                )),
                                i,
                                m,
                                d[n + 0],
                                20,
                                -373897302
                              )),
                              (r = md5_gg(
                                r,
                                (i = md5_gg(
                                  i,
                                  (m = md5_gg(
                                    m,
                                    f,
                                    r,
                                    i,
                                    d[n + 5],
                                    5,
                                    -701558691
                                  )),
                                  f,
                                  r,
                                  d[n + 10],
                                  9,
                                  38016083
                                )),
                                m,
                                f,
                                d[n + 15],
                                14,
                                -660478335
                              )),
                              i,
                              m,
                              d[n + 4],
                              20,
                              -405537848
                            )),
                            (r = md5_gg(
                              r,
                              (i = md5_gg(
                                i,
                                (m = md5_gg(
                                  m,
                                  f,
                                  r,
                                  i,
                                  d[n + 9],
                                  5,
                                  568446438
                                )),
                                f,
                                r,
                                d[n + 14],
                                9,
                                -1019803690
                              )),
                              m,
                              f,
                              d[n + 3],
                              14,
                              -187363961
                            )),
                            i,
                            m,
                            d[n + 8],
                            20,
                            1163531501
                          )),
                          (r = md5_gg(
                            r,
                            (i = md5_gg(
                              i,
                              (m = md5_gg(
                                m,
                                f,
                                r,
                                i,
                                d[n + 13],
                                5,
                                -1444681467
                              )),
                              f,
                              r,
                              d[n + 2],
                              9,
                              -51403784
                            )),
                            m,
                            f,
                            d[n + 7],
                            14,
                            1735328473
                          )),
                          i,
                          m,
                          d[n + 12],
                          20,
                          -1926607734
                        )),
                        (r = md5_hh(
                          r,
                          (i = md5_hh(
                            i,
                            (m = md5_hh(m, f, r, i, d[n + 5], 4, -378558)),
                            f,
                            r,
                            d[n + 8],
                            11,
                            -2022574463
                          )),
                          m,
                          f,
                          d[n + 11],
                          16,
                          1839030562
                        )),
                        i,
                        m,
                        d[n + 14],
                        23,
                        -35309556
                      )),
                      (r = md5_hh(
                        r,
                        (i = md5_hh(
                          i,
                          (m = md5_hh(m, f, r, i, d[n + 1], 4, -1530992060)),
                          f,
                          r,
                          d[n + 4],
                          11,
                          1272893353
                        )),
                        m,
                        f,
                        d[n + 7],
                        16,
                        -155497632
                      )),
                      i,
                      m,
                      d[n + 10],
                      23,
                      -1094730640
                    )),
                    (r = md5_hh(
                      r,
                      (i = md5_hh(
                        i,
                        (m = md5_hh(m, f, r, i, d[n + 13], 4, 681279174)),
                        f,
                        r,
                        d[n + 0],
                        11,
                        -358537222
                      )),
                      m,
                      f,
                      d[n + 3],
                      16,
                      -722521979
                    )),
                    i,
                    m,
                    d[n + 6],
                    23,
                    76029189
                  )),
                  (r = md5_hh(
                    r,
                    (i = md5_hh(
                      i,
                      (m = md5_hh(m, f, r, i, d[n + 9], 4, -640364487)),
                      f,
                      r,
                      d[n + 12],
                      11,
                      -421815835
                    )),
                    m,
                    f,
                    d[n + 15],
                    16,
                    530742520
                  )),
                  i,
                  m,
                  d[n + 2],
                  23,
                  -995338651
                )),
                (r = md5_ii(
                  r,
                  (i = md5_ii(
                    i,
                    (m = md5_ii(m, f, r, i, d[n + 0], 6, -198630844)),
                    f,
                    r,
                    d[n + 7],
                    10,
                    1126891415
                  )),
                  m,
                  f,
                  d[n + 14],
                  15,
                  -1416354905
                )),
                i,
                m,
                d[n + 5],
                21,
                -57434055
              )),
              (r = md5_ii(
                r,
                (i = md5_ii(
                  i,
                  (m = md5_ii(m, f, r, i, d[n + 12], 6, 1700485571)),
                  f,
                  r,
                  d[n + 3],
                  10,
                  -1894986606
                )),
                m,
                f,
                d[n + 10],
                15,
                -1051523
              )),
              i,
              m,
              d[n + 1],
              21,
              -2054922799
            )),
            (r = md5_ii(
              r,
              (i = md5_ii(
                i,
                (m = md5_ii(m, f, r, i, d[n + 8], 6, 1873313359)),
                f,
                r,
                d[n + 15],
                10,
                -30611744
              )),
              m,
              f,
              d[n + 6],
              15,
              -1560198380
            )),
            i,
            m,
            d[n + 13],
            21,
            1309151649
          )),
          (r = md5_ii(
            r,
            (i = md5_ii(
              i,
              (m = md5_ii(m, f, r, i, d[n + 4], 6, -145523070)),
              f,
              r,
              d[n + 11],
              10,
              -1120210379
            )),
            m,
            f,
            d[n + 2],
            15,
            718787259
          )),
          i,
          m,
          d[n + 9],
          21,
          -343485551
        )),
          (m = safe_add(m, h)),
          (f = safe_add(f, t)),
          (r = safe_add(r, g)),
          (i = safe_add(i, e));
      }
      return Array(m, f, r, i);
    }
    function md5_cmn(d, _, m, f, r, i) {
      return safe_add(bit_rol(safe_add(safe_add(_, d), safe_add(f, i)), r), m);
    }
    function md5_ff(d, _, m, f, r, i, n) {
      return md5_cmn((_ & m) | (~_ & f), d, _, r, i, n);
    }
    function md5_gg(d, _, m, f, r, i, n) {
      return md5_cmn((_ & f) | (m & ~f), d, _, r, i, n);
    }
    function md5_hh(d, _, m, f, r, i, n) {
      return md5_cmn(_ ^ m ^ f, d, _, r, i, n);
    }
    function md5_ii(d, _, m, f, r, i, n) {
      return md5_cmn(m ^ (_ | ~f), d, _, r, i, n);
    }
    function safe_add(d, _) {
      var m = (65535 & d) + (65535 & _);
      return (((d >> 16) + (_ >> 16) + (m >> 16)) << 16) | (65535 & m);
    }
    function bit_rol(d, _) {
      return (d << _) | (d >>> (32 - _));
    }


})();


