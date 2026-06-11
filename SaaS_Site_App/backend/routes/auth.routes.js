const bcrypt = require("bcryptjs");
const express = require("express");
const jwt = require("jsonwebtoken");
const db = require("../db");
const { authenticateToken, requireAdmin } = require("../middleware/auth.middleware");

const router = express.Router();
const userColumns = `
  id, name, email, role, status,
  created_at AS createdAt, updated_at AS updatedAt`;

function createToken(user) {
  return jwt.sign(
    { id: user.id, name: user.name, email: user.email, role: user.role },
    process.env.JWT_SECRET || "synapsea_local_secret_2026",
    { expiresIn: process.env.JWT_EXPIRES_IN || "8h" }
  );
}

router.post("/register", async (request, response, next) => {
  const name = String(request.body?.name || "").trim();
  const email = String(request.body?.email || "").trim().toLowerCase();
  const password = String(request.body?.password || "");
  const role = request.body?.role === "admin" ? "admin" : "user";

  if (!name || !email || password.length < 6) {
    return response.status(400).json({
      error: "Nome, email e senha com no minimo 6 caracteres sao obrigatorios."
    });
  }
  if (role === "admin" && request.body?.adminCode !== process.env.ADMIN_INVITE_CODE) {
    return response.status(403).json({ error: "Codigo de administrador invalido." });
  }

  try {
    const existing = await db.query("SELECT id FROM users WHERE email = ?", [email]);
    if (existing[0]) {
      return response.status(409).json({ error: "Este email ja esta cadastrado." });
    }
    const idRows = await db.query("SELECT UUID() AS id");
    const id = idRows[0].id;
    const passwordHash = await bcrypt.hash(password, 12);
    await db.query(
      `INSERT INTO users (id, name, email, password_hash, role, status)
       VALUES (?, ?, ?, ?, ?, 'active')`,
      [id, name, email, passwordHash, role]
    );
    const users = await db.query(`SELECT ${userColumns} FROM users WHERE id = ?`, [id]);
    const user = users[0];
    response.status(201).json({ data: { token: createToken(user), user } });
  } catch (error) {
    next(error);
  }
});

router.post("/login", async (request, response, next) => {
  const email = String(request.body?.email || "").trim().toLowerCase();
  const password = String(request.body?.password || "");
  if (!email || !password) {
    return response.status(400).json({ error: "Email e senha sao obrigatorios." });
  }
  try {
    const rows = await db.query(
      `SELECT id, name, email, password_hash, role, status,
        created_at AS createdAt, updated_at AS updatedAt
       FROM users WHERE email = ? LIMIT 1`,
      [email]
    );
    const user = rows[0];
    if (!user || !(await bcrypt.compare(password, user.password_hash))) {
      return response.status(401).json({ error: "Email ou senha invalidos." });
    }
    if (user.status !== "active") {
      return response.status(403).json({ error: "Conta de usuario inativa." });
    }
    delete user.password_hash;
    response.json({ data: { token: createToken(user), user } });
  } catch (error) {
    next(error);
  }
});

router.get("/me", authenticateToken, async (request, response, next) => {
  try {
    const rows = await db.query(`SELECT ${userColumns} FROM users WHERE id = ?`, [
      request.user.id
    ]);
    if (!rows[0]) return response.status(404).json({ error: "Usuario nao encontrado." });
    response.json({ data: rows[0] });
  } catch (error) {
    next(error);
  }
});

router.get("/users", authenticateToken, requireAdmin, async (_request, response, next) => {
  try {
    const rows = await db.query(`SELECT ${userColumns} FROM users ORDER BY name`);
    response.json({ data: rows });
  } catch (error) {
    next(error);
  }
});

router.post("/logout", authenticateToken, (_request, response) => {
  response.json({ data: { success: true } });
});

module.exports = router;
