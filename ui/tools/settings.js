const SettingsTool = {
  current: { theme: 'dark', fontSize: 'medium' },

  load() {
    sendToolRequest('settings', 'load', '', (response) => {
      if (response.status !== 'ok') return;
      this.current = JSON.parse(response.result);
      this.applyTheme(this.current.theme);
      this.applyFontSize(this.current.fontSize);
      this.syncControls();
    });
  },

  save() {
    const theme = document.getElementById('settings-theme').value;
    const fontSize = document.getElementById('settings-font-size').value;
    this.current = { theme, fontSize };

    this.applyTheme(theme);
    this.applyFontSize(fontSize);

    sendToolRequest('settings', 'save', JSON.stringify(this.current), (response) => {
      document.getElementById('settings-status').textContent =
        response.status === 'ok' ? 'Настройки сохранены' : 'Ошибка: ' + response.message;
    });
  },

  applyTheme(theme) {
    document.body.classList.remove('theme-dark', 'theme-light');
    document.body.classList.add('theme-' + theme);
  },

  applyFontSize(size) {
    document.body.classList.remove('font-small', 'font-medium', 'font-large');
    document.body.classList.add('font-' + size);
  },

  syncControls() {
    document.getElementById('settings-theme').value = this.current.theme;
    document.getElementById('settings-font-size').value = this.current.fontSize;
  }
};

document.addEventListener('DOMContentLoaded', () => {
  SettingsTool.load();
});