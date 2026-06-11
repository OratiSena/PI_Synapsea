const express = require("express");
const db = require("../db");
const { optionalAuthenticate } = require("../middleware/auth.middleware");
const { buildReadingScope } = require("../utils/access");
const { generateEducationalInsights } = require("../services/insight.service");

const router = express.Router();

router.get("/latest", optionalAuthenticate, async (request, response, next) => {
  try {
    const vitalScope = buildReadingScope(request, "v");
    const temperatureScope = buildReadingScope(request, "t");
    const alertConditions = [];
    const alertParams = [];
    if (request.user?.role === "user") {
      alertConditions.push(`(
        EXISTS (SELECT 1 FROM patients p WHERE p.id = a.patient_id AND p.user_id = ?)
        OR EXISTS (
          SELECT 1 FROM device_assignments da
          WHERE da.device_id = a.device_id AND da.user_id = ? AND da.active = TRUE
        )
      )`);
      alertParams.push(request.user.id, request.user.id);
    }
    const alertWhere = alertConditions.length ? ` WHERE ${alertConditions.join(" AND ")}` : "";
    const [vitals, temperatures, alerts] = await Promise.all([
      db.query(
        `SELECT bpm, spo2, temperature, stress_index AS stressIndex
         FROM vital_signs v${vitalScope.sql} ORDER BY timestamp DESC LIMIT 1`,
        vitalScope.params
      ),
      db.query(
        `SELECT max_temp AS maxTemp FROM temperature_readings t${temperatureScope.sql}
         ORDER BY timestamp DESC LIMIT 1`,
        temperatureScope.params
      ),
      db.query(
        `SELECT a.id FROM alerts a${alertWhere} ORDER BY a.timestamp DESC LIMIT 20`,
        alertParams
      )
    ]);
    response.json({
      data: generateEducationalInsights({
        vitals: vitals[0] || null,
        temperature: temperatures[0] || null,
        alerts
      })
    });
  } catch (error) {
    next(error);
  }
});

module.exports = router;
