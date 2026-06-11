const express = require("express");
const db = require("../db");

const router = express.Router();

router.get("/", async (_request, response, next) => {
  try {
    const result = await db.query(`
      SELECT id, name, type, status, last_sync AS "lastSync", metadata, created_at AS "createdAt"
      FROM devices ORDER BY name`);
    response.json({ data: result.rows });
  } catch (error) {
    next(error);
  }
});

router.put("/:id", async (request, response, next) => {
  const data = request.body || {};
  try {
    const result = await db.query(
      `INSERT INTO devices (id, name, type, status, last_sync, metadata)
       VALUES ($1, $2, $3, $4, COALESCE($5, NOW()), $6)
       ON CONFLICT (id) DO UPDATE SET
         name = EXCLUDED.name, type = EXCLUDED.type, status = EXCLUDED.status,
         last_sync = EXCLUDED.last_sync, metadata = EXCLUDED.metadata
       RETURNING id, name, type, status, last_sync AS "lastSync", metadata`,
      [request.params.id, data.name, data.type, data.status || "waiting", data.lastSync || null, JSON.stringify(data.metadata || {})]
    );
    response.json({ data: result.rows[0] });
  } catch (error) {
    next(error);
  }
});

module.exports = router;
