# Guia de Desenvolvimento e Integração Backend

## Fase 1: Setup Inicial ✅ (Concluído)

- [x] Estrutura Next.js 15
- [x] Componentes reutilizáveis
- [x] Design system premium
- [x] Mock data completo
- [x] Zustand store
- [x] API service preparado
- [x] Dashboard funcional
- [x] Todas as páginas

## Fase 2: Integração com Backend (Próxima)

### 2.1 Setup do Backend

```bash
# Backend TypeScript/Node.js
mkdir synapsea-backend
cd synapsea-backend

# Inicializar projeto NestJS (recomendado)
npm i -g @nestjs/cli
nest new synapsea-api

# Ou usar Express.js
npm init -y
npm install express typescript ts-node
```

### 2.2 Dependências Recomendadas

```json
{
  "dependencies": {
    "@nestjs/core": "^10.0.0",
    "@nestjs/common": "^10.0.0",
    "@nestjs/platform-express": "^10.0.0",
    "prisma": "^5.0.0",
    "@prisma/client": "^5.0.0",
    "pg": "^8.0.0",
    "socket.io": "^4.7.0",
    "cors": "^2.8.5",
    "dotenv": "^16.0.0",
    "bcrypt": "^5.1.0",
    "jsonwebtoken": "^9.1.0"
  }
}
```

### 2.3 Estrutura do Backend

```
synapsea-backend/
├── src/
│   ├── modules/
│   │   ├── vitals/
│   │   │   ├── vitals.controller.ts
│   │   │   ├── vitals.service.ts
│   │   │   ├── vitals.module.ts
│   │   │   └── dto/
│   │   ├── temperature/
│   │   ├── devices/
│   │   ├── alerts/
│   │   └── users/
│   ├── common/
│   │   ├── filters/
│   │   ├── guards/
│   │   └── interceptors/
│   ├── prisma/
│   │   └── schema.prisma
│   ├── websocket/
│   │   └── events.gateway.ts
│   └── main.ts
├── prisma/
│   ├── schema.prisma
│   └── migrations/
├── .env.example
├── package.json
└── tsconfig.json
```

### 2.4 Schema Prisma

```prisma
// prisma/schema.prisma
datasource db {
  provider = "postgresql"
  url      = env("DATABASE_URL")
}

generator client {
  provider = "prisma-client-js"
}

model VitalSigns {
  id            String   @id @default(cuid())
  userId        String
  timestamp     DateTime @default(now())
  bpm           Float
  spo2          Float
  temperature   Float
  stressIndex   Float
  respiration   Float
  status        String
  createdAt     DateTime @default(now())
  updatedAt     DateTime @updatedAt

  @@index([userId])
  @@index([timestamp])
}

model TemperatureData {
  id        String   @id @default(cuid())
  userId    String
  timestamp DateTime @default(now())
  grid      Json     // Array 8x8
  maxTemp   Float
  minTemp   Float
  avgTemp   Float
  createdAt DateTime @default(now())

  @@index([userId])
  @@index([timestamp])
}

model Device {
  id       String  @id @default(cuid())
  userId   String
  name     String
  type     String
  model    String
  status   String
  metadata Json?
  lastSync DateTime

  @@index([userId])
}

model Alert {
  id          String   @id @default(cuid())
  userId      String
  title       String
  description String
  type        String
  timestamp   DateTime @default(now())
  read        Boolean  @default(false)
  createdAt   DateTime @default(now())

  @@index([userId])
  @@index([timestamp])
}

model User {
  id       String @id @default(cuid())
  email    String @unique
  name     String
  password String
  role     String
  // ... outros campos

  vitals       VitalSigns[]
  temperatures TemperatureData[]
  devices      Device[]
  alerts       Alert[]
}
```

### 2.5 Controllers NestJS

```typescript
// src/modules/vitals/vitals.controller.ts
import { Controller, Get, Post, Body, Query } from '@nestjs/common';
import { VitalsService } from './vitals.service';

@Controller('api/vitals')
export class VitalsController {
  constructor(private readonly vitalsService: VitalsService) {}

  @Get()
  async getVitals(@Query('period') period: string) {
    // Filtrar por período
    return this.vitalsService.getByPeriod(period);
  }

  @Get('latest')
  async getLatestVital() {
    return this.vitalsService.getLatest();
  }

  @Post()
  async createVital(@Body() data: any) {
    return this.vitalsService.create(data);
  }
}
```

### 2.6 WebSocket Gateway

