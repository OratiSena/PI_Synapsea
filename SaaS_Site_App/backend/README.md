# Synapsea Backend

API REST educacional em Node.js, Express e MySQL para dados do ESP32,
MAX30102 e AMG8833.

## Requisitos

- Node.js 18 ou superior
- MySQL 8.0.13 ou superior
- npm

## Instalacao

1. Inicie o MySQL local.
2. Execute, nesta ordem:

```cmd
mysql -u root -p < sql/01_create_database.sql
mysql -u root -p synapsea < sql/02_create_tables.sql
mysql -u root -p synapsea < sql/03_seed_initial_data.sql
mysql -u root -p synapsea < sql/05_auth_adjustments.sql
mysql -u root -p synapsea < sql/06_auth_and_insights.sql
mysql -u root -p synapsea < sql/07_temperature_interpolation.sql
```

3. Copie `.env.example` para `.env` e ajuste as credenciais.
4. Instale as dependencias:

```cmd
npm install
```

5. Crie os usuarios locais com senhas bcrypt:

```cmd
npm run seed:users
```

6. Inicie a API:

```cmd
npm run dev
```

7. Acesse o site em `http://localhost:3001`.
8. Acesse a API em `http://localhost:3001/api`.

O Express serve os arquivos estáticos da pasta `../public`. Não é necessário
usar Live Server.

## Credenciais locais

Administrador:

- Email: `admin@synapsea.local`
- Senha: `123456`

Usuario:

- Email: `user@synapsea.local`
- Senha: `123456`

Codigo para cadastro de administrador: `123456`

Essas credenciais e segredos sao exclusivos para desenvolvimento local
academico. Altere todos os valores antes de qualquer implantacao.

## Autenticacao

- `POST /api/auth/register`
- `POST /api/auth/login`
- `GET /api/auth/me`
- `GET /api/auth/users` (admin)
- `POST /api/auth/logout`

Envie o JWT em `Authorization: Bearer TOKEN`.

## Monitoramento

- `GET /api`
- `GET /api/system/status`
- `GET /api/vitals/latest`
- `GET /api/vitals/history`
- `POST /api/vitals`
- `GET /api/temperature/latest`
- `GET /api/temperature/history`
- `POST /api/temperature`
- `GET /api/insights/latest`

Os endpoints de leitura aceitam `patientId` e `deviceId`. Requisicoes
autenticadas sao filtradas pelas permissoes da conta. O envio dos sensores
permanece temporariamente compativel sem JWT para futura autenticacao propria
do ESP32.

O endpoint de temperatura aceita a matriz bruta `grid` 8x8 e, opcionalmente,
`interpolatedGrid`, `interpolationWidth`, `interpolationHeight`,
`heatmapPixels` e `heatmapColors`. Clientes antigos continuam compatíveis
usando apenas `grid`.

## Gestao

- `GET /api/devices`
- `GET /api/devices/assignments`
- `PUT /api/devices/:id`
- `POST /api/devices/:id/assign`
- `GET /api/patients`
- `POST /api/patients`
- `GET /api/alerts`
- `PATCH /api/alerts/:id/read`
- `GET /api/reports`
- `POST /api/reports`
- `POST /api/reports/generate`
- `PATCH /api/settings`

Respostas de sucesso usam `{ "data": ... }`. Erros usam
`{ "error": "mensagem" }`.

Alertas, insights e relatorios usam linguagem educacional e nao representam
diagnostico, tratamento ou avaliacao medica profissional.
