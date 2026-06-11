(function () {
  "use strict";

  const metrics = [
    { key: "bpm", valueId: "metric-bpm", statusId: "status-bpm", decimals: 0 },
    { key: "spo2", valueId: "metric-spo2", statusId: "status-spo2", decimals: 0 },
    { key: "temperature", valueId: "metric-temperature", statusId: "status-temperature", decimals: 1 },
    { key: "stressIndex", valueId: "metric-stress", statusId: "status-stress", decimals: 0 },
    { key: "respiration", valueId: "metric-respiration", statusId: "status-respiration", decimals: 0 }
  ];

  function resetMetrics() {
    metrics.forEach((metric) => {
      setText(metric.valueId, "--");
      setText(metric.statusId, "Aguardando leitura");
    });
    setText("metric-general", "--");
    setText("status-general", "Aguardando dados");
  }

  function renderVitals(vitals) {
    resetMetrics();
    if (!vitals) return;
    metrics.forEach((metric) => {
      setText(metric.valueId, formatValue(vitals[metric.key], metric.decimals));
      setText(metric.statusId, vitals[metric.key] === null || vitals[metric.key] === undefined ? "Sem leitura" : "Leitura recebida");
    });
    const status = calculateGeneralStatus(vitals);
    setText("metric-general", getStatusLabel(status));
    setText("status-general", status === "unknown" ? "Aguardando dados" : "Indicador estimado");
  }

  function renderAlerts(alerts) {
    const container = document.getElementById("dashboard-alerts");
    if (!container) return;
    if (!alerts.length) {
      showEmptyState(container, "Nenhum alerta recebido até o momento.");
      return;
    }
    container.innerHTML = `<div class="compact-list">${alerts.slice(0, 4).map((alert) => `
      <div class="compact-item">
        <div class="compact-item-main">
          <strong>${escapeHtml(alert.title || "Alerta")}</strong>
          <span>${escapeHtml(formatRelativeTime(alert.timestamp))}</span>
        </div>
        <span class="badge badge-${getStatusColor(alert.type)}">${escapeHtml(getAlertTypeLabel(alert.type))}</span>
      </div>`).join("")}</div>`;
  }

  function renderDevices(devices) {
    const container = document.getElementById("dashboard-devices");
    if (!container) return;
    if (!devices.length) {
      showEmptyState(container, "Nenhum dispositivo informado pela API.");
      return;
    }
    container.innerHTML = `<div class="compact-list">${devices.slice(0, 5).map((device) => `
      <div class="compact-item">
        <div class="compact-item-main">
          <strong>${escapeHtml(device.name || device.type || "Dispositivo")}</strong>
          <span>${escapeHtml(formatRelativeTime(device.lastSync))}</span>
        </div>
        <span class="badge badge-${getStatusColor(device.status)}">${escapeHtml(getDeviceStatusLabel(device.status))}</span>
      </div>`).join("")}</div>`;
  }

  function renderInsight(insight) {
    const container = document.getElementById("dashboard-insight");
    const badge = document.getElementById("insight-level");
    if (!container || !badge || !insight) return;
    const color = getStatusColor(insight.level);
    badge.className = `badge badge-${color}`;
    badge.textContent = getStatusLabel(insight.level);
    container.innerHTML = `
      <div class="insight-content">
        <span class="insight-mark" aria-hidden="true">IA</span>
        <div>
          <h3>${escapeHtml(insight.title || "Análise educacional")}</h3>
          <p>${escapeHtml(insight.message || "Sem observações no momento.")}</p>
          <strong>${escapeHtml(insight.recommendation || "")}</strong>
          <small>${escapeHtml(insight.disclaimer || "")}</small>
        </div>
      </div>`;
  }

  async function loadDashboard() {
    const message = document.getElementById("dashboard-api-message");
    const [vitals, history, alerts, devices, insight] = await Promise.all([
      getLatestVitals(),
      getVitalsHistory(),
      getAlerts(),
      getDevices(),
      getLatestInsight()
    ]);

    renderVitals(vitals);
    renderVitalsChart("vitals-chart", history);
    renderAlerts(Array.isArray(alerts) ? alerts : []);
    renderDevices(Array.isArray(devices) ? devices : []);
    renderInsight(insight);

    const hasData = Boolean(vitals) || (history?.length || alerts?.length || devices?.length);
    if (message) {
      message.classList.toggle("hidden", hasData);
      message.querySelector("span").textContent = SynapseaAPI.isOnline === false
        ? "Sistema aguardando conexão com a API Node.js/Express do Synapsea para exibir dados reais dos sensores."
        : "Nenhuma leitura recebida ainda. Conecte o circuito Synapsea ou configure a API para iniciar o monitoramento.";
    }
  }

  document.addEventListener("DOMContentLoaded", () => {
    resetMetrics();
    loadDashboard();
    window.setInterval(loadDashboard, API_CONFIG.refreshInterval);
    window.addEventListener("resize", () => {
      window.clearTimeout(window.__synapseaChartResize);
      window.__synapseaChartResize = window.setTimeout(() => getVitalsHistory().then((data) => renderVitalsChart("vitals-chart", data)), 180);
    });
  });
})();
