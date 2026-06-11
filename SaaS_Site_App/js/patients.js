(function () {
  "use strict";
  let patients = [];

  function initials(name) {
    return String(name || "P").split(/\s+/).slice(0, 2).map((part) => part[0]).join("").toUpperCase();
  }

  function renderPatients() {
    const container = document.getElementById("patients-content");
    if (!patients.length) {
      showEmptyState(container, "Nenhum paciente carregado.");
      return;
    }
    container.innerHTML = `
      <div class="table-wrap"><table class="data-table">
        <thead><tr><th>Nome</th><th>Idade</th><th>Identificação</th><th>Última sessão</th><th>Status</th><th>Ações</th></tr></thead>
        <tbody>${patients.map((patient) => `
          <tr>
            <td><div class="patient-name"><span class="patient-avatar">${escapeHtml(initials(patient.name))}</span><strong>${escapeHtml(patient.name || "Sem nome")}</strong></div></td>
            <td>${escapeHtml(patient.age ?? "--")}</td>
            <td>${escapeHtml(patient.identifier || patient.id || "--")}</td>
            <td>${escapeHtml(formatDateTime(patient.lastSession))}</td>
            <td><span class="badge badge-${getStatusColor(patient.status)}">${escapeHtml(getStatusLabel(patient.status))}</span></td>
            <td><button class="btn btn-sm" type="button" disabled title="Detalhes dependem da API">Detalhes</button></td>
          </tr>`).join("")}</tbody>
      </table></div>`;
  }

  function toggleModal(open) {
    document.getElementById("patient-modal")?.classList.toggle("open", open);
  }

  async function loadPatients() {
    const result = await getPatients();
    patients = Array.isArray(result) ? result : [];
    renderPatients();
  }

  document.addEventListener("DOMContentLoaded", () => {
    showLoading("patients-content");
    loadPatients();
    document.getElementById("open-patient-modal")?.addEventListener("click", () => toggleModal(true));
    document.querySelectorAll("[data-close-patient-modal]").forEach((button) => button.addEventListener("click", () => toggleModal(false)));
    document.getElementById("patient-form")?.addEventListener("submit", async (event) => {
      event.preventDefault();
      const form = event.currentTarget;
      const submit = form.querySelector('[type="submit"]');
      submit.disabled = true;
      const payload = Object.fromEntries(new FormData(form));
      payload.age = safeNumber(payload.age);
      const created = await createPatient(payload);
      submit.disabled = false;
      if (!created) {
        showToast("Cadastro não enviado. A API está offline ou recusou os dados.", "error");
        return;
      }
      form.reset();
      toggleModal(false);
      showToast("Paciente cadastrado.");
      await loadPatients();
    });
  });
})();
