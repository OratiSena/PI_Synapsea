const express = require("express");
const db = require("../db");
const { authenticateToken } = require("../middleware/auth.middleware");

const router = express.Router();
const columns = `
  a.id, a.patient_id AS patientId, a.device_id AS deviceId,
  a.title, a.description, a.type, a.is_read AS \`read\`,
  a.timestamp, a.created_at AS createdAt,
  p.name AS patientName, d.name AS deviceName`;

router.get("/", authenticateToken, async (request, response, next) => {
  try {
    const conditions = [];
    const params = [];
    if (request.user.role !== "admin") {
      conditions.push(`(
        p.user_id = ?
        OR EXISTS (
          SELECT 1 FROM device_assignments da
          WHERE da.device_id = a.device_id AND da.user_id = ? AND da.active = TRUE
        )
      )`);
      params.push(request.user.id, request.user.id);
    }
    if (request.query.type) {
      conditions.push("a.type = ?");
      params.push(request.query.type);
    }
    const where = conditions.length ? `WHERE ${conditions.join(" AND ")}` : "";
    const rows = await db.query(
      `SELECT ${columns}
       FROM alerts a
       LEFT JOIN patients p ON p.id = a.patient_id
       LEFT JOIN devices d ON d.id = a.device_id
       ${where}
       ORDER BY a.timestamp DESC LIMIT 200`,
      params
    );
    response.json({ data: rows });
  } catch (error) {
    next(error);
  }
});

router.patch("/:id/read", authenticateToken, async (request, response, next) => {
  try {
    const conditions = ["a.id = ?"];
    const params = [request.params.id];
    if (request.user.role !== "admin") {
      conditions.push(`(
        p.user_id = ?
        OR EXISTS (
          SELECT 1 FROM device_assignments da
          WHERE da.device_id = a.device_id AND da.user_id = ? AND da.active = TRUE
        )
      )`);
      params.push(request.user.id, request.user.id);
    }
    const allowed = await db.query(
      `SELECT a.id FROM alerts a
       LEFT JOIN patients p ON p.id = a.patient_id
       WHERE ${conditions.join(" AND ")}`,
      params
    );
    if (!allowed[0]) return response.status(404).json({ error: "Alerta nao encontrado." });
    await db.query("UPDATE alerts SET is_read = TRUE WHERE id = ?", [request.params.id]);
    const rows = await db.query(
      `SELECT ${columns} FROM alerts a
       LEFT JOIN patients p ON p.id = a.patient_id
       LEFT JOIN devices d ON d.id = a.device_id
       WHERE a.id = ?`,
      [request.params.id]
    );
    response.json({ data: rows[0] });
  } catch (error) {
    next(error);
  }
});

module.exports = router;
