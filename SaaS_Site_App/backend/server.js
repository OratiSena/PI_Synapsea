require("dotenv").config();

const cors = require("cors");
const express = require("express");
const vitalsRoutes = require("./routes/vitals.routes");
const temperatureRoutes = require("./routes/temperature.routes");
const devicesRoutes = require("./routes/devices.routes");
const alertsRoutes = require("./routes/alerts.routes");
const patientsRoutes = require("./routes/patients.routes");
const reportsRoutes = require("./routes/reports.routes");
const systemRoutes = require("./routes/system.routes");
const settingsRoutes = require("./routes/settings.routes");
const authRoutes = require("./routes/auth.routes");
const insightsRoutes = require("./routes/insights.routes");

const app = express();
const port = Number(process.env.PORT) || 3001;

app.use(cors({
  origin: process.env.FRONTEND_URL || "http://127.0.0.1:5500"
}));
app.use(express.json({ limit: "1mb" }));

app.get("/api", (_request, response) => {
  response.json({
    data: {
      name: "Synapsea API",
      version: "1.0.0",
      database: "MySQL",
      message: "API preparada para receber dados reais do circuito Synapsea."
    }
  });
});

app.use("/api/vitals", vitalsRoutes);
app.use("/api/temperature", temperatureRoutes);
app.use("/api/devices", devicesRoutes);
app.use("/api/alerts", alertsRoutes);
app.use("/api/patients", patientsRoutes);
app.use("/api/reports", reportsRoutes);
app.use("/api/system", systemRoutes);
app.use("/api/settings", settingsRoutes);
app.use("/api/auth", authRoutes);
app.use("/api/insights", insightsRoutes);

app.use((_request, response) => {
  response.status(404).json({ error: "Endpoint nao encontrado." });
});

app.use((error, _request, response, _next) => {
  console.error(error);
  const duplicateEntry = error?.code === "ER_DUP_ENTRY";
  response.status(error.status || (duplicateEntry ? 409 : 500)).json({
    error: duplicateEntry
      ? "Ja existe um registro com um valor unico informado."
      : "Nao foi possivel processar a solicitacao.",
    detail: process.env.NODE_ENV === "development" ? error.message : undefined
  });
});

app.listen(port, () => {
  console.log(`Synapsea API disponivel em http://localhost:${port}/api`);
});
