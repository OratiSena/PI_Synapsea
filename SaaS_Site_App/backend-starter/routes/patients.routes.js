const express = require("express");
const db = require("../db");

const router = express.Router();

router.get("/", async (_request, response, next) => {
  try {
    const result = await db.query(`
      SELECT p.id, p.name, p.age, p.identifier, p.status, p.created_at AS "createdAt",
        MAX(v.timestamp) AS "lastSession"
      FROM patients p
      LEFT JOIN vital_signs v ON v.patient_id = p.id
      GROUP BY p.id
      ORDER BY p.name`);
    response.json({ data: result.rows });
  } catch (error) {
    next(error);
  }
});

router.post("/", async (request, response, next) => {
  const { name, age, identifier, status = "active" } = request.body || {};
  if (!name || !identifier) return response.status(400).json({ error: "Nome e identificação são obrigatórios." });
  try {
    const result = await db.query(
      `INSERT INTO patients (name, age, identifier, status)
       VALUES ($1, $2, $3, $4)
       RETURNING id, name, age, identifier, status, created_at AS "createdAt"`,
      [name, age || null, identifier, status]
    );
    response.status(201).json({ data: result.rows[0] });
  } catch (error) {
    next(error);
  }
});

module.exports = router;
