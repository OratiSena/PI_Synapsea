# Exemplos de Uso - Synapsea Components

## 1. VitalIndicator - Exibir Métrica Vital

```tsx
import { VitalIndicator } from '@/components';
import { Heart } from 'lucide-react';

export function MyComponent() {
  return (
    <VitalIndicator
      icon={<Heart className="w-6 h-6" />}
      label="Batimentos Cardíacos"
      value={78}
      unit="BPM"
      status="normal"
      color="red"
    />
  );
}
```

### Props Disponíveis
- `icon`: ReactNode - Ícone do indicador
- `label`: string - Rótulo descritivo
- `value`: number | string - Valor principal
- `unit`: string - Unidade de medida
- `status`: 'normal' | 'warning' | 'critical'
- `color`: 'red' | 'blue' | 'purple' | 'orange' | 'cyan' | 'green'

---

## 2. VitalSignsChart - Exibir Gráfico

```tsx
import { VitalSignsChart } from '@/components';
import { useDashboardStore } from '@/lib';

export function MyChart() {
  const vitals = useDashboardStore((state) => state.vitals);

  return (
    <VitalSignsChart
      data={vitals}
      period="Últimas 6 horas"
    />
  );
}
```

### Dados Esperados
```typescript
VitalSigns[] = [
  {
    timestamp: 1234567890,
    bpm: 78,
    spo2: 98,
    temperature: 36.8,
    stressIndex: 32,
    respiration: 16,
    status: 'normal'
  },
  // ... mais registros
]
```

---

## 3. Card - Componente de Container

```tsx
import { Card, Badge } from '@/components';

export function MyCard() {
  return (
    <Card glass={true} hover={true} className="p-6">
      <h3 className="text-lg font-semibold text-white">Título</h3>
      <Badge variant="success">Ativo</Badge>
      <p className="text-sm text-text-secondary mt-4">
        Conteúdo do card
      </p>
    </Card>
  );
}
```

### Props
- `glass`: boolean - Aplicar efeito glassmorphism (default: true)
- `hover`: boolean - Efeito hover (default: false)
- `className`: string - Classes Tailwind adicionais

---

## 4. Button - Diferentes Variantes

```tsx
import { Button } from '@/components';

export function MyButtons() {
  return (
    <>
      {/* Variantes */}
      <Button variant="primary">Primário</Button>
      <Button variant="secondary">Secundário</Button>
      <Button variant="ghost">Ghost</Button>
      <Button variant="danger">Perigo</Button>

      {/* Tamanhos */}
      <Button size="sm">Pequeno</Button>
      <Button size="md">Médio</Button>
      <Button size="lg">Grande</Button>

      {/* Estados */}
      <Button disabled>Desabilitado</Button>
      <Button loading>Carregando...</Button>
    </>
  );
}
```

---

## 5. ThermalHeatmap - Exibir Mapa Térmico

```tsx
import { ThermalHeatmap } from '@/components';
import { useDashboardStore } from '@/lib';

export function MyHeatmap() {
  const temperature = useDashboardStore((state) => state.temperature);

  if (!temperature) return <div>Carregando...</div>;

  return (
    <ThermalHeatmap
      data={temperature}
      width={600}
      height={600}
    />
  );
}
```

### Estrutura de TemperatureData
```typescript
{
  timestamp: 1234567890,
  grid: [
    [20.5, 21.0, ...], // 8 elementos
    [...],             // 8 linhas
  ],                   // Matriz 8x8
  maxTemp: 37.2,
  minTemp: 20.0,
  avgTemp: 26.3,
  hotspotX: 4,
  hotspotY: 4
}
```

---

## 6. AlertsList - Exibir Alertas

```tsx
import { AlertsList } from '@/components';
import { useDashboardStore } from '@/lib';

export function MyAlerts() {
  const alerts = useDashboardStore((state) => state.alerts);

  return <AlertsList alerts={alerts} />;
}
```

### Estrutura de Alert
```typescript
{
  id: '1',
  title: 'Título do Alerta',
  description: 'Descrição detalhada',
  type: 'success' | 'warning' | 'critical' | 'info',
  timestamp: 1234567890,
  read: false,
  icon: 'CheckCircle2' // Lucide icon name
}
```

---

## 7. Usar Zustand Store

```tsx
import { useDashboardStore } from '@/lib/store';

export function MyComponent() {
  // Ler estado
  const vitals = useDashboardStore((state) => state.vitals);
  const alerts = useDashboardStore((state) => state.alerts);
  const selectedPeriod = useDashboardStore((state) => state.selectedPeriod);

  // Atualizar estado
  const setVitals = useDashboardStore((state) => state.setVitals);
  const addAlert = useDashboardStore((state) => state.addAlert);
  const setPeriod = useDashboardStore((state) => state.setPeriod);

  return (
    <button onClick={() => setVitals([...newVitals])}>
      Atualizar Vitais
    </button>
  );
}
```

