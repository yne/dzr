--[[

 Radio-Browser.info 0.7 add-on/lua script for VLC (Search window)

 Copyright © 2020-2023 Andrew Jackson (https://github.com/ceever)

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.


--- BUGS & REQUESTS: ---

Send me a mail or a ticket on github: https://github.com/ceever/Radio-Browser.info.lua
In case you use LXQt, Lubuntu or Gnome, checkout my other project: https://github.com/ceever/PCManFM-Qt-Context-Menu


--- INSTALLATION ---:

Put the relevant .lua file(s) into the according subfolder (see below) of the VLC lua directory. VLC lua directory by default:
* Windows (all users): %ProgramFiles%\VideoLAN\VLC\lua\
* Windows (current user): %APPDATA%\VLC\lua\
* Linux (all users): /usr/share/vlc/lua/
* Linux (current user): ~/.local/share/vlc/lua/
(create directories if they don't exist)

.lua files and according subfolder:
* ex_Radio_Browser_info.lua => ...\lua\extensions\
* sd_Radio_Browser_info.lua => ...\lua\sd\
* pl_Radio_Browser_info.lua => ...\lua\playlist\

(In case you want the nice smiley with the search extension as in the screenshots, place the "Radio-Browser.png" picture from the Github "gfx/" folder or the zip repository into "...\lua\extensions\" and change the path of the picture 'd:add_image("PATH")' in the "ex_Radio_Browser_info.lua" script.
Note, under MS Windows you may need to use double backslashes, e.g. 'd:add_image("C:\\Program Files\\VideoLAN\\VLC\\lua\\extensions\\Radio-Browser.png")')

Restart VLC.


--- EXPLANATION & USAGE ---:

**Important**: With these add-ons the VLC columns *Album*, *Genre* and *Description* will hold relevant information for each radio station, namely: 1) Album: either *Count: XXXX* or *Clicks: XXXX* (to sort on number of stations or popularity), 2) Genre: a genre desciption, and 3) Description: sortable Bitrate information. So, have them displayed and try to use them if feasible.

Inside the Service Discover / Internet tab of VLC you are better off transferring all (relevant) stations into the regular VLC playlist first (right click >> "Add to Playlist"), before trying to sort anything.

pl_Radio-Browser_info.lua (playlist plugin for Service Discovery):
* This plugin is needed by sd_Radio-Browser_info.lua (!).
* It converts Radio-Browser.info API specific links into lists or readable radio links.
* Generally you would not add such links manually.

sd_Radio-Browser_info.lua (Service Discovery / Internet):
* Service Discovery / Internet add-on for VLC ... i.e. listed on the left panel under "Internet".
* Explore and crawls through all radio stations classified by categories (codec, language, country, tag).
* It depends on pl_Radio-Browser_info.lua—both need to be installed at the same time.
* After having found one or more stations in the Service Discovery / Internet, it is best to copy them into the playlist (right click >> "Add to Playlist") and continue searching and them sorting there. The Service Discovery is a little limited in its sorting capabilities, especially after having explored several sub categories.

ex_Radio-Browser_info.lua (Search window):
* Found under the VLC menu: View >> "Radio-Browser.info (Search)"
* This can work standalone, without the other two add-ons/lua scripts.
* Searches are sent unencrypted via http, due to performance and cerificate error reasons. Change the http to https in the lua script if you want.
* Found radio stations are counted, and can be added to the regular (empty or non-empty) VLC playlist.
* Dropdown lists will not update (counts nor values) if one of the others search parameters is specified. Thus, even when specific stations exist, e.g. "Codec: AAC+ (102)" and "Language: Albania (27)", together they might not produce any results.
* In general, the more specific the search the less radio stations—some search parameters might exclude each other, e.g. language=afrikaans and country=Russia.

--]]

function descriptor()
	return { title="Radio-Browser.info (Search)",
		description = "Radio-Browser.info (Search)",
		version = "0.7",
		author = "Andrew Jackson",
		capabilities = {},
		url = "https://github.com/ceever"
	}
end

-- Parse CSV line ... from http://lua-users.org/wiki/LuaCsv
function ParseCSVLine(line,sep) 
	local res = {}
	local pos = 1
	sep = sep or ','
	while true do 
		local c = string.sub(line,pos,pos)
		if (c == "") then break end
		if (c == '"') then   -- quoted value (ignore separator within)
			local startp,endp = string.find(line,'^%b""',pos)
			if endp then -- In case of incomplete quotes, we will continue just using separators
				local txt = ""
				repeat
					local startp,endp = string.find(line,'^%b""',pos)
					txt = txt..string.sub(line,startp+1,endp-1)
					pos = endp + 1
					c = string.sub(line,pos,pos) 
					if (c == '"') then txt = txt..'"' end 
					-- check first char AFTER quoted string, if it is another
					-- quoted string without separator, then append it
					-- this is the way to "escape" the quote char in a quote. example:
					--   value1,"blub""blip""boing",value3  will result in blub"blip"boing  for the middle
				until (c ~= '"')
				table.insert(res,txt)
				assert(c == sep or c == "")
			end
			pos = pos + 1
		else   -- no quotes used, just look for the first separator
			local startp,endp = string.find(line,sep,pos)
			if (startp) then 
				table.insert(res,string.sub(line,pos,startp-1))
				pos = endp + 1
			else
				-- no separator found -> use rest of string and terminate
				table.insert(res,string.sub(line,pos))
				break
			end 
		end
	end
	return res
