const express = require("express");
const db = require("../db");

const router = express.Router();
const columns = `
  id, patient_id AS "patientId", timestamp, grid, max_temp AS "maxTemp",
  min_temp AS "minTemp", avg_temp AS "avgTemp", hotspot_x AS "hotspotX", hotspot_y AS "hotspotY"`;

router.get("/latest", async (_request, response, next) => {
  try {
    const result = await db.query(`SELECT ${columns} FROM temperature_readings ORDER BY timestamp DESC LIMIT 1`);
    response.json({ data: result.rows[0] || null });
  } catch (error) {
    next(error);
  }
});

router.get("/history", async (request, response, next) => {
  try {
    const limit = Math.min(Math.max(Number(request.query.limit) || 100, 1), 1000);
    const result = await db.query(`SELECT ${columns} FROM temperature_readings ORDER BY timestamp DESC LIMIT $1`, [limit]);
    response.json({ data: result.rows.reverse() });
  } catch (error) {
    next(error);
  }
});

router.post("/", async (request, response, next) => {
  const data = request.body || {};
  if (!Array.isArray(data.grid) || data.grid.length !== 8 || data.grid.some((row) => !Array.isArray(row) || row.length !== 8)) {
    return response.status(400).json({ error: "grid deve ser uma matriz 8x8." });
  }
  try {
    const result = await db.query(
      `INSERT INTO temperature_readings
        (patient_id, timestamp, grid, max_temp, min_temp, avg_temp, hotspot_x, hotspot_y)
       VALUES ($1, COALESCE($2, NOW()), $3, $4, $5, $6, $7, $8)
       RETURNING ${columns}`,
      [data.patientId || null, data.timestamp || null, JSON.stringify(data.grid), data.maxTemp,
        data.minTemp, data.avgTemp, data.hotspotX, data.hotspotY]
    );
    response.status(201).json({ data: result.rows[0] });
  } catch (error) {
    next(error);
  }
});

module.exports = router;
