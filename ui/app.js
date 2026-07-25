function updateTabbar(item) {
  const icon = item.querySelector('.sidebar-item-icon').textContent;
  const label = item.querySelector('.sidebar-item-text').textContent;
  const group = item.closest('.sidebar-group').dataset.groupName;

  document.getElementById('tabbar-icon').textContent = icon;
  document.getElementById('tabbar-label').textContent = label;
  document.getElementById('tabbar-group').textContent = group;
}

document.querySelectorAll('.sidebar-item').forEach(item => {
  item.addEventListener('click', () => {
    document.querySelectorAll('.sidebar-item').forEach(i => i.classList.remove('active'));
    document.querySelectorAll('.tool-panel').forEach(p => p.classList.remove('active'));

    item.classList.add('active');
    const toolId = item.dataset.tool;
    document.getElementById('tool-' + toolId).classList.add('active');

    updateTabbar(item);
  });
});

document.getElementById('sidebar-search').addEventListener('input', (e) => {
  const term = e.target.value.toLowerCase();
  document.querySelectorAll('.sidebar-group').forEach(group => {
    let anyVisible = false;
    group.querySelectorAll('.sidebar-item').forEach(item => {
      const text = item.querySelector('.sidebar-item-text').textContent.toLowerCase();
      const match = text.includes(term);
      item.style.display = match ? '' : 'none';
      if (match) anyVisible = true;
    });
    group.style.display = anyVisible ? '' : 'none';
  });
});

document.addEventListener('DOMContentLoaded', () => {
  const activeItem = document.querySelector('.sidebar-item.active');
  if (activeItem) updateTabbar(activeItem);
});