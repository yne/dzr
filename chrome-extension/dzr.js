(function(){
    
    const API_DEEZER = "https://api.deezer.com";

    var root = document.getElementById("root");
    
    var searchKeys = [
      { name: "Track", path: "/search/track?q=" },
      { name: "Artist", path: "/search/artist?q=" },
      { name: "Album", path: "/search/album?q=" },
      { name: "Playlist", path: "/search/playlist?q=" },
      { name: "User", path: "/search/user?q=" },
      { name: "Radio", path: "/search/radio?q=" },
    ];

    var searchUser = [{ name: "User", path: "/user/0" }];

    var searchList = [
      { name: "Genre", path: "/genre" },
      { name: "Radio", path: "/radio" },
    ];

    var inputSearch = document.createElement("input");
    inputSearch.setAttribute("type", "text");
    inputSearch.setAttribute("placeholder", "Search");
    inputSearch.setAttribute("id", "inputSearch");
    root.appendChild(inputSearch);

    var fieldset = document.createElement("fieldset");
    fieldset.style = "margin: 10px 0";
    var legend = document.createElement("legend");
    legend.innerText = "Search results";
    fieldset.appendChild(legend);

    
    searchKeys.map((v) => {
        var populate = (text) => {
          var json = JSON.parse(text);
          for (data of json.data) {
            var label = [];
            if (data.artist && data.artist.name) {
              label.push(data.artist.name);
            }
            if (data.album && data.album.title) {
              label.push(data.album.title);
            }
            if (data.title) {
              label.push(data.title);
            }
            if (data.name) {
              label.push(data.name);
            }
            var li = document.createElement("li");
            li.innerText = label.join(" - ");
            fieldset.appendChild(li);
          }
          return json;
        };
        var button = document.createElement("button");
        button.style = 'margin: 5px 10px';
        button.innerText = v.name;
        button.addEventListener("click", () => {
            fieldset.replaceChildren([])
            if(inputSearch.value){
                var url = API_DEEZER + v.path + inputSearch.value;
                get(url, { "Content-Type": "application/json" }, (text) => {
                    var json = populate(text);
                    if(json.next){
                      var next = document.createElement("button");
                      next.innerText = "Next";
                      next.addEventListener('click', () => {
                        get(json.next, { "Content-Type": "application/json" }, (text) => {
                          json = populate(text);
                          if(!json.next){
                            root.removeChild(root.lastChild);
                          }
                        })
                      });
                      root.appendChild(next);
                    }
                })
            }
        });
        return button;
    }).forEach(v => root.appendChild(v));
    root.appendChild(fieldset);

    
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
    
})();


