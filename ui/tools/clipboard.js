const ClipboardTool = {
  refresh() {
    sendToolRequest('clipboard', 'list', '', (response) => {
      if (response.status !== 'ok') {
        document.getElementById('clipboard-status').textContent = 'Ошибка: ' + response.message;
        return;
      }
      const result = JSON.parse(response.result);
      this.render(result.history);
    });
  },

  render(history) {
    const container = document.getElementById('clipboard-list');
    container.innerHTML = '';

    if (history.length === 0) {
      container.innerHTML = '<div style="color: #888;">История пуста. Скопируйте текст или изображение (Ctrl+C) в любом приложении.</div>';
      return;
    }

    history.forEach((entry, index) => {
      const item = document.createElement('div');
      item.className = 'clipboard-item';

      if (entry.type === 'image') {
        item.innerHTML = `
          <div class="clipboard-item-image">
            <img src="data:image/png;base64,${entry.content}" alt="clipboard image">
            <div class="clipboard-image-meta">${entry.width}×${entry.height}</div>
          </div>
          <button class="clipboard-restore-btn" data-index="${index}">Восстановить</button>
        `;
      } else {
        const preview = entry.content.length > 200 ? entry.content.substring(0, 200) + '…' : entry.content;
        item.innerHTML = `
          <div class="clipboard-item-text">${this.escapeHtml(preview)}</div>
          <button class="clipboard-restore-btn" data-index="${index}">Восстановить</button>
        `;
      }
      container.appendChild(item);
    });

    container.querySelectorAll('.clipboard-restore-btn').forEach(btn => {
      btn.addEventListener('click', () => this.restore(btn.dataset.index));
    });

    const textCount = history.filter(e => e.type === 'text').length;
    const imageCount = history.filter(e => e.type === 'image').length;
    document.getElementById('clipboard-status').textContent =
      `Текстовых записей: ${textCount} / 50, изображений: ${imageCount} / 10`;
  },

  restore(index) {
    sendToolRequest('clipboard', 'restore', String(index), (response) => {
      if (response.status !== 'ok') {
        document.getElementById('clipboard-status').textContent = 'Ошибка: ' + response.message;
        return;
      }
      document.getElementById('clipboard-status').textContent = 'Скопировано в буфер обмена';
    });
  },

  clear() {
    if (!confirm('Очистить всю историю буфера обмена?')) return;
    sendToolRequest('clipboard', 'clear', '', () => {
      this.refresh();
    });
  },

  escapeHtml(text) {
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
  }
};

document.addEventListener('DOMContentLoaded', () => {
  const clipboardTab = document.querySelector('[data-tool="clipboard"]');
  if (clipboardTab) {
    clipboardTab.addEventListener('click', () => ClipboardTool.refresh());
  }
});