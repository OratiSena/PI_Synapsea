(function () {
  "use strict";

  const locale = "pt-BR";

  function safeNumber(value, fallback = null) {
    const number = Number(value);
    return Number.isFinite(number) ? number : fallback;
  }

  function formatDateTime(value) {
    if (!value) return "--";
    const date = new Date(value);
    if (Number.isNaN(date.getTime())) return "--";
    return new Intl.DateTimeFormat(locale, {
      dateStyle: "short",
      timeStyle: "short"
    }).format(date);
  }

  function formatRelativeTime(value) {
    if (!value) return "Sem sincronização";
    const date = new Date(value);
    if (Number.isNaN(date.getTime())) return "Data inválida";
    const seconds = Math.round((date.getTime() - Date.now()) / 1000);
    const divisions = [
      { amount: 60, unit: "second" },
      { amount: 60, unit: "minute" },
      { amount: 24, unit: "hour" },
      { amount: 7, unit: "day" },
      { amount: 4.345, unit: "week" },
      { amount: 12, unit: "month" },
      { amount: Infinity, unit: "year" }
    ];
    let duration = seconds;
    for (const division of divisions) {
      if (Math.abs(duration) < division.amount) {
        return new Intl.RelativeTimeFormat(locale, { numeric: "auto" })
          .format(Math.round(duration), division.unit);
      }
      duration /= division.amount;
    }
    return formatDateTime(value);
  }

  function formatValue(value, decimals = 0, fallback = "--") {
    const number = safeNumber(value);
    if (number === null) return fallback;
    return number.toLocaleString(locale, {
      minimumFractionDigits: decimals,
      maximumFractionDigits: decimals
    });
  }

  function formatTemperature(value) {
    const formatted = formatValue(value, 1);
    return formatted === "--" ? formatted : `${formatted} °C`;
  }

  function formatPercentage(value) {
    const formatted = formatValue(value, 0);
    return formatted === "--" ? formatted : `${formatted}%`;
  }

  function getStatusLabel(status) {
    const labels = {
      normal: "Normal",
      stable: "Estável",
      warning: "Atenção",
      critical: "Crítico",
      error: "Erro",
      unknown: "Aguardando dados",
      connected: "Conectado",
      waiting: "Aguardando",
      offline: "Offline",
      active: "Ativo",
      inactive: "Inativo"
    };
    return labels[String(status || "unknown").toLowerCase()] || String(status || "Aguardando");
  }

  function getStatusColor(status) {
    const value = String(status || "").toLowerCase();
    if (["normal", "stable", "connected", "active", "success"].includes(value)) return "success";
    if (["warning", "waiting", "pending"].includes(value)) return "warning";
    if (["critical", "error", "offline", "inactive"].includes(value)) return "danger";
    return "info";
  }

  function getAlertTypeLabel(type) {
    const labels = {
      info: "Informativo",
      success: "Sucesso",
      warning: "Aviso",
      critical: "Crítico"
    };
    return labels[String(type || "info").toLowerCase()] || "Informativo";
  }

  function getDeviceStatusLabel(status) {
    return getStatusLabel(status);
  }

  function createElement(tag, options = {}, children = []) {
    const element = document.createElement(tag);
    Object.entries(options).forEach(([key, value]) => {
      if (key === "className") element.className = value;
      else if (key === "text") element.textContent = value;
      else if (key === "html") element.innerHTML = value;
      else if (key.startsWith("on") && typeof value === "function") {
        element.addEventListener(key.slice(2).toLowerCase(), value);
      } else if (value !== null && value !== undefined) {
        element.setAttribute(key, value);
      }
    });
    const items = Array.isArray(children) ? children : [children];
    items.filter(Boolean).forEach((child) => {
      element.append(child instanceof Node ? child : document.createTextNode(String(child)));
    });
    return element;
  }

  function renderState(target, type, title, message) {
    const container = typeof target === "string" ? document.getElementById(target) : target;
    if (!container) return;
    const icons = { loading: "", empty: "—", error: "!" };
    container.innerHTML = `
      <div class="${type}-state">
        <div class="state-content">
          ${type === "loading" ? '<div class="spinner" aria-hidden="true"></div>' : `<div class="state-icon" aria-hidden="true">${icons[type]}</div>`}
          <strong class="state-title">${escapeHtml(title)}</strong>
          <p class="state-message">${escapeHtml(message)}</p>
        </div>
      </div>`;
  }

  function showLoading(target, message = "Buscando dados reais...") {
    renderState(target, "loading", "Carregando", message);
  }

  function showEmptyState(target, message = "Nenhum dado disponível.") {
    renderState(target, "empty", "Aguardando dados", message);
  }

  function showErrorState(target, message = "Não foi possível carregar os dados.") {
    renderState(target, "error", "API indisponível", message);
  }

  function setText(target, value, fallback = "--") {
    const element = typeof target === "string" ? document.getElementById(target) : target;
    if (element) element.textContent = value === null || value === undefined || value === "" ? fallback : value;
  }

  function calculateGeneralStatus(vitals) {
    if (!vitals || typeof vitals !== "object") return "unknown";
    if (vitals.status) return String(vitals.status).toLowerCase();
    const bpm = safeNumber(vitals.bpm);
    const spo2 = safeNumber(vitals.spo2);
    const temperature = safeNumber(vitals.temperature);
    if (bpm === null && spo2 === null && temperature === null) return "unknown";
    if ((spo2 !== null && spo2 < 90) || (bpm !== null && (bpm < 45 || bpm > 130))) return "critical";
    if ((spo2 !== null && spo2 < 95) || (temperature !== null && temperature > 37.8)) return "warning";
    return "normal";
  }

  function escapeHtml(value) {
    return String(value ?? "")
      .replaceAll("&", "&amp;")
      .replaceAll("<", "&lt;")
      .replaceAll(">", "&gt;")
      .replaceAll('"', "&quot;")
      .replaceAll("'", "&#039;");
  }

  function showToast(message, type = "info") {
    let region = document.querySelector(".toast-region");
    if (!region) {
      region = createElement("div", { className: "toast-region", "aria-live": "polite" });
      document.body.append(region);
    }
    const toast = createElement("div", { className: `toast ${type}`, text: message });
    region.append(toast);
    window.setTimeout(() => toast.remove(), 4200);
  }

  window.SynapseaUtils = {
    formatDateTime,
    formatRelativeTime,
    formatValue,
    getStatusLabel,
    getStatusColor,
    getAlertTypeLabel,
    getDeviceStatusLabel,
    createElement,
    showLoading,
    showEmptyState,
    showErrorState,
    setText,
    safeNumber,
    calculateGeneralStatus,
    formatTemperature,
    formatPercentage,
    escapeHtml,
    showToast
  };

  Object.assign(window, window.SynapseaUtils);
})();
