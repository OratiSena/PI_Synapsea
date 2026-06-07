'use client';

import { Card, Badge } from '../ui/Card';
import { Alert } from '@/lib/types';
import { formatTime } from '@/lib/utils';
import * as LucideIcons from 'lucide-react';

interface AlertsListProps {
  alerts: Alert[];
}

type IconName = keyof typeof LucideIcons;

export function AlertsList({ alerts }: AlertsListProps) {
  const getAlertIcon = (iconName?: string) => {
    if (!iconName) return <LucideIcons.AlertCircle className="w-5 h-5" />;
    
    const Icon = LucideIcons[iconName as IconName];
    return Icon ? <Icon className="w-5 h-5" /> : <LucideIcons.AlertCircle className="w-5 h-5" />;
  };

  const getAlertBgColor = (type: string) => {
    switch (type) {
      case 'success':
        return 'bg-success/20';
      case 'warning':
        return 'bg-warning/20';
      case 'critical':
        return 'bg-danger/20';
      default:
        return 'bg-blue-500/20';
    }
  };

  const getAlertTextColor = (type: string) => {
    switch (type) {
      case 'success':
        return 'text-success';
      case 'warning':
        return 'text-warning';
      case 'critical':
        return 'text-danger';
      default:
        return 'text-blue-400';
    }
  };

  return (
    <Card className="p-6 col-span-1">
      <div className="space-y-4">
        <div className="flex items-center justify-between">
          <h3 className="text-lg font-semibold text-white">Alertas recentes</h3>
          <a href="/alerts" className="text-xs text-primary hover:text-cyan-300">
            Ver todos
          </a>
        </div>

        <div className="space-y-3">
          {alerts.slice(0, 5).map((alert) => (
            <div
              key={alert.id}
              className={`p-3 rounded-lg border border-border hover:bg-white/5 transition-colors ${
                alert.read ? 'opacity-60' : ''
              }`}
            >
              <div className="flex items-start gap-3">
                <div className={`p-2 rounded-lg ${getAlertBgColor(alert.type)}`}>
                  <div className={getAlertTextColor(alert.type)}>
                    {getAlertIcon(alert.icon)}
                  </div>
                </div>

                <div className="flex-1 min-w-0">
                  <p className="text-sm font-medium text-white truncate">
                    {alert.title}
                  </p>
                  <p className="text-xs text-text-secondary truncate">
                    {alert.description}
                  </p>
                </div>

                <div className="text-xs text-text-secondary whitespace-nowrap">
                  {formatTime(alert.timestamp)}
                </div>
              </div>
            </div>
          ))}
        </div>

        {alerts.length === 0 && (
          <div className="text-center py-6">
            <p className="text-sm text-text-secondary">Nenhum alerta no momento</p>
          </div>
        )}
      </div>
    </Card>
  );
}
