USE synapsea;

CREATE TABLE IF NOT EXISTS users (
  id CHAR(36) PRIMARY KEY DEFAULT (UUID()),
  name VARCHAR(120) NOT NULL,
  email VARCHAR(160) NOT NULL,
  password_hash VARCHAR(255) NOT NULL,
  role ENUM('admin', 'user') NOT NULL DEFAULT 'user',
  status ENUM('active', 'inactive') NOT NULL DEFAULT 'active',
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  UNIQUE INDEX idx_users_email (email)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS patients (
  id CHAR(36) PRIMARY KEY DEFAULT (UUID()),
  user_id CHAR(36) NULL,
  name VARCHAR(120) NOT NULL,
  age INT NULL,
  identifier VARCHAR(80) NOT NULL UNIQUE,
  status ENUM('active', 'inactive') NOT NULL DEFAULT 'active',
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  CONSTRAINT chk_patients_age CHECK (age IS NULL OR age BETWEEN 0 AND 130),
  CONSTRAINT fk_patients_user
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS devices (
  id VARCHAR(80) PRIMARY KEY,
  name VARCHAR(120) NOT NULL,
  type VARCHAR(80) NOT NULL,
  status ENUM('connected', 'waiting', 'offline', 'error') NOT NULL DEFAULT 'waiting',
  last_sync DATETIME NULL,
  metadata JSON NULL,
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  INDEX idx_devices_status (status)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS device_assignments (
  id CHAR(36) PRIMARY KEY DEFAULT (UUID()),
  device_id VARCHAR(80) NOT NULL,
  user_id CHAR(36) NULL,
  patient_id CHAR(36) NULL,
  assigned_by CHAR(36) NULL,
  active BOOLEAN DEFAULT TRUE,
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  INDEX idx_device_assignments_device (device_id),
  INDEX idx_device_assignments_user (user_id),
  INDEX idx_device_assignments_patient (patient_id),
  CONSTRAINT fk_device_assignments_device
    FOREIGN KEY (device_id) REFERENCES devices(id) ON DELETE CASCADE,
  CONSTRAINT fk_device_assignments_user
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE SET NULL,
  CONSTRAINT fk_device_assignments_patient
    FOREIGN KEY (patient_id) REFERENCES patients(id) ON DELETE SET NULL,
  CONSTRAINT fk_device_assignments_assigned_by
    FOREIGN KEY (assigned_by) REFERENCES users(id) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS vital_signs (
  id CHAR(36) PRIMARY KEY DEFAULT (UUID()),
  patient_id CHAR(36) NULL,
  device_id VARCHAR(80) NULL,
  timestamp DATETIME NOT NULL,
  bpm INT NULL,
  spo2 DECIMAL(5,2) NULL,
  temperature DECIMAL(5,2) NULL,
  stress_index DECIMAL(6,2) NULL,
  respiration DECIMAL(6,2) NULL,
  hrv DECIMAL(6,2) NULL,
  pi DECIMAL(6,2) NULL,
  ppg JSON NULL,
  signal_quality VARCHAR(40) NULL,
  finger_detected BOOLEAN DEFAULT FALSE,
  status VARCHAR(40) DEFAULT 'normal',
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  INDEX idx_vital_signs_timestamp (timestamp),
  INDEX idx_vital_signs_patient (patient_id),
  INDEX idx_vital_signs_device (device_id),
  CONSTRAINT fk_vital_signs_patient
    FOREIGN KEY (patient_id) REFERENCES patients(id) ON DELETE SET NULL,
  CONSTRAINT fk_vital_signs_device
    FOREIGN KEY (device_id) REFERENCES devices(id) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS temperature_readings (
  id CHAR(36) PRIMARY KEY DEFAULT (UUID()),
  patient_id CHAR(36) NULL,
  device_id VARCHAR(80) NULL,
  timestamp DATETIME NOT NULL,
  grid JSON NOT NULL,
  interpolated_grid JSON NULL,
  interpolation_width INT NULL,
  interpolation_height INT NULL,
  heatmap_pixels JSON NULL,
  heatmap_colors JSON NULL,
  max_temp DECIMAL(5,2) NULL,
  min_temp DECIMAL(5,2) NULL,
  avg_temp DECIMAL(5,2) NULL,
  hotspot_x INT NULL,
  hotspot_y INT NULL,
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  INDEX idx_temperature_readings_timestamp (timestamp),
  INDEX idx_temperature_readings_patient (patient_id),
  INDEX idx_temperature_readings_device (device_id),
  CONSTRAINT fk_temperature_readings_patient
    FOREIGN KEY (patient_id) REFERENCES patients(id) ON DELETE SET NULL,
  CONSTRAINT fk_temperature_readings_device
    FOREIGN KEY (device_id) REFERENCES devices(id) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS alerts (
  id CHAR(36) PRIMARY KEY DEFAULT (UUID()),
  patient_id CHAR(36) NULL,
  device_id VARCHAR(80) NULL,
  title VARCHAR(160) NOT NULL,
  description TEXT NULL,
  type ENUM('info', 'success', 'warning', 'critical') NOT NULL DEFAULT 'info',
  is_read BOOLEAN DEFAULT FALSE,
  timestamp DATETIME NOT NULL,
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  INDEX idx_alerts_timestamp (timestamp),
  INDEX idx_alerts_patient (patient_id),
  CONSTRAINT fk_alerts_patient
    FOREIGN KEY (patient_id) REFERENCES patients(id) ON DELETE SET NULL,
  CONSTRAINT fk_alerts_device
    FOREIGN KEY (device_id) REFERENCES devices(id) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS reports (
  id CHAR(36) PRIMARY KEY DEFAULT (UUID()),
  patient_id CHAR(36) NULL,
  title VARCHAR(160) NOT NULL,
  type VARCHAR(80) NOT NULL,
  content JSON NULL,
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  CONSTRAINT fk_reports_patient
    FOREIGN KEY (patient_id) REFERENCES patients(id) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS settings (
  id INT AUTO_INCREMENT PRIMARY KEY,
  config_key VARCHAR(120) NOT NULL UNIQUE,
  config_value JSON NULL,
  updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
