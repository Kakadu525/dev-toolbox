const ProcessTool = {
  processes: [],
  sortColumn: 'memoryBytes',
  sortDirection: 'desc',
  selectedPid: null,

  checkElevation() {
    sendToolRequest('process', 'status', '', (response) => {
      if (response.status !== 'ok') return;
      const { elevated } = JSON.parse(response.result);
      document.getElementById('process-elevation-banner').style.display = elevated ? 'none' : 'flex';
    });
  },

  restartElevated() {
    // On success this window closes and an elevated one opens; a declined UAC
    // prompt just leaves everything as it was.
    sendToolRequest('process', 'restartElevated', '', (response) => {
      if (response.status !== 'ok') alert(I18n.t('Error: ') + response.message);
    });
  },

  refresh() {
    document.getElementById('process-stat-count').textContent = '…';
    sendToolRequest('process', 'list', '', (response) => {
      if (response.status !== 'ok') {
        document.getElementById('process-stat-count').textContent = I18n.t('Error');
        return;
      }
      const result = JSON.parse(response.result);
      this.processes = result.processes;
      this.selectedPid = null;
      document.getElementById('process-detail').style.display = 'none';
      this.render();
      this.renderStats();
    });
  },

  renderStats() {
    const totalMemBytes = this.processes.reduce((sum, p) => sum + (p.memoryBytes || 0), 0);
    const totalThreads = this.processes.reduce((sum, p) => sum + (p.threads || 0), 0);
    document.getElementById('process-stat-count').textContent = this.processes.length;
    document.getElementById('process-stat-mem').textContent = (totalMemBytes / (1024 * 1024 * 1024)).toFixed(1) + ' GB';
    document.getElementById('process-stat-threads').textContent = totalThreads;
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
      ? this.processes.filter(p =>
          p.name.toLowerCase().includes(filter) ||
          p.user.toLowerCase().includes(filter) ||
          p.path.toLowerCase().includes(filter))
      : [...this.processes];

    const col = this.sortColumn;
    const dir = this.sortDirection === 'asc' ? 1 : -1;

    filtered.sort((a, b) => {
      let valA = a[col], valB = b[col];
      if (col === 'memoryBytes') {
        const aUnknown = valA === 0, bUnknown = valB === 0;
        if (aUnknown && !bUnknown) return 1;
        if (!aUnknown && bUnknown) return -1;
        if (aUnknown && bUnknown) return 0;
      }
      if (typeof valA === 'string') return valA.localeCompare(valB) * dir;
      return (valA - valB) * dir;
    });

    filtered.forEach(p => {
      const row = document.createElement('tr');
      if (p.pid === this.selectedPid) row.classList.add('selected-row');
      row.innerHTML = `
        <td>${p.pid}</td>
        <td>${p.name}</td>
        <td class="process-path">${p.user}</td>
        <td>${p.memory}</td>
        <td>${p.threads}</td>
        <td class="process-path">${p.started}</td>
      `;
      row.addEventListener('click', () => this.selectProcess(p.pid));
      tbody.appendChild(row);
    });

    this.updateSortIndicators();
  },

  selectProcess(pid) {
    this.selectedPid = pid;
    const p = this.processes.find(x => x.pid === pid);
    this.render();
    if (!p) return;

    const detail = document.getElementById('process-detail');
    detail.style.display = 'block';
    detail.innerHTML = `
      <div class="process-detail-header">
        <div><strong>${p.name}</strong> <span class="text-muted">pid ${p.pid}</span></div>
        <button class="process-kill-btn" onclick="ProcessTool.terminate(${p.pid}, '${p.name.replace(/'/g, "\\'")}')">Kill</button>
      </div>
      <div class="process-detail-path mono">${p.path}</div>
      <div class="process-detail-meta">
        <span>USER <b>${p.user}</b></span>
        <span>MEM <b>${p.memory}</b></span>
        <span>THREADS <b>${p.threads}</b></span>
        <span>STARTED <b>${p.started}</b></span>
      </div>
    `;
  },

  updateSortIndicators() {
    document.querySelectorAll('#tool-process th[data-sort]').forEach(th => {
      const indicator = th.querySelector('.sort-indicator');
      if (!indicator) return;
      indicator.textContent = th.dataset.sort === this.sortColumn
        ? (this.sortDirection === 'asc' ? ' ▲' : ' ▼') : '';
    });
  },

  terminate(pid, name) {
    if (!confirm(I18n.t('End process "{name}" (PID {pid})?', { name, pid }))) return;
    sendToolRequest('process', 'terminate', String(pid), (response) => {
      if (response.status !== 'ok') { alert(I18n.t('Error: ') + response.message); return; }
      const result = JSON.parse(response.result);
      if (!result.success) { alert(result.message); }
      else { this.refresh(); }
    });
  }
};

document.addEventListener('DOMContentLoaded', () => ProcessTool.checkElevation());
