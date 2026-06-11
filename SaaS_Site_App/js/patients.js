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
        <thead><tr><th>Nome</th><th>Conta</th><th>Dispositivo</th><th>Última sessão</th><th>Status</th></tr></thead>
        <tbody>${patients.map((patient) => `
          <tr>
            <td><div class="patient-name"><span class="patient-avatar">${escapeHtml(initials(patient.name))}</span><strong>${escapeHtml(patient.name || "Sem nome")}</strong></div></td>
            <td>${escapeHtml(patient.userEmail || "Conta atual")}</td>
            <td>${escapeHtml(patient.devices || "Não vinculado")}</td>
            <td>${escapeHtml(formatDateTime(patient.lastSession))}</td>
            <td><span class="badge badge-${getStatusColor(patient.status)}">${escapeHtml(getStatusLabel(patient.status))}</span></td>
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

  async function loadUsers() {
    if (!isAdmin()) return;
    const users = await getUsers();
    const select = document.getElementById("patient-user");
    if (select && Array.isArray(users)) {
      select.innerHTML = users.map((user) =>
        `<option value="${escapeHtml(user.id)}">${escapeHtml(user.name)} - ${escapeHtml(user.email)}</option>`
      ).join("");
    }
    const container = document.getElementById("users-content");
    if (!container || !Array.isArray(users) || !users.length) {
      if (container) showEmptyState(container, "Nenhuma conta cadastrada.");
      return;
    }
    container.innerHTML = `<div class="table-wrap"><table class="data-table">
      <thead><tr><th>Nome</th><th>Email</th><th>Permissão</th><th>Status</th></tr></thead>
      <tbody>${users.map((user) => `<tr>
        <td><strong>${escapeHtml(user.name)}</strong></td>
        <td>${escapeHtml(user.email)}</td>
        <td>${escapeHtml(user.role === "admin" ? "Administrador" : "Usuário")}</td>
        <td><span class="badge badge-${getStatusColor(user.status)}">${escapeHtml(getStatusLabel(user.status))}</span></td>
      </tr>`).join("")}</tbody>
    </table></div>`;
  }

  document.addEventListener("DOMContentLoaded", () => {
    if (!isAdmin()) {
      setText("patients-list-title", "Meu perfil monitorado");
      setText("patients-scope-copy", "Dados associados exclusivamente à sua conta.");
    }
    showLoading("patients-content");
    loadPatients();
    loadUsers();
    document.getElementById("patient-search")?.addEventListener("input", (event) => {
      const term = event.target.value.trim().toLowerCase();
      document.querySelectorAll("#patients-content tbody tr").forEach((row) => {
        row.classList.toggle("hidden", !row.textContent.toLowerCase().includes(term));
      });
    });
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
