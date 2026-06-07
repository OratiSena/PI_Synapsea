'use client';

import { Card } from '../ui/Card';
import { VitalSigns } from '@/lib/types';
import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  Legend,
  ResponsiveContainer,
} from 'recharts';
import { formatTime } from '@/lib/utils';

interface VitalSignsChartProps {
  data: VitalSigns[];
  period: string;
}

export function VitalSignsChart({ data, period }: VitalSignsChartProps) {
  const chartData = data.map((vital) => ({
    timestamp: new Date(vital.timestamp).toLocaleTimeString('pt-BR', {
      hour: '2-digit',
      minute: '2-digit',
    }),
    BPM: Math.round(vital.bpm),
    SpO2: vital.spo2,
    Temperatura: vital.temperature,
    Estresse: vital.stressIndex,
    Respiração: Math.round(vital.respiration),
  }));

  return (
    <Card className="p-6 col-span-3">
      <div className="mb-6">
        <h3 className="text-lg font-semibold text-white mb-2">
          Sinais vitais ao longo do tempo
        </h3>
        <div className="flex items-center justify-between">
          <p className="text-sm text-text-secondary">{period}</p>
        </div>
      </div>

      <ResponsiveContainer width="100%" height={350}>
        <LineChart
          data={chartData}
          margin={{ top: 5, right: 30, left: 0, bottom: 5 }}
        >
          <CartesianGrid strokeDasharray="3 3" stroke="rgba(255,255,255,0.1)" />
          <XAxis
            dataKey="timestamp"
            stroke="rgba(148,163,184,0.5)"
            style={{ fontSize: '12px' }}
          />
          <YAxis stroke="rgba(148,163,184,0.5)" style={{ fontSize: '12px' }} />
          <Tooltip
            contentStyle={{
              backgroundColor: 'rgba(12, 22, 40, 0.95)',
              border: '1px solid rgba(255,255,255,0.1)',
              borderRadius: '8px',
              color: '#fff',
            }}
            labelStyle={{ color: '#fff' }}
          />
          <Legend
            wrapperStyle={{ paddingTop: '20px' }}
            iconType="line"
          />
          <Line
            type="monotone"
            dataKey="BPM"
            stroke="#ef4444"
            strokeWidth={2}
            dot={false}
            isAnimationActive={false}
          />
          <Line
            type="monotone"
            dataKey="SpO2"
            stroke="#3b82f6"
            strokeWidth={2}
            dot={false}
            isAnimationActive={false}
          />
          <Line
            type="monotone"
            dataKey="Temperatura"
            stroke="#a855f7"
            strokeWidth={2}
            dot={false}
            isAnimationActive={false}
          />
          <Line
            type="monotone"
            dataKey="Estresse"
            stroke="#f59e0b"
            strokeWidth={2}
            dot={false}
            isAnimationActive={false}
          />
          <Line
            type="monotone"
            dataKey="Respiração"
            stroke="#06b6d4"
            strokeWidth={2}
            dot={false}
            isAnimationActive={false}
          />
        </LineChart>
      </ResponsiveContainer>
    </Card>
  );
}
