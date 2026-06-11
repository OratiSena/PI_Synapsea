CREATE EXTENSION IF NOT EXISTS pgcrypto;

CREATE TABLE IF NOT EXISTS patients (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  name VARCHAR(120) NOT NULL,
  age SMALLINT CHECK (age BETWEEN 0 AND 130),
  identifier VARCHAR(60) UNIQUE NOT NULL,
  status VARCHAR(30) NOT NULL DEFAULT 'active',
  created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS vital_signs (
  id BIGSERIAL PRIMARY KEY,
  patient_id UUID REFERENCES patients(id) ON DELETE SET NULL,
  timestamp TIMESTAMPTZ NOT NULL DEFAULT NOW(),
  bpm NUMERIC(6, 2),
  spo2 NUMERIC(5, 2),
  temperature NUMERIC(5, 2),
  stress_index NUMERIC(7, 2),
  respiration NUMERIC(6, 2),
  hrv NUMERIC(8, 2),
  pi NUMERIC(8, 3),
  ppg JSONB NOT NULL DEFAULT '[]'::jsonb,
  signal_quality VARCHAR(30),
  finger_detected BOOLEAN,
  status VARCHAR(30),
  created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS temperature_readings (
  id BIGSERIAL PRIMARY KEY,
  patient_id UUID REFERENCES patients(id) ON DELETE SET NULL,
  timestamp TIMESTAMPTZ NOT NULL DEFAULT NOW(),
  grid JSONB NOT NULL,
  max_temp NUMERIC(5, 2),
  min_temp NUMERIC(5, 2),
  avg_temp NUMERIC(5, 2),
  hotspot_x SMALLINT,
  hotspot_y SMALLINT,
  created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
  CONSTRAINT grid_is_array CHECK (jsonb_typeof(grid) = 'array')
);

CREATE TABLE IF NOT EXISTS devices (
  id VARCHAR(80) PRIMARY KEY,
  name VARCHAR(120) NOT NULL,
  type VARCHAR(60) NOT NULL,
  status VARCHAR(30) NOT NULL DEFAULT 'waiting',
  last_sync TIMESTAMPTZ,
  metadata JSONB NOT NULL DEFAULT '{}'::jsonb,
  created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS alerts (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  title VARCHAR(160) NOT NULL,
  description TEXT,
  type VARCHAR(30) NOT NULL DEFAULT 'info',
  read BOOLEAN NOT NULL DEFAULT FALSE,
  timestamp TIMESTAMPTZ NOT NULL DEFAULT NOW(),
  created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS reports (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  patient_id UUID REFERENCES patients(id) ON DELETE SET NULL,
  title VARCHAR(160) NOT NULL,
  type VARCHAR(60) NOT NULL,
  content JSONB NOT NULL DEFAULT '{}'::jsonb,
  created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS vital_signs_timestamp_idx ON vital_signs (timestamp DESC);
CREATE INDEX IF NOT EXISTS vital_signs_patient_idx ON vital_signs (patient_id, timestamp DESC);
CREATE INDEX IF NOT EXISTS temperature_timestamp_idx ON temperature_readings (timestamp DESC);
CREATE INDEX IF NOT EXISTS alerts_timestamp_idx ON alerts (timestamp DESC);
