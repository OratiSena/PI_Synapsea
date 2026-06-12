(function () {
  "use strict";

  function renderTemperature(data) {
    const valid = window.isValidTemperatureReading?.(data);
    const reading = valid ? data : null;
    renderThermalHeatmap("thermal-canvas", reading);
    setText("temp-max", reading ? formatTemperature(reading.maxTemp) : "--");
    setText("temp-avg", reading ? formatTemperature(reading.avgTemp) : "--");
    setText("temp-min", reading ? formatTemperature(reading.minTemp) : "--");
    setText("heatmap-time", reading?.timestamp ? `Atualizado ${formatRelativeTime(reading.timestamp)}` : "Aguardando leitura válida");
    setText("scale-min", reading ? formatTemperature(reading.minTemp) : "--");
    setText("scale-max", reading ? formatTemperature(reading.maxTemp) : "--");
    setText(
      "heatmap-source",
      reading?.interpolatedGrid
        ? `${reading.interpolationWidth || reading.interpolatedGrid[0]?.length}x${reading.interpolationHeight || reading.interpolatedGrid.length} recebido do ESP32`
        : reading ? "Fallback 8x8 interpolado no navegador" : "Aguardando matriz válida"
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
