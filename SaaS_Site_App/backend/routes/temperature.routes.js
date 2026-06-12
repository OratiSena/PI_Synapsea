const express = require("express");
const db = require("../db");
const { optionalAuthenticate } = require("../middleware/auth.middleware");
const { toMysqlDateTime } = require("../utils/date");
const { buildReadingScope } = require("../utils/access");
const { buildTemperatureAlerts } = require("../services/insight.service");

const router = express.Router();
const MIN_VALID_TEMPERATURE = -20;
const MAX_VALID_TEMPERATURE = 120;
const columns = `
  t.id, t.patient_id AS patientId, t.device_id AS deviceId, t.timestamp,
  t.grid, t.interpolated_grid AS interpolatedGrid,
  t.interpolation_width AS interpolationWidth,
  t.interpolation_height AS interpolationHeight,
  t.heatmap_pixels AS heatmapPixels, t.heatmap_colors AS heatmapColors,
  t.max_temp AS maxTemp, t.min_temp AS minTemp,
  t.avg_temp AS avgTemp, t.hotspot_x AS hotspotX, t.hotspot_y AS hotspotY,
  t.created_at AS createdAt`;
const validReadingSql = `
  t.min_temp BETWEEN ${MIN_VALID_TEMPERATURE} AND ${MAX_VALID_TEMPERATURE}
  AND t.avg_temp BETWEEN ${MIN_VALID_TEMPERATURE} AND ${MAX_VALID_TEMPERATURE}
  AND t.max_temp BETWEEN ${MIN_VALID_TEMPERATURE} AND ${MAX_VALID_TEMPERATURE}
  AND t.min_temp <= t.avg_temp
  AND t.avg_temp <= t.max_temp`;

function appendScopeCondition(scopeSql, condition) {
  return `${scopeSql}${scopeSql ? " AND " : " WHERE "}${condition}`;
}

function isValidTemperature(value) {
  return typeof value === "number"
    && Number.isFinite(value)
    && value >= MIN_VALID_TEMPERATURE
    && value <= MAX_VALID_TEMPERATURE;
}

function isValidTemperatureMatrix(matrix, rows, columns) {
  return Array.isArray(matrix)
    && matrix.length === rows
    && matrix.every(
      (row) => Array.isArray(row)
        && row.length === columns
        && row.every(isValidTemperature)
    );
}

function temperatureDiagnostics(data, reason) {
  const gridIsArray = Array.isArray(data.grid);
  const interpolatedGridIsArray = Array.isArray(data.interpolatedGrid);
  console.warn("Leitura AMG8833 descartada:", {
    reason,
    maxTemp: data.maxTemp,
    minTemp: data.minTemp,
    avgTemp: data.avgTemp,
    gridType: typeof data.grid,
    gridIsArray,
    gridRows: gridIsArray ? data.grid.length : undefined,
    firstRowCols: gridIsArray && Array.isArray(data.grid[0])
      ? data.grid[0].length
      : undefined,
    interpolatedGridIsArray,
    interpolatedRows: interpolatedGridIsArray
      ? data.interpolatedGrid.length
      : undefined,
    interpolatedFirstRowCols: interpolatedGridIsArray
      && Array.isArray(data.interpolatedGrid[0])
      ? data.interpolatedGrid[0].length
      : undefined,
    interpolationWidth: data.interpolationWidth,
    interpolationHeight: data.interpolationHeight,
    hotspotX: data.hotspotX,
    hotspotY: data.hotspotY
  });
}

router.get("/latest", optionalAuthenticate, async (request, response, next) => {
  try {
    const scope = buildReadingScope(request, "t");
    const rows = await db.query(
      `SELECT ${columns} FROM temperature_readings t${appendScopeCondition(scope.sql, validReadingSql)}
       ORDER BY t.timestamp DESC LIMIT 1`,
      scope.params
    );
    response.json({ data: rows[0] || null });
  } catch (error) {
    next(error);
  }
});

router.get("/history", optionalAuthenticate, async (request, response, next) => {
  try {
    const limit = Math.min(Math.max(Number(request.query.limit) || 100, 1), 500);
    const scope = buildReadingScope(request, "t");
    const rows = await db.query(
      `SELECT ${columns} FROM temperature_readings t${appendScopeCondition(scope.sql, validReadingSql)}
       ORDER BY t.timestamp DESC LIMIT ${limit}`,
      scope.params
    );
    response.json({ data: rows.reverse() });
  } catch (error) {
    next(error);
  }
});