```typescript
// src/websocket/events.gateway.ts
import { WebSocketGateway, WebSocketServer, SubscribeMessage } from '@nestjs/websockets';
import { Server, Socket } from 'socket.io';

@WebSocketGateway({
  cors: {
    origin: process.env.FRONTEND_URL,
    credentials: true,
  },
})
export class EventsGateway {
  @WebSocketServer()
  server: Server;

  @SubscribeMessage('vitals')
  handleVitalsUpdate(client: Socket, data: any) {
    this.server.emit('vitals', data);
  }

  @SubscribeMessage('temperature')
  handleTemperatureUpdate(client: Socket, data: any) {
    this.server.emit('temperature', data);
  }

  @SubscribeMessage('alerts')
  handleAlertsUpdate(client: Socket, data: any) {
    this.server.emit('alerts', data);
  }
}
```

## Fase 3: Ativação do Backend Real

### 3.1 Frontend - Ativar Dados Reais

```typescript
// lib/api-service.ts
class ApiService {
  private useMockData = false; // ← Mudar para false
  private api: AxiosInstance;

  constructor() {
    this.api = axios.create({
      baseURL: process.env.NEXT_PUBLIC_API_URL,
      timeout: 10000,
    });
  }
  // ... resto do código
}
```

### 3.2 Frontend - Ativar WebSocket

```typescript
// lib/api-service.ts
import { io } from 'socket.io-client';

subscribeToUpdates(
  callback: (data: any) => void
): () => void {
  const socket = io(process.env.NEXT_PUBLIC_WS_URL);

  socket.on('vitals', (data) => callback({ type: 'vitals', data }));
  socket.on('temperature', (data) => callback({ type: 'temperature', data }));
  socket.on('alerts', (data) => callback({ type: 'alerts', data }));

  return () => socket.disconnect();
}
```

### 3.3 Frontend - Usar Dados em Tempo Real

```typescript
// app/page.tsx
useEffect(() => {
  // Dados reais da API
  const unsubscribe = apiService.subscribeToUpdates((data) => {
    if (data.type === 'vitals') {
      addVital(data.data);
    }
    // ... outros tipos
  });

  return () => unsubscribe();
}, []);
```

## Fase 4: Autenticação e Segurança

### 4.1 JWT Authentication

```typescript
// Backend: auth.guard.ts
import { Injectable } from '@nestjs/common';
import { AuthGuard } from '@nestjs/passport';

@Injectable()
export class JwtAuthGuard extends AuthGuard('jwt') {}
```

```typescript
// Frontend: middleware
import axios from 'axios';

const api = axios.create({
  baseURL: process.env.NEXT_PUBLIC_API_URL,
});

api.interceptors.request.use((config) => {
  const token = localStorage.getItem('token');
  if (token) {
    config.headers.Authorization = `Bearer ${token}`;
  }
  return config;
});
```

### 4.2 Variáveis de Ambiente

```bash
# Frontend: .env.local
NEXT_PUBLIC_API_URL=http://localhost:3001/api
NEXT_PUBLIC_WS_URL=ws://localhost:3001

# Backend: .env
DATABASE_URL=postgresql://user:password@localhost:5432/synapsea
JWT_SECRET=your-secret-key
NODE_ENV=development
PORT=3001
FRONTEND_URL=http://localhost:3000
```

## Fase 5: Deployment

### 5.1 Frontend - Vercel

```bash
# Deploy automático
npm i -g vercel
vercel login
vercel
```

### 5.2 Backend - Railway/Render/Heroku

```bash
# Docker
docker build -t synapsea-api .
docker run -p 3001:3001 synapsea-api
```

## Checklist de Implementação

### Backend
- [ ] Setup NestJS
- [ ] Database schema com Prisma
- [ ] Controllers para CRUD
- [ ] Services com lógica de negócio
- [ ] Autenticação JWT
- [ ] WebSocket Gateway
- [ ] Rate limiting
- [ ] Error handling
- [ ] Logging
- [ ] Testes

### Frontend
- [ ] Ativar `useMockData = false`
- [ ] Configurar `.env.local`
- [ ] Integrar axios interceptors
- [ ] Implementar WebSocket
- [ ] Adicionar loading states
- [ ] Error boundaries
- [ ] Retry logic
- [ ] Cache strategy

### DevOps
- [ ] CI/CD pipeline
- [ ] Database migrations
- [ ] Backups
- [ ] Monitoring
- [ ] Error tracking (Sentry)
- [ ] Performance monitoring (Vercel Analytics)

## Recursos Úteis

- [NestJS Documentation](https://docs.nestjs.com)
- [Prisma Documentation](https://www.prisma.io/docs)
- [Socket.io Documentation](https://socket.io/docs)
- [Next.js API Routes](https://nextjs.org/docs/api-routes/introduction)
- [PostgreSQL Documentation](https://www.postgresql.org/docs)

## Support

Para dúvidas sobre integração backend, abra uma issue no repositório.

---

**Última atualização**: 2025
