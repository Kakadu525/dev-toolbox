const ColorTool = {
  convert() {
    const input = document.getElementById('color-input').value;

    sendToolRequest('color', 'convert', input, (response) => {
      if (response.status !== 'ok') {
        document.getElementById('color-output').textContent = 'Ошибка: ' + response.message;
        document.getElementById('color-preview').style.background = 'transparent';
        return;
      }

      const result = JSON.parse(response.result);
      document.getElementById('color-output').innerHTML =
        'HEX: ' + result.hex + '<br>' +
        'RGB: ' + result.rgb + '<br>' +
        'HSL: ' + result.hsl;
      document.getElementById('color-preview').style.background = result.hex;
    });
  }
};