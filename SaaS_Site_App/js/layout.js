(function () {
  "use strict";

  const icons = {
    overview: "⌂",
    patients: "P",
    monitoring: "∿",
    history: "H",
    alerts: "!",
    reports: "R",
    devices: "D",
    settings: "⚙"
  };

  const menuItems = [
    ["overview", "Visão Geral", "index.html"],
    ["patients", "Pacientes", "patients.html"],
    ["monitoring", "Monitoramento", "monitoring.html"],
    ["history", "Histórico", "history.html"],
    ["alerts", "Alertas", "alerts.html"],
    ["reports", "Relatórios", "reports.html"],
    ["devices", "Dispositivos", "devices.html"],
    ["settings", "Configurações", "settings.html"]
  ];

  function isInsidePages() {
    return window.location.pathname.replaceAll("\\", "/").includes("/pages/");
  }

  function resolveHref(page) {
    if (page === "index.html") return isInsidePages() ? "../index.html" : "index.html";
    return isInsidePages() ? page : `pages/${page}`;
  }

  function buildSidebar(activePage) {
    const nav = menuItems.map(([key, label, page]) => `
      <a class="nav-link ${key === activePage ? "active" : ""}" href="${resolveHref(page)}" ${key === activePage ? 'aria-current="page"' : ""}>
        <span class="nav-icon" aria-hidden="true">${icons[key]}</span>
        <span>${label}</span>
      </a>`).join("");

    return `
      <aside class="sidebar" id="sidebar">
        <a class="brand" href="${resolveHref("index.html")}">
          <span class="brand-mark" aria-hidden="true">S</span>
          <span class="brand-copy"><strong>Synapsea</strong><span>Health Tech Platform</span></span>
        </a>
        <div class="nav-label">Plataforma</div>
        <nav class="sidebar-nav" aria-label="Navegação principal">${nav}</nav>
        <div class="sidebar-footer">
          <div class="about-card">
            <strong>Monitoramento educacional</strong>
            <p>Apoio à observação com ESP32, MAX30102 e AMG8833. Não substitui equipamento médico profissional.</p>
          </div>
        </div>
      </aside>
      <button class="sidebar-overlay" id="sidebar-overlay" aria-label="Fechar menu"></button>`;
  }

  function buildHeader(title) {
    return `
      <header class="topbar">
        <div class="topbar-actions">
          <button class="menu-toggle" id="menu-toggle" type="button" aria-label="Abrir menu" aria-expanded="false">☰</button>
          <div class="topbar-title">
            <strong>${escapeHtml(title || "Synapsea")}</strong>
            <span>Dados reais do circuito e da API</span>
          </div>
        </div>
        <div class="topbar-actions">
          <div class="api-status" id="api-status" title="Estado da API">
            <span class="status-dot"></span><span class="api-label">Verificando API</span>
          </div>
          <time class="clock" id="header-clock"></time>
          <div class="user-chip">
            <span class="avatar">SY</span>
            <span class="user-copy"><strong>Synapsea</strong><span>Projeto acadêmico</span></span>
          </div>
        </div>
      </header>`;
  }

  function updateClock() {
    const clock = document.getElementById("header-clock");
    if (!clock) return;
    const now = new Date();
    clock.dateTime = now.toISOString();
    clock.textContent = new Intl.DateTimeFormat("pt-BR", {
      day: "2-digit",
      month: "short",
      hour: "2-digit",
      minute: "2-digit"
    }).format(now);
  }

  function updateApiIndicator(online) {
    const indicator = document.getElementById("api-status");
    if (!indicator) return;
    indicator.classList.remove("online", "offline");
    const label = indicator.querySelector(".api-label");
    if (online === true) {
      indicator.classList.add("online");
      if (label) label.textContent = "API conectada";
    } else if (online === false) {
      indicator.classList.add("offline");
      if (label) label.textContent = "API offline";
    } else if (label) {
      label.textContent = "Verificando API";
    }
  }

  function initLayout() {
    const shell = document.querySelector(".app-shell");
    if (!shell) return;
    const activePage = document.body.dataset.page || "overview";
    const title = document.body.dataset.title || document.title;
    shell.insertAdjacentHTML("afterbegin", buildSidebar(activePage) + buildHeader(title));

    const toggle = document.getElementById("menu-toggle");
    const overlay = document.getElementById("sidebar-overlay");
    const closeMenu = () => {
      document.body.classList.remove("menu-open");
      toggle?.setAttribute("aria-expanded", "false");
    };
    toggle?.addEventListener("click", () => {
      const isOpen = document.body.classList.toggle("menu-open");
      toggle.setAttribute("aria-expanded", String(isOpen));
    });
    overlay?.addEventListener("click", closeMenu);
    document.querySelectorAll(".nav-link").forEach((link) => link.addEventListener("click", closeMenu));
    window.addEventListener("keydown", (event) => {
      if (event.key === "Escape") closeMenu();
    });

    updateClock();
    window.setInterval(updateClock, 30000);
    window.addEventListener("synapsea:api-status", (event) => updateApiIndicator(event.detail.online));

    if (window.getSystemStatus) {
      window.getSystemStatus().then((status) => updateApiIndicator(status ? true : window.SynapseaAPI?.isOnline));
    }
  }

  document.addEventListener("DOMContentLoaded", initLayout);
})();
