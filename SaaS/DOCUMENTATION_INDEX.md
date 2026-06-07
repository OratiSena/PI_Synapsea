# 📚 Índice de Documentação - Synapsea

Bem-vindo ao Synapsea! Este arquivo ajuda você a navegar pela documentação do projeto.

## 🚀 Comece Aqui

### Para Iniciantes
1. **[QUICKSTART.md](./QUICKSTART.md)** - Como rodar o projeto em 5 minutos
2. **[README.md](./README.md)** - Visão geral do projeto
3. **[PROJECT_STRUCTURE.md](./PROJECT_STRUCTURE.md)** - Entenda a estrutura de pastas

### Primeira Execução
```bash
npm install
npm run dev
# Abra http://localhost:3000
```

---

## 📖 Documentação Completa

### Guides (Guias Passo a Passo)
| Documento | Descrição | Quando Ler |
|-----------|-----------|-----------|
| [QUICKSTART.md](./QUICKSTART.md) | Começar em 5 min | Agora |
| [README.md](./README.md) | Visão geral + setup | Entender projeto |
| [ARCHITECTURE.md](./ARCHITECTURE.md) | Design técnico | Entender código |
| [DEVELOPMENT_GUIDE.md](./DEVELOPMENT_GUIDE.md) | Integrar backend | Implementar backend |
| [PROJECT_STRUCTURE.md](./PROJECT_STRUCTURE.md) | Estrutura de pastas | Navegar projeto |
| [USAGE_EXAMPLES.md](./USAGE_EXAMPLES.md) | Exemplos de código | Desenvolver novo |
| [FAQ.md](./FAQ.md) | Perguntas frequentes | Resolver problema |
| [PROJECT_CHECKLIST.md](./PROJECT_CHECKLIST.md) | Status do projeto | Acompanhar progresso |

---

## 🎯 Por Caso de Uso

### "Quero rodar o projeto agora"
1. Leia [QUICKSTART.md](./QUICKSTART.md)
2. Execute `npm install && npm run dev`
3. Visite http://localhost:3000

### "Quero entender a arquitetura"
1. Leia [PROJECT_STRUCTURE.md](./PROJECT_STRUCTURE.md)
2. Leia [ARCHITECTURE.md](./ARCHITECTURE.md)
3. Explore os arquivos em `/lib` e `/components`

### "Quero adicionar um novo componente"
1. Leia [ARCHITECTURE.md](./ARCHITECTURE.md) - Componentização
2. Consulte [USAGE_EXAMPLES.md](./USAGE_EXAMPLES.md) - Padrões
3. Crie o componente seguindo os exemplos

### "Quero integrar com backend"
1. Leia [DEVELOPMENT_GUIDE.md](./DEVELOPMENT_GUIDE.md)
2. Setup Node.js/NestJS
3. Configure `.env.local`
4. Altere `useMockData = false` em `api-service.ts`

### "Algo deu erro"
1. Consulte [FAQ.md](./FAQ.md)
2. Verifique console do browser (F12)
3. Tente limpar cache: `rm -rf .next && npm run dev`

### "Quero fazer deploy"
1. Leia [README.md](./README.md) - Build para Produção
2. Use Vercel (recomendado): `npm i -g vercel && vercel`

### "Quero customizar dados"
1. Leia [USAGE_EXAMPLES.md](./USAGE_EXAMPLES.md) - Usar Mock Data
2. Edite `/lib/mock-data.ts`

### "Quero verificar o status do projeto"
1. Consulte [PROJECT_CHECKLIST.md](./PROJECT_CHECKLIST.md)
2. Veja o que falta implementar

---

## 📚 Referência Rápida

### Instalação e Setup
```bash
npm install              # Instalar dependências
npm run dev              # Modo desenvolvimento
npm run build            # Build otimizado
npm start                # Modo produção
npm run type-check       # Verificar tipos
npm run lint             # Lint de código
```

### Estrutura de Pastas
```
SaaS/
├── /app                 # Páginas (Next.js)
├── /components          # Componentes React
├── /lib                 # Lógica & serviços
├── /public              # Arquivos estáticos
├── package.json         # Dependências
├── tsconfig.json        # TypeScript
├── tailwind.config.js   # Estilos
└── README.md            # Este arquivo
```

