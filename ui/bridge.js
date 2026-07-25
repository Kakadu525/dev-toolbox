const pendingCallbacks = {};

window.chrome.webview.addEventListener('message', (event) => {
  const response = JSON.parse(event.data);
  const callback = pendingCallbacks[response.tool];
  if (callback) {
    callback(response);
    delete pendingCallbacks[response.tool];
  }
});

function sendToolRequest(tool, action, payload, callback) {
  pendingCallbacks[tool] = callback;
  window.chrome.webview.postMessage(JSON.stringify({ tool, action, payload }));
}

function clearFields(...fieldIds) {
  fieldIds.forEach(id => {
    const el = document.getElementById(id);
    if (!el) return;

    if (el.tagName === 'TEXTAREA' || el.tagName === 'INPUT') {
      el.value = '';
    } else {
      el.textContent = '';
      el.innerHTML = '';
    }
  });
}