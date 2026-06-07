# Synapsea - Medical SaaS Platform

Sistema de monitoramento de saúde de baixo custo integrado com sensores biomédicos para fins educacionais, pesquisa e prevenção.

## 🚀 Características

- **Dashboard Premium**: Interface moderna e profissional com design dark premium
- **Monitoramento em Tempo Real**: Sinais vitais (BPM, SpO₂, Temperatura, Índice de Estresse, Respiração)
- **Heatmap Térmico**: Visualização avançada de temperatura corporal com sensor AMG8833
- **Sistema de Alertas**: Notificações inteligentes de eventos críticos
- **Arquitetura Escalável**: Preparado para integração com backend Node.js/NestJS
- **Responsivo**: Funciona perfeitamente em desktops, tablets e smartphones
- **Data Mockada**: Sistema completo funcional com dados simulados

## 🛠️ Tecnologias

- **Frontend**: Next.js 15, React 19, TypeScript
- **Styling**: Tailwind CSS, Framer Motion
- **Componentes**: Shadcn/UI, Lucide React
- **Gráficos**: Recharts
- **Estado**: Zustand
- **API**: Axios, React Query
- **Canvas**: Heatmap com Canvas nativo

## 📦 Instalação

### Pré-requisitos
- Node.js 18+ ou superior
- npm, yarn ou pnpm

### Passos

1. Instale as dependências:
```bash
npm install
# ou
yarn install
```

2. Configure as variáveis de ambiente:
```bash
cp .env.example .env.local
```

3. Inicie o servidor de desenvolvimento:
```bash
npm run dev
# ou
yarn dev
```

4. Abra [http://localhost:3000](http://localhost:3000) no navegador

## 📂 Estrutura do Projeto

```
SaaS/
├── app/                          # Páginas Next.js App Router
│   ├── layout.tsx               # Layout raiz
│   ├── page.tsx                 # Dashboard principal
│   ├── globals.css              # Estilos globais
│   ├── monitoring/
│   ├── patients/
│   ├── history/
│   ├── alerts/
│   ├── reports/
│   ├── devices/
│   └── settings/
├── components/
│   ├── layout/                  # Componentes de layout
│   │   ├── Sidebar.tsx
│   │   └── Header.tsx
│   ├── dashboard/               # Componentes da dashboard
│   │   ├── VitalIndicator.tsx
│   │   ├── VitalSignsChart.tsx
│   │   ├── AlertsList.tsx
│   │   ├── ThermalHeatmap.tsx
│   │   ├── TemperatureScale.tsx
│   │   ├── DevicesPanel.tsx
│   │   └── SensorInfo.tsx
│   └── ui/                      # Componentes base
│       ├── Button.tsx
│       └── Card.tsx
├── lib/
│   ├── types.ts                 # Tipos TypeScript
│   ├── store.ts                 # Zustand store
│   ├── api-service.ts           # Serviço de API
│   ├── mock-data.ts             # Dados mockados
│   └── utils.ts                 # Utilitários
├── public/                      # Arquivos estáticos
├── package.json
├── tsconfig.json
├── tailwind.config.js
├── postcss.config.js
└── next.config.js
```

## 🎨 Design System

### Cores
- **Background**: `#050816`
- **Sidebar**: `#08111f`
- **Card**: `#0c1628`
- **Primary**: `#00D4FF` (Cyan)
- **Success**: `#22c55e` (Green)
- **Warning**: `#f59e0b` (Orange)
- **Danger**: `#ef4444` (Red)
- **Text Secondary**: `#94a3b8`

### Tipografia
- **Font**: Inter (400, 500, 600, 700)
- **Headings**: Bold (700)
- **Body**: Regular (400)
- **Accent**: Semibold (600)

## 🔌 Integração com Backend

O sistema está preparado para integração com backend Node.js/NestJS. Endpoints esperados:

### REST API
```
GET /api/vitals           - Sinais vitais
GET /api/temperature      - Dados de temperatura
GET /api/devices         - Status dos dispositivos
GET /api/alerts          - Alertas do sistema
```

### WebSocket (Futuro)
```
events: ['vitals', 'temperature', 'alerts']
```

Para ativar dados reais, altere `useMockData = false` em `lib/api-service.ts`.

## 📱 Responsividade

- **Desktop**: 1920px
- **Notebook**: 1366px
- **Tablet**: 768px
- **Smartphone**: 390px

## 🚀 Build para Produção

```bash
npm run build
npm start
```

## 📝 Dados Mockados

O sistema utiliza dados mockados que são regenerados em tempo real:
- Sinais vitais com variações realistas
- Matriz 8x8 de temperatura do heatmap
- Dispositivos conectados com status
- Alertas de diferentes tipos e severidades

## 🔒 Segurança

- TypeScript para type-safety
- Validação de dados
- Variáveis de ambiente protegidas
- Headers de segurança configuráveis

## 📄 Licença

Projeto acadêmico - Synapsea 2025

## 👨‍💼 Suporte

Para dúvidas ou sugestões sobre o projeto, entre em contato através das issues do repositório.

---

**Desenvolvido com ❤️ para fins educacionais e pesquisa em saúde digital.**
