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


 const audioContext = new (window.AudioContext || window.webkitAudioContext)();

 // Função para tocar áudio a partir de um byteBuffer
 function playAudioFromByteBuffer(byteBuffer) {
   // Decodificar o byteBuffer (ArrayBuffer) como um buffer de áudio
   audioContext.decodeAudioData(
     byteBuffer,
     function (decodedData) {
       // Criar um AudioBufferSourceNode para tocar o áudio decodificado
       const audioSource = audioContext.createBufferSource();
       audioSource.buffer = decodedData;

       // Conectar o nó de origem ao destino (alto-falantes)
       audioSource.connect(audioContext.destination);

       // Tocar o áudio
       audioSource.start(0);
     },
     function (error) {
       console.error("Erro ao decodificar o áudio:", error);
     }
   );
 }