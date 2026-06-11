const express = require("express");

const router = express.Router();

router.patch("/", (request, response) => {
  const allowed = ["refreshInterval", "deviceId", "connectionMode", "compactMode", "reducedMotion"];
  const accepted = Object.fromEntries(
    Object.entries(request.body || {}).filter(([key]) => allowed.includes(key))
  );
  response.json({
    data: accepted,
    message: "Preferências recebidas. O starter não persiste configurações no banco."
  });
});

module.exports = router;
