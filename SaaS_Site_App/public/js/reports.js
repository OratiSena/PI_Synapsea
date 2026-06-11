(function () {
  "use strict";
  let reports = [];

  function exportReport(report) {
    const blob = new Blob([JSON.stringify(report, null, 2)], { type: "application/json" });
    const url = URL.createObjectURL(blob);
    const link = document.createElement("a");
    link.href = url;
    link.download = `synapsea-relatorio-${report.id || "educacional"}.json`;
    link.click();
    URL.revokeObjectURL(url);
  }

  function metricsHtml(content) {
    const metrics = content?.metrics || {};
    return `<div class="report-metrics">
      <span>BPM médio <strong>${escapeHtml(formatValue(metrics.avgBpm, 1))}</strong></span>
      <span>SpO₂ média <strong>${escapeHtml(formatValue(metrics.avgSpo2, 1))}</strong></span>
      <span>Temperatura média <strong>${escapeHtml(formatValue(metrics.avgTemperature, 1))}</strong></span>
      <span>Alertas <strong>${escapeHtml(metrics.alertsCount ?? "--")}</strong></span>
    </div>`;
  }

  function renderReports() {
    const container = document.getElementById("reports-grid");
    if (!reports.length) return showEmptyState(container, "Nenhum relatório disponível.");
    container.innerHTML = `<div class="reports-grid">${reports.map((report) => `
      <article class="card report-card">
        <span class="report-icon">R</span>
        <h3>${escapeHtml(report.title || "Relatório educacional")}</h3>
        <p>${escapeHtml(report.content?.summary || "Relatório de apoio educacional.")}</p>
        ${metricsHtml(report.content)}
        <div class="report-meta">
          <span>Paciente: ${escapeHtml(report.patientName || report.patientId || "--")}</span>
          <span>Criado em: ${escapeHtml(formatDateTime(report.createdAt))}</span>
        </div>
        <div class="report-actions"><button class="btn btn-sm" data-export="${escapeHtml(report.id)}">Exportar JSON</button></div>
      </article>`).join("")}</div>`;
    container.querySelectorAll("[data-export]").forEach((button) => {
      button.addEventListener("click", () => exportReport(reports.find((report) => String(report.id) === button.dataset.export)));
    });
  }

  async function loadReports() {
    const result = await getReports();
    reports = Array.isArray(result) ? result : [];
    renderReports();
  }

  document.addEventListener("DOMContentLoaded", async () => {
    showLoading("reports-grid");
    const patients = await getPatients();
    const select = document.getElementById("report-patient");
    if (select) select.innerHTML = (Array.isArray(patients) ? patients : []).map((patient) =>
      `<option value="${escapeHtml(patient.id)}">${escapeHtml(patient.name)}</option>`
    ).join("");
    loadReports();
    document.getElementById("open-report-modal")?.addEventListener("click", () => document.getElementById("report-modal").classList.add("open"));
    document.querySelectorAll("[data-close-report-modal]").forEach((button) => button.addEventListener("click", () => document.getElementById("report-modal").classList.remove("open")));
    document.getElementById("report-form")?.addEventListener("submit", async (event) => {
      event.preventDefault();
      const result = await generateReport(Object.fromEntries(new FormData(event.currentTarget)));
      if (!result) return showToast("Relatório não gerado. Verifique os dados e a API.", "error");
      document.getElementById("report-modal").classList.remove("open");
      showToast("Relatório educacional gerado.");
      loadReports();
    });
  });
})();
