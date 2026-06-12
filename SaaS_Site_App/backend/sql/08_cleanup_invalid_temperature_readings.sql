-- Remove somente leituras termicas claramente invalidas.
-- Nao apaga usuarios, pacientes, dispositivos ou sinais vitais.
DELETE FROM temperature_readings
WHERE min_temp < -20
   OR min_temp > 120
   OR avg_temp < -20
   OR avg_temp > 120
   OR max_temp < -20
   OR max_temp > 120
   OR min_temp IS NULL
   OR avg_temp IS NULL
   OR max_temp IS NULL;

-- Limpeza total opcional para ambiente de teste.
-- Execute manualmente somente quando quiser apagar todo o historico termico:
-- DELETE FROM temperature_readings;
