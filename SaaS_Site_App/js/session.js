(function () {
  "use strict";

  const TOKEN_KEY = "synapsea.auth.token";
  const USER_KEY = "synapsea.auth.user";
  const isInsidePages = () => window.location.pathname.replaceAll("\\", "/").includes("/pages/");
  const loginUrl = () => isInsidePages() ? "../login.html" : "login.html";

  function saveSession(token, user) {
    localStorage.setItem(TOKEN_KEY, token);
    localStorage.setItem(USER_KEY, JSON.stringify(user));
  }
  function getToken() { return localStorage.getItem(TOKEN_KEY); }
  function getCurrentUser() {
    try { return JSON.parse(localStorage.getItem(USER_KEY) || "null"); }
    catch (_error) { return null; }
  }
  function isAuthenticated() { return Boolean(getToken() && getCurrentUser()); }
  function isAdmin() { return getCurrentUser()?.role === "admin"; }
  function logout() {
    localStorage.removeItem(TOKEN_KEY);
    localStorage.removeItem(USER_KEY);
    window.location.href = loginUrl();
  }
  function requireAuth() {
    if (isAuthenticated()) return true;
    window.location.replace(loginUrl());
    return false;
  }
  function requireAdmin() {
    if (!requireAuth()) return false;
    if (isAdmin()) return true;
    window.location.replace(isInsidePages() ? "../index.html" : "index.html");
    return false;
  }
  async function authFetch(url, options = {}) {
    const token = getToken();
    const response = await fetch(url, {
      ...options,
      headers: {
        ...(options.headers || {}),
        ...(token ? { Authorization: `Bearer ${token}` } : {})
      }
    });
    if (response.status === 401 && getToken()) {
      localStorage.removeItem(TOKEN_KEY);
      localStorage.removeItem(USER_KEY);
      window.location.replace(loginUrl());
    }
    return response;
  }
  function applyPermissions() {
    document.querySelectorAll("[data-admin-only]").forEach((element) => {
      element.classList.toggle("hidden", !isAdmin());
    });
    document.querySelectorAll("[data-user-only]").forEach((element) => {
      element.classList.toggle("hidden", isAdmin());
    });
  }

  window.SynapseaSession = {
    saveSession, getToken, getCurrentUser, isAuthenticated, isAdmin,
    logout, requireAuth, requireAdmin, authFetch, applyPermissions
  };
  Object.assign(window, window.SynapseaSession);
  if (document.body?.dataset.authRequired !== "false") requireAuth();
  document.addEventListener("DOMContentLoaded", applyPermissions);
})();
