const JsonTool = {
  pretty() {
    const input = document.getElementById('json-input').value;
    sendToolRequest('json', 'pretty', input, (response) => {
      document.getElementById('json-output').value =
        response.status === 'ok' ? response.result : I18n.t('Error: ') + response.message;
    });
  },
  minify() {
    const input = document.getElementById('json-input').value;
    sendToolRequest('json', 'minify', input, (response) => {
      document.getElementById('json-output').value =
        response.status === 'ok' ? response.result : I18n.t('Error: ') + response.message;
    });
  }
};