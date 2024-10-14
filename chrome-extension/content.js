

chrome.runtime.onMessage.addListener((request, sender, sendResponse) => {
  if (request.type === "get") {
    console.log(request);
    get(request.url, request.headers, sendResponse);
  } else if (request.type === "post") {
    post(request.url, request.headers, request.data, sendResponse);
  } else if (request.type === "browse") {
    browse(request.url, sendResponse);
  }
});
