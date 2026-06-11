const express = require("express");
const db = require("../db");

const router = express.Router();

router.get("/", async (_request, response, next) => {
  try {
    const result = await db.query(`
      SELECT id, patient_id AS "patientId", title, type, content, created_at AS "createdAt"
      FROM reports ORDER BY created_at DESC`);
    response.json({ data: result.rows });
  } catch (error) {
    next(error);
  }
});

router.post("/", async (request, response, next) => {
  const { patientId, title, type, content = {} } = request.body || {};
  if (!title || !type) return response.status(400).json({ error: "Título e tipo são obrigatórios." });
  try {
    const educationalContent = {
      ...content,
      disclaimer: "Relatório de apoio educacional. Não substitui avaliação médica profissional."
    };
    const result = await db.query(
      `INSERT INTO reports (patient_id, title, type, content)
       VALUES ($1, $2, $3, $4)
       RETURNING id, patient_id AS "patientId", title, type, content, created_at AS "createdAt"`,
      [patientId || null, title, type, JSON.stringify(educationalContent)]
    );
    response.status(201).json({ data: result.rows[0] });
  } catch (error) {
    next(error);
  }
});

module.exports = router;
