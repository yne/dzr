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

default_colspan = 2
default_rowspan = 1

search_keys = {
    {   "Track",           "/search/track?q="      }, 
    {   "Artist",          "/search/artist?q="     }, 
    {   "Album",           "/search/album?q="      },
    {   "Playlist",        "/search/playlist?q="   }, 
    {   "User (name)",     "/search/user?q="       }, 
    {   "Radio",           "/search/radio?q="      }
}

search_user = {
    {   "User (id)",        "/user/0"              }
}

search_list = {
    {   "Genre (List)",     "/genre"                },
    {   "Radio (List)",     "/radio"                }
}

map_selection = {}
selection = {}

tracks = {};

play_type = nil

ui = {}

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
    ui['main_window'] = vlc.dialog(title)
    ui['main_window']:add_label("description:", 1, 1, default_colspan, default_rowspan)
    ui['search_input'] = ui['main_window']:add_text_input("", 1, 2, default_colspan, default_rowspan)
    ui['options'] = ui['main_window']:add_dropdown(1, 3, default_colspan, default_rowspan)
    ui['search'] = ui['main_window']:add_button("Search", search_api, 1, 4, default_colspan, default_rowspan)
    ui['list'] = ui['main_window']:add_list(1, 5, default_colspan, default_rowspan)
    for idx, val in ipairs(search_keys) do
        ui['options']:add_value(search_keys[idx][1], idx)
    end

    ui['main_window']:show()
end

function search_api()
    if json_next then
        browse(json_next)
    else
        if search_input and search_input ~= ui['search_input']:get_text() then
            json_next = nil
            ui['search']:set_text("Search")
            ui['main_window']:update()
        end
        search_input = ui['search_input']:get_text()
        if #search_input > 0 then
            local id = ui['options']:get_value()
            play_type = search_keys[id][1]
            local url = url_encode(API_DEEZER .. search_keys[id][2] .. search_input)
            browse(url)
        end
    end
    if next(map_selection) then
        ui['main_window']:add_button("Play", play, 1, 6, default_colspan, default_rowspan)
        for i, p in ipairs(map_selection) do
            ui['list']:add_value(p.label, tostring(p.id))
        end
    end
    if json_next then
        ui['search']:set_text("More")
    end
    ui['main_window']:update()
end

function deactivate()
    -- what should be done on deactivation of extension
    if ui['main_window'] then
        ui['main_window']:hide()
    end
    if logout then
        ui['main_window']:deactivate()
    end
end

function close()
    -- function triggered on dialog box close event
    -- for example to deactivate extension on dialog box close:
    vlc.deactivate()
end

function play()
    debug("playing .....")
    select_itens(ui['list']:get_selection())
    for i, v in ipairs(selection) do
        if v.play_type == "Track" then
            table.insert(tracks, #tracks + 1, v['entry'])
        else
            local tracklist = v['entry']['tracklist']
            debug(tracklist)
            if v['entry']['nb_tracks'] then
                tracklist = tracklist .. "&limit=" .. v['entry']['nb_tracks']
            end
            fetch(tracklist, function (response)
                local json = dkjson.decode(response)
                local data = json['data']
                for i = 1, #data do
                    table.insert(tracks, #tracks + 1, data[i])
                end
            end)
        end
    end
    debug(dkjson.encode(tracks))
end

function select_itens(sel_itens)
    selection = {}
    for k, v in pairs(sel_itens) do
        select(k)
    end
end

function select(value)
    for i, itens in ipairs(map_selection) do
        for k, v in pairs(itens) do
            if k == 'id' and tostring(v) == tostring(value) then
                table.insert(selection, #selection + 1, itens )
            end
        end
    end
end

function browse(url)
    ui['list']:clear()
    fetch(url, function (response)
        local json = dkjson.decode(response)
        local data = json['data']
        for i = 1, #data do
            local label = {}
            if data[i]['artist'] and data[i]['artist']['name'] then
                table.insert(label, data[i]['artist']['name'])
            end
            if data[i]['title'] or data[i]['name'] then
                table.insert(label, (data[i]['title'] or data[i]['name']))
            end
            if data[i]['nb_tracks'] then
                table.insert(label, string.format(" (%d)", data[i]['nb_tracks']))
            end
            table.insert(map_selection, #map_selection + 1, {
                id = data[i]['id'],
                play_type = play_type,
                label = table.concat(label, ' '),
                entry = data[i]
            })
        end
        if json.next then
            json_next = json.next
        end

    end)
end

-- local json = dkjson.decode(response)

-- if json.next then
--     return json.next
-- end


function fetch(url, callback)
    local stream = vlc.stream(url)
    if stream then
       local response = try(function()
            return stream:readline()
        end)
        if response then
           callback(response)
        end
    end
end

function add_to_map_selection(map_selection, value)
    
end

function try(f, ...)
    local args = {...}
    local status, output = pcall(function()
        if #args > 0 then
            return f(table.unpack(args))
        end
        return f()
    end)
    if status then
        return output
    end
    return nil
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

function filter(table, predicate)
    local result = {}
    for key, value in pairs(table) do
        if predicate(value) then
            result[key] = value
        end
    end
    return result
end

function url_encode0(str)
    -- Mantém caracteres URL seguros: : / ? & =
    str = string.gsub(str, "([^%w-_.~:/?&=])", function(c)
        return string.format("%%%02X", string.byte(c))
    end)
    return str
end

function url_encode(text)
    -- Preservar prefixo de URL se existir (http, https, ftp, etc.)
    local prefix, rest = string.match(text, "^(%a+://)(.*)$")
    if not prefix then
        prefix = ""
        rest = text
    end

    -- Substituir espaços por hífens
    rest = string.gsub(rest, " ", "-")
    -- Converter para minúsculas
    rest = string.lower(rest)
    -- Remover todos os caracteres não alfanuméricos (exceto hífens e os já permitidos em URLs)
    rest = string.gsub(rest, "[^%w-_.~:/?&=]", "")
    -- Aplicar percent-encoding
    rest = url_encode0(rest)

    return prefix .. rest
end
