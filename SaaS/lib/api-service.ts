import axios, { AxiosInstance } from 'axios';
import {
  VitalSigns,
  TemperatureData,
  Device,
  Alert,
  ApiResponse,
} from './types';
import {
  generateMockVitals,
  generateMockTemperature,
  generateMockDevices,
  generateMockAlerts,
} from './mock-data';

class ApiService {
  private api: AxiosInstance;
  private useMockData = true; // Alternar para false quando integrar com backend

  constructor() {
    this.api = axios.create({
      baseURL: process.env.NEXT_PUBLIC_API_URL || 'http://localhost:3001/api',
      timeout: 10000,
    });

    // Interceptadores
    this.api.interceptors.response.use(
      (response) => response,
      (error) => {
        console.error('API Error:', error);
        return Promise.reject(error);
      }
    );
  }

  // Métodos para Sinais Vitais
  async getVitals(period: string = '6h'): Promise<VitalSigns[]> {
    if (this.useMockData) {
      return generateMockVitals();
    }

    try {
      const response = await this.api.get<ApiResponse<VitalSigns[]>>(
        '/vitals',
        { params: { period } }
      );
      return response.data.data || [];
    } catch (error) {
      console.error('Error fetching vitals:', error);
      return generateMockVitals();
    }
  }

  async getLatestVital(): Promise<VitalSigns | null> {
    if (this.useMockData) {
      const vitals = generateMockVitals();
      return vitals[vitals.length - 1] || null;
    }

    try {
      const response = await this.api.get<ApiResponse<VitalSigns>>(
        '/vitals/latest'
      );
      return response.data.data || null;
    } catch (error) {
      console.error('Error fetching latest vital:', error);
      return null;
    }
  }

  // Métodos para Temperatura
  async getTemperature(): Promise<TemperatureData> {
    if (this.useMockData) {
      return generateMockTemperature();
    }

    try {
      const response = await this.api.get<ApiResponse<TemperatureData>>(
        '/temperature'
      );
      return response.data.data || generateMockTemperature();
    } catch (error) {
      console.error('Error fetching temperature:', error);
      return generateMockTemperature();
    }
  }

  async getTemperatureHistory(period: string = '6h'): Promise<TemperatureData[]> {
    if (this.useMockData) {
      return [generateMockTemperature()];
    }

    try {
      const response = await this.api.get<ApiResponse<TemperatureData[]>>(
        '/temperature/history',
        { params: { period } }
      );
      return response.data.data || [];
    } catch (error) {
      console.error('Error fetching temperature history:', error);
      return [];
    }
  }

  // Métodos para Dispositivos
  async getDevices(): Promise<Device[]> {
    if (this.useMockData) {
      return generateMockDevices();
    }

    try {
      const response = await this.api.get<ApiResponse<Device[]>>(
        '/devices'
      );
      return response.data.data || [];
    } catch (error) {
      console.error('Error fetching devices:', error);
      return generateMockDevices();
    }
  }

  async getDeviceStatus(deviceId: string): Promise<Device | null> {
    if (this.useMockData) {
      const devices = generateMockDevices();
      return devices.find((d) => d.id === deviceId) || null;
    }

    try {
      const response = await this.api.get<ApiResponse<Device>>(
        `/devices/${deviceId}`
      );
      return response.data.data || null;
    } catch (error) {
      console.error('Error fetching device status:', error);
      return null;
    }
  }

  // Métodos para Alertas
  async getAlerts(): Promise<Alert[]> {
    if (this.useMockData) {
      return generateMockAlerts();
    }

    try {
      const response = await this.api.get<ApiResponse<Alert[]>>(
        '/alerts'
      );
      return response.data.data || [];
    } catch (error) {
      console.error('Error fetching alerts:', error);
      return generateMockAlerts();
    }
  }

  async markAlertAsRead(alertId: string): Promise<void> {
    if (this.useMockData) return;

    try {
      await this.api.patch(`/alerts/${alertId}/read`);
    } catch (error) {
      console.error('Error marking alert as read:', error);
    }
  }

  // WebSocket para atualizações em tempo real (preparado para implementação futura)
  subscribeToUpdates(
    callback: (data: {
      type: 'vitals' | 'temperature' | 'alerts' | 'devices';
      data: any;
    }) => void
  ): () => void {
    // Implementação futura com WebSocket
    // const socket = io(process.env.NEXT_PUBLIC_API_URL);
    // socket.on('vitals', (data) => callback({ type: 'vitals', data }));
    // socket.on('temperature', (data) => callback({ type: 'temperature', data }));
    // socket.on('alerts', (data) => callback({ type: 'alerts', data }));
    // return () => socket.disconnect();

    // Por enquanto, simulamos atualizações periódicas
    const interval = setInterval(() => {
      callback({
        type: 'vitals',
        data: generateMockVitals()[generateMockVitals().length - 1],
      });
    }, 30000);

    return () => clearInterval(interval);
  }
}

export const apiService = new ApiService();
