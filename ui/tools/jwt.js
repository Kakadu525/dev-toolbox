const JwtTool = {
  decode() {
    const input = document.getElementById('jwt-input').value;
    sendToolRequest('jwt', 'decode', input, (response) => {
      document.getElementById('jwt-output').value =
        response.status === 'ok' ? response.result : 'Ошибка: ' + response.message;
    });
  }
};