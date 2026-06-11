require("dotenv").config();

const bcrypt = require("bcryptjs");
const db = require("../db");

const users = [
  {
    id: "10000000-0000-4000-8000-000000000001",
    name: "Administrador Synapsea",
    email: "admin@synapsea.local",
    role: "admin"
  },
  {
    id: "10000000-0000-4000-8000-000000000002",
    name: "Usuario Synapsea",
    email: "user@synapsea.local",
    role: "user"
  }
];

async function seed() {
  const passwordHash = await bcrypt.hash("123456", 12);
  for (const user of users) {
    await db.query(
      `INSERT INTO users (id, name, email, password_hash, role, status)
       VALUES (?, ?, ?, ?, ?, 'active')
       ON DUPLICATE KEY UPDATE
         name = VALUES(name),
         email = VALUES(email),
         password_hash = VALUES(password_hash),
         role = VALUES(role),
         status = 'active'`,
      [user.id, user.name, user.email, passwordHash, user.role]
    );
  }
  await db.query(
    `UPDATE patients SET user_id = ?
     WHERE identifier = 'DEV-PATIENT-001' AND user_id IS NULL`,
    [users[1].id]
  );
  await db.query(
    `UPDATE device_assignments da
     JOIN patients p ON p.id = da.patient_id
     SET da.user_id = p.user_id
     WHERE da.user_id IS NULL AND p.user_id IS NOT NULL`
  );
  console.log("Usuarios de desenvolvimento criados ou atualizados.");
}

seed()
  .catch((error) => {
    console.error(error);
    process.exitCode = 1;
  })
  .finally(() => db.pool.end());
