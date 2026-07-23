const Base64Tool = {
  encode() {
    const input = document.getElementById('base64-input').value;
    sendToolRequest('base64', 'encode', input, (response) => {
      document.getElementById('base64-output').value =
        response.status === 'ok' ? response.result : 'Ошибка: ' + response.message;
    });
  },
  decode() {
    const input = document.getElementById('base64-input').value;
    sendToolRequest('base64', 'decode', input, (response) => {
      document.getElementById('base64-output').value =
        response.status === 'ok' ? response.result : 'Ошибка: ' + response.message;
    });
  }
};