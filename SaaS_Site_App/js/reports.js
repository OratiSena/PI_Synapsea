(function () {
  "use strict";
  let reports = [];

  function exportReport(report) {
    const payload = {
      ...report,
      disclaimer: "Relatório de apoio educacional. Não substitui equipamento ou avaliação médica profissional."
    };
    const blob = new Blob([JSON.stringify(payload, null, 2)], { type: "application/json" });
    const url = URL.createObjectURL(blob);
    const link = document.createElement("a");
    link.href = url;
    link.download = `synapsea-relatorio-${report.id || "educacional"}.json`;
    link.click();
    URL.revokeObjectURL(url);
  }

  function renderReports() {
    const container = document.getElementById("reports-grid");
    if (!reports.length) {
      showEmptyState(container, "Nenhum relatório disponível.");
      return;
    }
    container.innerHTML = `<div class="reports-grid">${reports.map((report) => `
      <article class="card report-card">
        <span class="report-icon">R</span>
        <h3>${escapeHtml(report.title || "Relatório educacional")}</h3>
        <p>Relatório de apoio educacional. Não apresenta diagnóstico médico.</p>
        <div class="report-meta">
          <span>Tipo: ${escapeHtml(report.type || "Não informado")}</span>
          <span>Paciente: ${escapeHtml(report.patientId || report.patient_id || "--")}</span>
          <span>Criado em: ${escapeHtml(formatDateTime(report.createdAt || report.created_at))}</span>
        </div>
        <div class="report-actions"><button class="btn btn-sm" data-export="${escapeHtml(report.id)}">Exportar JSON</button></div>
      </article>`).join("")}</div>`;
    container.querySelectorAll("[data-export]").forEach((button) => {
      button.addEventListener("click", () => exportReport(reports.find((report) => String(report.id) === button.dataset.export)));
    });
  }

  function toggleModal(open) {
    document.getElementById("report-modal")?.classList.toggle("open", open);
  }

  async function loadReports() {
    const result = await getReports();
    reports = Array.isArray(result) ? result : [];
    renderReports();
  }

  document.addEventListener("DOMContentLoaded", () => {
    showLoading("reports-grid");
    loadReports();
    document.getElementById("open-report-modal")?.addEventListener("click", () => toggleModal(true));
    document.querySelectorAll("[data-close-report-modal]").forEach((button) => button.addEventListener("click", () => toggleModal(false)));
    document.getElementById("report-form")?.addEventListener("submit", async (event) => {
      event.preventDefault();
      const payload = Object.fromEntries(new FormData(event.currentTarget));
      payload.content = { note: payload.note, educationalOnly: true };
      delete payload.note;
      const result = await createReport(payload);
      if (!result) {
        showToast("Relatório não criado. Verifique a conexão com a API.", "error");
        return;
      }
      event.currentTarget.reset();
      toggleModal(false);
      showToast("Solicitação de relatório enviada.");
      loadReports();
    });
  });
})();