router.post("/", async (request, response, next) => {
  const data = request.body || {};
  const validGrid = isValidTemperatureMatrix(data.grid, 8, 8);
  if (!validGrid) {
    temperatureDiagnostics(data, "grid 8x8 invalido");
    return response.status(400).json({
      error: "grid deve ser uma matriz 8x8 com temperaturas entre -20 e 120 graus."
    });
  }
  const hasInterpolatedGrid = Array.isArray(data.interpolatedGrid);
  const interpolationHeight = Number(data.interpolationHeight)
    || (hasInterpolatedGrid ? data.interpolatedGrid.length : null);
  const interpolationWidth = Number(data.interpolationWidth)
    || (hasInterpolatedGrid && Array.isArray(data.interpolatedGrid[0])
      ? data.interpolatedGrid[0].length
      : null);
  const validInterpolatedGrid = !hasInterpolatedGrid || (
    Number.isInteger(interpolationWidth)
    && Number.isInteger(interpolationHeight)
    && interpolationWidth === 30
    && interpolationHeight === 30
    && data.interpolatedGrid.length === interpolationHeight
    && isValidTemperatureMatrix(
      data.interpolatedGrid,
      interpolationHeight,
      interpolationWidth
    )
  );
  if (!validInterpolatedGrid) {
    temperatureDiagnostics(data, "interpolatedGrid invalido");
    return response.status(400).json({
      error: "interpolatedGrid deve ter dimensoes validas e temperaturas entre -20 e 120 graus."
    });
  }

  const validStats = ["minTemp", "avgTemp", "maxTemp"].every(
    (field) => isValidTemperature(data[field])
  ) && data.minTemp <= data.avgTemp && data.avgTemp <= data.maxTemp;
  const validHotspot = Number.isInteger(data.hotspotX)
    && Number.isInteger(data.hotspotY)
    && data.hotspotX >= 0
    && data.hotspotX < 8
    && data.hotspotY >= 0
    && data.hotspotY < 8;
  if (!validStats || !validHotspot) {
    temperatureDiagnostics(data, "estatisticas ou hotspot invalidos");
    return response.status(400).json({
      error: "minTemp, avgTemp, maxTemp e hotspot devem representar uma leitura AMG8833 valida."
    });
  }

  const rawValues = data.grid.flat();
  const calculatedMin = Math.min(...rawValues);
  const calculatedMax = Math.max(...rawValues);
  const calculatedAvg = rawValues.reduce((sum, value) => sum + value, 0) / rawValues.length;
  const statsMatchGrid = Math.abs(data.minTemp - calculatedMin) <= 0.05
    && Math.abs(data.maxTemp - calculatedMax) <= 0.05
    && Math.abs(data.avgTemp - calculatedAvg) <= 0.05;
  if (!statsMatchGrid) {
    temperatureDiagnostics(data, "estatisticas nao correspondem ao grid");
    return response.status(400).json({
      error: "maxTemp, minTemp e avgTemp nao correspondem a matriz grid recebida."
    });
  }

  try {
    const idRows = await db.query("SELECT UUID() AS id");
    const id = idRows[0].id;
    const timestamp = toMysqlDateTime(data.timestamp) || toMysqlDateTime(new Date());
    await db.query(
      `INSERT INTO temperature_readings (
        id, patient_id, device_id, timestamp, grid, interpolated_grid,
        interpolation_width, interpolation_height, heatmap_pixels,
        heatmap_colors, max_temp, min_temp, avg_temp, hotspot_x, hotspot_y
      ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)`,
      [
        id, data.patientId ?? null, data.deviceId ?? null, timestamp,
        JSON.stringify(data.grid),
        hasInterpolatedGrid ? JSON.stringify(data.interpolatedGrid) : null,
        hasInterpolatedGrid ? interpolationWidth : null,
        hasInterpolatedGrid ? interpolationHeight : null,
        data.heatmapPixels ? JSON.stringify(data.heatmapPixels) : null,
        data.heatmapColors ? JSON.stringify(data.heatmapColors) : null,
        data.maxTemp ?? null, data.minTemp ?? null, data.avgTemp ?? null,
        data.hotspotX ?? null, data.hotspotY ?? null
      ]
    );

    for (const alert of buildTemperatureAlerts(data)) {
      await db.query(
        `INSERT INTO alerts (
          id, patient_id, device_id, title, description, type, is_read, timestamp
        ) VALUES (UUID(), ?, ?, ?, ?, ?, FALSE, ?)`,
        [
          data.patientId ?? null, data.deviceId ?? null, alert.title,
          alert.description, alert.type, timestamp
        ]
      );
    }

    const rows = await db.query(
      `SELECT ${columns} FROM temperature_readings t WHERE t.id = ?`,
      [id]
    );
    response.status(201).json({ data: rows[0] });
  } catch (error) {
    next(error);
  }
});

module.exports = router;
