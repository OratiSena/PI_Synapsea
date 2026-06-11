const express = require("express");
const db = require("../db");
const { authenticateToken } = require("../middleware/auth.middleware");

const router = express.Router();
const allowed = [
  "refreshInterval",
  "deviceId",
  "connectionMode",
  "compactMode",
  "reducedMotion"
];

router.patch("/", authenticateToken, async (request, response, next) => {
  const accepted = Object.fromEntries(
    Object.entries(request.body || {}).filter(([key]) => allowed.includes(key))
  );

  try {
    await Promise.all(
      Object.entries(accepted).map(([key, value]) => db.query(
        `INSERT INTO settings (config_key, config_value)
         VALUES (?, ?)
         ON DUPLICATE KEY UPDATE config_value = VALUES(config_value)`,
        [key, JSON.stringify(value)]
      ))
    );

    response.json({ data: accepted });
  } catch (error) {
    next(error);
  }
});

module.exports = router;
