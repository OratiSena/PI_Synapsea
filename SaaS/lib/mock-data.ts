import { VitalSigns, TemperatureData, Device, Alert } from './types';

// Mock data para Sinais Vitais
export const generateMockVitals = (): VitalSigns[] => {
  const vitals: VitalSigns[] = [];
  const now = Date.now();

  for (let i = 0; i < 24; i++) {
    vitals.push({
      timestamp: now - (24 - i) * 15 * 60 * 1000,
      bpm: 78 + Math.sin(i / 4) * 10 + (Math.random() - 0.5) * 5,
      spo2: 98 + (Math.random() - 0.5) * 2,
      temperature: 36.8 + Math.sin(i / 6) * 0.5 + (Math.random() - 0.5) * 0.3,
      stressIndex: 32 + Math.sin(i / 3) * 8 + (Math.random() - 0.5) * 3,
      respiration: 16 + Math.sin(i / 5) * 2 + (Math.random() - 0.5) * 1,
      status: 'normal',
    });
  }

  return vitals;
};

// Mock data para Temperatura (8x8 grid)
export const generateMockTemperature = (): TemperatureData => {
  const grid: number[][] = [];
  const centerX = 4;
  const centerY = 4;

  for (let i = 0; i < 8; i++) {
    const row: number[] = [];
    for (let j = 0; j < 8; j++) {
      const distance = Math.sqrt(Math.pow(i - centerX, 2) + Math.pow(j - centerY, 2));
      const temp = 20 + Math.max(0, 17.2 - distance * 1.5) + (Math.random() - 0.5) * 1;
      row.push(Math.round(temp * 10) / 10);
    }
    grid.push(row);
  }

  const temps = grid.flat();
  const maxTemp = Math.max(...temps);
  const minTemp = Math.min(...temps);
  const avgTemp = temps.reduce((a, b) => a + b) / temps.length;

  return {
    timestamp: Date.now(),
    grid,
    maxTemp: 37.2,
    minTemp: 20.0,
    avgTemp: 26.3,
    hotspotX: centerX,
    hotspotY: centerY,
  };
};

// Mock data para Dispositivos
export const generateMockDevices = (): Device[] => [
  {
    id: 'amg8833',
    name: 'AMG8833',
    type: 'sensor',
    model: 'Array Térmico IR 8x8',
    status: 'connected',
    lastSync: Date.now(),
    metadata: {
      range: '-20°C a 80°C',
      precision: '±2.5°C',
      interface: 'I2C',
      address: '0x68',
    },
  },
  {
    id: 'max30102',
    name: 'MAX30102',
    type: 'sensor',
    model: 'Sensor de Sinais Vitais',
    status: 'connected',
    lastSync: Date.now(),
    metadata: {
      readings: ['SpO2', 'BPM', 'Heart Rate Variability'],
    },
  },
  {
    id: 'esp32',
    name: 'ESP32',
    type: 'microcontroller',
    model: 'Microcontrolador Principal',
    status: 'connected',
    lastSync: Date.now(),
    metadata: {
      ram: '520KB',
      storage: '4MB',
      connectivity: 'WiFi, Bluetooth',
    },
  },
  {
    id: 'system',
    name: 'Sistema',
    type: 'system',
    model: 'Plataforma Synapsea',
    status: 'connected',
    lastSync: Date.now(),
    metadata: {
      version: '1.0.0',
      uptime: '24h',
      status: 'Funcionando normalmente',
    },
  },
];

// Mock data para Alertas
export const generateMockAlerts = (): Alert[] => [
  {
    id: '1',
    title: 'Todos os sinais estáveis',
    description: 'Não há alterações significativas',
    type: 'success',
    timestamp: Date.now() - 5 * 60 * 1000,
    read: false,
    icon: 'CheckCircle2',
  },
  {
    id: '2',
    title: 'Índice de estresse elevado',
    description: 'Valor momentaneamente alto',
    type: 'warning',
    timestamp: Date.now() - 15 * 60 * 1000,
    read: false,
    icon: 'AlertCircle',
  },
  {
    id: '3',
    title: 'SpO₂ normalizado',
    description: 'Níveis de oxigenação estáveis',
    type: 'success',
    timestamp: Date.now() - 47 * 60 * 1000,
    read: false,
    icon: 'CheckCircle2',
  },
  {
    id: '4',
    title: 'Batimentos acima do normal',
    description: 'Pico detectado e normalizado',
    type: 'warning',
    timestamp: Date.now() - 1 * 60 * 60 * 1000,
    read: true,
    icon: 'Heart',
  },
  {
    id: '5',
    title: 'Temperatura estável',
    description: 'Sem variações relevantes',
    type: 'info',
    timestamp: Date.now() - 6 * 60 * 60 * 1000,
    read: true,
    icon: 'Thermometer',
  },
];

// Mock data para Usuário
export const mockCurrentUser = {
  id: 'user-123',
  name: 'Nelis',
  email: 'nelis@synapsea.com',
  role: 'Educadora',
  avatar: 'NE',
};
