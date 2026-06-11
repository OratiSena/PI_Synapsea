(function () {
  "use strict";

  const STORAGE_KEY = "synapsea.settings";
  const defaults = {
    baseUrl: "http://localhost:3001/api",
    wsUrl: "ws://localhost:3001",
    refreshInterval: 5000,
    demoMode: false
  };

  function loadStoredSettings() {
    try {
      return JSON.parse(localStorage.getItem(STORAGE_KEY) || "{}");
    } catch (error) {
      console.warn("Configurações locais inválidas:", error);
      return {};
    }
  }

  const stored = loadStoredSettings();
  const API_CONFIG = {
    ...defaults,
    baseUrl: stored.baseUrl || defaults.baseUrl,
    wsUrl: stored.wsUrl || defaults.wsUrl,
    refreshInterval: Math.max(1000, Number(stored.refreshInterval) || defaults.refreshInterval)
  };

  let apiOnline = null;
  let realtimeSocket = null;

  function setApiStatus(online, detail = "") {
    apiOnline = online;
    window.dispatchEvent(new CustomEvent("synapsea:api-status", {
      detail: { online, message: detail }
    }));
  }

  async function request(path, options = {}, fallback = null) {
    const controller = new AbortController();
    const timeout = window.setTimeout(() => controller.abort(), 4500);
    try {
      const requestFn = window.authFetch || window.fetch.bind(window);
      const response = await requestFn(`${API_CONFIG.baseUrl}${path}`, {
        ...options,
        headers: {
          Accept: "application/json",
          ...(options.body ? { "Content-Type": "application/json" } : {}),
          ...(options.headers || {})
        },
        signal: controller.signal
      });
      if (!response.ok) throw new Error(`HTTP ${response.status}`);
      const payload = response.status === 204 ? null : await response.json();
      setApiStatus(true);
      return payload && Object.prototype.hasOwnProperty.call(payload, "data") ? payload.data : payload;
    } catch (error) {
      const reason = error.name === "AbortError" ? "Tempo limite excedido" : error.message;
      console.warn(`API Synapsea indisponível em ${path}: ${reason}`);
      setApiStatus(false, reason);
      return fallback;
    } finally {
      window.clearTimeout(timeout);
    }
  }

  const getLatestVitals = () => request("/vitals/latest", {}, null);
  const getVitalsHistory = (query = "") => request(`/vitals/history${query}`, {}, []);
  const getLatestTemperature = () => request("/temperature/latest", {}, null);
  const getTemperatureHistory = (query = "") => request(`/temperature/history${query}`, {}, []);
  const getDevices = () => request("/devices", {}, []);
  const getAlerts = () => request("/alerts", {}, []);
  const getPatients = () => request("/patients", {}, []);
  const getReports = () => request("/reports", {}, []);
  const getSystemStatus = () => request("/system/status", {}, null);
  const getLatestInsight = () => request("/insights/latest", {}, null);
  const getDeviceAssignments = () => request("/devices/assignments", {}, []);
  const getUsers = () => request("/auth/users", {}, []);

  const updateAlertReadStatus = (alertId) => request(`/alerts/${encodeURIComponent(alertId)}/read`, {
    method: "PATCH",
    body: JSON.stringify({ read: true })
  }, null);

  const createPatient = (patientData) => request("/patients", {
    method: "POST",
    body: JSON.stringify(patientData)
  }, null);

  const createReport = (reportData) => request("/reports", {
    method: "POST",
    body: JSON.stringify(reportData)
  }, null);

  const generateReport = (reportData) => request("/reports/generate", {
    method: "POST",
    body: JSON.stringify(reportData)
  }, null);

  const assignDevice = (deviceId, assignment) => request(`/devices/${encodeURIComponent(deviceId)}/assign`, {
    method: "POST",
    body: JSON.stringify(assignment)
  }, null);

  const updateSettings = (settings) => request("/settings", {
    method: "PATCH",
    body: JSON.stringify(settings)
  }, null);

  function connectRealtime() {
    if (realtimeSocket && realtimeSocket.readyState <= WebSocket.OPEN) return realtimeSocket;
    try {
      realtimeSocket = new WebSocket(API_CONFIG.wsUrl);
      realtimeSocket.addEventListener("open", () => setApiStatus(true));
      realtimeSocket.addEventListener("message", (event) => {
        try {
          handleRealtimeMessage(JSON.parse(event.data));
        } catch (error) {
          console.warn("Mensagem WebSocket inválida:", error);
        }
      });
      realtimeSocket.addEventListener("close", () => {
        realtimeSocket = null;
      });
      realtimeSocket.addEventListener("error", () => setApiStatus(false, "WebSocket indisponível"));
      return realtimeSocket;
    } catch (error) {
      console.warn("Não foi possível iniciar o WebSocket:", error);
      return null;
    }
  }

  function disconnectRealtime() {
    if (realtimeSocket) realtimeSocket.close();
    realtimeSocket = null;
  }

  function handleRealtimeMessage(data) {
    window.dispatchEvent(new CustomEvent("synapsea:realtime", { detail: data }));
  }

  window.API_CONFIG = API_CONFIG;
  window.SynapseaAPI = {
    API_CONFIG,
    getLatestVitals,
    getVitalsHistory,
    getLatestTemperature,
    getTemperatureHistory,
    getDevices,
    getAlerts,
    getPatients,
    getReports,
    getSystemStatus,
    getLatestInsight,
    getDeviceAssignments,
    getUsers,
    updateAlertReadStatus,
    createPatient,
    createReport,
    generateReport,
    assignDevice,
    updateSettings,
    connectRealtime,
    disconnectRealtime,
    handleRealtimeMessage,
    get isOnline() { return apiOnline; }
  };

  Object.assign(window, {
    getLatestVitals,
    getVitalsHistory,
    getLatestTemperature,
    getTemperatureHistory,
    getDevices,
    getAlerts,
    getPatients,
    getReports,
    getSystemStatus,
    getLatestInsight,
    getDeviceAssignments,
    getUsers,
    updateAlertReadStatus,
    createPatient,
    createReport,
    generateReport,
    assignDevice,
    updateSettings,
    connectRealtime,
    disconnectRealtime,
    handleRealtimeMessage
  });
})();
