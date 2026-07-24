const YamlTool = {
  format() {
    const input = document.getElementById('yaml-input').value;
    sendToolRequest('yaml', 'format', input, (response) => {
      document.getElementById('yaml-output').value =
        response.status === 'ok' ? response.result : 'Ошибка: ' + response.message;
    });
  }
};