end

-- Getting a csv from Radio-Browser.info .. since xml breaks
function csv_request(path)
	local shuffled = {} -- The api demands a better handling here, but lua does not come with dns (reverse) lookup, so we have to rely on the current (Dec 2020) three servers.
	csv = nil
	while ( nil == csv ) do
		shuffled = {}
		math.randomseed(os.time())
		for i, v in ipairs( servers ) do
			table.insert(shuffled, math.random(1, #shuffled+1), v)
		end
		for _, server in pairs( shuffled ) do
			-- Fuck that broken vlc xml streaming (!)
			csv = vlc.stream( "http://" .. server .. "/csv/" .. path )
			if csv then
				break
			end
		end
	end
	return csv
end

function add_dropdown(d, path, row)
	local csv = csv_request(path)
	if csv then
		csv:readline() -- Drop the headers
		obj = d:add_dropdown( 2, row, 2, 1 )
		i = 1
		obj:add_value( "", i ) -- Empty line at the beginning
		
		if "codecs" ~= path then
			i = i + 1
			obj:add_value( "<EMPTY> (?)", i )
		end -- The empty option
		tmp = csv:readline()
		while tmp do
			i = i + 1
			if "codecs" == path then
				obj:add_value( tmp:match('^(.+),%d+$'):gsub('"', '') .. " (" .. tmp:match(',(%d+)$') .. ")", i )
			else
				obj:add_value( tmp:match('^([^,]+),') .. " (" .. tmp:match(',(%d+)$') .. ")", i )
			end
			tmp = csv:readline()
		end
	else 
		obj = d:add_text_input( "", 2, row, 2, 1 )
	end
	return obj
end

-- Creating the interaction windows
function activate()
	servers = {}
	local tmp = nil
	while nil == tmp do
		tmp = vlc.stream( "http://de1.api.radio-browser.info/json/servers" ):read( 100000 )
	end
	for server in tmp:gmatch '"ip"%s*:%s*"[%d%.]+"%s*,%s*"name"%s*:%s*"([^"]+)"' do
		table.insert( servers, server )
	end
	
	tracks = {}
	list_wgt = nil
	d = vlc.dialog("Radio-Browser.info (Search)")

	d:add_label( "Name (the shorter, the more):", 1, 1, 1, 1)
	d:add_label( "Tags (the fewer, the more):", 1, 2, 1, 1)
	d:add_label( "Codec:", 1, 3, 1, 1)
	d:add_label( "Country:", 1, 4, 1, 1)
	d:add_label( "Language:", 1, 5, 1, 1)
	
	d:add_label( "&nbsp; &nbsp; loading ...", 2, 3, 2, 1)
	d:add_label( "&nbsp; &nbsp; loading ...", 2, 4, 2, 1)
	d:add_label( "&nbsp; &nbsp; loading ...", 2, 5, 2, 1)
	
	name = d:add_text_input( "", 2, 1, 2, 1)
	tags = d:add_text_input( "TAG0,TAG 1,LONG TAG 2,#TAG_3", 2, 2, 2, 1)

	button = d:add_button("Search", main, 2, 6, 2, 1)
	button_c = d:add_button("Close", close, 1, 6, 1, 1)
	d:show()
	
	d:update()
	codec = add_dropdown(d, "codecs", 3)
	d:update()
	country = add_dropdown(d, "countries", 4)
	d:update()
	language = add_dropdown(d, "languages", 5)
	
	d:add_image("/PATH/TO/YOUR/Radio-Browser.png", 4, 1, 1, 6)
end

-- Getting the HTML search string with nice "&"s to be placed after "...search?"
function get_strg()
	strg = ""
	local tmp = name:get_text()
	if "" ~= tmp then
		tmp = tmp:gsub("^[%s%c]+", ""):gsub("[%s%c]+$", "")
		strg = strg .. "&name=" .. vlc.strings.encode_uri_component( tmp )
		name:set_text(tmp)
	end
	tmp = tags:get_text()
	if "" ~= tmp then
		if "TAG" ~= tmp:sub(1,3) then
			tmp = tmp:gsub("^[%s%c,]+", ""):gsub("[%s%c,]+$", ""):gsub(",[%s%c]+", ","):gsub("[%s%c]+,", ",")
			strg = strg .. "&tagList=" .. vlc.strings.encode_uri_component( tmp )
			tags:set_text(tmp)
		else
			tags:set_text("")
		end
	end
	if codec:get_text() then
		if "" ~= codec:get_text() then
			strg = strg .. "&codec=" .. vlc.strings.encode_uri_component( codec:get_text():gsub(" %(%d+%)", "") )
		end
	end
	if country:get_text() then
		if "" ~= country:get_text() then
			strg = strg .. "&country=" .. vlc.strings.encode_uri_component( country:get_text():gsub(" %([%d%?]+%)", ""):gsub("<EMPTY>", "") )
		end
	end
	if language:get_text() then
		if "" ~= language:get_text() then
			strg = strg .. "&languageExact=true&language=" .. vlc.strings.encode_uri_component( language:get_text():gsub(" %([%d%?]+%)", ""):gsub("<EMPTY>", "") )
		end
	end
	
	return strg
end

function close()
	vlc.deactivate()
end

function deactivate()
	if d then
		d:hide() 
	end
end

function meta_changed()
	vlc.deactivate()
end

function enqueue()
	local final = {}
	for k, _ in pairs( list_wgt:get_selection() ) do
		table.insert( final, tracks[k] )
	end
	if nil == next(final) then
		vlc.playlist.enqueue( tracks )
	else
		vlc.playlist.enqueue( final )
	end
end

function mainf()
	d:del_widget( button_f )
	if list_wgt then
		d:del_widget( list_wgt )
		list_wgt = nil
	end
	main()
end
-- Let's check for too large searches
function main()
	if "" == get_strg() then
		d:del_widget( button )
		button = d:add_button("Empty searches may timeout or fail! ... CHANGE or/and CONTINUE?", mainer, 2, 6, 2, 1)
		button_c = d:add_button("Close", close, 1, 6, 1, 1)
	else
		mainer()
	end
end

-- Let's search and fill the playlist ... this strongly depends on a stable/fixex csv structure (as of Dec 2020)
function mainer()
	d:del_widget( button )
	if button_c then d:del_widget( button_c ) end
	search = d:add_label( "Searching ...", 2, 6, 2, 1)
	d:update()

	local csv = csv_request( "stations/search" .. get_strg():gsub("^&", "?") )
	tmp = csv:readline()
	if tmp then
		list_wgt = d:add_list(1, 7, 4, 1)
		local size = 1
		local headers = {}
		for i, head in ipairs( ParseCSVLine(tmp) ) do
			headers[head] = i
			size = i
		end
		
		k = 0
		tracks = {}
		tmp = csv:readline()
		while tmp do
			search:set_text( "Searching ... " .. k .. "")
			d:update()
			
			line = ParseCSVLine( tmp )
			while not line[size] do
				tmp = tmp .. csv:readline()
				line = ParseCSVLine( tmp )
			end

			local cstrg = "     " .. line[headers["clickcount"]]
			local cstrlen = string.len(cstrg)
			local bstrg = "     " .. line[headers["bitrate"]]
			local bstrlen = string.len(bstrg)

			-- We cannot work with "m3u/url/stationuuid" paths for click counting, because VLC just resolves them and does not continue playing in SD. In playlist they get resolved, but the player jumps to the next item, which would be the resolved url unless VLC is in random—then it goes mental. ... >> "https://de1.api.radio-browser.info/m3u/url/" .. line[headers["stationuuid"]] <<
			local path = line[headers["url"]]
			if string.match(string.lower(line[headers["url"]]), ".pls$")
			or string.match(string.lower(line[headers["url"]]), ".m3u$")
			or string.match(string.lower(line[headers["url"]]), ".m3u8$")
			or string.match(string.lower(line[headers["url"]]), ".xspf$")
			or string.match(string.lower(line[headers["url"]]), ".asx$")
			or string.match(string.lower(line[headers["url"]]), ".smil$")
			or string.match(string.lower(line[headers["url"]]), ".vlc$")
			or string.match(string.lower(line[headers["url"]]), ".wpl$") then
				path = line[headers["url_resolved"]]
			end
			table.insert( tracks, {
				path = path, 
				title = line[headers["name"]],
				album = "Clicks:" .. cstrg:sub(cstrlen-5, cstrlen),
				description = "Bitrate:" .. bstrg:sub(bstrlen-5, bstrlen) .. " / " .. line[headers["codec"]] .. " / " .. line[headers["language"]] .. " / " .. line[headers["country"]],
				copyright = line[headers["homepage"]],
				arturl = line[headers["favicon"]],
				genre = line[headers["tags"]]
			} )
			
			k = k + 1
			list_wgt:add_value((line[headers["name"]] .. "  (" .. bstrg:gsub("^ +", ""):gsub("^0$", "?") .. " kbps / " .. line[headers["codec"]] .. " / " .. line[headers["countrycode"]]):gsub(" +/ +/ +", " / ") ..  " ... " .. line[headers["homepage"]]:gsub("/$", "") .. ")", k)
			
			tmp = csv:readline()
		end
		
		d:del_widget( search )
		button = d:add_button( "Change and Retry?", mainf, 2, 6, 1, 1)
		button_f = d:add_button( "Found: " .. k .. ". Add (selected)?", enqueue, 3, 6, 1, 1)
		button_c = d:add_button("Close", close, 1, 6, 1, 1)
	else
		d:del_widget( search )
		button = d:add_button( "Nothing found! ... Try with different parameters? ... Change and Retry!", main, 2, 6, 2, 1)
		button_c = d:add_button("Close", close, 1, 6, 1, 1)
	end
	
end
