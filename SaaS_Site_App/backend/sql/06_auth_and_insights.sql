USE synapsea;

DROP PROCEDURE IF EXISTS add_column_if_missing;
DROP PROCEDURE IF EXISTS add_index_if_missing;

DELIMITER //
CREATE PROCEDURE add_column_if_missing(
  IN table_name_value VARCHAR(64),
  IN column_name_value VARCHAR(64),
  IN column_definition_value TEXT
)
BEGIN
  IF NOT EXISTS (
    SELECT 1
    FROM information_schema.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = table_name_value
      AND COLUMN_NAME = column_name_value
  ) THEN
    SET @statement = CONCAT(
      'ALTER TABLE `', table_name_value, '` ADD COLUMN `',
      column_name_value, '` ', column_definition_value
    );
    PREPARE migration_statement FROM @statement;
    EXECUTE migration_statement;
    DEALLOCATE PREPARE migration_statement;
  END IF;
END//
CREATE PROCEDURE add_index_if_missing(
  IN table_name_value VARCHAR(64),
  IN index_name_value VARCHAR(64),
  IN index_columns_value VARCHAR(255)
)
BEGIN
  IF NOT EXISTS (
    SELECT 1
    FROM information_schema.STATISTICS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = table_name_value
      AND INDEX_NAME = index_name_value
  ) THEN
    SET @statement = CONCAT(
      'CREATE INDEX `', index_name_value, '` ON `',
      table_name_value, '` (', index_columns_value, ')'
    );
    PREPARE migration_statement FROM @statement;
    EXECUTE migration_statement;
    DEALLOCATE PREPARE migration_statement;
  END IF;
END//
DELIMITER ;

CALL add_column_if_missing('alerts', 'patient_id', 'CHAR(36) NULL');
CALL add_column_if_missing('alerts', 'device_id', 'VARCHAR(80) NULL');
CALL add_column_if_missing('alerts', 'is_read', 'BOOLEAN DEFAULT FALSE');
CALL add_index_if_missing('alerts', 'idx_alerts_device', '`device_id`');
CALL add_index_if_missing('device_assignments', 'idx_assignments_active_user', '`active`, `user_id`');
CALL add_index_if_missing('reports', 'idx_reports_patient_created', '`patient_id`, `created_at`');

ALTER TABLE reports MODIFY COLUMN content JSON NULL;

DROP PROCEDURE IF EXISTS add_column_if_missing;
DROP PROCEDURE IF EXISTS add_index_if_missing;

-- O servico de insights usa as leituras existentes e nao exige dados simulados.
