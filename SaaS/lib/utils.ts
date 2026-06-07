import { clsx, type ClassValue } from "clsx"
import { twMerge } from "tailwind-merge"

export function cn(...inputs: ClassValue[]) {
  return twMerge(clsx(inputs))
}

export function formatTime(timestamp: number): string {
  const date = new Date(timestamp);
  const now = new Date();
  const diffMs = now.getTime() - date.getTime();
  const diffMins = Math.floor(diffMs / 60000);
  const diffHours = Math.floor(diffMins / 60);
  const diffDays = Math.floor(diffHours / 24);

  if (diffMins < 1) return 'Agora';
  if (diffMins < 60) return `${diffMins}m`;
  if (diffHours < 24) return `${diffHours}h`;
  if (diffDays < 7) return `${diffDays}d`;

  return date.toLocaleDateString('pt-BR');
}

export function formatValue(value: number, unit: string): string {
  return `${value.toFixed(1)} ${unit}`;
}

export function getStatusColor(status: string): string {
  switch (status) {
    case 'normal':
      return 'text-success';
    case 'warning':
      return 'text-warning';
    case 'critical':
      return 'text-danger';
    default:
      return 'text-text-secondary';
  }
}

export function getStatusBgColor(status: string): string {
  switch (status) {
    case 'normal':
      return 'bg-success/20 text-success';
    case 'warning':
      return 'bg-warning/20 text-warning';
    case 'critical':
      return 'bg-danger/20 text-danger';
    default:
      return 'bg-slate-500/20 text-slate-400';
  }
}
