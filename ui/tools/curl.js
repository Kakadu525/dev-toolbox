const CurlTool = {
  headerCount: 0,

  addHeader() {
    const container = document.getElementById('curl-headers');
    const id = this.headerCount++;
    const row = document.createElement('div');
    row.style.cssText = 'display: flex; gap: 8px; margin-bottom: 8px;';
    row.innerHTML = `
      <input type="text" class="curl-header-key" placeholder="Header name" style="flex: 1; padding: 6px; background: #2d2d30; color: white; border: 1px solid #3c3c3c;">
      <input type="text" class="curl-header-value" placeholder="Header value" style="flex: 1; padding: 6px; background: #2d2d30; color: white; border: 1px solid #3c3c3c;">
    `;
    container.appendChild(row);
  },

  generate() {
    const method = document.getElementById('curl-method').value;
    const url = document.getElementById('curl-url').value;
    const body = document.getElementById('curl-body').value;

    const headers = [];
    document.querySelectorAll('#curl-headers > div').forEach(row => {
      const key = row.querySelector('.curl-header-key').value;
      const value = row.querySelector('.curl-header-value').value;
      if (key) headers.push({ key, value });
    });

    const payload = JSON.stringify({ method, url, body, headers });

    sendToolRequest('curl', 'generate', payload, (response) => {
      document.getElementById('curl-output').value =
        response.status === 'ok' ? response.result : 'Ошибка: ' + response.message;
    });
  }
};