// Tipos para Sinais Vitais
export interface VitalSigns {
  timestamp: number;
  bpm: number;
  spo2: number;
  temperature: number;
  stressIndex: number;
  respiration: number;
  status: 'normal' | 'warning' | 'critical';
}

// Tipos para Temperatura (Heatmap)
export interface TemperatureData {
  timestamp: number;
  grid: number[][]; // 8x8 matrix
  maxTemp: number;
  minTemp: number;
  avgTemp: number;
  hotspotX: number;
  hotspotY: number;
}

// Tipos para Dispositivos
export interface Device {
  id: string;
  name: string;
  type: 'sensor' | 'microcontroller' | 'system';
  model: string;
  status: 'connected' | 'disconnected' | 'error';
  lastSync: number;
  metadata?: Record<string, any>;
}

// Tipos para Alertas
export interface Alert {
  id: string;
  title: string;
  description: string;
  type: 'info' | 'success' | 'warning' | 'critical';
  timestamp: number;
  read: boolean;
  icon?: string;
}

// Tipos para Paciente
export interface Patient {
  id: string;
  name: string;
  email: string;
  role: string;
  avatar?: string;
}

// Tipos para API Response
export interface ApiResponse<T> {
  success: boolean;
  data?: T;
  error?: string;
  timestamp: number;
}
