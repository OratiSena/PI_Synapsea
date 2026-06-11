# FAQ & Troubleshooting - Synapsea

## ❓ Perguntas Frequentes

### P: Como faço para rodar o projeto?
**R:** Execute:
```bash
npm install
npm run dev
```
Depois acesse `http://localhost:3000`

### P: Como customizar as cores?
**R:** Edite `tailwind.config.js`:
```javascript
colors: {
  primary: '#00D4FF', // Altere aqui
  // ... outras cores
}
```

### P: Como adicionar uma nova página?
**R:** Crie uma pasta em `/app` e adicione `page.tsx`:
```
/app/nova-pagina/page.tsx
```
Será acessível em `/nova-pagina`

### P: Como integrar com backend?
**R:** Siga o guia em [DEVELOPMENT_GUIDE.md](./DEVELOPMENT_GUIDE.md)

### P: Como fazer o heatmap renderizar diferente?
**R:** Edite `lib/mock-data.ts` na função `generateMockTemperature()`

### P: Como adicionar mais alertas?
**R:** Edite `lib/mock-data.ts` na função `generateMockAlerts()`

### P: Posso usar em produção?
**R:** Sim! Execute `npm run build` e depois `npm start`

### P: Como fazer deploy?
**R:** Use Vercel (recomendado para Next.js):
```bash
npm i -g vercel
vercel
```

---

## 🔧 Troubleshooting

### ❌ Erro: "Cannot find module 'react'"

**Solução:**
```bash
rm -rf node_modules package-lock.json
npm install
```

### ❌ Porta 3000 já está em uso

**Solução:**
```bash
npm run dev -- -p 3001
```

### ❌ Componentes não aparecem

**Verificar:**
1. Arquivo começa com `'use client';`?
2. Todos os imports estão corretos?
3. Props estão tipadas corretamente?

### ❌ Gráfico não renderiza

**Verificar:**
1. Dados estão no formato correto?
2. Array de vitals não está vazio?
3. Console mostra erros?

### ❌ Tailwind CSS não funciona

**Solução:**
```bash
npm run dev
```
Rebuild automático geralmente resolve.

### ❌ TypeScript erros de tipos

**Verificar:**
```bash
npm run type-check
```

### ❌ Dados não atualizam

**Verificar:**
1. Store está sendo atualizado?
2. Componente está subscrito ao store?
3. Hook `useEffect` está correto?

### ❌ Build falha em produção

**Verificar:**
```bash
npm run build -- --debug
```

### ❌ Heatmap não mostra cores

**Verificar:**
1. Canvas está disponível?
2. Dados de temperatura estão corretos?
3. Função `getTempColor` está correta?

### ❌ WebSocket não conecta

**Verificar:**
```typescript
// Conferir em api-service.ts
useMockData = false; // Se true, não tentará WebSocket
```

---

## 📊 Performance

### Otimizar Gráfico
```typescript
// Limitar pontos de dados
const chartData = vitals.slice(-50); // Últimos 50
```

### Otimizar Heatmap
```typescript
// Diminuir frequência de atualização
const interval = setInterval(loadData, 60000); // 60s em vez de 30s
```

### Otimizar Estado
```typescript
// Usar seletores específicos
const bpm = useDashboardStore((state) => state.vitals[0]?.bpm);
// Em vez de
const vitals = useDashboardStore((state) => state.vitals);
```

---

## 🔐 Segurança

### Proteger API Keys
```bash
# .env.local (não commitar)
NEXT_PUBLIC_API_KEY=seu_secret_aqui
```

### Validação de Dados
```typescript
// Sempre validar dados da API
if (!data || !data.bpm) {
  throw new Error('Dados inválidos');
}
```

### CORS Configuration
```typescript
// No backend
app.use(cors({
  origin: process.env.FRONTEND_URL,
  credentials: true,
}));
```

---

## 📱 Responsividade

### Testar Breakpoints
- `sm` (640px): Use DevTools
- `md` (768px): Use DevTools
- `lg` (1024px): Use DevTools
- `xl` (1280px): Use DevTools

### Debug Tailwind
```html
<!-- Adicionar elemento de debug -->
<div className="block sm:hidden">mobile</div>
<div className="hidden sm:block md:hidden">tablet</div>
<div className="hidden md:block">desktop</div>
```

---

## 🐛 Debug

### Verificar Estado Zustand
```typescript
// No console do browser
import { useDashboardStore } from '@/lib/store';
const state = useDashboardStore.getState();
console.log(state);
```

### Verificar Dados da API
```typescript
// No console
const vitals = await apiService.getVitals();
console.log(vitals);
```

### DevTools
```bash
# Instalar React DevTools
# Chrome Web Store: React Developer Tools
```

### Next.js Debug
```bash
npm run dev -- --debug
```

---

## 📈 Monitoring

### Lighthouse Score
- Abra DevTools (F12)
- Clique em "Lighthouse"
- Analise resultados

### Core Web Vitals
- LCP (Largest Contentful Paint) < 2.5s
- FID (First Input Delay) < 100ms
- CLS (Cumulative Layout Shift) < 0.1

---

## 🚀 Performance Tips

### 1. Memoize Componentes
```typescript
export const MyComponent = memo(function MyComponent() {
  // ...
});
```

### 2. Usar useCallback
```typescript
const handleClick = useCallback(() => {
  // ...
}, [dependencies]);
```

### 3. Lazy Load
```typescript
const HeavyComponent = dynamic(() => import('./Heavy'));
```

### 4. Otimizar Imagens
```typescript
import Image from 'next/image';
```

---

## 🎓 Aprendizado

### Recursos Recomendados
- [Next.js Docs](https://nextjs.org/docs)
- [React Docs](https://react.dev)
- [Tailwind CSS](https://tailwindcss.com/docs)
- [TypeScript Handbook](https://www.typescriptlang.org/docs)

### Conceitos Importantes
1. **React Hooks**: useState, useEffect, useContext
2. **Zustand**: State management
3. **TypeScript**: Type safety
4. **Tailwind**: Utility-first CSS
5. **Recharts**: Data visualization

---

## 📞 Suporte Adicional

### Não encontrou resposta aqui?

1. Consulte [ARCHITECTURE.md](./ARCHITECTURE.md)
2. Consulte [DEVELOPMENT_GUIDE.md](./DEVELOPMENT_GUIDE.md)
3. Consulte [USAGE_EXAMPLES.md](./USAGE_EXAMPLES.md)
4. Abra uma issue no repositório

---

## 🎯 Checklist de Debugging

Quando algo não funcionar:

- [ ] Limpei o cache? (`rm -rf .next`)
- [ ] Reinstalei dependências? (`npm install`)
- [ ] Reiniciei o servidor? (`npm run dev`)
- [ ] Verificei o console? (F12)
- [ ] Verificei os tipos? (`npm run type-check`)
- [ ] Testei em incógnito?
- [ ] Verifiquei a rede? (DevTools → Network)
- [ ] Verifiquei os dados? (Console → store)

---

**Última atualização**: 2025  
**Versão**: 1.0.0
