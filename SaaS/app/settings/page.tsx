'use client';

import { Container, Card } from '@/components/ui/Card';
import { Settings } from 'lucide-react';

export default function SettingsPage() {
  return (
    <Container>
      <div className="space-y-4">
        <div className="flex items-center gap-3">
          <Settings className="w-8 h-8 text-primary" />
          <h1 className="text-3xl font-bold text-white">Configurações</h1>
        </div>
        <Card className="p-12 text-center">
          <p className="text-text-secondary mb-4">
            Seção de Configurações em desenvolvimento
          </p>
          <p className="text-sm text-text-secondary">
            Esta página irá permitir configurar as preferências do sistema.
          </p>
        </Card>
      </div>
    </Container>
  );
}
