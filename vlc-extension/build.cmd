@ECHO ON
echo %0
luac5.1 -o %CD%/vlc-extension/ex_dzr.luac -s %CD%/vlc-extension/ex_dzr.lua  
cp %CD%/vlc-extension/ex_dzr.lua C:\\Users\\manoel.messias\\AppData\\Roaming\\vlc\\lua\\extensions\\ex_dzr.lua 
start vlc