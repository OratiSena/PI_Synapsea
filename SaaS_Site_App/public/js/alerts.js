(function () {
  "use strict";
  let alerts = [];
  let currentFilter = "all";

  function render() {
    const container = document.getElementById("alerts-list");
    const filtered = currentFilter === "all" ? alerts : alerts.filter((alert) => String(alert.type).toLowerCase() === currentFilter);
    if (!filtered.length) {
      showEmptyState(container, currentFilter === "all" ? "Nenhum alerta recebido até o momento." : "Nenhum alerta encontrado neste filtro.");
      return;
    }
    container.innerHTML = `<div class="alerts-list">${filtered.map((alert) => {
      const type = String(alert.type || "info").toLowerCase();
      return `
        <article class="alert-item ${alert.read ? "" : "unread"}">
          <span class="alert-symbol" aria-hidden="true">${type === "critical" ? "!" : type === "warning" ? "△" : "i"}</span>
          <div class="alert-copy">
            <h3>${escapeHtml(alert.title || "Alerta")}</h3>
            <p>${escapeHtml(alert.description || "Sem descrição.")}</p>
            <div class="alert-meta">
              <span class="badge badge-${getStatusColor(type)}">${escapeHtml(getAlertTypeLabel(type))}</span>
              <span>Paciente: ${escapeHtml(alert.patientName || "Não informado")}</span>
              <span>Dispositivo: ${escapeHtml(alert.deviceName || "Não informado")}</span>
              <span>${escapeHtml(formatDateTime(alert.timestamp))}</span>
              <span>${alert.read ? "Lido" : "Não lido"}</span>
            </div>
          </div>
          <div class="alert-actions">
            ${alert.read ? "" : `<button class="btn btn-sm" data-read-id="${escapeHtml(alert.id)}">Marcar como lido</button>`}
          </div>
        </article>`;
    }).join("")}</div>`;

    container.querySelectorAll("[data-read-id]").forEach((button) => {
      button.addEventListener("click", async () => {
        button.disabled = true;
        const result = await updateAlertReadStatus(button.dataset.readId);
        if (!result) {
          button.disabled = false;
          showToast("Não foi possível atualizar o alerta. Verifique a API.", "error");
          return;
        }
        alerts = alerts.map((alert) => String(alert.id) === button.dataset.readId ? { ...alert, read: true } : alert);
        render();
      });
    });
  }

  async function loadAlerts() {
    const result = await getAlerts();
    alerts = Array.isArray(result) ? result : [];
    render();
  }

  document.addEventListener("DOMContentLoaded", () => {
    document.querySelectorAll("[data-filter]").forEach((button) => {
      button.addEventListener("click", () => {
        currentFilter = button.dataset.filter;
        document.querySelectorAll("[data-filter]").forEach((item) => item.classList.toggle("active", item === button));
        render();
      });
    });
    showLoading("alerts-list");
    loadAlerts();
  });
})();
