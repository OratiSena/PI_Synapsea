(function () {
  "use strict";
  let patients = [];

  function renderMetadata(metadata) {
    if (!metadata || typeof metadata !== "object" || !Object.keys(metadata).length) {
      return '<div class="metadata-row"><span>Metadados</span><strong>Não informados</strong></div>';
    }
    return Object.entries(metadata).slice(0, 5).map(([key, value]) => `
      <div class="metadata-row"><span>${escapeHtml(key)}</span><strong>${escapeHtml(typeof value === "object" ? JSON.stringify(value) : value)}</strong></div>`).join("");
  }

  function renderDevices(devices) {
    const container = document.getElementById("devices-grid");
    if (!devices.length) return showEmptyState(container, "Nenhum dispositivo disponível para este usuário.");
    container.innerHTML = `<div class="device-grid">${devices.map((device) => `
      <article class="card device-card">
        <div class="device-card-header">
          <span class="device-icon">${escapeHtml(String(device.name || device.type || "D").slice(0, 2).toUpperCase())}</span>
          <span class="badge badge-${getStatusColor(device.status)}">${escapeHtml(getDeviceStatusLabel(device.status))}</span>
        </div>
        <h3>${escapeHtml(device.name || "Dispositivo sem nome")}</h3>
        <div class="device-type">${escapeHtml(device.type || "Tipo não informado")}</div>
        <p class="device-sync">Sincronização: ${escapeHtml(formatRelativeTime(device.lastSync))}</p>
        <div class="assignment-summary">
          <span>Usuário: <strong>${escapeHtml(device.userName || "Não vinculado")}</strong></span>
          <span>Paciente: <strong>${escapeHtml(device.patientName || "Não vinculado")}</strong></span>
        </div>
        <div class="metadata-list">${renderMetadata(device.metadata)}</div>
        ${isAdmin() ? `<button class="btn btn-sm assignment-button" data-device-id="${escapeHtml(device.id)}">Vincular dispositivo</button>` : ""}
      </article>`).join("")}</div>`;
    container.querySelectorAll("[data-device-id]").forEach((button) => {
      button.addEventListener("click", () => {
        document.getElementById("assignment-device").value = button.dataset.deviceId;
        document.getElementById("assignment-modal").classList.add("open");
      });
    });
  }

  document.addEventListener("DOMContentLoaded", async () => {
    showLoading("devices-grid");
    const [devices, loadedPatients] = await Promise.all([getDevices(), getPatients()]);
    patients = Array.isArray(loadedPatients) ? loadedPatients : [];
    const select = document.getElementById("assignment-patient");
    if (select) select.innerHTML = patients.map((patient) =>
      `<option value="${escapeHtml(patient.id)}">${escapeHtml(patient.name)} - ${escapeHtml(patient.userEmail || "sem email")}</option>`
    ).join("");
    renderDevices(Array.isArray(devices) ? devices : []);
    document.querySelectorAll("[data-close-assignment]").forEach((button) =>
      button.addEventListener("click", () => document.getElementById("assignment-modal").classList.remove("open"))
    );
    document.getElementById("assignment-form")?.addEventListener("submit", async (event) => {
      event.preventDefault();
      const payload = Object.fromEntries(new FormData(event.currentTarget));
      const patient = patients.find((item) => item.id === payload.patientId);
      const result = await assignDevice(payload.deviceId, {
        patientId: payload.patientId,
        userId: patient?.userId || null
      });
      if (!result) return showToast("Não foi possível vincular o dispositivo.", "error");
      showToast("Dispositivo vinculado.");
      window.location.reload();
    });
  });
})();
