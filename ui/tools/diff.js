const DiffTool = {
  compare() {
    const textA = document.getElementById('diff-text-a').value;
    const textB = document.getElementById('diff-text-b').value;
    const payload = JSON.stringify({ textA, textB });

    sendToolRequest('diff', 'compare', payload, (response) => {
      const container = document.getElementById('diff-output');
      container.innerHTML = '';

      if (response.status !== 'ok') {
        container.textContent = 'Ошибка: ' + response.message;
        return;
      }

      const result = JSON.parse(response.result);
      result.diff.forEach(entry => {
        const line = document.createElement('div');
        line.textContent = (entry.type === 'added' ? '+ ' : entry.type === 'removed' ? '- ' : '  ') + entry.line;
        line.className = 'diff-line diff-' + entry.type;
        container.appendChild(line);
      });
    });
  }
};