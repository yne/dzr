(function(){

    const API_DEEZER = "https://api.deezer.com";

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
      onclick: () => {
        function gw({ method, api_token = '', sid = '', opt={}, data }) {
          const URL = `https://www.deezer.com/ajax/gw-light.php?input=3&api_version=1.0&method=${method}&api_token=${api_token}`;

          const response = {};

          const headers = {
            Cookie: `sid=${sid}`,
          };

          if (opt && opt.method === "POST") {
            post(URL, headers, data, (r) => {
              response["r"] = r;
            });
          } else {
            get(URL, headers, (r) => {
              response["r"] = r;
            });
          }
          return response["r"];
        }
        function _gw({ method, api_token, sid, opt, data }) {
          const response = gw({ method, api_token, sid, opt, data });
          if (response) {
            if (response.error) {
              alert(response.error);
              return {};
            }
            return response.results;
          }
          return {};
        }

        const DZR_PNG = _gw({
          method: "deezer.ping",
          api_token: "",
        });

        const USR_NFO = _gw({
          method: "deezer.getUserData",
          sid: DZR_PNG["SESSION"],
        });

        const SNG_NFO = _gw({
          method: "song.getListData",
          sid: DZR_PNG["SESSION"],
          api_token: USR_NFO["checkForm"],
          opt: { method: "POST" },
          data: {
            sng_ids: Array.prototype.slice
              .call(fieldset.children)
              .map((i, el) => {
                if (el instanceof HTMLLIElement) {
                  return el.id;
                }
                return "";
              })
              .filter((v) => {
                if (v) return true;
                return false;
              }),
          },
        });
      },
    });

    searchKeys.map((v) => {
        var populate = (text) => {
          fieldset.appendChild(legend);
          var json = JSON.parse(text);
          for (data of json.data) {
            var li = createElement('li', {
              id: data.id,
              style: 'cursor: pointer; padding: 5px 10px;',
              innerText : buildLabel(data),
              data: data,
              onclick: function(){
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
                  get(
                    this.data.tracklist +
                      (this.data.nb_tracks
                        ? "&limit=" + this.data.nb_tracks
                        : ""),
                    { "Content-Type": "application/json" },
                    (text) => {
                      const json = JSON.parse(text);
                      for(data of json.data){
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
                  );
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
        button.addEventListener("click", () => {
            fieldset.replaceChildren([])
            play.style = "display: none";
            if(inputSearch.value){
                var url = API_DEEZER + v.path + inputSearch.value;
                get(url, { "Content-Type": "application/json" }, (text) => {
                    var json = populate(text);
                    if(json.next){
                      next.style = 'display: block';
                      next.addEventListener("click", () => {
                        get(json.next, { "Content-Type": "application/json" }, (text) => {
                            json = populate(text);
                            if (!json.next) {
                              next.style = 'display: none';
                            }
                          }
                        );
                      })
                    }else{
                      next.style = 'display: none';
                    }
                   
                })
            }
        });
        return button;
    }).forEach(v => root.appendChild(v));
    root.appendChild(fieldset);
    root.appendChild(next);
    root.appendChild(play);
    
    function sendMessageContentScript(message, callback) {
        chrome.tabs.query({active: true, currentWindow: true}, function (tabs) {
            chrome.tabs.sendMessage(tabs[0].id, message, callback);
        })
    }

    function sendMessage(message, callback) {
        chrome.runtime.sendMessage(message, callback);
    }

    function get(url, headers, callback) {
      const { origin } = new URL(url);
      chrome.permissions.request(
        {
          permissions: ["tabs"],
          origins: [origin + "/*"],
        },
        (grant) => {
          if (grant) {
            fetch(url, {
              method: "GET",
              headers: headers || {},
            })
              .then((response) => {
                if (response.ok) {
                  response.text().then(callback);
                } else {
                  console.error(response.statusText);
                }
              })
              .catch((error) => console.error(error));
          } else {
            console.error("Permission denied");
          }
        }
      );
    }

    function post(url, headers, data, callback) {
        const { origin } = new URL(url);
        chrome.permissions.request(
        {
            permissions: ["tabs"],
            origins: [origin + "/*"],
        },
        (grant) => {
            if (grant) {
                fetch(url, {
                  method: "POST",
                  headers: headers || {},
                  body: JSON.stringify(data || {}),
                })
                  .then(callback)
                  .catch(console.error);
            } else {
                console.error("Permission denied");
            }
        }
        );
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
        label.push(" " + data["nb_tracks"]);
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

})();


