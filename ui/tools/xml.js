const XmlTool = {
  pretty() {
    const input = document.getElementById('xml-input').value;
    sendToolRequest('xml', 'pretty', input, (response) => {
      document.getElementById('xml-output').value =
        response.status === 'ok' ? response.result : 'Ошибка: ' + response.message;
    });
  },
  minify() {
    const input = document.getElementById('xml-input').value;
    sendToolRequest('xml', 'minify', input, (response) => {
      document.getElementById('xml-output').value =
        response.status === 'ok' ? response.result : 'Ошибка: ' + response.message;
    });
  }
};