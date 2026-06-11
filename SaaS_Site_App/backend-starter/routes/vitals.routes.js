const express = require("express");
const db = require("../db");

const router = express.Router();
const columns = `
  id, patient_id AS "patientId", timestamp, bpm, spo2, temperature,
  stress_index AS "stressIndex", respiration, hrv, pi, ppg,
  signal_quality AS "signalQuality", finger_detected AS "fingerDetected", status`;

router.get("/latest", async (_request, response, next) => {
  try {
    const result = await db.query(`SELECT ${columns} FROM vital_signs ORDER BY timestamp DESC LIMIT 1`);
    response.json({ data: result.rows[0] || null });
  } catch (error) {
    next(error);
  }
});

router.get("/history", async (request, response, next) => {
  try {
    const limit = Math.min(Math.max(Number(request.query.limit) || 100, 1), 1000);
    const result = await db.query(`SELECT ${columns} FROM vital_signs ORDER BY timestamp DESC LIMIT $1`, [limit]);
    response.json({ data: result.rows.reverse() });
  } catch (error) {
    next(error);
  }
});

router.post("/", async (request, response, next) => {
  const data = request.body || {};
  try {
    const result = await db.query(
      `INSERT INTO vital_signs
        (patient_id, timestamp, bpm, spo2, temperature, stress_index, respiration, hrv, pi, ppg, signal_quality, finger_detected, status)
       VALUES ($1, COALESCE($2, NOW()), $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13)
       RETURNING ${columns}`,
      [data.patientId || null, data.timestamp || null, data.bpm, data.spo2, data.temperature,
        data.stressIndex, data.respiration, data.hrv, data.pi, JSON.stringify(data.ppg || []),
        data.signalQuality, data.fingerDetected, data.status]
    );
    response.status(201).json({ data: result.rows[0] });
  } catch (error) {
    next(error);
  }
});

module.exports = router;
