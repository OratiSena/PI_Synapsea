(function () {
  "use strict";

  const series = [
    { key: "bpm", label: "BPM", color: "#ef4444", source: "vitals" },
    { key: "spo2", label: "SpO2", color: "#38bdf8", source: "vitals" },
    { key: "avgTemp", label: "Temperatura", color: "#a78bfa", source: "temperature" },
    { key: "stressIndex", label: "Estresse", color: "#f59e0b", source: "vitals" },
    { key: "respiration", label: "Respiracao", color: "#00d4ff", source: "vitals" }
  ];

  function renderVitalsChart(containerId, vitalsData, temperatureData = []) {
    const container = document.getElementById(containerId);
    if (!container) return;

    const vitals = Array.isArray(vitalsData) ? vitalsData.filter(Boolean) : [];
    const temperatures = Array.isArray(temperatureData)
      ? temperatureData.filter((row) => window.isValidTemperatureReading?.(row))
      : [];
    const allRows = [...vitals, ...temperatures]
      .filter((row) => row.timestamp && !Number.isNaN(new Date(row.timestamp).getTime()))
      .sort((a, b) => new Date(a.timestamp) - new Date(b.timestamp));

    if (!allRows.length) {
      showEmptyState(container, "Aguardando historico de sinais vitais.");
      return;
    }

    container.innerHTML = '<canvas class="chart-canvas" aria-label="Grafico de sinais vitais"></canvas>';
    const canvas = container.querySelector("canvas");
    const dpr = window.devicePixelRatio || 1;
    const width = Math.max(320, container.clientWidth);
    const height = 320;
    canvas.width = width * dpr;
    canvas.height = height * dpr;
    canvas.style.width = `${width}px`;
    canvas.style.height = `${height}px`;
    const ctx = canvas.getContext("2d");
    ctx.scale(dpr, dpr);

    const padding = { top: 22, right: 18, bottom: 38, left: 42 };
    const chartWidth = width - padding.left - padding.right;
    const chartHeight = height - padding.top - padding.bottom;

    ctx.clearRect(0, 0, width, height);
    ctx.strokeStyle = "rgba(255,255,255,.07)";
    ctx.fillStyle = "#64748b";
    ctx.font = "11px system-ui";
    ctx.lineWidth = 1;
    for (let index = 0; index <= 4; index += 1) {
      const y = padding.top + (chartHeight / 4) * index;
      ctx.beginPath();
      ctx.moveTo(padding.left, y);
      ctx.lineTo(width - padding.right, y);
      ctx.stroke();
    }

    const firstTimestamp = new Date(allRows[0].timestamp).getTime();
    const lastTimestamp = new Date(allRows[allRows.length - 1].timestamp).getTime();
    const timeRange = Math.max(1, lastTimestamp - firstTimestamp);

    series.forEach((item) => {
      const sourceRows = item.source === "temperature" ? temperatures : vitals;
      const points = sourceRows
        .map((row) => ({
          timestamp: new Date(row.timestamp).getTime(),
          value: safeNumber(row[item.key])
        }))
        .filter((point) => Number.isFinite(point.timestamp) && point.value !== null)
        .sort((a, b) => a.timestamp - b.timestamp);
      if (!points.length) return;

      const values = points.map((point) => point.value);
      let min = Math.min(...values);
      let max = Math.max(...values);
      if (min === max) {
        min -= 1;
        max += 1;
      }
      const range = max - min;
      ctx.strokeStyle = item.color;
      ctx.lineWidth = 2;
      ctx.lineJoin = "round";
      ctx.lineCap = "round";
      ctx.beginPath();
      points.forEach((point, pointIndex) => {
        const x = padding.left
          + ((point.timestamp - firstTimestamp) / timeRange) * chartWidth;
        const normalized = (point.value - min) / range;
        const y = padding.top + chartHeight - normalized * chartHeight;
        if (pointIndex === 0) ctx.moveTo(x, y);
        else ctx.lineTo(x, y);
      });
      ctx.stroke();
      if (points.length === 1) {
        const x = padding.left
          + ((points[0].timestamp - firstTimestamp) / timeRange) * chartWidth;
        const normalized = (points[0].value - min) / range;
        const y = padding.top + chartHeight - normalized * chartHeight;
        ctx.fillStyle = item.color;
        ctx.beginPath();
        ctx.arc(x, y, 3, 0, Math.PI * 2);
        ctx.fill();
      }
    });

    ctx.fillStyle = "#64748b";
    ctx.textAlign = "left";
    ctx.fillText(timeLabel(allRows[0]?.timestamp), padding.left, height - 12);
    ctx.textAlign = "right";
    ctx.fillText(timeLabel(allRows[allRows.length - 1]?.timestamp), width - padding.right, height - 12);
  }

  function timeLabel(value) {
    if (!value) return "";
    const date = new Date(value);
    if (Number.isNaN(date.getTime())) return "";
    return date.toLocaleTimeString("pt-BR", { hour: "2-digit", minute: "2-digit" });
  }

  window.renderVitalsChart = renderVitalsChart;
  window.VITAL_CHART_SERIES = series;
})();
