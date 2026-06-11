USE synapsea;

-- Execute `npm run seed:users` para criar os usuarios de desenvolvimento
-- com senhas protegidas por bcrypt.

INSERT INTO patients (id, user_id, name, age, identifier, status)
VALUES (
  '20000000-0000-4000-8000-000000000001',
  NULL,
  'Paciente de desenvolvimento',
  NULL,
  'DEV-PATIENT-001',
  'active'
)
ON DUPLICATE KEY UPDATE
  user_id = VALUES(user_id),
  name = VALUES(name),
  status = VALUES(status);

INSERT INTO devices (id, name, type, status, last_sync, metadata)
VALUES
  ('esp32-001', 'Circuito ESP32', 'ESP32', 'waiting', NULL, JSON_OBJECT()),
  ('max30102-001', 'Sensor MAX30102', 'MAX30102', 'waiting', NULL, JSON_OBJECT()),
  ('amg8833-001', 'Sensor AMG8833', 'AMG8833', 'waiting', NULL, JSON_OBJECT()),
  ('ili9341-001', 'Display ILI9341', 'ILI9341', 'waiting', NULL, JSON_OBJECT())
ON DUPLICATE KEY UPDATE
  name = VALUES(name),
  type = VALUES(type),
  status = VALUES(status),
  last_sync = VALUES(last_sync),
  metadata = VALUES(metadata);

INSERT INTO device_assignments (
  id,
  device_id,
  user_id,
  patient_id,
  assigned_by,
  active
)
VALUES (
  '30000000-0000-4000-8000-000000000001',
  'esp32-001',
  NULL,
  '20000000-0000-4000-8000-000000000001',
  NULL,
  TRUE
)
ON DUPLICATE KEY UPDATE
  device_id = VALUES(device_id),
  user_id = VALUES(user_id),
  patient_id = VALUES(patient_id),
  assigned_by = VALUES(assigned_by),
  active = VALUES(active);

-- Nenhum sinal vital ou mapa termico ficticio e inserido por este seed.
