(function () {
  "use strict";

  const series = [
    { key: "bpm", label: "BPM", color: "#ef4444" },
    { key: "spo2", label: "SpO₂", color: "#38bdf8" },
    { key: "temperature", label: "Temperatura", color: "#a78bfa" },
    { key: "stressIndex", label: "Estresse", color: "#f59e0b" },
    { key: "respiration", label: "Respiração", color: "#00d4ff" }
  ];

  function renderVitalsChart(containerId, data) {
    const container = document.getElementById(containerId);
    if (!container) return;
    const rows = Array.isArray(data) ? data.filter(Boolean) : [];
    if (!rows.length) {
      showEmptyState(container, "Aguardando histórico de sinais vitais.");
      return;
    }

    container.innerHTML = '<canvas class="chart-canvas" aria-label="Gráfico de sinais vitais"></canvas>';
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

    series.forEach((item) => {
      const points = rows
        .map((row, index) => ({ index, value: safeNumber(row[item.key]) }))
        .filter((point) => point.value !== null);
      if (points.length < 1) return;
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
        const x = padding.left + (point.index / Math.max(1, rows.length - 1)) * chartWidth;
        const normalized = (point.value - min) / range;
        const y = padding.top + chartHeight - normalized * chartHeight;
        if (pointIndex === 0) ctx.moveTo(x, y);
        else ctx.lineTo(x, y);
      });
      ctx.stroke();
    });

    const firstTime = rows[0]?.timestamp;
    const lastTime = rows[rows.length - 1]?.timestamp;
    ctx.fillStyle = "#64748b";
    ctx.textAlign = "left";
    ctx.fillText(timeLabel(firstTime), padding.left, height - 12);
    ctx.textAlign = "right";
    ctx.fillText(timeLabel(lastTime), width - padding.right, height - 12);
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
