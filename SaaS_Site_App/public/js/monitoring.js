(function () {
  "use strict";

  function renderTemperature(data) {
    renderThermalHeatmap("thermal-canvas", data);
    setText("temp-max", data ? formatTemperature(data.maxTemp) : "--");
    setText("temp-avg", data ? formatTemperature(data.avgTemp) : "--");
    setText("temp-min", data ? formatTemperature(data.minTemp) : "--");
    setText("heatmap-time", data?.timestamp ? `Atualizado ${formatRelativeTime(data.timestamp)}` : "Aguardando leitura real");
    setText("scale-min", data ? formatTemperature(data.minTemp) : "--");
    setText("scale-max", data ? formatTemperature(data.maxTemp) : "--");
    setText(
      "heatmap-source",
      data?.interpolatedGrid
        ? `${data.interpolationWidth || data.interpolatedGrid[0]?.length}x${data.interpolationHeight || data.interpolatedGrid.length} recebido do ESP32`
        : "Fallback 8x8 interpolado no navegador"
    );
  }

  function renderSensorDevice(devices) {
    const container = document.getElementById("sensor-device-status");
    const device = devices.find((item) => String(item.type || item.name).toLowerCase().includes("amg8833"));
    if (!device) {
      showEmptyState(container, "O AMG8833 ainda não foi informado pela API.");
      return;
    }
    container.innerHTML = `
      <div class="compact-item">
        <div class="compact-item-main">
          <strong>${escapeHtml(device.name || "AMG8833")}</strong>
          <span>${escapeHtml(formatRelativeTime(device.lastSync))}</span>
        </div>
        <span class="badge badge-${getStatusColor(device.status)}">${escapeHtml(getDeviceStatusLabel(device.status))}</span>
      </div>`;
  }

  async function loadMonitoring() {
    const [temperature, devices] = await Promise.all([getLatestTemperature(), getDevices()]);
    renderTemperature(temperature);
    renderSensorDevice(Array.isArray(devices) ? devices : []);
  }

  document.addEventListener("DOMContentLoaded", () => {
    renderTemperature(null);
    loadMonitoring();
    window.setInterval(loadMonitoring, API_CONFIG.refreshInterval);
  });
})();
