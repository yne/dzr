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
    const audioContext = new (window.AudioContext || window.webkitAudioContext)();

 // Função para tocar áudio a partir de um byteBuffer
 function playAudioFromByteBuffer(byteBuffer) {
   // Decodificar o byteBuffer (ArrayBuffer) como um buffer de áudio
   audioContext.decodeAudioData(
     byteBuffer,
     function (decodedData) {
       // Criar um AudioBufferSourceNode para tocar o áudio decodificado
       const audioSource = audioContext.createBufferSource();
       audioSource.buffer = decodedData;

       // Conectar o nó de origem ao destino (alto-falantes)
       audioSource.connect(audioContext.destination);

       // Tocar o áudio
       audioSource.start(0);
     },
     function (error) {
       console.error("Erro ao decodificar o áudio:", error);
     }
   );
 }

})();


