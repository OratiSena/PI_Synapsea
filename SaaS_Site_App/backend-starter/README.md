# Synapsea Backend Starter

Base REST para integrar o frontend estático aos dados reais enviados pelo ESP32, MAX30102 e AMG8833.

## Requisitos

- Node.js 18 ou superior
- PostgreSQL

## Início rápido

1. Crie o banco `synapsea`.
2. Ative a extensão necessária para UUIDs:
   `CREATE EXTENSION IF NOT EXISTS pgcrypto;`
3. Execute `schema.sql` no banco.
4. Copie `.env.example` para `.env` e ajuste `DATABASE_URL`.
5. Execute `npm install`.
6. Execute `npm run dev`.

A API será iniciada em `http://localhost:3001/api`. O frontend deve ser aberto com Live Server em `http://127.0.0.1:5500`.

## Endpoints

- `GET /api/vitals/latest`
- `GET /api/vitals/history`
- `POST /api/vitals`
- `GET /api/temperature/latest`
- `GET /api/temperature/history`
- `POST /api/temperature`
- `GET /api/devices`
- `PUT /api/devices/:id`
- `GET /api/alerts`
- `PATCH /api/alerts/:id/read`
- `GET /api/patients`
- `POST /api/patients`
- `GET /api/reports`
- `POST /api/reports`
- `GET /api/system/status`

O starter não insere dados demonstrativos. Tabelas vazias produzem `null` ou listas vazias, permitindo que o frontend mostre seus estados de espera.

## Observação

O Synapsea tem finalidade acadêmica e educacional. Os dados e relatórios não substituem equipamentos ou avaliação médica profissional.
