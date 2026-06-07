'use client';

import { Container, Card } from '@/components/ui/Card';

export default function PatientsPage() {
  return (
    <Container>
      <div className="space-y-4">
        <h1 className="text-3xl font-bold text-white">Pacientes</h1>
        <Card className="p-12 text-center">
          <p className="text-text-secondary mb-4">
            Seção de Pacientes em desenvolvimento
          </p>
          <p className="text-sm text-text-secondary">
            Esta página irá listar e gerenciar todos os pacientes do sistema.
          </p>
        </Card>
      </div>
    </Container>
  );
}
