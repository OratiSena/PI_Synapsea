const express = require("express");
const db = require("../db");
const { authenticateToken } = require("../middleware/auth.middleware");
const { canUsePatient } = require("../utils/access");
const { generateEducationalInsights } = require("../services/insight.service");

const router = express.Router();
const columns = `
  r.id, r.patient_id AS patientId, r.title, r.type, r.content,
  r.created_at AS createdAt, p.name AS patientName`;

async function getPatient(patientId) {
  const rows = await db.query(
    "SELECT id, user_id AS userId, name FROM patients WHERE id = ?",
    [patientId]
  );
  return rows[0] || null;
}

router.get("/", authenticateToken, async (request, response, next) => {
  try {
    const conditions = [];
    const params = [];
    if (request.user.role !== "admin") {
      conditions.push("p.user_id = ?");
      params.push(request.user.id);
    }
    if (request.query.patientId) {
      conditions.push("r.patient_id = ?");
      params.push(request.query.patientId);
    }
    const where = conditions.length ? `WHERE ${conditions.join(" AND ")}` : "";
    const rows = await db.query(
      `SELECT ${columns} FROM reports r
       LEFT JOIN patients p ON p.id = r.patient_id
       ${where} ORDER BY r.created_at DESC`,
      params
    );
    response.json({ data: rows });
  } catch (error) {
    next(error);
  }
});

router.post("/generate", authenticateToken, async (request, response, next) => {
  const patientId = request.body?.patientId;
  const period = request.body?.period || "last_24h";
  if (!patientId) return response.status(400).json({ error: "patientId e obrigatorio." });
  try {
    const patient = await getPatient(patientId);
    if (!patient) return response.status(404).json({ error: "Paciente nao encontrado." });
    if (!canUsePatient(request.user, patient)) {
      return response.status(403).json({ error: "Paciente fora do seu escopo de acesso." });
    }
    const hours = period === "last_7d" ? 168 : period === "last_30d" ? 720 : 24;
    const [metricsRows, thermalMetrics, latestVitals, latestTemperature, alertRows] = await Promise.all([
      db.query(
        `SELECT AVG(bpm) AS avgBpm, AVG(spo2) AS avgSpo2,
         AVG(temperature) AS avgTemperature, MAX(temperature) AS maxTemperature
         FROM vital_signs
         WHERE patient_id = ? AND timestamp >= DATE_SUB(NOW(), INTERVAL ${hours} HOUR)`,
        [patientId]
      ),
      db.query(
        `SELECT MAX(max_temp) AS maxTemperature
         FROM temperature_readings
         WHERE patient_id = ? AND timestamp >= DATE_SUB(NOW(), INTERVAL ${hours} HOUR)`,
        [patientId]
      ),
      db.query(
        `SELECT bpm, spo2, temperature, stress_index AS stressIndex
         FROM vital_signs WHERE patient_id = ? ORDER BY timestamp DESC LIMIT 1`,
        [patientId]
      ),
      db.query(
        `SELECT max_temp AS maxTemp FROM temperature_readings
         WHERE patient_id = ? ORDER BY timestamp DESC LIMIT 1`,
        [patientId]
      ),
      db.query(
        `SELECT id FROM alerts
         WHERE patient_id = ? AND timestamp >= DATE_SUB(NOW(), INTERVAL ${hours} HOUR)`,
        [patientId]
      )
    ]);
    const metrics = metricsRows[0] || {};
    const maximumTemperatures = [
      metrics.maxTemperature,
      thermalMetrics[0]?.maxTemperature
    ].filter((value) => value !== null && value !== undefined).map(Number);
    const insight = generateEducationalInsights({
      vitals: latestVitals[0] || null,
      temperature: latestTemperature[0] || null,
      alerts: alertRows
    });
    const content = {
      summary: insight.message,
      metrics: {
        avgBpm: metrics.avgBpm === null ? null : Number(metrics.avgBpm),
        avgSpo2: metrics.avgSpo2 === null ? null : Number(metrics.avgSpo2),
        avgTemperature: metrics.avgTemperature === null ? null : Number(metrics.avgTemperature),
        maxTemperature: maximumTemperatures.length
          ? Math.max(...maximumTemperatures)
          : null,
        alertsCount: alertRows.length
      },
      educationalInsights: [
        insight.recommendation,
        "Indicadores utilizados apenas para apoio educacional.",
        "Recomenda-se validar leituras com equipamentos certificados em contexto clinico."
      ],
      disclaimer: "Este relatorio e educacional e nao substitui avaliacao medica profissional."
    };
    const idRows = await db.query("SELECT UUID() AS id");
    const id = idRows[0].id;
    await db.query(
      `INSERT INTO reports (id, patient_id, title, type, content)
       VALUES (?, ?, ?, ?, ?)`,
      [
        id, patientId, `Relatorio educacional - ${patient.name}`,
        period, JSON.stringify(content)
      ]
    );
    const rows = await db.query(
      `SELECT ${columns} FROM reports r
       LEFT JOIN patients p ON p.id = r.patient_id WHERE r.id = ?`,
      [id]
    );
    response.status(201).json({ data: rows[0] });
  } catch (error) {
    next(error);
  }
});

router.post("/", authenticateToken, async (request, response, next) => {
  const { patientId, title, type, content = {} } = request.body || {};
  if (!patientId || !title || !type) {
    return response.status(400).json({ error: "Paciente, titulo e tipo sao obrigatorios." });
  }
  try {
    const patient = await getPatient(patientId);
    if (!patient || !canUsePatient(request.user, patient)) {
      return response.status(403).json({ error: "Paciente fora do seu escopo de acesso." });
    }
    const idRows = await db.query("SELECT UUID() AS id");
    const id = idRows[0].id;
    const educationalContent = {
      ...content,
      disclaimer: "Relatorio educacional. Nao substitui avaliacao medica profissional."
    };
    await db.query(
      `INSERT INTO reports (id, patient_id, title, type, content)
       VALUES (?, ?, ?, ?, ?)`,
      [id, patientId, title, type, JSON.stringify(educationalContent)]
    );
    const rows = await db.query(
      `SELECT ${columns} FROM reports r
       LEFT JOIN patients p ON p.id = r.patient_id WHERE r.id = ?`,
      [id]
    );
    response.status(201).json({ data: rows[0] });
  } catch (error) {
    next(error);
  }
});

module.exports = router;
