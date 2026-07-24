const HttpTool = {
  headerCount: 0,

  addHeader() {
    const container = document.getElementById('http-headers');
    const row = document.createElement('div');
    row.style.cssText = 'display: flex; gap: 8px; margin-bottom: 8px;';
    row.innerHTML = `
      <input type="text" class="http-header-key" placeholder="Header name" style="flex: 1; padding: 6px; background: #2d2d30; color: white; border: 1px solid #3c3c3c;">
      <input type="text" class="http-header-value" placeholder="Header value" style="flex: 1; padding: 6px; background: #2d2d30; color: white; border: 1px solid #3c3c3c;">
    `;
    container.appendChild(row);
  },

  send() {
    const method = document.getElementById('http-method').value;
    const url = document.getElementById('http-url').value;
    const body = document.getElementById('http-body').value;

    const headers = [];
    document.querySelectorAll('#http-headers > div').forEach(row => {
      const key = row.querySelector('.http-header-key').value;
      const value = row.querySelector('.http-header-value').value;
      if (key) headers.push({ key, value });
    });

    const payload = JSON.stringify({ method, url, body, headers });
    const statusEl = document.getElementById('http-status');
    statusEl.textContent = 'Отправка запроса...';

    sendToolRequest('http', 'send', payload, (response) => {
      if (response.status !== 'ok') {
        statusEl.textContent = 'Ошибка: ' + response.message;
        document.getElementById('http-response-headers').value = '';
        document.getElementById('http-response-body').value = '';
        return;
      }

      const result = JSON.parse(response.result);
      const statusColor = result.status >= 200 && result.status < 300 ? '#4caf50' :
                           result.status >= 400 ? '#f44336' : '#ffa726';

      statusEl.innerHTML = `<span style="color: ${statusColor}; font-weight: bold;">Status: ${result.status}</span> — ${result.timeMs} ms`;
      document.getElementById('http-response-headers').value = result.headers;
      document.getElementById('http-response-body').value = result.body;
    });
  }
};