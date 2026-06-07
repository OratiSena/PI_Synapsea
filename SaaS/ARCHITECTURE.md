# Arquitetura do Synapsea

## Visão Geral

Synapsea é uma plataforma SaaS médica moderna desenvolvida com Next.js 15 e React 19. A arquitetura segue princípios SOLID e Clean Architecture, permitindo escalabilidade e manutenibilidade.

## Camadas Arquiteturais

### 1. **Presentation Layer** (`/app` e `/components`)

#### App Router Pages
- `/app/page.tsx` - Dashboard principal
- `/app/monitoring/page.tsx` - Monitoramento de temperatura
- `/app/patients/page.tsx` - Gestão de pacientes
- `/app/history/page.tsx` - Histórico de dados
- `/app/alerts/page.tsx` - Sistema de alertas
- `/app/reports/page.tsx` - Relatórios
- `/app/devices/page.tsx` - Status de dispositivos
- `/app/settings/page.tsx` - Configurações

#### Components Reutilizáveis
- **Layout Components**: Sidebar, Header
- **Dashboard Components**: Indicadores vitais, Gráficos, Alertas, Heatmap
- **UI Components**: Button, Card, Badge, Container

### 2. **State Management** (`/lib/store.ts`)

Utiliza Zustand para gerenciamento de estado global:
- Vitals: array de sinais vitais
- Temperature: dados de temperatura em tempo real
- Devices: status dos dispositivos conectados
- Alerts: alertas do sistema
- UI State: período selecionado, tema

**Por que Zustand?**
- Simples e leve
- Sem boilerplate
- DevTools integrado
- Performance otimizada

### 3. **API Service Layer** (`/lib/api-service.ts`)

Serviço de API centralizado com:
- Interceptadores de request/response
- Fallback para dados mockados
- Preparado para WebSocket futuro
- Tipagem TypeScript completa

**Endpoints Esperados:**
```
GET /api/vitals           - Lista de sinais vitais
GET /api/vitals/latest    - Último sinal vital
GET /api/temperature      - Dados atuais de temperatura
GET /api/temperature/history - Histórico de temperatura
GET /api/devices          - Lista de dispositivos
GET /api/devices/:id      - Status de dispositivo específico
GET /api/alerts           - Lista de alertas
PATCH /api/alerts/:id/read - Marcar alerta como lido
```

### 4. **Data Layer** (`/lib/mock-data.ts`)

Geradores de dados mockados:
- `generateMockVitals()` - Sinais vitais com variações realistas
- `generateMockTemperature()` - Matriz 8x8 do heatmap
- `generateMockDevices()` - Dispositivos conectados
- `generateMockAlerts()` - Alertas diversos

**Transição para Backend Real:**
Altere `useMockData = false` em `api-service.ts`

### 5. **Types & Interfaces** (`/lib/types.ts`)

Definições TypeScript:
- `VitalSigns` - Sinais vitais do paciente
- `TemperatureData` - Dados de temperatura
- `Device` - Dispositivo conectado
- `Alert` - Evento de alerta
- `Patient` - Informações do paciente
- `ApiResponse<T>` - Estrutura padrão de resposta

### 6. **Utilities** (`/lib/utils.ts`)

Funções auxiliares:
- `cn()` - Merge de Tailwind classes
- `formatTime()` - Formatação relativa de tempo
- `formatValue()` - Formatação de valores com unidades
- `getStatusColor()` - Cor baseada em status
- `getStatusBgColor()` - Background baseado em status

## Fluxo de Dados

```
User Action (UI)
    ↓
Component State Update
    ↓
Zustand Store Update
    ↓
API Service Call (com mock fallback)
    ↓
Data Transformation
    ↓
Component Re-render
    ↓
UI Update
```

## Integração com Backend

### Fase 1: Dados Mockados (ATUAL)
- Sistema funcional completo
- Dados regenerados em tempo real
- Sem dependência de backend

