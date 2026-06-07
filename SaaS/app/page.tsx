'use client';

import { useEffect, useState } from 'react';
import {
  Heart,
  Wind,
  Thermometer,
  Brain,
  Zap,
  CheckCircle2,
} from 'lucide-react';
import { useDashboardStore } from '@/lib/store';
import { apiService } from '@/lib/api-service';
import { VitalIndicator } from '@/components/dashboard/VitalIndicator';
import { VitalSignsChart } from '@/components/dashboard/VitalSignsChart';
import { AlertsList } from '@/components/dashboard/AlertsList';
import { Container } from '@/components/ui/Card';

export default function DashboardPage() {
  const { vitals, alerts, selectedPeriod, setVitals, setAlerts } =
    useDashboardStore();
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    const loadData = async () => {
      setLoading(true);
      try {
        const [vitalsData, alertsData] = await Promise.all([
          apiService.getVitals(selectedPeriod),
          apiService.getAlerts(),
        ]);

        setVitals(vitalsData);
        setAlerts(alertsData);
      } catch (error) {
        console.error('Error loading dashboard data:', error);
      } finally {
        setLoading(false);
      }
    };

    loadData();
  }, [selectedPeriod, setVitals, setAlerts]);

  const latestVital = vitals[vitals.length - 1];

  return (
    <Container>
      {/* Indicadores Vitais */}
      <div className="grid grid-cols-6 gap-4 mb-8">
        <VitalIndicator
          icon={<Heart className="w-6 h-6" />}
          label="Batimentos"
          value={latestVital?.bpm.toFixed(0) || '--'}
          unit="BPM"
          status={latestVital?.status || 'normal'}
          color="red"
        />
        <VitalIndicator
          icon={<Wind className="w-6 h-6" />}
          label="SpO₂"
          value={latestVital?.spo2.toFixed(0) || '--'}
          unit="%"
          status={latestVital?.status || 'normal'}
          color="blue"
        />
        <VitalIndicator
          icon={<Thermometer className="w-6 h-6" />}
          label="Temperatura"
          value={latestVital?.temperature.toFixed(1) || '--'}
          unit="°C"
          status={latestVital?.status || 'normal'}
          color="purple"
        />
        <VitalIndicator
          icon={<Brain className="w-6 h-6" />}
          label="Índice de Estresse"
          value={latestVital?.stressIndex.toFixed(0) || '--'}
          unit=""
          status={latestVital?.status || 'normal'}
          color="orange"
        />
        <VitalIndicator
          icon={<Zap className="w-6 h-6" />}
          label="Respiração Estimada"
          value={latestVital?.respiration.toFixed(0) || '--'}
          unit="rpm"
          status={latestVital?.status || 'normal'}
          color="cyan"
        />
        <VitalIndicator
          icon={<CheckCircle2 className="w-6 h-6" />}
          label="Status Geral"
          value="Estável"
          unit=""
          status="normal"
          color="green"
        />
      </div>

      {/* Gráfico e Alertas */}
      <div className="grid grid-cols-4 gap-4">
        <VitalSignsChart data={vitals} period="Últimas 6 horas" />
        <AlertsList alerts={alerts} />
      </div>
    </Container>
  );
}
