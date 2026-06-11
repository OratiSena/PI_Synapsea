const express = require("express");
const db = require("../db");
const { optionalAuthenticate } = require("../middleware/auth.middleware");
const { toMysqlDateTime } = require("../utils/date");
const { buildReadingScope } = require("../utils/access");
const { buildVitalAlerts } = require("../services/insight.service");

const router = express.Router();
const columns = `
  v.id, v.patient_id AS patientId, v.device_id AS deviceId, v.timestamp,
  v.bpm, v.spo2, v.temperature, v.stress_index AS stressIndex,
  v.respiration, v.hrv, v.pi, v.ppg,
  v.signal_quality AS signalQuality, v.finger_detected AS fingerDetected,
  v.status, v.created_at AS createdAt`;

router.get("/latest", optionalAuthenticate, async (request, response, next) => {
  try {
    const scope = buildReadingScope(request, "v");
    const rows = await db.query(
      `SELECT ${columns} FROM vital_signs v${scope.sql}
       ORDER BY v.timestamp DESC LIMIT 1`,
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
    const scope = buildReadingScope(request, "v");
    const rows = await db.query(
      `SELECT ${columns} FROM vital_signs v${scope.sql}
       ORDER BY v.timestamp DESC LIMIT ${limit}`,
      scope.params
    );
    response.json({ data: rows.reverse() });
  } catch (error) {
    next(error);
  }
});

router.post("/", async (request, response, next) => {
  const data = request.body || {};
  try {
    const idRows = await db.query("SELECT UUID() AS id");
    const id = idRows[0].id;
    const timestamp = toMysqlDateTime(data.timestamp) || toMysqlDateTime(new Date());
    await db.query(
      `INSERT INTO vital_signs (
        id, patient_id, device_id, timestamp, bpm, spo2, temperature,
        stress_index, respiration, hrv, pi, ppg, signal_quality,
        finger_detected, status
      ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)`,
      [
        id, data.patientId ?? null, data.deviceId ?? null, timestamp,
        data.bpm ?? null, data.spo2 ?? null, data.temperature ?? null,
        data.stressIndex ?? null, data.respiration ?? null, data.hrv ?? null,
        data.pi ?? null, JSON.stringify(data.ppg ?? []),
        data.signalQuality ?? null, data.fingerDetected ?? false,
        data.status ?? "normal"
      ]
    );

    for (const alert of buildVitalAlerts(data)) {
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

    const rows = await db.query(`SELECT ${columns} FROM vital_signs v WHERE v.id = ?`, [id]);
    response.status(201).json({ data: rows[0] });
  } catch (error) {
    next(error);
  }
});

module.exports = router;
