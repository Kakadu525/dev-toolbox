const SettingsTool = {
  current: { theme: 'dark', fontSize: 'medium', accentColor: '#06b6d4', language: 'en' },
  fontSizes: ['small', 'medium', 'large'],
  fontSizeLabels: { small: 'Small', medium: 'Medium', large: 'Large' },

  load() {
    sendToolRequest('settings', 'load', '', (response) => {
      if (response.status !== 'ok') return;
      this.current = Object.assign({ accentColor: '#06b6d4', language: 'en' }, JSON.parse(response.result));
      this.applyAll();
      this.syncControls();
    });
  },

  save() {
    sendToolRequest('settings', 'save', JSON.stringify(this.current), (response) => {
      document.getElementById('settings-status').textContent =
        response.status === 'ok' ? I18n.t('Settings saved') : I18n.t('Error: ') + response.message;
    });
  },

  setTheme(theme) {
    this.current.theme = theme;
    this.applyAll();
    this.syncControls();
    this.save();
  },

  setAccent(color) {
    this.current.accentColor = color;
    this.applyAll();
    this.syncControls();
    this.save();
  },

  setLanguage(language) {
    this.current.language = language;
    this.applyAll();
    this.syncControls();
    this.save();
  },

  stepFontSize(direction) {
    const idx = this.fontSizes.indexOf(this.current.fontSize);
    const newIdx = Math.min(this.fontSizes.length - 1, Math.max(0, idx + direction));
    this.current.fontSize = this.fontSizes[newIdx];
    this.applyAll();
    this.syncControls();
    this.save();
  },

  applyAll() {
    document.body.classList.remove('theme-dark', 'theme-light');
    document.body.classList.add('theme-' + this.current.theme);

    document.body.classList.remove('font-small', 'font-medium', 'font-large');
    document.body.classList.add('font-' + this.current.fontSize);

    document.body.style.setProperty('--accent', this.current.accentColor);

    I18n.setLanguage(this.current.language);
  },

  syncControls() {
    document.querySelectorAll('#settings-theme-group .segmented-btn').forEach(btn => {
      btn.classList.toggle('active', btn.dataset.value === this.current.theme);
    });
    document.querySelectorAll('#settings-accent-group .swatch').forEach(btn => {
      btn.classList.toggle('active', btn.dataset.value === this.current.accentColor);
    });
    document.querySelectorAll('#settings-language-group .segmented-btn').forEach(btn => {
      btn.classList.toggle('active', btn.dataset.value === this.current.language);
    });
    document.getElementById('settings-font-size-value').textContent =
      this.fontSizeLabels[this.current.fontSize];
  }
};

document.addEventListener('DOMContentLoaded', () => {
  SettingsTool.load();

  document.querySelectorAll('#settings-theme-group .segmented-btn').forEach(btn => {
    btn.addEventListener('click', () => SettingsTool.setTheme(btn.dataset.value));
  });
  document.querySelectorAll('#settings-accent-group .swatch').forEach(btn => {
    btn.addEventListener('click', () => SettingsTool.setAccent(btn.dataset.value));
  });
  document.querySelectorAll('#settings-language-group .segmented-btn').forEach(btn => {
    btn.addEventListener('click', () => SettingsTool.setLanguage(btn.dataset.value));
  });
});