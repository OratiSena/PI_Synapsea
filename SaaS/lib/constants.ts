// Constantes da aplicação
export const APP_CONFIG = {
  APP_NAME: 'Synapsea',
  APP_VERSION: '1.0.0',
  DESCRIPTION: 'Medical SaaS Platform with Biomedical Sensors Integration',
};

export const THEME_COLORS = {
  primary: '#00D4FF',
  secondary: '#08111f',
  background: '#050816',
  card: '#0c1628',
  success: '#22c55e',
  warning: '#f59e0b',
  danger: '#ef4444',
  text: '#ffffff',
  textSecondary: '#94a3b8',
  border: 'rgba(255,255,255,0.08)',
};

export const VITAL_RANGES = {
  bpm: { min: 60, max: 100, critical: { min: 40, max: 120 } },
  spo2: { min: 95, max: 100, critical: { min: 90, max: 100 } },
  temperature: { min: 36.1, max: 37.2, critical: { min: 35, max: 39 } },
  respiration: { min: 12, max: 20, critical: { min: 8, max: 25 } },
  stressIndex: { min: 0, max: 100, critical: { min: 0, max: 100 } },
};

export const TEMPERATURE_GRID = {
  ROWS: 8,
  COLS: 8,
  CELL_SIZE: 50,
};

export const API_ENDPOINTS = {
  VITALS: '/api/vitals',
  TEMPERATURE: '/api/temperature',
  DEVICES: '/api/devices',
  ALERTS: '/api/alerts',
};

export const PERIODS = [
  { label: '1 hora', value: '1h' },
  { label: '6 horas', value: '6h' },
  { label: '24 horas', value: '24h' },
  { label: '7 dias', value: '7d' },
];

export const DEVICES_INFO = {
  AMG8833: {
    name: 'AMG8833',
    type: 'Sensor Térmico',
    model: 'Array Térmico IR 8x8',
    specs: {
      resolution: '8x8 pixels',
      fieldOfView: '60°',
      updateRate: '1 Hz',
      rangeMin: -20,
      rangeMax: 80,
      accuracy: '±2.5°C',
      interface: 'I2C',
      address: '0x68',
    },
  },
  MAX30102: {
    name: 'MAX30102',
    type: 'Sensor de Sinais Vitais',
    model: 'Sensor Óptico',
    specs: {
      measurements: ['SpO2', 'Heart Rate', 'Heart Rate Variability'],
      interface: 'I2C',
      address: '0x57',
      wavelengths: ['Red', 'IR'],
    },
  },
  ESP32: {
    name: 'ESP32',
    type: 'Microcontrolador',
    model: 'Microcontrolador Principal',
    specs: {
      processor: 'Dual-core 32-bit',
      ram: '520 KB',
      storage: '4 MB',
      connectivity: ['WiFi 802.11 b/g/n', 'Bluetooth 4.2', 'BLE'],
    },
  },
};

export const ALERT_TYPES = {
  INFO: 'info',
  SUCCESS: 'success',
  WARNING: 'warning',
  CRITICAL: 'critical',
};

export const STATUS = {
  CONNECTED: 'connected',
  DISCONNECTED: 'disconnected',
  ERROR: 'error',
};

export const ANIMATION_DURATION = {
  FAST: 150,
  NORMAL: 300,
  SLOW: 500,
};