---

## 8. Usar API Service

```tsx
import { apiService } from '@/lib/api-service';
import { useEffect, useState } from 'react';

export function MyComponent() {
  const [loading, setLoading] = useState(true);
  const [vitals, setVitals] = useState([]);

  useEffect(() => {
    const loadData = async () => {
      try {
        const data = await apiService.getVitals('6h');
        setVitals(data);
      } catch (error) {
        console.error('Erro:', error);
      } finally {
        setLoading(false);
      }
    };

    loadData();
  }, []);

  if (loading) return <div>Carregando...</div>;
  return <div>{vitals.length} registros</div>;
}
```

---

## 9. Criar Novo Componente Reutilizável

```tsx
'use client';

import { ReactNode } from 'react';
import { Card } from './ui/Card';
import { cn } from '@/lib/utils';

interface MyComponentProps {
  title: string;
  children: ReactNode;
  variant?: 'primary' | 'secondary';
  className?: string;
}

export function MyComponent({
  title,
  children,
  variant = 'primary',
  className,
}: MyComponentProps) {
  return (
    <Card
      className={cn(
        'space-y-4',
        variant === 'primary' && 'border-primary',
        className
      )}
    >
      <h2 className="text-lg font-semibold text-white">{title}</h2>
      {children}
    </Card>
  );
}
```

---

## 10. Usar Dados Mockados

```tsx
import {
  generateMockVitals,
  generateMockTemperature,
  generateMockDevices,
  generateMockAlerts,
} from '@/lib/mock-data';

export function MyComponent() {
  const vitals = generateMockVitals();      // Array de VitalSigns
  const temperature = generateMockTemperature(); // TemperatureData
  const devices = generateMockDevices();    // Array de Device
  const alerts = generateMockAlerts();      // Array de Alert

  return (
    <div>
      <p>Últimos vitais: {vitals[vitals.length - 1].bpm} BPM</p>
      <p>Temperatura: {temperature.avgTemp}°C</p>
      <p>Dispositivos: {devices.length}</p>
      <p>Alertas: {alerts.length}</p>
    </div>
  );
}
```

---

## 11. Integrar com Recharts Customizado

```tsx
import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  ResponsiveContainer,
} from 'recharts';

export function CustomChart({ data }) {
  return (
    <ResponsiveContainer width="100%" height={300}>
      <LineChart data={data}>
        <CartesianGrid strokeDasharray="3 3" />
        <XAxis dataKey="timestamp" />
        <YAxis />
        <Tooltip />
        <Line type="monotone" dataKey="value" stroke="#00D4FF" />
      </LineChart>
    </ResponsiveContainer>
  );
}
```

---

## 12. Usar Framer Motion para Animações

```tsx
import { motion } from 'framer-motion';

export function AnimatedCard() {
  return (
    <motion.div
      initial={{ opacity: 0, y: 20 }}
      animate={{ opacity: 1, y: 0 }}
      transition={{ duration: 0.5 }}
      className="glass p-6 rounded-lg"
    >
      Conteúdo animado
    </motion.div>
  );
}
```

---

## 13. Usar Utilities

```tsx
import { cn, formatTime, formatValue, getStatusColor } from '@/lib/utils';

export function MyComponent() {
  // Merge de classes Tailwind
  const classes = cn('p-4', true && 'text-white', false && 'text-red-500');

  // Formatar tempo relativo
  const time = formatTime(Date.now() - 5 * 60 * 1000); // "5m"

  // Formatar valor com unidade
  const formatted = formatValue(78, 'BPM'); // "78.0 BPM"

  // Cor baseada em status
  const color = getStatusColor('warning'); // "text-warning"

  return <div className={classes}>{time} - {formatted}</div>;
}
```

---

## 14. Padrão de Responsividade

```tsx
export function ResponsiveGrid() {
  return (
    <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-4">
      {/* 1 coluna em mobile, 2 em tablet, 4 em desktop */}
      <Card>Item 1</Card>
      <Card>Item 2</Card>
      <Card>Item 3</Card>
      <Card>Item 4</Card>
    </div>
  );
}
```

---

## 15. WebSocket Simulado (Futuro)

```tsx
import { apiService } from '@/lib/api-service';
import { useEffect } from 'react';
import { useDashboardStore } from '@/lib/store';

export function RealtimeUpdates() {
  const addVital = useDashboardStore((state) => state.addVital);

  useEffect(() => {
    const unsubscribe = apiService.subscribeToUpdates((data) => {
      if (data.type === 'vitals') {
        addVital(data.data);
      }
    });

    return () => unsubscribe();
  }, [addVital]);

  return <div>Aguardando atualizações...</div>;
}
```

---

**Próximas implementações podem seguir esses padrões!**
