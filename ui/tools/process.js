const ProcessTool = {
  processes: [],
  sortColumn: 'memoryBytes',
  sortDirection: 'desc', // 'asc' | 'desc'

  refresh() {
    document.getElementById('process-status').textContent = 'Загрузка...';
    sendToolRequest('process', 'list', '', (response) => {
      if (response.status !== 'ok') {
        document.getElementById('process-status').textContent = 'Ошибка: ' + response.message;
        return;
      }
      const result = JSON.parse(response.result);
      this.processes = result.processes;
      this.render();
      document.getElementById('process-status').textContent = `Всего процессов: ${this.processes.length}`;
    });
  },

  setSort(column) {
    if (this.sortColumn === column) {
      this.sortDirection = this.sortDirection === 'asc' ? 'desc' : 'asc';
    } else {
      this.sortColumn = column;
      this.sortDirection = column === 'memoryBytes' ? 'desc' : 'asc';
    }
    this.render();
  },

  render() {
    const filter = document.getElementById('process-filter').value.toLowerCase();
    const tbody = document.getElementById('process-tbody');
    tbody.innerHTML = '';

    let filtered = filter
      ? this.processes.filter(p => p.name.toLowerCase().includes(filter))
      : [...this.processes];

    const col = this.sortColumn;
    const dir = this.sortDirection === 'asc' ? 1 : -1;
    filtered.sort((a, b) => {
      let valA = a[col], valB = b[col];
      if (typeof valA === 'string') {
        return valA.localeCompare(valB) * dir;
      }
      return (valA - valB) * dir;
    });

    filtered.forEach(p => {
      const row = document.createElement('tr');
      row.innerHTML = `
        <td>${p.pid}</td>
        <td>${p.name}</td>
        <td>${p.memory}</td>
        <td>${p.threads}</td>
        <td class="process-path">${p.path}</td>
        <td><button class="process-kill-btn" data-pid="${p.pid}" data-name="${p.name}">Завершить</button></td>
      `;
      tbody.appendChild(row);
    });

    tbody.querySelectorAll('.process-kill-btn').forEach(btn => {
      btn.addEventListener('click', () => {
        this.terminate(btn.dataset.pid, btn.dataset.name);
      });
    });

    this.updateSortIndicators();
  },

  updateSortIndicators() {
    document.querySelectorAll('#tool-process th[data-sort]').forEach(th => {
      const indicator = th.querySelector('.sort-indicator');
      if (!indicator) return;
      if (th.dataset.sort === this.sortColumn) {
        indicator.textContent = this.sortDirection === 'asc' ? ' ▲' : ' ▼';
      } else {
        indicator.textContent = '';
      }
    });
  },

  terminate(pid, name) {
    if (!confirm(`Завершить процесс "${name}" (PID ${pid})?`)) return;

    sendToolRequest('process', 'terminate', pid, (response) => {
      if (response.status !== 'ok') {
        alert('Ошибка: ' + response.message);
        return;
      }
      const result = JSON.parse(response.result);
      if (!result.success) {
        alert(result.message);
      } else {
        this.refresh();
      }
    });
  }
};