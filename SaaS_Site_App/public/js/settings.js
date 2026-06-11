(function () {
  "use strict";
  const STORAGE_KEY = "synapsea.settings";

  function getSettings() {
    try {
      return JSON.parse(localStorage.getItem(STORAGE_KEY) || "{}");
    } catch {
      return {};
    }
  }

  function updatePreview(settings) {
    setText("preview-api", API_CONFIG.baseUrl);
    setText("preview-interval", `${settings.refreshInterval || API_CONFIG.refreshInterval} ms`);
    setText("preview-device", settings.deviceId || "Não definido");
    setText("preview-mode", settings.connectionMode || "REST");
  }

  document.addEventListener("DOMContentLoaded", () => {
    const form = document.getElementById("settings-form");
    const stored = getSettings();
    form.baseUrl.value = API_CONFIG.baseUrl;
    form.baseUrl.readOnly = true;
    form.refreshInterval.value = stored.refreshInterval || API_CONFIG.refreshInterval;
    form.deviceId.value = stored.deviceId || "";
    form.connectionMode.value = stored.connectionMode || "rest";
    form.compactMode.checked = Boolean(stored.compactMode);
    form.reducedMotion.checked = Boolean(stored.reducedMotion);
    updatePreview(stored);

    form.addEventListener("submit", async (event) => {
      event.preventDefault();
      const values = Object.fromEntries(new FormData(form));
      const settings = {
        baseUrl: API_CONFIG.baseUrl,
        refreshInterval: Math.max(1000, Number(values.refreshInterval) || 5000),
        deviceId: String(values.deviceId || "").trim(),
        connectionMode: values.connectionMode || "rest",
        compactMode: form.compactMode.checked,
        reducedMotion: form.reducedMotion.checked
      };
      localStorage.setItem(STORAGE_KEY, JSON.stringify(settings));
      updatePreview(settings);
      showToast("Configurações salvas neste navegador. Recarregue para aplicar a URL e o intervalo.");
      await updateSettings(settings);
    });

    document.getElementById("reset-settings")?.addEventListener("click", () => {
      localStorage.removeItem(STORAGE_KEY);
      location.reload();
    });

    document.getElementById("test-api")?.addEventListener("click", async (event) => {
      const button = event.currentTarget;
      button.disabled = true;
      const status = await getSystemStatus();
      button.disabled = false;
      showToast(status ? "API respondeu com sucesso." : "API não respondeu. Confira a URL e o backend.", status ? "info" : "error");
    });
  });
})();
