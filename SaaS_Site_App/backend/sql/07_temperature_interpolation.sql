USE synapsea;

DROP PROCEDURE IF EXISTS add_temperature_column_if_missing;

DELIMITER //
CREATE PROCEDURE add_temperature_column_if_missing(
  IN column_name_value VARCHAR(64),
  IN column_definition_value TEXT
)
BEGIN
  IF NOT EXISTS (
    SELECT 1
    FROM information_schema.COLUMNS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'temperature_readings'
      AND COLUMN_NAME = column_name_value
  ) THEN
    SET @statement = CONCAT(
      'ALTER TABLE temperature_readings ADD COLUMN `',
      column_name_value, '` ', column_definition_value
    );
    PREPARE migration_statement FROM @statement;
    EXECUTE migration_statement;
    DEALLOCATE PREPARE migration_statement;
  END IF;
END//
DELIMITER ;

CALL add_temperature_column_if_missing('interpolated_grid', 'JSON NULL AFTER grid');
CALL add_temperature_column_if_missing('interpolation_width', 'INT NULL AFTER interpolated_grid');
CALL add_temperature_column_if_missing('interpolation_height', 'INT NULL AFTER interpolation_width');
CALL add_temperature_column_if_missing('heatmap_pixels', 'JSON NULL AFTER interpolation_height');
CALL add_temperature_column_if_missing('heatmap_colors', 'JSON NULL AFTER heatmap_pixels');

DROP PROCEDURE IF EXISTS add_temperature_column_if_missing;
