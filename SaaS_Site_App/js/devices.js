(function () {
  "use strict";

  function renderMetadata(metadata) {
    if (!metadata || typeof metadata !== "object" || !Object.keys(metadata).length) {
      return '<div class="metadata-row"><span>Metadados</span><strong>Não informados</strong></div>';
    }
    return Object.entries(metadata).slice(0, 5).map(([key, value]) => `
      <div class="metadata-row"><span>${escapeHtml(key)}</span><strong>${escapeHtml(typeof value === "object" ? JSON.stringify(value) : value)}</strong></div>`).join("");
  }

  function renderDevices(devices) {
    const container = document.getElementById("devices-grid");
    if (!devices.length) {
      showEmptyState(container, "Nenhum dispositivo recebido. ESP32, sensores, API e PostgreSQL aparecerão quando forem cadastrados no backend.");
      return;
    }
    container.innerHTML = `<div class="device-grid">${devices.map((device) => `
      <article class="card device-card">
        <div class="device-card-header">
          <span class="device-icon">${escapeHtml(String(device.name || device.type || "D").slice(0, 2).toUpperCase())}</span>
          <span class="badge badge-${getStatusColor(device.status)}">${escapeHtml(getDeviceStatusLabel(device.status))}</span>
        </div>
        <h3>${escapeHtml(device.name || "Dispositivo sem nome")}</h3>
        <div class="device-type">${escapeHtml(device.type || "Tipo não informado")}</div>
        <p class="device-sync">Sincronização: ${escapeHtml(formatRelativeTime(device.lastSync))}</p>
        <div class="metadata-list">${renderMetadata(device.metadata)}</div>
      </article>`).join("")}</div>`;
  }

  document.addEventListener("DOMContentLoaded", async () => {
    showLoading("devices-grid");
    const devices = await getDevices();
    renderDevices(Array.isArray(devices) ? devices : []);
  });
})();
