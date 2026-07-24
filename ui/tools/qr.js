const QrTool = {
  generate() {
    const text = document.getElementById('qr-text').value;
    const ecc = document.getElementById('qr-ecc').value;
    const payload = JSON.stringify({ text, ecc });

    sendToolRequest('qr', 'generate', payload, (response) => {
      const container = document.getElementById('qr-output');
      if (response.status !== 'ok') {
        container.textContent = 'Ошибка: ' + response.message;
        return;
      }
      const result = JSON.parse(response.result);
      container.innerHTML = result.svg;
    });
  }
};