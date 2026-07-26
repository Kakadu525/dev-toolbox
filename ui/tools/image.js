const ImageTool = {
  loadedImageBase64: null,

  onFileSelected(input) {
    const file = input.files[0];
    if (!file) return;

    document.getElementById('image-file-name').textContent = file.name;

    const reader = new FileReader();
    reader.onload = (e) => {
      this.loadedImageBase64 = e.target.result.split(',')[1];
      document.getElementById('image-status').textContent = 'Файл загружен: ' + file.name;
    };
    reader.readAsDataURL(file);
  },

  convert() {
    if (!this.loadedImageBase64) {
      document.getElementById('image-status').textContent = 'Сначала выберите файл';
      return;
    }

    const format = document.getElementById('image-format').value;
    const quality = parseInt(document.getElementById('image-quality').value, 10);
    const width = parseInt(document.getElementById('image-width').value, 10) || 0;
    const height = parseInt(document.getElementById('image-height').value, 10) || 0;

    const payload = JSON.stringify({
      image: this.loadedImageBase64,
      format, quality, width, height
    });

    document.getElementById('image-status').textContent = 'Обработка...';

    sendToolRequest('image', 'convert', payload, (response) => {
      if (response.status !== 'ok') {
        document.getElementById('image-status').textContent = 'Ошибка: ' + response.message;
        return;
      }

      const result = JSON.parse(response.result);
      const mime = result.format === 'jpeg' || result.format === 'jpg' ? 'image/jpeg' :
                   result.format === 'bmp' ? 'image/bmp' : 'image/png';
      const dataUrl = `data:${mime};base64,${result.image}`;

      const preview = document.getElementById('image-preview');
      preview.src = dataUrl;
      preview.style.display = 'block';

      const link = document.getElementById('image-download');
      link.href = dataUrl;
      link.download = 'converted.' + result.format;
      link.style.display = 'inline-block';

      document.getElementById('image-status').textContent =
        `Готово: ${result.width}x${result.height}, ${(result.sizeBytes / 1024).toFixed(1)} KB`;
    });
  }
};