### Fase 2: Backend REST API (PRÓXIMA)
```javascript
// lib/api-service.ts
useMockData = false; // Ativar requisições reais

// Backend esperado: Node.js/NestJS + PostgreSQL
```

### Fase 3: WebSocket Real-time (FUTURO)
```javascript
// Implementar Socket.io
subscribeToUpdates((data) => {
  // Atualizar store com dados em tempo real
});
```

## Componentização

### VitalIndicator
- Card reutilizável para cada métrica vital
- Ícone colorido, valor destacado, status
- Props tipadas e flexíveis

### VitalSignsChart
- Gráfico Recharts com múltiplas linhas
- Linhas: BPM (vermelho), SpO₂ (azul), Temperatura (roxo), Estresse (laranja), Respiração (ciano)
- Tooltip customizado, legenda interativa

### ThermalHeatmap
- Canvas nativo para renderização de heatmap
- Matriz 8x8 interpolada
- Cores: azul (frio) → verde → amarelo → vermelho (quente)
- Marcador do ponto mais quente

### AlertsList
- Display de alertas com ícones dinâmicos
- Filtros por tipo (info, success, warning, critical)
- Timestamp formatado relativamente

## Estilização

### Tailwind CSS Configuration
- Cores customizadas (primary, success, warning, danger)
- Efeitos glassmorphism
- Animações suaves
- Shadows e blur customizados

### Design Tokens
```javascript
colors: {
  primary: '#00D4FF',
  success: '#22c55e',
  warning: '#f59e0b',
  danger: '#ef4444',
  // ... mais cores
}
```

## Performance

### Otimizações
- Componentes memoizados com React
- Recharts otimizado para grandes datasets
- Canvas para rendering de heatmap
- Zustand para atualizações de estado eficientes
- Next.js Image optimization (quando aplicável)

### Recomendações
- Limitar histórico de vitals a 100 últimos registros
- Debounce em atualizações de temperatura
- Lazy loading em páginas secundárias
- Service Workers para offline capability

## Testing Strategy

### Unit Tests
- Funções de utils
- Componentes simples (Button, Badge)
- Store actions

### Integration Tests
- Fluxo completo de dados
- API service com mock
- Componentes com store

### E2E Tests
- Navegação entre páginas
- Interações do usuário
- WebSocket real-time (quando implementado)

## Segurança

### Implementado
- HTTPS only
- Content Security Policy headers
- Type safety com TypeScript

### Recomendado
- Authentication/Authorization
- Rate limiting na API
- Input validation e sanitization
- CORS configuration
- Encrypted sensitive data

## Escalabilidade

### Melhorias Futuras
1. **Database**: PostgreSQL com Prisma ORM
2. **Backend**: NestJS com arquitetura modular
3. **Real-time**: WebSocket via Socket.io
4. **Caching**: Redis para dados frequentes
5. **Monitoring**: Grafana + Prometheus
6. **CI/CD**: GitHub Actions
7. **Containerization**: Docker + Kubernetes

## Convenções de Código

### Nomeação
- Componentes: PascalCase (`VitalIndicator.tsx`)
- Arquivos: kebab-case ou PascalCase
- Variáveis/funções: camelCase
- Constantes: UPPER_SNAKE_CASE

### Estrutura de Componentes
```typescript
'use client'; // Se necessário

import { dependencies };

interface IProps {
  // Props tipadas
}

export function ComponentName({ props }: IProps) {
  // Lógica
  return (
    // JSX
  );
}
```

### Imports
- Imports de bibliotecas no topo
- Imports locais abaixo
- Organizar por tipo (types, components, utils)

## Documentação

- README.md - Instruções de setup
- ARCHITECTURE.md - Este arquivo
- Comentários inline para lógica complexa
- JSDoc para funções públicas

---

**Última atualização**: 2025
**Versão**: 1.0.0
