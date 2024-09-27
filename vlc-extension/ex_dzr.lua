-- "extension.lua"
-- VLC Extension basic structure (template): ----------------
-- Install
-- Windows: %APPDATA%/vlc/lua/extensions/basic.lua
-- Mac:     /Applications/VLC/.../lua/extensions/basic.lua
-- Linux:   ~/.local/share/vlc/lua/extensions/basic.lua
dkjson = require "dkjson"
bit32 = require "bit32"

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
        ui['main_window']:add_button("Play", compile_tracks, 1, 6, default_colspan, default_rowspan)
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
    play()
end

function gw(callableTable)
    setmetatable(callableTable, GW_META)
    return callableTable()
end

function play()
    local function _gw(args)
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
        end, table.unpack(tracks))}
    })
    local URL_NFO = dkjson.decode(post('https://media.deezer.com/v1/get_url', {}, {
        track_tokens = map(function (i, d)
            return d['TRACK_TOKEN']
        end, table.unpack(SNG_NFO['data'])),
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
        if v['erros'] then
            for j, e in ipairs(v['erros']) do
                debug( tracks[i] .. ' -> ' .. '(' .. e['code'] .. ') ' .. e['message'])
            end
        end
    end

    local songs = map(function (i, d) 
        return {
            id = d['id'],
            md5_image = SNG_NFO['data'][i]['ALB_PICTURE'],
            duration = SNG_NFO['data'][i]['DURATION'],
            title = SNG_NFO['data'][i]['TITLE'],
            artists = map(function (i, a) 
                return {
                    id = a['ART_ID'],
                    name = a['ART_NAME'],
                    md5 = a['ART_PICTURE']
                } 
            end, table.unpack(SNG_NFO['data'][i]['ARTISTS'])),
            size = SNG_NFO['data'][i]['FILESIZE'],
            expire = SNG_NFO['data'][i]['TRACK_TOKEN_EXPIRE'],
            url = URL_NFO['data'][i]['media'][1]['sources'][1]['url']
        }    
    end, table.unpack(tracks))

    local hex = function (str) 
        local cs = {}
        for i in str:gmatch(".") do
            table.insert(cs, string.byte(i))
        end
        return cs
    end
    
    debug(dkjson.encode(songs,{indent=true}))

    
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
            return stream:readline()
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
    local handle = io.popen(command)
    local response = handle:read("*a")
    handle:close()
    return response
end

function get(url, headers)
    
    headers = headers or ''
    
    if type(headers) == 'table' then
        headers = heading(headers)
    end

    return popen('curl -s ' .. headers .. ' "' .. url .. '" ')
end

function post(url, headers, data)
    
    headers = headers or ''
    
    if type(headers) == 'table' then
        headers = heading(headers)
    end
    
    data = dkjson.encode(data) or ''
    
    return popen('curl -s -X POST ' .. headers .. ' -d \''.. data ..'\' "' .. url .. '"')
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
local function AND(a, b)
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

local function OR(a, b)
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

local function XOR(a, b)
    local r = 0
    local m = 1
    for i = 0, 31 do
        if (a % 2 ~= b % 2) then
            r = r + m
        end
        a = math.floor(a / 2)
        b = math.floor(b / 2)
        m = m * 2
    end
    return r
end

local function NOT(a)
    return 4294967295 - a
end

local function LEFTSHIFT(a, b)
    return (a * 2^b) % 4294967296
end

local function RIGHTSHIFT(a, b)
    return math.floor(a / 2^b)
end

-- Função para rotacionar bits à esquerda
local function leftrotate(x, c)
    return OR(LEFTSHIFT(x, c), RIGHTSHIFT(x, 32 - c))
end

local function md5(message)
    -- Sine parts
    local k = {}
    for i = 0, 63 do
        k[i] = math.floor(2^32 * math.abs(math.sin(i + 1)))
    end

    -- Initial variables
    local h0 = 0x67452301
    local h1 = 0xefcdab89
    local h2 = 0x98badcfe
    local h3 = 0x10325476

    -- Pre-processing
    local original_len_in_bits = #message * 8
    message = message .. "\128"
    while (#message * 8) % 512 ~= 448 do
        message = message .. "\0"
    end
    message = message .. string.pack("<I8", original_len_in_bits)

    -- Process the message in successive 512-bit chunks
    for chunk_start = 1, #message, 64 do
        local chunk = {string.unpack("<I4I4I4I4I4I4I4I4I4I4I4I4I4I4I4I4", message, chunk_start)}

        local a = h0
        local b = h1
        local c = h2
        local d = h3

        for i = 0, 63 do
            local f, g
            if i < 16 then
                f = OR(AND(b, c), AND(NOT(b), d))
                g = i
            elseif i < 32 then
                f = OR(AND(d, b), AND(NOT(d), c))
                g = (5 * i + 1) % 16
            elseif i < 48 then
                f = XOR(b, c, d)
                g = (3 * i + 5) % 16
            else
                f = XOR(c, OR(b, NOT(d)))
                g = (7 * i) % 16
            end

            local temp = d
            d = c
            c = b
            
            -- Determina a quantidade de rotação a ser aplicada
            local s
            if i < 16 then
                s = {7, 12, 17, 22}
            elseif i < 32 then
                s = {5, 9, 14, 20}
            elseif i < 48 then
                s = {4, 11, 16, 23}
            else
                s = {6, 10, 15, 21}
            end

            -- Aplica a rotação de acordo com o índice
            b = b + leftrotate(a + f + k[i] + chunk[g + 1], s[(i % 4) + 1])
            a = temp
        end

        h0 = AND(h0 + a, 0xFFFFFFFF)
        h1 = AND(h1 + b, 0xFFFFFFFF)
        h2 = AND(h2 + c, 0xFFFFFFFF)
        h3 = AND(h3 + d, 0xFFFFFFFF)
    end

    -- Produz o valor hash final (big-endian)
    return string.format("%08x%08x%08x%08x", h0, h1, h2, h3)
end

-- Função XOR entre dois blocos de bytes
local function xor_bytes(block1, block2)
    local result = {}
    for i = 1, #block1 do
        result[i] = string.char(bit.bxor(string.byte(block1, i), string.byte(block2, i)))
    end
    return table.concat(result)
end

-- CBC decrypt function
local function cbc_decrypt(ciphertext, key, iv, decrypt_block_func)
    local block_size = 16 -- bloco de 16 bytes para AES
    local plaintext = ""
    local previous_block = iv

    -- Processa cada bloco de ciphertext
    for i = 1, #ciphertext, block_size do
        local current_block = ciphertext:sub(i, i + block_size - 1)
        -- Descriptografa o bloco
        local decrypted_block = decrypt_block_func(current_block, key)
        -- XOR com o bloco anterior (ou IV para o primeiro bloco)
        local xor_result = xor_bytes(decrypted_block, previous_block)
        -- Adiciona o resultado ao texto plano final
        plaintext = plaintext .. xor_result
        -- Atualiza o bloco anterior
        previous_block = current_block
    end

    -- Retira o padding do último bloco (PKCS#7)
    local padding_length = string.byte(plaintext:sub(-1))
    plaintext = plaintext:sub(1, -padding_length - 1)

    return plaintext
end
