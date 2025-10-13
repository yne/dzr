// const playlist = {
//     shuffle: false,
//     tracks:[],
//     addPlaylist(track){
//         this.tracks.push(track);
//     }
// };
chrome.runtime.onMessage.addListener((request, sender, sendResponse) => {
    if(request.action === 'add'){
        playlist.addPlaylist(request.track);
    }
  if (request.action === "play") {
    console.log(`Playing: ....`);
    playAudioFromByteBuffer(request.songs[0]);
    
    // sendResponse({ songs: request.songs });
  }
});


 