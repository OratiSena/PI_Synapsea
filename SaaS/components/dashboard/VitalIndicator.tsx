'use client';

import { ReactNode } from 'react';
import { Card, Badge } from '../ui/Card';
import { cn } from '@/lib/utils';

interface VitalIndicatorProps {
  icon: ReactNode;
  label: string;
  value: number | string;
  unit: string;
  status: 'normal' | 'warning' | 'critical';
  color: 'red' | 'blue' | 'purple' | 'orange' | 'cyan' | 'green';
}

const colorClasses = {
  red: 'bg-danger/20 text-danger border-danger/30',
  blue: 'bg-blue-500/20 text-blue-400 border-blue-500/30',
  purple: 'bg-purple-500/20 text-purple-400 border-purple-500/30',
  orange: 'bg-warning/20 text-warning border-warning/30',
  cyan: 'bg-primary/20 text-primary border-primary/30',
  green: 'bg-success/20 text-success border-success/30',
};

const statusClasses = {
  normal: 'text-success',
  warning: 'text-warning',
  critical: 'text-danger',
};

export function VitalIndicator({
  icon,
  label,
  value,
  unit,
  status,
  color,
}: VitalIndicatorProps) {
  return (
    <Card className="p-6 hover">
      <div className="space-y-4">
        {/* Icon */}
        <div
          className={cn(
            'w-12 h-12 rounded-full flex items-center justify-center border',
            colorClasses[color]
          )}
        >
          {icon}
        </div>

        {/* Value */}
        <div>
          <p className="text-3xl font-bold text-white">{value}</p>
          <p className="text-xs text-text-secondary">{unit}</p>
        </div>

        {/* Status */}
        <Badge
          variant={
            status === 'normal'
              ? 'success'
              : status === 'warning'
              ? 'warning'
              : 'danger'
          }
        >
          {status === 'normal' && '● Normal'}
          {status === 'warning' && '● Atenção'}
          {status === 'critical' && '● Crítico'}
        </Badge>

        {/* Label */}
        <p className="text-xs font-medium text-text-secondary">{label}</p>
      </div>
    </Card>
  );
}
