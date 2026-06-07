'use client';

import { Card } from '../ui/Card';
import { Device } from '@/lib/types';

interface SensorInfoProps {
  device: Device;
}

export function SensorInfo({ device }: SensorInfoProps) {
  const infoItems = [
    { label: 'Modelo', value: device.model },
    { label: 'Tipo', value: device.metadata?.['Tipo'] || 'Array Térmico IR 8x8' },
    {
      label: 'Faixa de Medição',
      value: device.metadata?.['range'] || '-20°C a 80°C',
    },
    { label: 'Precisão', value: device.metadata?.['precision'] || '±2.5°C' },
    { label: 'Interface', value: device.metadata?.['interface'] || 'I2C' },
    {
      label: 'Endereço I2C',
      value: device.metadata?.['address'] || '0x68',
    },
  ];

  return (
    <Card className="p-6 space-y-4">
      <h3 className="text-lg font-semibold text-white">Informações do Sensor</h3>

      <div className="space-y-3">
        {infoItems.map((item, index) => (
          <div key={index} className="flex justify-between items-start pb-3 border-b border-border last:pb-0 last:border-0">
            <span className="text-sm text-text-secondary">{item.label}</span>
            <span className="text-sm font-medium text-white text-right">
              {item.value}
            </span>
          </div>
        ))}
      </div>
    </Card>
  );
}
