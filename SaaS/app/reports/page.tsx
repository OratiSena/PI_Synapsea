'use client';

import { Container, Card } from '@/components/ui/Card';
import { FileText } from 'lucide-react';

export default function ReportsPage() {
  return (
    <Container>
      <div className="space-y-4">
        <div className="flex items-center gap-3">
          <FileText className="w-8 h-8 text-primary" />
          <h1 className="text-3xl font-bold text-white">Relatórios</h1>
        </div>
        <Card className="p-12 text-center">
          <p className="text-text-secondary mb-4">
            Seção de Relatórios em desenvolvimento
          </p>
          <p className="text-sm text-text-secondary">
            Esta página irá gerar e exibir relatórios de monitoramento de saúde.
          </p>
        </Card>
      </div>
    </Container>
  );
}
