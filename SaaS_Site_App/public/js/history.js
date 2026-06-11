(function () {
  "use strict";
  let readings = [];

  function normalizeReadings(vitals, temperatures) {
    const vitalRows = vitals.map((item) => ({
      ...item,
      readingType: "vitals",
      summary: [
        item.bpm !== null && item.bpm !== undefined ? `${formatValue(item.bpm)} BPM` : null,
        item.spo2 !== null && item.spo2 !== undefined ? `${formatValue(item.spo2)}% SpO₂` : null,
        item.temperature !== null && item.temperature !== undefined ? formatTemperature(item.temperature) : null
      ].filter(Boolean).join(" · ") || "Leitura sem métricas"
    }));
    const thermalRows = temperatures.map((item) => ({
      ...item,
      readingType: "temperature",
      summary: `Máx ${formatTemperature(item.maxTemp)} · Média ${formatTemperature(item.avgTemp)} · Mín ${formatTemperature(item.minTemp)}`
    }));
    return [...vitalRows, ...thermalRows].sort((a, b) => new Date(b.timestamp) - new Date(a.timestamp));
  }

  function applyFilters() {
    const patient = document.getElementById("history-patient").value.trim().toLowerCase();
    const date = document.getElementById("history-date").value;
    const type = document.getElementById("history-type").value;
    return readings.filter((item) => {
      const itemPatient = String(item.patientId || item.patient_id || "").toLowerCase();
      const itemDate = item.timestamp ? new Date(item.timestamp).toISOString().slice(0, 10) : "";
      return (!patient || itemPatient.includes(patient))
        && (!date || itemDate === date)
        && (!type || item.readingType === type);
    });
  }

  function render() {
    const container = document.getElementById("history-content");
    const filtered = applyFilters();
    setText("history-total", readings.length);
    setText("history-vitals", readings.filter((item) => item.readingType === "vitals").length);
    setText("history-thermal", readings.filter((item) => item.readingType === "temperature").length);
    if (!filtered.length) {
      showEmptyState(container, "Nenhum histórico encontrado.");
      return;
    }
    container.innerHTML = `
      <div class="table-wrap"><table class="data-table">
        <thead><tr><th>Data e hora</th><th>Paciente</th><th>Tipo</th><th>Resumo da leitura</th><th>Status</th></tr></thead>
        <tbody>${filtered.map((item) => `
          <tr>
            <td>${escapeHtml(formatDateTime(item.timestamp))}</td>
            <td>${escapeHtml(item.patientId || item.patient_id || "--")}</td>
            <td class="reading-type">${item.readingType === "vitals" ? "Sinais vitais" : "Mapa térmico"}</td>
            <td>${escapeHtml(item.summary)}</td>
            <td><span class="badge badge-${getStatusColor(item.status)}">${escapeHtml(getStatusLabel(item.status))}</span></td>
          </tr>`).join("")}</tbody>
      </table></div>`;
  }

  document.addEventListener("DOMContentLoaded", async () => {
    showLoading("history-content");
    const [vitals, temperatures] = await Promise.all([getVitalsHistory(), getTemperatureHistory()]);
    readings = normalizeReadings(Array.isArray(vitals) ? vitals : [], Array.isArray(temperatures) ? temperatures : []);
    render();
    document.querySelectorAll("#history-filters input, #history-filters select").forEach((field) => field.addEventListener("input", render));
    document.getElementById("clear-history-filters")?.addEventListener("click", () => {
      document.getElementById("history-filters").reset();
      render();
    });
  });
})();
