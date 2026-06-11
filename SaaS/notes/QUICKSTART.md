# Quick Start Guide - Synapsea

## 🚀 Começar em 5 Minutos

### 1. Instale as dependências

```bash
cd SaaS
npm install
```

### 2. Configure o ambiente

```bash
# Copie o exemplo de env
cp .env.example .env.local

# Se necessário, edite .env.local com suas configurações
```

### 3. Inicie o servidor

```bash
npm run dev
```

### 4. Abra no navegador

Acesse [http://localhost:3000](http://localhost:3000)

Pronto! 🎉 Você deve ver a dashboard com:
- 6 cards de sinais vitais
- Gráfico de sinais vitais ao longo do tempo
- Alertas recentes
- Sidebar com navegação
- Header com greeting personalizado

## 📍 Navegação

### Páginas Disponíveis

| Página | URL | Status |
|--------|-----|--------|
| Dashboard | `/` | ✅ Completa |
| Monitoramento Térmico | `/monitoring` | ✅ Completa |
| Pacientes | `/patients` | 🔄 Placeholder |
| Histórico | `/history` | 🔄 Placeholder |
| Alertas | `/alerts` | ✅ Completa |
| Relatórios | `/reports` | 🔄 Placeholder |
| Dispositivos | `/devices` | ✅ Completa |
| Configurações | `/settings` | 🔄 Placeholder |

## 🎨 Explorar o Design

### Dashboard Principal (`/`)
- Indicadores vitais coloridos
- Gráfico interativo com 5 métricas
- Alertas em tempo real simulado
- Sidebar com menu

### Monitoramento Térmico (`/monitoring`)
- Heatmap 8x8 do sensor AMG8833
- Escala de temperatura interativa
- Painel de dispositivos conectados
- Informações técnicas do sensor

## 🔄 Dados em Tempo Real

O sistema simula atualizações de dados a cada 30 segundos. Você pode:
- Ver os gráficos atualizando
- Receber novos alertas
- Acompanhar mudanças de temperatura

## 📦 Build para Produção

```bash
npm run build
npm start
```

## 🧪 Verificar Qualidade

```bash
npm run type-check
npm run lint
```

## 📚 Documentação Completa

- [README.md](./README.md) - Visão geral e setup
- [ARCHITECTURE.md](./ARCHITECTURE.md) - Arquitetura detalhada
- [DEVELOPMENT_GUIDE.md](./DEVELOPMENT_GUIDE.md) - Integração backend
- [PROJECT_CHECKLIST.md](./PROJECT_CHECKLIST.md) - Status do projeto

## 🔌 Integração com Backend (Futuro)

Quando estiver pronto para conectar com um backend real:

1. Setup o backend Node.js/NestJS
2. Configure `.env.local`:
   ```
   NEXT_PUBLIC_API_URL=http://localhost:3001/api
   NEXT_PUBLIC_WS_URL=ws://localhost:3001
   ```
3. Em `lib/api-service.ts`, altere:
   ```typescript
   private useMockData = false;
   ```

Veja [DEVELOPMENT_GUIDE.md](./DEVELOPMENT_GUIDE.md) para detalhes completos.

## 🆘 Troubleshooting

### Porta 3000 já em uso
```bash
npm run dev -- -p 3001
```

### Limpar cache
```bash
rm -rf .next node_modules
npm install
npm run dev
```

### Reinstalar dependências
```bash
npm clean-install
npm run dev
```

## 📱 Testar Responsividade

Use o DevTools do navegador (F12) e teste os breakpoints:
- Desktop: 1920px
- Notebook: 1366px
- Tablet: 768px
- Mobile: 390px

## 💡 Recursos Úteis

### Dados Mockados
- Sinais vitais com variações realistas
- Matriz 8x8 de heatmap
- Dispositivos conectados
- Alertas de diferentes tipos

### Componentes Customizáveis
Todos os componentes aceitam props e podem ser reutilizados:
```tsx
<VitalIndicator
  icon={<Heart />}
  label="Batimentos"
  value={78}
  unit="BPM"
  status="normal"
  color="red"
/>
```

## 🎯 Próximos Passos

1. **Explorar o código**: Navegue pelos arquivos em `/components` e `/lib`
2. **Customizar dados**: Edite `lib/mock-data.ts`
3. **Preparar backend**: Siga [DEVELOPMENT_GUIDE.md](./DEVELOPMENT_GUIDE.md)
4. **Deploy**: Use Vercel para frontend

## 📞 Suporte

Dúvidas? Confira a documentação ou abra uma issue no repositório.

---

**Aproveite o Synapsea!** 🚀💙
