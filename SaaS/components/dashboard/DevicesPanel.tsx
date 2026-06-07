'use client';

import { Card } from '../ui/Card';
import { Device } from '@/lib/types';
import { Zap, Thermometer, Cpu, Activity } from 'lucide-react';

interface DevicesPanelProps {
  devices: Device[];
}

const deviceIcons: Record<string, React.ComponentType<{ className: string }>> = {
  amg8833: Thermometer,
  max30102: Activity,
  esp32: Cpu,
  system: Zap,
};

export function DevicesPanel({ devices }: DevicesPanelProps) {
  return (
    <Card className="p-6 space-y-4">
      <h3 className="text-lg font-semibold text-white">Dispositivos Conectados</h3>

      <div className="space-y-3">
        {devices.map((device) => {
          const IconComponent = deviceIcons[device.id] || Zap;

          return (
            <div
              key={device.id}
              className="flex items-center justify-between p-3 bg-white/5 rounded-lg border border-border hover:bg-white/10 transition-colors"
            >
              <div className="flex items-center gap-3">
                <div className="p-2 bg-primary/20 rounded-lg">
                  <IconComponent className="w-4 h-4 text-primary" />
                </div>
                <div>
                  <p className="text-sm font-medium text-white">{device.name}</p>
                  <p className="text-xs text-text-secondary">{device.type}</p>
                </div>
              </div>

              <div className="flex items-center gap-2">
                <div className="w-2 h-2 bg-success rounded-full animate-pulse-glow" />
                <span className="text-xs text-success font-medium">
                  {device.status === 'connected' ? 'Conectado' : 'Desconectado'}
                </span>
              </div>
            </div>
          );
        })}
      </div>
    </Card>
  );
}
