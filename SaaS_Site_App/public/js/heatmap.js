(function () {
  "use strict";

  function colorFor(value, min, max) {
    const ratio = max === min ? 0.5 : Math.max(0, Math.min(1, (value - min) / (max - min)));
    const stops = [
      [0, [14, 76, 146]],
      [0.33, [0, 190, 160]],
      [0.66, [250, 204, 21]],
      [1, [239, 68, 68]]
    ];
    let start = stops[0];
    let end = stops[stops.length - 1];
    for (let index = 0; index < stops.length - 1; index += 1) {
      if (ratio >= stops[index][0] && ratio <= stops[index + 1][0]) {
        start = stops[index];
        end = stops[index + 1];
        break;
      }
    }
    const local = (ratio - start[0]) / (end[0] - start[0] || 1);
    const rgb = start[1].map((channel, index) => Math.round(channel + (end[1][index] - channel) * local));
    return `rgb(${rgb.join(",")})`;
  }

  function renderThermalHeatmap(canvasId, temperatureData) {
    const canvas = document.getElementById(canvasId);
    if (!canvas) return;
    const rawGrid = temperatureData?.grid;
    const interpolatedGrid = temperatureData?.interpolatedGrid;
    const hasInterpolatedGrid = Array.isArray(interpolatedGrid)
      && interpolatedGrid.length > 0
      && interpolatedGrid.every(
        (row) => Array.isArray(row) && row.length === interpolatedGrid[0].length
      );
    const grid = hasInterpolatedGrid ? interpolatedGrid : rawGrid;
    if (!Array.isArray(grid) || !grid.length || !Array.isArray(grid[0]) || !grid[0].length) {
      canvas.classList.add("hidden");
      const empty = document.getElementById(`${canvasId}-empty`);
      if (empty) {
        empty.classList.remove("hidden");
        showEmptyState(empty, "Aguardando leitura real do AMG8833.");
      }
      return;
    }

    canvas.classList.remove("hidden");
    document.getElementById(`${canvasId}-empty`)?.classList.add("hidden");
    const values = grid.flat().map(Number).filter(Number.isFinite);
    const min = safeNumber(temperatureData.minTemp, Math.min(...values));
    const max = safeNumber(temperatureData.maxTemp, Math.max(...values));
    const displaySize = Math.min(620, Math.max(280, canvas.parentElement?.clientWidth || 480));
    const dpr = window.devicePixelRatio || 1;
    canvas.width = displaySize * dpr;
    canvas.height = displaySize * dpr;
    canvas.style.width = `${displaySize}px`;
    canvas.style.height = `${displaySize}px`;
    const ctx = canvas.getContext("2d");
    ctx.scale(dpr, dpr);

    const source = document.createElement("canvas");
    const rows = grid.length;
    const columns = grid[0].length;
    source.width = columns;
    source.height = rows;
    const sourceCtx = source.getContext("2d");
    const pixels = sourceCtx.createImageData(columns, rows);
    grid.flat().forEach((value, index) => {
      const color = colorFor(Number(value), min, max).match(/\d+/g).map(Number);
      pixels.data[index * 4] = color[0];
      pixels.data[index * 4 + 1] = color[1];
      pixels.data[index * 4 + 2] = color[2];
      pixels.data[index * 4 + 3] = 255;
    });
    sourceCtx.putImageData(pixels, 0, 0);
    ctx.imageSmoothingEnabled = true;
    ctx.imageSmoothingQuality = "high";
    ctx.drawImage(source, 0, 0, displaySize, displaySize);

    const hotspotX = safeNumber(temperatureData.hotspotX);
    const hotspotY = safeNumber(temperatureData.hotspotY);
    if (hotspotX !== null && hotspotY !== null) {
      const rawColumns = Array.isArray(rawGrid?.[0]) ? rawGrid[0].length : columns;
      const rawRows = Array.isArray(rawGrid) ? rawGrid.length : rows;
      const x = ((hotspotX + 0.5) / rawColumns) * displaySize;
      const y = ((hotspotY + 0.5) / rawRows) * displaySize;
      const cell = displaySize / Math.max(rawColumns, rawRows);
      ctx.strokeStyle = "#ffffff";
      ctx.lineWidth = 2;
      ctx.beginPath();
      ctx.arc(x, y, Math.max(9, cell * 0.18), 0, Math.PI * 2);
      ctx.stroke();
      ctx.beginPath();
      ctx.moveTo(x - 14, y);
      ctx.lineTo(x + 14, y);
      ctx.moveTo(x, y - 14);
      ctx.lineTo(x, y + 14);
      ctx.stroke();
    }
  }

  window.renderThermalHeatmap = renderThermalHeatmap;
})();
