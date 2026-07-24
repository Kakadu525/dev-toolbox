const SqlTool = {
  format() {
    const input = document.getElementById('sql-input').value;
    sendToolRequest('sql', 'format', input, (response) => {
      document.getElementById('sql-output').value =
        response.status === 'ok' ? response.result : 'Ошибка: ' + response.message;
    });
  }
};