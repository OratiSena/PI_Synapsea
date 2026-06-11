# 📁 Estrutura Completa do Projeto Synapsea

```
SaaS/
│
├── 📄 Configuração
│   ├── package.json                # Dependências e scripts
│   ├── tsconfig.json              # Configuração TypeScript
│   ├── next.config.js             # Configuração Next.js
│   ├── tailwind.config.js         # Tema Tailwind CSS
│   ├── postcss.config.js          # Processamento CSS
│   ├── .editorconfig              # Formatação de código
│   ├── .gitignore                 # Arquivos ignorados
│   ├── .env.example               # Variáveis de ambiente
│   └── README.md                  # Instruções principais
│
├── 📖 Documentação
│   ├── ARCHITECTURE.md            # Arquitetura técnica
│   ├── DEVELOPMENT_GUIDE.md       # Guia de integração backend
│   ├── PROJECT_CHECKLIST.md       # Status do projeto
│   ├── QUICKSTART.md              # Início rápido
│   └── PROJECT_STRUCTURE.md       # Este arquivo
│
├── 📱 App (Next.js App Router)
│   ├── layout.tsx                 # Layout raiz com Sidebar/Header
│   ├── page.tsx                   # Dashboard principal
│   ├── globals.css                # Estilos globais
│   │
│   ├── monitoring/
│   │   └── page.tsx              # Página de monitoramento térmico
│   ├── patients/
│   │   └── page.tsx              # Gestão de pacientes
│   ├── history/
│   │   └── page.tsx              # Histórico de dados
│   ├── alerts/
│   │   └── page.tsx              # Lista de alertas
│   ├── reports/
│   │   └── page.tsx              # Relatórios
│   ├── devices/
│   │   └── page.tsx              # Status de dispositivos
│   └── settings/
│       └── page.tsx              # Configurações
│
├── 🧩 Components
│   ├── index.ts                   # Barrel export
│   │
│   ├── layout/                    # Componentes de layout
│   │   ├── Sidebar.tsx           # Sidebar com menu
│   │   └── Header.tsx            # Header com controles
│   │
│   ├── dashboard/                 # Componentes da dashboard
│   │   ├── VitalIndicator.tsx    # Cards de sinais vitais
│   │   ├── VitalSignsChart.tsx   # Gráfico Recharts
│   │   ├── AlertsList.tsx        # Lista de alertas
│   │   ├── ThermalHeatmap.tsx    # Heatmap do sensor
│   │   ├── TemperatureScale.tsx  # Escala de temperatura
│   │   ├── DevicesPanel.tsx      # Painel de dispositivos
│   │   └── SensorInfo.tsx        # Info do sensor
│   │
│   └── ui/                        # Componentes base reutilizáveis
│       ├── Button.tsx            # Botão com variantes
│       └── Card.tsx              # Card, Badge, Container
│
├── 📚 Lib (Lógica de negócio)
│   ├── index.ts                   # Barrel export
│   ├── types.ts                   # Tipos TypeScript
│   ├── store.ts                   # Zustand store (estado)
│   ├── api-service.ts             # Serviço de API
│   ├── mock-data.ts               # Dados mockados
│   ├── utils.ts                   # Funções utilitárias
│   └── constants.ts               # Constantes da app
│
├── 📦 Public (Arquivos estáticos)
│   └── (arquivos imagens, fonts, etc)
│
└── node_modules/                  # Dependências (gitignore)
```

## 📊 Camadas da Aplicação

```
┌─────────────────────────────────────────┐
│         User Interface (UI)              │ ← Components + Pages
├─────────────────────────────────────────┤
│      State Management (Zustand)         │ ← Store
├─────────────────────────────────────────┤
│     Services & API Integration          │ ← API Service
├─────────────────────────────────────────┤
│        Data Layer (Mock/Real)           │ ← Mock Data / Backend
├─────────────────────────────────────────┤
│   Styling & Utilities (Tailwind)        │ ← CSS + Utils
└─────────────────────────────────────────┘
```

## 🔄 Fluxo de Dados

```
User Interaction
    ↓
Component (React Hook)
    ↓
Zustand Store (State Update)
    ↓
API Service (Fetch Data)
    ↓
Mock Data / Backend API
    ↓
Store Update
    ↓
Component Re-render
    ↓
UI Update
```

## 📋 Arquivos Principais

### Componentes Críticos
```
VitalIndicator.tsx  →  6x cards de métricas
VitalSignsChart.tsx →  Gráfico multi-linha
AlertsList.tsx      →  Lista de alertas
ThermalHeatmap.tsx  →  Heatmap 8x8
```

### Services Críticos
```
store.ts        →  Estado global (vitals, temperature, devices, alerts)
api-service.ts  →  Comunicação com backend (mock/real)
mock-data.ts    →  Geração de dados realistas
```

### Páginas Funcionais
```
page.tsx           →  Dashboard (✅ Completa)
monitoring/        →  Monitoramento térmico (✅ Completa)
alerts/            →  Alertas (✅ Completa)
devices/           →  Dispositivos (✅ Completa)
patients/          →  Pacientes (🔄 Placeholder)
```

## 🎨 Assets & Styling

```
globals.css
├── @tailwind base;
├── @tailwind components;
├── @tailwind utilities;
├── Glassmorphism effects
├── Custom animations
└── Scrollbar styling

tailwind.config.js
├── Custom colors (primary, success, warning, danger)
├── Extended spacing
├── Custom shadows (glass, glow, soft)
└── Animations (pulse-glow)
```

## 📦 Dependências Principais

```json
{
  "runtime": [
    "react@19.0.0-rc",
    "next@15.0.0",
    "tailwindcss@3.4.1",
    "zustand@4.4.2",
    "axios@1.6.2",
    "recharts@2.10.3",
    "framer-motion@10.16.18",
    "lucide-react@0.294.0"
  ],
  "dev": [
    "typescript@5.3.3",
    "@types/node@20.10.6",
    "@types/react@18.2.46"
  ]
}
```

## 🔌 Integrações

### Bibliotecas Externas
- **Recharts**: Gráficos 📊
- **Framer Motion**: Animações ✨
- **Lucide React**: Ícones 🎯
- **Zustand**: Estado 📦
- **Axios**: HTTP 🌐
- **Tailwind**: Styling 🎨

### Preparado para:
- **Backend**: Node.js/NestJS
- **Database**: PostgreSQL
- **Real-time**: WebSocket/Socket.io
- **Auth**: JWT tokens

## 🚀 Scripts Disponíveis

```bash
npm run dev        # Desenvolvimento (localhost:3000)
npm run build      # Build otimizado
npm start          # Produção
npm run lint       # Lint com ESLint
npm run type-check # Verificação TypeScript
```

## 📱 Breakpoints (Tailwind)

```javascript
sm: '640px'   // Mobile
md: '768px'   // Tablet
lg: '1024px'  // Desktop
xl: '1280px'  // Desktop grande
```

## 🎯 Convenções

### Nomeação
```
Components:   PascalCase (VitalIndicator.tsx)
Hooks:        camelCase (useVitals())
Files:        kebab-case ou PascalCase
Variables:    camelCase
Constants:    UPPER_SNAKE_CASE
```

### Estrutura de Componente
```typescript
'use client';                          // Client component
import { Dependencies };               // Imports
interface IProps { ... }              // Props interface
export function ComponentName(props) { // Component
  return JSX;                         // Render
}
```

---

**Versão**: 1.0.0  
**Última atualização**: 2025  
**Status**: ✅ Estrutura Completa
