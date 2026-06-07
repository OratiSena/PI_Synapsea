'use client';

import { create } from 'zustand';
import { devtools } from 'zustand/middleware';
import { VitalSigns, TemperatureData, Device, Alert } from './types';

interface DashboardStore {
  vitals: VitalSigns[];
  temperature: TemperatureData | null;
  devices: Device[];
  alerts: Alert[];
  selectedPeriod: '1h' | '6h' | '24h' | '7d';
  theme: 'dark' | 'light';

  setVitals: (vitals: VitalSigns[]) => void;
  addVital: (vital: VitalSigns) => void;
  setTemperature: (data: TemperatureData) => void;
  setDevices: (devices: Device[]) => void;
  addDevice: (device: Device) => void;
  setAlerts: (alerts: Alert[]) => void;
  addAlert: (alert: Alert) => void;
  markAlertAsRead: (alertId: string) => void;
  setPeriod: (period: '1h' | '6h' | '24h' | '7d') => void;
  toggleTheme: () => void;
}

export const useDashboardStore = create<DashboardStore>()(
  devtools((set) => ({
    vitals: [],
    temperature: null,
    devices: [],
    alerts: [],
    selectedPeriod: '6h',
    theme: 'dark',

    setVitals: (vitals) => set({ vitals }),
    addVital: (vital) =>
      set((state) => ({
        vitals: [...state.vitals, vital].slice(-100),
      })),
    setTemperature: (data) => set({ temperature: data }),
    setDevices: (devices) => set({ devices }),
    addDevice: (device) =>
      set((state) => ({
        devices: [...state.devices, device],
      })),
    setAlerts: (alerts) => set({ alerts }),
    addAlert: (alert) =>
      set((state) => ({
        alerts: [alert, ...state.alerts].slice(0, 20),
      })),
    markAlertAsRead: (alertId) =>
      set((state) => ({
        alerts: state.alerts.map((alert) =>
          alert.id === alertId ? { ...alert, read: true } : alert
        ),
      })),
    setPeriod: (period) => set({ selectedPeriod: period }),
    toggleTheme: () =>
      set((state) => ({
        theme: state.theme === 'dark' ? 'light' : 'dark',
      })),
  }))
);
