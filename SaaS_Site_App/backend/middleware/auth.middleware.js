const jwt = require("jsonwebtoken");

function readToken(request) {
  const header = request.headers.authorization || "";
  return header.startsWith("Bearer ") ? header.slice(7).trim() : null;
}

function verifyToken(token) {
  return jwt.verify(token, process.env.JWT_SECRET || "synapsea_local_secret_2026");
}

function optionalAuthenticate(request, response, next) {
  const token = readToken(request);
  if (!token) return next();
  try {
    request.user = verifyToken(token);
  } catch (_error) {
    return response.status(401).json({ error: "Token de acesso invalido ou expirado." });
  }
  next();
}

function authenticateToken(request, response, next) {
  const token = readToken(request);
  if (!token) {
    return response.status(401).json({ error: "Token de acesso nao informado." });
  }
  try {
    request.user = verifyToken(token);
    next();
  } catch (_error) {
    response.status(401).json({ error: "Token de acesso invalido ou expirado." });
  }
}

function requireAdmin(request, response, next) {
  if (request.user?.role !== "admin") {
    return response.status(403).json({ error: "Acesso exclusivo para administradores." });
  }
  next();
}

module.exports = {
  optionalAuthenticate,
  authenticateToken,
  requireAdmin
};
