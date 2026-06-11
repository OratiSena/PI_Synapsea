(function () {
  "use strict";
  const API_URL = "http://localhost:3001/api";

  function setMessage(message, type = "error") {
    const target = document.getElementById("auth-message");
    if (!target) return;
    target.textContent = message;
    target.className = `auth-message ${type}`;
  }
  async function submitAuth(path, payload) {
    const response = await fetch(`${API_URL}${path}`, {
      method: "POST",
      headers: { "Content-Type": "application/json", Accept: "application/json" },
      body: JSON.stringify(payload)
    });
    const result = await response.json().catch(() => ({}));
    if (!response.ok) throw new Error(result.error || "Nao foi possivel concluir a solicitacao.");
    return result.data;
  }
  function bindLogin() {
    const form = document.getElementById("login-form");
    form?.addEventListener("submit", async (event) => {
      event.preventDefault();
      const button = form.querySelector('[type="submit"]');
      button.disabled = true;
      setMessage("Verificando credenciais...", "info");
      try {
        const data = await submitAuth("/auth/login", Object.fromEntries(new FormData(form)));
        saveSession(data.token, data.user);
        window.location.replace("index.html");
      } catch (error) {
        setMessage(error.message);
        button.disabled = false;
      }
    });
  }
  function bindRegister() {
    const form = document.getElementById("register-form");
    const role = document.getElementById("register-role");
    const adminField = document.getElementById("admin-code-field");
    const updateRole = () => adminField?.classList.toggle("hidden", role?.value !== "admin");
    role?.addEventListener("change", updateRole);
    updateRole();
    form?.addEventListener("submit", async (event) => {
      event.preventDefault();
      const payload = Object.fromEntries(new FormData(form));
      if (payload.password !== payload.confirmPassword) {
        setMessage("As senhas informadas nao coincidem.");
        return;
      }
      delete payload.confirmPassword;
      const button = form.querySelector('[type="submit"]');
      button.disabled = true;
      setMessage("Criando conta...", "info");
      try {
        const data = await submitAuth("/auth/register", payload);
        saveSession(data.token, data.user);
        window.location.replace("index.html");
      } catch (error) {
        setMessage(error.message);
        button.disabled = false;
      }
    });
  }
  document.addEventListener("DOMContentLoaded", () => {
    if (isAuthenticated()) return window.location.replace("index.html");
    bindLogin();
    bindRegister();
  });
})();
