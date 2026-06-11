const express = require("express");
const db = require("../db");
const { authenticateToken } = require("../middleware/auth.middleware");

const router = express.Router();

router.get("/", authenticateToken, async (request, response, next) => {
  try {
    const conditions = [];
    const params = [];
    if (request.user.role !== "admin") {
      conditions.push("p.user_id = ?");
      params.push(request.user.id);
    } else if (request.query.userId) {
      conditions.push("p.user_id = ?");
      params.push(request.query.userId);
    }
    if (request.query.name) {
      conditions.push("p.name LIKE ?");
      params.push(`%${request.query.name}%`);
    }
    const where = conditions.length ? `WHERE ${conditions.join(" AND ")}` : "";
    const rows = await db.query(
      `SELECT
        p.id, p.user_id AS userId, p.name, p.age, p.identifier, p.status,
        p.created_at AS createdAt, p.updated_at AS updatedAt,
        u.email AS userEmail, MAX(v.timestamp) AS lastSession,
        GROUP_CONCAT(DISTINCT d.name ORDER BY d.name SEPARATOR ', ') AS devices
       FROM patients p
       LEFT JOIN users u ON u.id = p.user_id
       LEFT JOIN vital_signs v ON v.patient_id = p.id
       LEFT JOIN device_assignments da ON da.patient_id = p.id AND da.active = TRUE
       LEFT JOIN devices d ON d.id = da.device_id
       ${where}
       GROUP BY p.id, p.user_id, p.name, p.age, p.identifier, p.status,
         p.created_at, p.updated_at, u.email
       ORDER BY p.name`,
      params
    );
    response.json({ data: rows });
  } catch (error) {
    next(error);
  }
});

router.post("/", authenticateToken, async (request, response, next) => {
  const data = request.body || {};
  const userId = request.user.role === "admin"
    ? data.userId ?? request.user.id
    : request.user.id;
  if (!data.name || !data.identifier) {
    return response.status(400).json({ error: "Nome e identificacao sao obrigatorios." });
  }
  try {
    const idRows = await db.query("SELECT UUID() AS id");
    const id = idRows[0].id;
    await db.query(
      `INSERT INTO patients (id, user_id, name, age, identifier, status)
       VALUES (?, ?, ?, ?, ?, ?)`,
      [id, userId, data.name, data.age ?? null, data.identifier, data.status ?? "active"]
    );
    const rows = await db.query(
      `SELECT id, user_id AS userId, name, age, identifier, status,
       created_at AS createdAt, updated_at AS updatedAt
       FROM patients WHERE id = ?`,
      [id]
    );
    response.status(201).json({ data: rows[0] });
  } catch (error) {
    next(error);
  }
});

module.exports = router;
