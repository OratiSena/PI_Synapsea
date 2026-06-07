'use client';

import { useEffect, useState } from 'react';
import { Zap } from 'lucide-react';
import { useDashboardStore } from '@/lib/store';
import { apiService } from '@/lib/api-service';
import { Container, Card } from '@/components/ui/Card';
import { DevicesPanel } from '@/components/dashboard/DevicesPanel';

export default function DevicesPage() {
  const { devices, setDevices } = useDashboardStore();
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    const loadData = async () => {
      setLoading(true);
      try {
        const devicesData = await apiService.getDevices();
        setDevices(devicesData);
      } catch (error) {
        console.error('Error loading devices:', error);
      } finally {
        setLoading(false);
      }
    };

    loadData();
  }, [setDevices]);

  return (
    <Container>
      <div className="space-y-4">
        <div className="flex items-center gap-3">
          <Zap className="w-8 h-8 text-primary" />
          <h1 className="text-3xl font-bold text-white">Dispositivos</h1>
        </div>

        <div className="grid grid-cols-2 gap-8">
          <div>
            <DevicesPanel devices={devices} />
          </div>

          <Card className="p-8">
            <h3 className="text-lg font-semibold text-white mb-4">
              Status dos Dispositivos
            </h3>
            <div className="space-y-4">
              {devices.map((device) => (
                <div key={device.id} className="p-4 bg-white/5 rounded-lg">
                  <div className="flex justify-between items-center mb-2">
                    <span className="font-medium text-white">{device.name}</span>
                    <span className="text-xs bg-success/20 text-success px-2 py-1 rounded">
                      {device.status}
                    </span>
                  </div>
                  <p className="text-sm text-text-secondary">{device.model}</p>
                </div>
              ))}
            </div>
          </Card>
        </div>
      </div>
    </Container>
  );
}
