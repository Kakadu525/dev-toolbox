const Base64Tool = {
  encode() {
    const input = document.getElementById('base64-input').value;
    sendToolRequest('base64', 'encode', input, (response) => {
      const output = response.status === 'ok' ? response.result : 'Ошибка: ' + response.message;
      document.getElementById('base64-output').value = output;
      if (response.status === 'ok') this.updateInfo('Encode', input, response.result);
    });
  },
  decode() {
    const input = document.getElementById('base64-input').value;
    sendToolRequest('base64', 'decode', input, (response) => {
      const output = response.status === 'ok' ? response.result : 'Ошибка: ' + response.message;
      document.getElementById('base64-output').value = output;
      if (response.status === 'ok') this.updateInfo('Decode', input, response.result);
    });
  },
  updateInfo(mode, input, output) {
    document.getElementById('base64-info').style.display = 'flex';
    document.getElementById('base64-info-mode').textContent = mode;
    document.getElementById('base64-info-inlen').textContent = input.length + ' chars';
    document.getElementById('base64-info-outlen').textContent = output.length + ' chars';
    const ratio = input.length > 0 ? Math.round((output.length / input.length) * 100) : 0;
    document.getElementById('base64-info-ratio').textContent = ratio + '%';
  }
};