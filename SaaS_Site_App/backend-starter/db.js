const { Pool } = require("pg");

const pool = new Pool({
  connectionString: process.env.DATABASE_URL
});

pool.on("error", (error) => {
  console.error("Erro inesperado no pool PostgreSQL:", error);
});

module.exports = {
  query(text, params) {
    return pool.query(text, params);
  },
  pool
};
