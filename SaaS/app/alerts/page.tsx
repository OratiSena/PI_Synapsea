'use client';

import { useEffect, useState } from 'react';
import { Bell } from 'lucide-react';
import { useDashboardStore } from '@/lib/store';
import { apiService } from '@/lib/api-service';
import { Container, Card } from '@/components/ui/Card';
import { AlertsList } from '@/components/dashboard/AlertsList';

export default function AlertsPage() {
  const { alerts, setAlerts } = useDashboardStore();
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    const loadData = async () => {
      setLoading(true);
      try {
        const alertsData = await apiService.getAlerts();
        setAlerts(alertsData);
      } catch (error) {
        console.error('Error loading alerts:', error);
      } finally {
        setLoading(false);
      }
    };

    loadData();
  }, [setAlerts]);

  return (
    <Container>
      <div className="space-y-4">
        <div className="flex items-center gap-3">
          <Bell className="w-8 h-8 text-primary" />
          <h1 className="text-3xl font-bold text-white">Alertas</h1>
        </div>

        <Card className="p-8">
          <div className="space-y-4">
            {alerts.map((alert) => (
              <div
                key={alert.id}
                className="p-4 rounded-lg border border-border hover:bg-white/5 transition-colors"
              >
                <div className="flex items-start justify-between">
                  <div>
                    <p className="text-base font-semibold text-white">
                      {alert.title}
                    </p>
                    <p className="text-sm text-text-secondary mt-1">
                      {alert.description}
                    </p>
                  </div>
                  <div className="text-xs text-text-secondary">
                    {new Date(alert.timestamp).toLocaleString('pt-BR')}
                  </div>
                </div>
              </div>
            ))}
          </div>
        </Card>
      </div>
    </Container>
  );
}
