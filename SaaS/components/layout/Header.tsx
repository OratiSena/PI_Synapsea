'use client';

import { Bell, Moon, User, Calendar } from 'lucide-react';
import { useState } from 'react';
import { useDashboardStore } from '@/lib/store';

export function Header() {
  const [selectedDate, setSelectedDate] = useState(new Date().toISOString().split('T')[0]);
  const toggleTheme = useDashboardStore((state) => state.toggleTheme);

  return (
    <header className="fixed top-0 left-[280px] right-0 h-20 bg-background border-b border-border z-40">
      <div className="h-full px-8 flex items-center justify-between">
        {/* Left: Greeting */}
        <div>
          <h2 className="text-2xl font-bold text-white">Olá, Nelis 👋</h2>
          <p className="text-sm text-text-secondary">Aqui está o resumo da saúde educacional hoje.</p>
        </div>

        {/* Right: Controls */}
        <div className="flex items-center gap-6">
          {/* Date Picker */}
          <div className="flex items-center gap-2">
            <Calendar className="w-5 h-5 text-text-secondary" />
            <input
              type="date"
              value={selectedDate}
              onChange={(e) => setSelectedDate(e.target.value)}
              className="bg-card border border-border rounded-lg px-3 py-2 text-sm text-white focus:outline-none focus:border-primary"
            />
          </div>

          {/* Notification Icon */}
          <button className="relative p-2 hover:bg-white/5 rounded-lg transition-colors">
            <Bell className="w-5 h-5 text-text-secondary" />
            <span className="absolute top-1 right-1 w-2 h-2 bg-danger rounded-full" />
          </button>

          {/* Theme Toggle */}
          <button
            onClick={toggleTheme}
            className="p-2 hover:bg-white/5 rounded-lg transition-colors"
          >
            <Moon className="w-5 h-5 text-text-secondary" />
          </button>

          {/* User Avatar */}
          <div className="flex items-center gap-3 pl-6 border-l border-border">
            <div className="w-10 h-10 rounded-full bg-gradient-to-br from-primary to-cyan-400 flex items-center justify-center font-semibold text-black">
              NE
            </div>
            <div className="hidden sm:block">
              <p className="text-sm font-medium text-white">Nelis Evangelista</p>
              <p className="text-xs text-text-secondary">Educadora</p>
            </div>
            <User className="w-4 h-4 text-text-secondary hidden sm:block" />
          </div>
        </div>
      </div>
    </header>
  );
}
