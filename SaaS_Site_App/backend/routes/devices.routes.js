const express = require("express");
const db = require("../db");
const {
  optionalAuthenticate,
  authenticateToken
} = require("../middleware/auth.middleware");
const { toMysqlDateTime } = require("../utils/date");

const router = express.Router();
const columns = `
  d.id, d.name, d.type, d.status, d.last_sync AS lastSync, d.metadata,
  d.created_at AS createdAt, d.updated_at AS updatedAt,
  da.id AS assignmentId, da.user_id AS userId, da.patient_id AS patientId,
  u.name AS userName, p.name AS patientName`;

router.get("/assignments", authenticateToken, async (request, response, next) => {
  try {
    const where = request.user.role === "admin"
      ? ""
      : "WHERE da.user_id = ?";
    const params = request.user.role === "admin" ? [] : [request.user.id];
    const rows = await db.query(
      `SELECT da.id, da.device_id AS deviceId, da.user_id AS userId,
        da.patient_id AS patientId, da.assigned_by AS assignedBy,
        da.active, da.created_at AS createdAt, d.name AS deviceName,
        u.name AS userName, p.name AS patientName
       FROM device_assignments da
       JOIN devices d ON d.id = da.device_id
       LEFT JOIN users u ON u.id = da.user_id
       LEFT JOIN patients p ON p.id = da.patient_id
       ${where}
       ORDER BY da.created_at DESC`,
      params
    );
    response.json({ data: rows });
  } catch (error) {
    next(error);
  }
});

router.get("/", optionalAuthenticate, async (request, response, next) => {
  try {
    const where = request.user?.role === "user"
      ? "WHERE da.user_id = ? AND da.active = TRUE"
      : "";
    const params = request.user?.role === "user" ? [request.user.id] : [];
    const rows = await db.query(
      `SELECT ${columns}
       FROM devices d
       LEFT JOIN device_assignments da ON da.device_id = d.id AND da.active = TRUE
       LEFT JOIN users u ON u.id = da.user_id
       LEFT JOIN patients p ON p.id = da.patient_id
       ${where}
       ORDER BY d.name`,
      params
    );
    response.json({ data: rows });
  } catch (error) {
    next(error);
  }
});

router.put("/:id", async (request, response, next) => {
  const data = request.body || {};
  if (!data.name || !data.type) {
    return response.status(400).json({ error: "Nome e tipo sao obrigatorios." });
  }
  try {
    await db.query(
      `INSERT INTO devices (id, name, type, status, last_sync, metadata)
       VALUES (?, ?, ?, ?, ?, ?)
       ON DUPLICATE KEY UPDATE
         name = VALUES(name), type = VALUES(type), status = VALUES(status),
         last_sync = VALUES(last_sync), metadata = VALUES(metadata)`,
      [
        request.params.id, data.name, data.type, data.status ?? "waiting",
        toMysqlDateTime(data.lastSync) || toMysqlDateTime(new Date()),
        JSON.stringify(data.metadata ?? {})
      ]
    );
    const rows = await db.query(
      `SELECT id, name, type, status, last_sync AS lastSync, metadata,
       created_at AS createdAt, updated_at AS updatedAt
       FROM devices WHERE id = ?`,
      [request.params.id]
    );
    response.json({ data: rows[0] });
  } catch (error) {
    next(error);
  }
});

router.post("/:id/assign", authenticateToken, async (request, response, next) => {
  const requestedUserId = request.body?.userId ?? request.user.id;
  const patientId = request.body?.patientId ?? null;
  if (request.user.role !== "admin" && requestedUserId !== request.user.id) {
    return response.status(403).json({ error: "Voce so pode vincular um dispositivo a sua conta." });
  }
  try {
    const activeAssignments = await db.query(
      `SELECT user_id AS userId FROM device_assignments
       WHERE device_id = ? AND active = TRUE LIMIT 1`,
      [request.params.id]
    );
    if (
      request.user.role !== "admin"
      && activeAssignments[0]
      && activeAssignments[0].userId !== request.user.id
    ) {
      return response.status(403).json({
        error: "Este dispositivo ja esta vinculado a outra conta."
      });
    }
    if (patientId) {
      const patients = await db.query(
        "SELECT user_id AS userId FROM patients WHERE id = ?",
        [patientId]
      );
      if (!patients[0]) return response.status(404).json({ error: "Paciente nao encontrado." });
      if (request.user.role !== "admin" && patients[0].userId !== request.user.id) {
        return response.status(403).json({ error: "Paciente nao pertence ao usuario logado." });
      }
    }
    await db.query(
      "UPDATE device_assignments SET active = FALSE WHERE device_id = ? AND active = TRUE",
      [request.params.id]
    );
    const idRows = await db.query("SELECT UUID() AS id");
    const id = idRows[0].id;
    await db.query(
      `INSERT INTO device_assignments (
        id, device_id, user_id, patient_id, assigned_by, active
       ) VALUES (?, ?, ?, ?, ?, TRUE)`,
      [id, request.params.id, requestedUserId, patientId, request.user.id]
    );
    const rows = await db.query(
      `SELECT id, device_id AS deviceId, user_id AS userId,
       patient_id AS patientId, assigned_by AS assignedBy,
       active, created_at AS createdAt
       FROM device_assignments WHERE id = ?`,
      [id]
    );
    response.status(201).json({ data: rows[0] });
  } catch (error) {
    next(error);
  }
});

module.exports = router;
