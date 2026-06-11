const express = require("express");
const db = require("../db");

const router = express.Router();

router.get("/", async (_request, response, next) => {
  try {
    const result = await db.query(`
      SELECT id, title, description, type, read, timestamp, created_at AS "createdAt"
      FROM alerts ORDER BY timestamp DESC LIMIT 200`);
    response.json({ data: result.rows });
  } catch (error) {
    next(error);
  }
});

router.patch("/:id/read", async (request, response, next) => {
  try {
    const result = await db.query(
      `UPDATE alerts SET read = TRUE WHERE id = $1
       RETURNING id, title, description, type, read, timestamp`,
      [request.params.id]
    );
    if (!result.rows[0]) return response.status(404).json({ error: "Alerta não encontrado." });
    response.json({ data: result.rows[0] });
  } catch (error) {
    next(error);
  }
});

module.exports = router;
