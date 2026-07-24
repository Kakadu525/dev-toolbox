const CronTool = {
  parse() {
    const input = document.getElementById('cron-input').value;

    sendToolRequest('cron', 'parse', input, (response) => {
      const output = document.getElementById('cron-output');
      if (response.status !== 'ok') {
        output.textContent = 'Ошибка: ' + response.message;
        return;
      }

      const result = JSON.parse(response.result);
      output.innerHTML = '<strong>Ближайшие запуски:</strong><br>' +
        result.nextRuns.map(d => d).join('<br>');
    });
  }
};