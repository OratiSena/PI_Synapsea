const express = require("express");
const db = require("../db");
const { optionalAuthenticate } = require("../middleware/auth.middleware");
const { toMysqlDateTime } = require("../utils/date");
const { buildReadingScope } = require("../utils/access");
const { buildTemperatureAlerts } = require("../services/insight.service");

const router = express.Router();
const columns = `
  t.id, t.patient_id AS patientId, t.device_id AS deviceId, t.timestamp,
  t.grid, t.max_temp AS maxTemp, t.min_temp AS minTemp,
  t.avg_temp AS avgTemp, t.hotspot_x AS hotspotX, t.hotspot_y AS hotspotY,
  t.created_at AS createdAt`;

router.get("/latest", optionalAuthenticate, async (request, response, next) => {
  try {
    const scope = buildReadingScope(request, "t");
    const rows = await db.query(
      `SELECT ${columns} FROM temperature_readings t${scope.sql}
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
      `SELECT ${columns} FROM temperature_readings t${scope.sql}
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
  const validGrid = Array.isArray(data.grid)
    && data.grid.length === 8
    && data.grid.every((row) => Array.isArray(row) && row.length === 8);
  if (!validGrid) {
    return response.status(400).json({ error: "grid deve ser uma matriz 8x8." });
  }

  try {
    const idRows = await db.query("SELECT UUID() AS id");
    const id = idRows[0].id;
    const timestamp = toMysqlDateTime(data.timestamp) || toMysqlDateTime(new Date());
    await db.query(
      `INSERT INTO temperature_readings (
        id, patient_id, device_id, timestamp, grid, max_temp, min_temp,
        avg_temp, hotspot_x, hotspot_y
      ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)`,
      [
        id, data.patientId ?? null, data.deviceId ?? null, timestamp,
        JSON.stringify(data.grid), data.maxTemp ?? null, data.minTemp ?? null,
        data.avgTemp ?? null, data.hotspotX ?? null, data.hotspotY ?? null
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
