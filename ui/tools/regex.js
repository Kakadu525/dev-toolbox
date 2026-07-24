const RegexTool = {
  test() {
    const pattern = document.getElementById('regex-pattern').value;
    const text = document.getElementById('regex-text').value;
    const payload = JSON.stringify({ pattern, text });

    sendToolRequest('regex', 'test', payload, (response) => {
      document.getElementById('regex-output').value =
        response.status === 'ok' ? response.result : 'Ошибка: ' + response.message;
    });
  }
};