'use client';

import { Container, Card } from '@/components/ui/Card';

export default function HistoryPage() {
  return (
    <Container>
      <div className="space-y-4">
        <h1 className="text-3xl font-bold text-white">Histórico</h1>
        <Card className="p-12 text-center">
          <p className="text-text-secondary mb-4">
            Seção de Histórico em desenvolvimento
          </p>
          <p className="text-sm text-text-secondary">
            Esta página irá exibir o histórico completo de sinais vitais e eventos.
          </p>
        </Card>
      </div>
    </Container>
  );
}
