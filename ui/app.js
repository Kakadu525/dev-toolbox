window.chrome.webview.addEventListener('message', (event) => {
  document.getElementById('output').textContent = event.data;
});

function sendMessage() {
  const input = document.getElementById('input').value;
  window.chrome.webview.postMessage(input);
}