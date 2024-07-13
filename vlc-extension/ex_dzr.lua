-- "extension.lua"
-- VLC Extension basic structure (template): ----------------
-- Install
-- Windows: %APPDATA%/vlc/lua/extensions/basic.lua
-- Mac:     /Applications/VLC/.../lua/extensions/basic.lua
-- Linux:   ~/.local/share/vlc/lua/extensions/basic.lua
dkjson = require "dkjson"

API_DEEZER = "https://api.deezer.com"

title = "Accountless Deezer Player on VLC"
logout = false
CBC = "g4el58wc0zvf9na1"

default_colspan = 70
default_rowspan = 1

search_keys = {{"Track", "/search/track?q="}, {"Artist", "/search/artist?q="}, {"Album", "/search/album?q="},
               {"Playlist", "/search/playlist?q="}, {"User (name)", "/search/user?q="}, {"User (id)", "/user/0"},
               {"Radio", "/search/radio?q="}, {"Genre (List)", "/genre"}, -- list
{"Radio (List)", "/radio"} -- list
}

function descriptor()
    return {
        title = title,
        version = "1.0",
        author = "PitchHybrid",
        url = 'https://github.com/pitchhybrid/dzr',
        shortdesc = "Deezer player",
        description = "Accountless Deezer Player on VLC"
        -- capabilities = {"input-listener", "meta-listener", "playing-listener"}
    }
end

function activate()
    mainWindow = vlc.dialog(title)
    mainWindow:add_label("description:", 1, 1, default_colspan, default_rowspan)
    search_input = mainWindow:add_text_input("", 1, 2, default_colspan, default_rowspan)
    options = mainWindow:add_dropdown(1, 3, default_colspan, default_rowspan)
    mainWindow:add_button("Search", function()
        local id = options:get_value()
        local url = API_DEEZER .. search_keys[id][2] .. search_input:get_text()
        browse(url)
        mainWindow:update()

    end, 1, 4, default_colspan, default_rowspan)
    list = mainWindow:add_list(1, 5, default_colspan, default_rowspan)
    for idx, val in ipairs(search_keys) do
        options:add_value(search_keys[idx][1], idx)
    end

    mainWindow:show()
end

function deactivate()
    -- what should be done on deactivation of extension
    if mainWindow then
        mainWindow:hide()
    end

    if logout then
        mainWindow:deactivate()
    end
end

function close()
    -- function triggered on dialog box close event
    -- for example to deactivate extension on dialog box close:
    vlc.deactivate()
end

function browse(url)
    local stream = vlc.stream(url)
    if stream then
        local status, json = pcall(function()
            return stream:readline()
        end)
        if status then
            data = dkjson.decode(json)['data']
            for i = 1, #data do
                list:add_value(data[i]['title'] .. " [ " .. data[i]['nb_tracks'] .. " ] " .. "( ".. data[i]['user']['name'] .." )", data[i]['id'])
            end
        end
        if json.next then
            url = json.next
            debug(url)
        else
            json.next = nil
        end
    end
end

function debug(...)
    vlc.msg.dbg(...)
end

function map(func, ...)
    local args = {...}
    local resultados = {}
    for i, v in ipairs(args) do
        resultados[i] = func(v)
    end
    return resultados
end
