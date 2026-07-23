const HashTool = {
  md5() {
    const input = document.getElementById('hash-input').value;
    sendToolRequest('hash', 'md5', input, (response) => {
      document.getElementById('hash-output').value =
        response.status === 'ok' ? response.result : 'Ошибка: ' + response.message;
    });
  },
  sha256() {
    const input = document.getElementById('hash-input').value;
    sendToolRequest('hash', 'sha256', input, (response) => {
      document.getElementById('hash-output').value =
        response.status === 'ok' ? response.result : 'Ошибка: ' + response.message;
    });
  }
};