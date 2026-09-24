// Interface language. The markup and scripts are written in English; Russian
// comes from the dictionary below, keyed by the English text, so a string
// missing from the dictionary simply stays in English instead of breaking.
const I18n = {
  lang: 'en',

  ru: {
    // Placeholders and static text in index.html
    'Enter text': 'Введите текст',
    'Result': 'Результат',
    'Generate': 'Сгенерировать',
    'Example: {"name":"John","age":30}': 'Например: {"name":"Иван","age":30}',
    'Example: <root><name>John</name></root>': 'Например: <root><name>Иван</name></root>',
    'Example: name: John\nage: 30': 'Например: name: Иван\nage: 30',
    'Paste a JWT token': 'Вставьте JWT токен',
    'Example: \\d+': 'Например: \\d+',
    'Test text': 'Тестовый текст',
    'Text A': 'Текст A',
    'Text B': 'Текст B',
    'Example: #ff5733': 'Например: #ff5733',
    'Example: */5 9-17 * * 1-5': 'Например: */5 9-17 * * 1-5',
    "Example: select id,name from users where age>18 and city='London'":
      "Например: select id,name from users where age>18 and city='Moscow'",
    'Request body (JSON etc.), optional': 'Тело запроса (JSON и т.д.), необязательно',
    'Text or URL to encode': 'Текст или URL для кодирования',
    'Request body (for POST/PUT/PATCH)': 'Тело запроса (для POST/PUT/PATCH)',
    'No file selected': 'Файл не выбран',
    'auto': 'авто',
    'Download result': 'Скачать результат',
    'Filter (e.g. ERROR)': 'Фильтр (например: ERROR)',
    'Clear history': 'Очистить историю',

    // Messages built in tools/*.js
    'Error: ': 'Ошибка: ',
    'Error': 'Ошибка',
    'Next runs:': 'Ближайшие запуски:',
    'Sending request...': 'Отправка запроса...',
    'History is empty. Copy text or an image (Ctrl+C) in any application.':
      'История пуста. Скопируйте текст или изображение (Ctrl+C) в любом приложении.',
    'Restore': 'Восстановить',
    'Text entries: {text} / 50, images: {images} / 10': 'Текстовых записей: {text} / 50, изображений: {images} / 10',
    'Copied to clipboard': 'Скопировано в буфер обмена',
    'Clear the whole clipboard history?': 'Очистить всю историю буфера обмена?',
    'File loaded: {name}': 'Файл загружен: {name}',
    'Choose a file first': 'Сначала выберите файл',
    'Processing...': 'Обработка...',
    'Done: {width}x{height}, {size} KB': 'Готово: {width}x{height}, {size} KB',
    'Loaded: {size} KB': 'Загружено: {size} KB',
    'Settings saved': 'Настройки сохранены',
    'End process "{name}" (PID {pid})?': 'Завершить процесс "{name}" (PID {pid})?',
  },

  // t('Loaded: {size} KB', { size: 12 }) -> "Loaded: 12 KB" / "Загружено: 12 KB"
  t(text, params) {
    let out = this.lang === 'ru' && this.ru[text] !== undefined ? this.ru[text] : text;
    if (params) {
      for (const [key, value] of Object.entries(params)) {
        out = out.split('{' + key + '}').join(String(value));
      }
    }
    return out;
  },

  setLanguage(lang) {
    this.lang = lang === 'ru' ? 'ru' : 'en';
    document.documentElement.lang = this.lang;
    this.applyToDocument();
  },

  // Remembers the English original on first pass, so switching back and forth
  // always translates from English and never from an already translated string.
  applyToDocument() {
    document.querySelectorAll('[placeholder]').forEach(el => {
      if (el.dataset.i18nPlaceholder === undefined) el.dataset.i18nPlaceholder = el.placeholder;
      el.placeholder = this.t(el.dataset.i18nPlaceholder);
    });
    document.querySelectorAll('[data-i18n]').forEach(el => {
      if (el.dataset.i18n === '') el.dataset.i18n = el.textContent;
      el.textContent = this.t(el.dataset.i18n);
    });
  },
};
