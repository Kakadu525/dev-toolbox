const QrTool = {
  lastSvgString: null,

  generate() {
    const text = document.getElementById('qr-text').value;
    const ecc = document.getElementById('qr-ecc').value;
    const payload = JSON.stringify({ text, ecc });

    sendToolRequest('qr', 'generate', payload, (response) => {
      const container = document.getElementById('qr-output');
      if (response.status !== 'ok') {
        container.innerHTML = '';
        container.textContent = 'Ошибка: ' + response.message;
        document.getElementById('qr-info').style.display = 'none';
        document.getElementById('qr-download-btn').style.display = 'none';
        this.lastSvgString = null;
        return;
      }
      const result = JSON.parse(response.result);
      container.innerHTML = result.svg;
      this.lastSvgString = result.svg;

      document.getElementById('qr-info').style.display = 'flex';
      document.getElementById('qr-info-chars').textContent = text.length;
      document.getElementById('qr-info-ecc').textContent = ecc.charAt(0);
      document.getElementById('qr-download-btn').style.display = 'inline-block';
    });
  },

  download() {
    if (!this.lastSvgString) return;

    const svgBlob = new Blob([this.lastSvgString], { type: 'image/svg+xml;charset=utf-8' });
    const url = URL.createObjectURL(svgBlob);

    const img = new Image();
    img.onload = () => {

      const scale = 4;
      const canvas = document.createElement('canvas');
      canvas.width = img.width * scale;
      canvas.height = img.height * scale;

      const ctx = canvas.getContext('2d');
      ctx.drawImage(img, 0, 0, canvas.width, canvas.height);
      URL.revokeObjectURL(url);

      canvas.toBlob((blob) => {
        const downloadUrl = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = downloadUrl;
        a.download = 'qrcode.png';
        a.click();
        URL.revokeObjectURL(downloadUrl);
      }, 'image/png');
    };
    img.src = url;
  }
};