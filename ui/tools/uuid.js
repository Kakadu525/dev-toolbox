const UuidTool = {
  generate() {
    sendToolRequest('uuid', 'generate', '', (response) => {
      document.getElementById('uuid-output').value =
        response.status === 'ok' ? response.result : 'Ошибка: ' + response.message;
    });
  }
};