const express = require("express");
const db = require("../db");

const router = express.Router();

router.get("/status", async (_request, response) => {
  const startedAt = Date.now();
  try {
    await db.query("SELECT 1");
    response.json({
      data: {
        api: "online",
        database: "connected",
        latencyMs: Date.now() - startedAt,
        timestamp: new Date().toISOString()
      }
    });
  } catch (error) {
    response.status(503).json({
      error: "Banco de dados indisponível.",
      data: { api: "online", database: "offline", timestamp: new Date().toISOString() }
    });
  }
});

module.exports = router;
