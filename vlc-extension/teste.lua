-- Sample Extension

ext_title = "ext-Sample"
selected_item = ""
menu_items = {"Item 1", "Item 2", "Item 3", "Item 4"}

-- Descriptor: VLC looks for this to provide meta info about our extension
function descriptor()
	return {
		title = ext_title,
		version = "0.1",
		author = "verghost",
		url = 'https://github.com/verghost/vlc-lua-docs',
		description = "A Sample extension that does sample related things.",
		capabilities = {"menu", "input-listener"}
	}
end

function set_selection(s)
	selected_item = tostring(s)
	lbl_wgt:set_text("You have selected: " .. selected_item)
end

-- Callback function for the "Click Me" button
function click()
	local l1, l2 = dropd_wgt:get_value()
	set_selection(l2)
end

function activate()
	main_dlg = vlc.dialog(ext_title)
	
	-- VLC will always try to shrink the dialog window, so always define your col, row, hspan and vspan.
	main_dlg:add_label("Welcome to the Sample Extension!", 1, 1, 8, 1)
	
	-- add dropdown
	dropd_wgt = main_dlg:add_dropdown(1, 2, 8, 1)
	for i,e in pairs(menu_items) do
		dropd_wgt:add_value(e, i-1)
	end
	
	-- Add a label
	lbl_wgt = main_dlg:add_label("No item selected", 1, 3, 8, 1)
	main_dlg:add_button("Click Me", click)
	
	main_dlg:show() -- Once we've finished setting up our dialog, we make it visible
end

-- Called when the dialog window is closed
function close()
	vlc.deactivate() -- call deactivate() here, assuming that the extension should also be deactivated when the main dialog box closes
end

-- Called by vlc to get menu options
function menu()
	return menu_items
end

-- this function recieves the index of the chosen menu item
function trigger_menu(id)
	set_selection(menu_items[id])
end

-- input listener
function input_changed()
	vlc.msg.info("Input changed!")
end

-- Called when the extension is deactivated.
-- Deactivation closes the menu and stops the extension script entirely
function deactivate()
	main_dlg:delete() -- if we're deactivating, we should destroy our dialog
end