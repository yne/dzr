-- "extension.lua"
-- VLC Extension basic structure (template): ----------------

-- Install
-- Windows: %APPDATA%/vlc/lua/extensions/basic.lua
-- Mac:     /Applications/VLC/.../lua/extensions/basic.lua
-- Linux:   ~/.local/share/vlc/lua/extensions/basic.lua
title = "Accountless Deezer Player on VLC"
logout = false
CBC = "g4el58wc0zvf9na1"
search_keys = {
    {"Track", "/search/track?q=" },
    {"Artist", "/search/artist?q=" },
    {"Album", "/search/album?q=" },
    {"Playlist", "/search/playlist?q=" },
    {"User (name)", "/search/user?q=" },
    {"User (id)", "/user/0" },
    {"Radio", "/search/radio?q=" },
    {"Genre (List)", "/genre" }, -- list
    {"Radio (List)", "/radio" }, -- list
}
function descriptor()
    return {
        title = title,
        version = "1.0",
        author = "PitchHybrid",
        url = 'https://github.com/pitchhybrid/dzr',
        shortdesc = "Deezer player",
        description = "Accountless Deezer Player on VLC",
        capabilities = {"input-listener", "meta-listener", "playing-listener"}
    }
end

function activate()
    mainWindow = vlc.dialog(title)
    mainWindow:add_label("Search", 1, 1, 7, 1)
    search_input = mainWindow:add_text_input("", 1, 2, 7, 1)
    query_search = mainWindow:add_dropdown(1, 3, 7, 1)
    for idx, val in ipairs(search_keys) do
        query_search:add_value(search_keys[idx][1], idx)
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
    -- vlc.deactivate()
    deactivate()
end

function input_changed()
    -- related to capabilities={"input-listener"} in descriptor()
    -- triggered by Start/Stop media input event
end

function playing_changed()
    -- related to capabilities={"playing-listener"} in descriptor()
    -- triggered by Pause/Play madia input event
end

function meta_changed()
    -- related to capabilities={"meta-listener"} in descriptor()
    -- triggered by available media input meta data?
end

function menu()
    -- related to capabilities={"menu"} in descriptor()
    -- menu occurs in VLC menu: View > Extension title > ...
    return {"Menu item #1", "Menu item #2", "Menu item #3"}
end
-- Function triggered when an element from the menu is selected
function trigger_menu(id)
    if(id == 1) then
        --Menu_action1()
    elseif(id == 2) then
        --Menu_action2()
    elseif(id == 3) then
        --Menu_action3()
    end
end


-- Custom part, Dialog box example: -------------------------

greeting = "Welcome!<br />"  -- example of global variable
function create_dialog()
    w = vlc.dialog(title)
    w1 = w:add_text_input("Hello world!", 1, 1, 3, 1)
    w2 = w:add_html(greeting, 1, 2, 3, 1)
    w3 = w:add_button("Action!",click_Action, 1, 3, 1, 1)
    w4 = w:add_button("Clear",click_Clear, 2, 3, 1, 1)
end
function click_Action()
    local input_string = w1:get_text()  -- local variable
    local output_string = w2:get_text() .. input_string .. "<br />"
    --w1:set_text("")
    w2:set_text(output_string)
end
function click_Clear()
    w2:set_text("")
end