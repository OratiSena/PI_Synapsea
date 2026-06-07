'use client';

import { useEffect, useState } from 'react';
import { Thermometer } from 'lucide-react';
import { useDashboardStore } from '@/lib/store';
import { apiService } from '@/lib/api-service';
import { Card, Badge, Container } from '@/components/ui/Card';
import { ThermalHeatmap } from '@/components/dashboard/ThermalHeatmap';
import { TemperatureScale } from '@/components/dashboard/TemperatureScale';
import { DevicesPanel } from '@/components/dashboard/DevicesPanel';
import { SensorInfo } from '@/components/dashboard/SensorInfo';

export default function MonitoringPage() {
  const { temperature, devices, setTemperature, setDevices } =
    useDashboardStore();
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    const loadData = async () => {
      setLoading(true);
      try {
        const [tempData, devicesData] = await Promise.all([
          apiService.getTemperature(),
          apiService.getDevices(),
        ]);

        setTemperature(tempData);
        setDevices(devicesData);
      } catch (error) {
        console.error('Error loading monitoring data:', error);
      } finally {
        setLoading(false);
      }
    };

    loadData();

    // Simulando atualizações em tempo real
    const interval = setInterval(loadData, 30000);
    return () => clearInterval(interval);
  }, [setTemperature, setDevices]);

  if (loading || !temperature) {
    return (
      <Container>
        <div className="text-center py-12">
          <p className="text-text-secondary">Carregando dados...</p>
        </div>
      </Container>
    );
  }

  const amg8833Device = devices.find((d) => d.id === 'amg8833');

  return (
    <Container>
      {/* Header */}
      <div className="mb-8">
        <div className="flex items-start justify-between mb-4">
          <div>
            <div className="flex items-center gap-3 mb-2">
              <Thermometer className="w-8 h-8 text-primary" />
              <h1 className="text-3xl font-bold text-white">
                Monitoramento de Temperatura
              </h1>
            </div>
            <p className="text-text-secondary">
              Sensor AMG8833 – Mapa térmico em tempo real
            </p>
          </div>
          <Badge variant="success">Monitoramento ativo</Badge>
        </div>
      </div>

      {/* Main Content Grid */}
      <div className="grid grid-cols-4 gap-8 mb-8">
        {/* Heatmap Section */}
        <div className="col-span-3 space-y-6">
          <Card className="p-6">
            <div className="flex items-start gap-6">
              <div className="flex-1">
                <h3 className="text-lg font-semibold text-white mb-4">
                  Mapa Térmico – AMG8833
                </h3>
                <div className="bg-black/40 p-4 rounded-lg">
                  <ThermalHeatmap data={temperature} width={400} height={400} />
                </div>
              </div>

              <TemperatureScale data={temperature} height={400} />
            </div>
          </Card>

          {/* Temperature Stats */}
          <div className="grid grid-cols-3 gap-4">
            <Card className="p-6 text-center">
              <p className="text-sm text-text-secondary mb-2">
                Temperatura Máxima
              </p>
              <p className="text-3xl font-bold text-danger mb-1">
                {temperature.maxTemp}°C
              </p>
              <p className="text-xs text-text-secondary">Região Central</p>
            </Card>

            <Card className="p-6 text-center">
              <p className="text-sm text-text-secondary mb-2">
                Temperatura Atual
              </p>
              <p className="text-3xl font-bold text-primary mb-1">
                {temperature.avgTemp}°C
              </p>
              <p className="text-xs text-text-secondary">Média da Área</p>
            </Card>

            <Card className="p-6 text-center">
              <p className="text-sm text-text-secondary mb-2">
                Temperatura Mínima
              </p>
              <p className="text-3xl font-bold text-blue-400 mb-1">
                {temperature.minTemp}°C
              </p>
              <p className="text-xs text-text-secondary">Região Periférica</p>
            </Card>
          </div>
        </div>

        {/* Right Sidebar */}
        <div className="col-span-1 space-y-6">
          <DevicesPanel devices={devices} />
          {amg8833Device && <SensorInfo device={amg8833Device} />}
        </div>
      </div>
    </Container>
  );
}
