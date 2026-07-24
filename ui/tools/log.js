const LogTool = {
  currentPath: null,
  watching: false,
  rawContent: '', 

  pickFile() {
    sendToolRequest('log', 'pickFile', '', (response) => {
      if (response.status !== 'ok') {
        document.getElementById('log-status').textContent = 'Ошибка: ' + response.message;
        return;
      }
      const result = JSON.parse(response.result);
      if (result.cancelled) return;

      this.currentPath = result.path;
      document.getElementById('log-filename').textContent = result.path;
      this.loadContent();
    });
  },

  loadContent() {
    if (!this.currentPath) return;
    sendToolRequest('log', 'readAll', this.currentPath, (response) => {
      if (response.status !== 'ok') {
        document.getElementById('log-status').textContent = 'Ошибка: ' + response.message;
        return;
      }
      const result = JSON.parse(response.result);
      this.rawContent = result.content;
      this.renderFiltered();
      document.getElementById('log-status').textContent = `Загружено: ${(result.sizeBytes / 1024).toFixed(1)} KB`;
    });
  },

  toggleWatch() {
    if (!this.currentPath) {
      document.getElementById('log-status').textContent = 'Сначала выберите файл';
      return;
    }

    if (this.watching) {
      sendToolRequest('log', 'stopWatch', '', () => {
        this.watching = false;
        document.getElementById('log-watch-btn').textContent = 'Start Watching';
      });
    } else {

      const currentOffset = new Blob([this.rawContent]).size;

      const payload = JSON.stringify({ path: this.currentPath, offset: currentOffset });
      sendToolRequest('log', 'startWatch', payload, () => {
        this.watching = true;
        document.getElementById('log-watch-btn').textContent = 'Stop Watching';
      });
    }
  },

  filter() {
    this.renderFiltered();
  },

  renderFiltered() {
    const term = document.getElementById('log-filter').value.toLowerCase();
    const lines = this.rawContent.split('\n');
    const filtered = term ? lines.filter(l => l.toLowerCase().includes(term)) : lines;

    const output = document.getElementById('log-output');
    output.textContent = filtered.join('\n');
    output.scrollTop = output.scrollHeight;
  }
};

window.chrome.webview.addEventListener('message', (event) => {
  try {
    const data = JSON.parse(event.data);
    if (data.tool === 'log' && data.event === 'newLines') {

      LogTool.rawContent += data.content;
      LogTool.renderFiltered();
    }
  } catch (e) {

  }
});