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

GW_META = {
    __call = function(table)
        table = table or {}
        local method = table.method or ""
        local sid = table.sid or ""
        local api_token = table.api_token or "''"
        local opt = table.opt or {}
        local data = table.data or ""

        local type = 'GET'
        if next(opt) ~= nil then
            type = opt['method'] or 'GET'
        end
        
        local URL = "https://www.deezer.com/ajax/gw-light.php?input=3&api_version=1.0" .. "&method=" .. method .. "&api_token=" .. api_token
        
        local headers = {
            Cookie = "sid=" .. sid
        }

        if type == 'POST' then
            return post(URL, headers, data)
        else
            return get(URL, headers)
        end

    end
}

-- VLC DEFAULT FUNCTIONS ----------------

function descriptor()
    return {
        title = title,
        version = "1.0",
        author = "PitchHybrid",
        url = 'https://github.com/pitchhybrid/dzr',
        shortdesc = "Deezer player",
        description = "Accountless Deezer Player on VLC",
        capabilities = {"meta-listener"}
        -- capabilities = {"input-listener", , "playing-listener"}
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

function meta_changed()
    if ui['main_window'] then
        ui['main_window']:hide()
    end
end


-- USER FUNCTIONS

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
        ui['main_window']:add_button("Play", compile_tracks, 1, 6, default_colspan, default_rowspan)
        for k, v in ipairs(map_selection) do
            ui['list']:add_value(v.label, tostring(v.id))
        end
    end
    if json_next then
        ui['search']:set_text("More")
    end
    ui['main_window']:update()
end

function compile_tracks()
    select_itens(ui['list']:get_selection())
    for i, v in ipairs(selection) do
        if v.play_type == "Track" then
            table.insert(tracks, #tracks + 1, v['entry'])
        else
            local tracklist = v['entry']['tracklist']
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
    play(tracks)
end

function gw(callableTable)
    setmetatable(callableTable, GW_META)
    return callableTable()
end

function play(tracks)
    
    if next(tracks) == nil then
        return
    end

    local function _gw(args)
        local r = dkjson.decode(gw(args))
        if r['error'] and next(r['error']) then
            debug(dkjson.encode(r))
            return {}
        end
        return dkjson.decode(gw(args)).results
    end
    debug("playing .....")
    -- deezer.ping
    local DZR_PNG = _gw({
        method = 'deezer.ping', 
        api_token = ''
    })
    local USR_NFO = _gw({
        method = 'deezer.getUserData', 
        sid = DZR_PNG['SESSION']
    })
    local SNG_NFO = _gw({
        method = 'song.getListData',
        sid = DZR_PNG['SESSION'],
        api_token = USR_NFO['checkForm'],
        opt = { method = 'POST' },
        data = { sng_ids = map(function (i, t)
                return t.id
            end, unpack(tracks))
        }
    })
    

    if SNG_NFO['data'] == nil then
        return
    else
        if next(SNG_NFO['data']) == nil then
            return
        end
    end

    local URL_NFO = dkjson.decode(post('https://media.deezer.com/v1/get_url', {}, {
        track_tokens = map(function (i, d)
            return d['TRACK_TOKEN']
        end, unpack(SNG_NFO['data'])),
        license_token = USR_NFO['USER']['OPTIONS']['license_token'],
        media = {
            {
                type = 'FULL',
                formats = {
                    { cipher = "BF_CBC_STRIPE", format = 'MP3_128' }
                }
            }
        }
    }))
    
    for i, v in ipairs(URL_NFO['data']) do
        if v['errors'] and next(v['errors']) then
            for j, e in ipairs(v['errors']) do
                debug(tracks[i] .. ' -> ' .. '(' .. e['code'] .. ') ' .. e['message'])
            end
            return
        end
    end


    local songs = map(function (i, d) 
        return {
            id = tostring(d['id']),
            md5_image = SNG_NFO['data'][i]['ALB_PICTURE'],
            duration = SNG_NFO['data'][i]['DURATION'],
            title = SNG_NFO['data'][i]['TITLE'],
            artists = map(function (i, a) 
                return {
                    id = a['ART_ID'],
                    name = a['ART_NAME'],
                    md5 = a['ART_PICTURE']
                } 
            end, unpack(SNG_NFO['data'][i]['ARTISTS'])),
            size = SNG_NFO['data'][i]['FILESIZE'],
            expire = SNG_NFO['data'][i]['TRACK_TOKEN_EXPIRE'],
            url = URL_NFO['data'][i]['media'][1]['sources'][1]['url']
        }    
    end, unpack(tracks))

    local hex = function (s)
        local cs = {}
        for c in s:gmatch(".") do
            table.insert(cs, string.byte(c))
        end
        return cs
    end

    local iv = map(function (i, c)
        return string.format('%02X',c)
    end, unpack({0,1,2,3,4,5,6,7}))

    local decrypt = map(function (_, song)
        local md = hex(md5(song['id']))

        local cbc = hex(CBC)

        local key = map(function (i, c)
            return string.format('%02X', XOR(c, XOR(md[i], md[i + 16])))
        end, unpack(cbc))

        local decrypted_audio = download_and_decrypt( song['id'], song['url'], table.concat(key), table.concat(iv))
       
        return  {song = song, decrypted_audio = decrypted_audio}

    end, unpack(songs))

    vlc.playlist.add(map( function (i, track)
        return {
            path = 'file://' .. track['decrypted_audio'],
            name = track['song']['title'],
            artist = track['song']['artists'][1]['name'],
            arturl = string.format('https://e-cdns-images.dzcdn.net/images/%s/%s/%sx%s.jpg', 'cover', track['song']['md5_image'], 1000,1000) 
        }    
    end,unpack(decrypt))) 
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

function try(f, ...)
    local args = {...}
    local status, output = pcall(function()
        if #args > 0 then
            return f(unpack(args))
        end
        return f()
    end)
    if status then
        return output
    end
    return nil
end

function debug(...)
    vlc.msg.info(...)
end

function map(func, ...)
    local args = {...}
    local resultados = {}
    for i, v in ipairs(args) do
        resultados[i] = func(i, v)
    end
    return resultados
end

function filter(table, predicate)
    local result = {}
    for key, value in pairs(table) do
        if predicate(key, value) then
            result[key] = value
        end
    end
    return result
end


-- NET UTILS -- 

function fetch(url, callback)
    local stream = vlc.stream(url)
    if stream then
       local response = try(function()
            return stream:read(100000)
        end)
        if response then
           callback(response)
        end
    end
end

function heading(headers)
    local param = {}
    local cookie = ''
    if headers then
        for k, v in pairs(headers) do
            if k == 'Cookie' then
                cookie = '-b "' .. v .. '"'
            else
                table.insert(param, #param + 1, '-H "' .. k .. ': ' .. v .. '"')
            end
        end
    end
    table.insert(param, #param + 1, cookie)
    return table.concat(param, ' ')
end

function popen(command)
    debug(command)
    local handle = io.popen(command, 'r')
    local response = handle:read("*a*")
    handle:close()
    return response
end

function get(url, headers)
    
    headers = headers or ''
    
    if type(headers) == 'table' then
        headers = heading(headers)
    end

    return popen('curl ' .. headers .. ' "' .. url .. '" ')
end

function post(url, headers, data)
    
    headers = headers or ''
    
    if type(headers) == 'table' then
        headers = heading(headers)
    end
    
    data = dkjson.encode(data) or ''
    
    return popen('curl -X POST ' .. headers .. ' -d '.. escape_string(data) ..' "' .. url .. '"')
end

function escape_string(str)
    -- Substitui caracteres especiais por seus equivalentes escapados
    local replacements = {
        ['\\'] = '\\\\', -- Barra invertida
        ['"'] = '\\"', -- Aspas duplas
        ["'"] = "\\'", -- Aspas simples
        ['\n'] = '\\n', -- Quebra de linha
        ['\t'] = '\\t', -- Tabulação
        ['\r'] = '\\r', -- Retorno de carro
        ['\b'] = '\\b', -- Backspace
        ['\f'] = '\\f' -- Formfeed
    }

    -- Faz o gsub para substituir todos os caracteres especiais
    return '"' .. (str:gsub(".", replacements)) .. '"'
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


-- CRYPT UTILS --
-- Operações bitwise para Lua 5.1

local function safe_number(a)
    if type(a) ~= "number" then
        return 0
    end
    return a
end

function AND(a, b)
    -- Garante que os parâmetros são números válidos
    a = safe_number(a)
    b = safe_number(b)

    local r = 0
    local m = 1
    for i = 0, 31 do
        if (a % 2 == 1 and b % 2 == 1) then
            r = r + m
        end
        a = math.floor(a / 2)
        b = math.floor(b / 2)
        m = m * 2
    end
    return r
end

function OR(a, b)
    -- Garante que os parâmetros são números válidos
    a = safe_number(a)
    b = safe_number(b)

    local r = 0
    local m = 1
    for i = 0, 31 do
        if (a % 2 == 1 or b % 2 == 1) then
            r = r + m
        end
        a = math.floor(a / 2)
        b = math.floor(b / 2)
        m = m * 2
    end
    return r
end

function XOR(a, b)
    local result = 0
    local bitval = 1
    while a > 0 or b > 0 do
        local abit = safe_number(a) % 2
        local bbit = safe_number(b) % 2
        if abit ~= bbit then
            result = result + bitval
        end
        a = math.floor(safe_number(a) / 2)
        b = math.floor(safe_number(b) / 2)
        bitval = bitval * 2
    end
    return result
end

function NOT(a)
    -- Garante que o parâmetro é um número válido
    a = safe_number(a)
    return 4294967295 - a
end

function LEFTSHIFT(a, b)
    -- Garante que os parâmetros são números válidos
    a = safe_number(a)
    b = safe_number(b)

    return (a * 2^b) % 4294967296
end

function RIGHTSHIFT(a, b)
    -- Garante que os parâmetros são números válidos
    a = safe_number(a)
    b = safe_number(b)

    return math.floor(a / 2^b)
end

-- Função para rotacionar bits à esquerda
function leftrotate(x, c)
    -- Garante que os parâmetros são números válidos
    x = safe_number(x)
    c = safe_number(c)

    return OR(LEFTSHIFT(x, c), RIGHTSHIFT(x, 32 - c))
end

function md5(message)
    return popen("echo -n '" .. message .. "' | openssl md5"):match("%= (.+)"):match("^%s*(.-)%s*$")
end

function download_and_decrypt(id, url, key, iv)
    -- Diretório temporário do VLC (ajustável conforme necessário)
    local temp_dir = vlc.config.cachedir() .. get_separator()

    os.execute("mkdir -p " .. temp_dir)
    
    -- Arquivo temporário criptografado e o arquivo de saída descriptografado
    local encrypted_file = temp_dir .. tostring(id) .. ".enc"
    local decrypted_file = temp_dir .. tostring(id) .. ".mp3"
    
    -- Comando curl para baixar o arquivo criptografado
    popen("curl -o " .. encrypted_file .. " " .. url)

    -- Comando openssl para descriptografar o arquivo
    -- key: chave de criptografia fornecida
    -- iv: vetor de inicialização fornecido, que será {1, 2, 3, 4, 5, 6, 7}
    -- Aqui estamos assumindo AES-128-CBC
    local openssl_cmd = string.format(
        "openssl bf-cbc -iter 1 -d -nopad -bufsize 2048 -K %s -iv %s -provider legacy -in %s -out %s",
        key, iv, encrypted_file, decrypted_file
    )
    -- Executar o comando de descriptografia
    popen(openssl_cmd)

    -- Verificar se o arquivo descriptografado existe e retornar o caminho
    local file = io.open(decrypted_file, "r")
    if file then
        file:close()
        return decrypted_file  -- Sucesso, retornar o caminho do arquivo MP3
    else
        return nil  -- Falha na descriptografia
    end
end

function get_separator()
    -- O primeiro caractere de package.config é o separador de diretórios
    return package.config:sub(1, 1)
end