### Páginas Disponíveis
- `/` - Dashboard principal
- `/monitoring` - Monitoramento de temperatura
- `/alerts` - Alertas
- `/devices` - Dispositivos
- `/patients`, `/history`, `/reports`, `/settings` - Placeholders

---

## 🔑 Conceitos-Chave

### Tecnologias Principais
- **Next.js 15**: Framework React com SSR
- **React 19**: Biblioteca de UI
- **TypeScript**: Type safety
- **Tailwind CSS**: Utility-first styling
- **Zustand**: State management
- **Recharts**: Gráficos
- **Framer Motion**: Animações

### Arquitetura
- **Components**: Reutilizáveis, compostas
- **Store**: Estado global com Zustand
- **Services**: API + Mock data
- **Types**: Interfaces TypeScript
- **Utils**: Funções auxiliares

### Fluxo de Dados
```
User Action → Component → Store → API → Backend/Mock → UI
```

---

## 📱 Suporte a Dispositivos

- ✅ Desktop (1920px)
- ✅ Notebook (1366px)
- ✅ Tablet (768px)
- ✅ Smartphone (390px)

Teste usando DevTools (F12) → Responsive Design Mode

---

## 🎨 Design System

### Cores Principais
- **Primary**: `#00D4FF` (Cyan)
- **Success**: `#22c55e` (Green)
- **Warning**: `#f59e0b` (Orange)
- **Danger**: `#ef4444` (Red)
- **Background**: `#050816`

### Tipografia
- Font: Inter
- Pesos: 400, 500, 600, 700

---

## 🚀 Roadmap

### ✅ Fase 1: Estrutura Base (Concluída)
- Projeto Next.js
- Componentes
- Mock data

### 📋 Fase 2: Backend (Próxima)
- Node.js/NestJS
- PostgreSQL
- WebSocket

### 🔮 Fase 3: Melhorias (Futuro)
- Autenticação
- Histórico persistente
- Machine Learning

---

## 🆘 Precisa de Ajuda?

1. **Erro?** → Consulte [FAQ.md](./FAQ.md)
2. **Como fazer?** → Consulte [USAGE_EXAMPLES.md](./USAGE_EXAMPLES.md)
3. **Entender código?** → Consulte [ARCHITECTURE.md](./ARCHITECTURE.md)
4. **Setup backend?** → Consulte [DEVELOPMENT_GUIDE.md](./DEVELOPMENT_GUIDE.md)
5. **Estrutura?** → Consulte [PROJECT_STRUCTURE.md](./PROJECT_STRUCTURE.md)
6. **Começar?** → Consulte [QUICKSTART.md](./QUICKSTART.md)

---

## 📞 Contato & Suporte

- 📧 Email: contato@synapsea.com
- 🐙 GitHub: Abra uma issue
- 💬 Discussões: Use as discussions do repo

---

## 📊 Estatísticas do Projeto

| Métrica | Valor |
|---------|-------|
| Linhas de Código | ~2000+ |
| Componentes | 15+ |
| Páginas | 8 |
| Documentação | ~500KB |
| Dependências | 18 |
| Tempo de Desenvolvimento | 100% |

---

## 🎓 Aprendizado

### Conceitos Cobertos
- React Hooks
- Next.js App Router
- TypeScript avançado
- Tailwind CSS
- Zustand
- Canvas API
- Recharts

### Recursos Externo
- [Next.js Docs](https://nextjs.org/docs)
- [React Docs](https://react.dev)
- [TypeScript Handbook](https://www.typescriptlang.org/docs)
- [Tailwind CSS](https://tailwindcss.com)

---

## ✨ Destaques do Projeto

- 🎨 Design premium dark mode
- 📊 Gráficos interativos com Recharts
- 🔥 Heatmap Canvas 8x8
- ⚡ Performance otimizada
- 📱 Totalmente responsivo
- 🎯 Clean Architecture
- 📚 Documentação completa
- 🧪 Pronto para testes

---

## 📝 Changelog

### v1.0.0 (Atual)
- ✅ Setup inicial completo
- ✅ Dashboard funcional
- ✅ Monitoramento térmico
- ✅ Sistema de alertas
- ✅ Documentação completa

---

## 📄 Licença

Projeto acadêmico - Synapsea 2025

---

**Bem-vindo ao Synapsea! 🚀💙**

**Próximo passo?** → Abra [QUICKSTART.md](./QUICKSTART.md)
