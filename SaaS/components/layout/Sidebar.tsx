'use client';

import { useState } from 'react';
import Link from 'next/link';
import { usePathname } from 'next/navigation';
import {
  Home,
  Users,
  Activity,
  History,
  Bell,
  FileText,
  Settings,
  Zap,
  ExternalLink,
} from 'lucide-react';
import { cn } from '@/lib/utils';
import { Button } from '../ui/Button';

const menuItems = [
  { label: 'Visão Geral', href: '/', icon: Home },
  { label: 'Pacientes', href: '/patients', icon: Users },
  { label: 'Monitoramento', href: '/monitoring', icon: Activity },
  { label: 'Histórico', href: '/history', icon: History },
  { label: 'Alertas', href: '/alerts', icon: Bell },
  { label: 'Relatórios', href: '/reports', icon: FileText },
  { label: 'Dispositivos', href: '/devices', icon: Zap },
  { label: 'Configurações', href: '/settings', icon: Settings },
];

export function Sidebar() {
  const pathname = usePathname();

  return (
    <aside className="fixed left-0 top-0 h-screen w-[280px] bg-sidebar border-r border-border overflow-y-auto">
      <div className="p-6">
        {/* Logo */}
        <div className="flex items-center gap-3 mb-8">
          <div className="w-10 h-10 bg-gradient-to-br from-primary to-cyan-400 rounded-lg flex items-center justify-center">
            <Zap className="w-6 h-6 text-black" />
          </div>
          <h1 className="text-xl font-bold text-white">Synapsea</h1>
        </div>

        {/* Menu */}
        <nav className="space-y-2 mb-12">
          {menuItems.map((item) => {
            const Icon = item.icon;
            const isActive = pathname === item.href;

            return (
              <Link key={item.href} href={item.href}>
                <div
                  className={cn(
                    'flex items-center gap-3 px-4 py-3 rounded-lg transition-all duration-300 cursor-pointer',
                    isActive
                      ? 'bg-primary/20 text-primary border-l-2 border-primary'
                      : 'text-text-secondary hover:bg-white/5'
                  )}
                >
                  <Icon className="w-5 h-5" />
                  <span className="text-sm font-medium">{item.label}</span>
                </div>
              </Link>
            );
          })}
        </nav>

        {/* About Card */}
        <div className="glass p-4 space-y-3">
          <h3 className="text-sm font-semibold text-white">Sobre o Synapsea</h3>
          <p className="text-xs text-text-secondary leading-relaxed">
            Sistema de monitoramento de saúde de baixo custo para fins educacionais, pesquisa e prevenção.
          </p>
          <Button variant="primary" size="sm" className="w-full">
            <ExternalLink className="w-4 h-4 mr-2" />
            Saiba mais
          </Button>
        </div>
      </div>
    </aside>
  );
}
