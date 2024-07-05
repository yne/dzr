-- "extension.lua"
-- VLC Extension basic structure (template): ----------------

-- Install
-- Windows: %APPDATA%/vlc/lua/extensions/basic.lua
-- Mac:     /Applications/VLC/.../lua/extensions/basic.lua
-- Linux:   ~/.local/share/vlc/lua/extensions/basic.lua

function descriptor()
    return {
        title = "VLC Extension - Basic structure",
        version = "1.0",
        author = "",
        url = 'http://',
        shortdesc = "short description",
        description = "full description",
        capabilities = {"menu", "input-listener", "meta-listener", "playing-listener"}
    }
end

function activate()
    -- this is where extension starts
    -- for example activation of extension opens custom dialog box:
    create_dialog()
end
function deactivate()
    -- what should be done on deactivation of extension
end
function close()
    -- function triggered on dialog box close event
    -- for example to deactivate extension on dialog box close:
    -- vlc.deactivate()
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
    w = vlc.dialog("VLC Extension - Dialog box example")
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