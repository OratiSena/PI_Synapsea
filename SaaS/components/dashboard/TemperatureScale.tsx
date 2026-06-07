'use client';

import { TemperatureData } from '@/lib/types';

interface TemperatureScaleProps {
  data: TemperatureData;
  height?: number;
}

export function TemperatureScale({ data, height = 600 }: TemperatureScaleProps) {
  const steps = 10;
  const rangeSize = (data.maxTemp - data.minTemp) / steps;

  return (
    <div className="flex flex-col items-center gap-4 h-full">
      <div style={{ height }}>
        <div className="relative w-12 h-full rounded-lg overflow-hidden border border-border">
          {/* Gradiente */}
          <svg width="100%" height="100%" viewBox="0 0 48 600">
            <defs>
              <linearGradient
                id="tempGradient"
                x1="0%"
                y1="0%"
                x2="0%"
                y2="100%"
              >
                <stop offset="0%" stopColor="rgb(0, 100, 255)" />
                <stop offset="25%" stopColor="rgb(0, 200, 155)" />
                <stop offset="50%" stopColor="rgb(255, 200, 0)" />
                <stop offset="100%" stopColor="rgb(255, 0, 0)" />
              </linearGradient>
            </defs>
            <rect width="48" height="600" fill="url(#tempGradient)" />
          </svg>

          {/* Marcadores */}
          <div
            className="absolute left-0 right-0 flex items-center gap-2 bg-card/80 backdrop-blur px-2 py-1 border-b border-border"
            style={{ top: '0%' }}
          >
            <span className="text-xs font-semibold text-danger">
              {data.maxTemp}°C
            </span>
          </div>

          <div
            className="absolute left-0 right-0 flex items-center gap-2 bg-card/80 backdrop-blur px-2 py-1 border-b border-border"
            style={{
              top: `${((data.avgTemp - data.minTemp) / (data.maxTemp - data.minTemp)) * 100}%`,
              transform: 'translateY(-50%)',
            }}
          >
            <span className="text-xs font-semibold text-primary">
              {data.avgTemp}°C
            </span>
          </div>

          <div
            className="absolute left-0 right-0 flex items-center gap-2 bg-card/80 backdrop-blur px-2 py-1 border-t border-border"
            style={{ bottom: '0%' }}
          >
            <span className="text-xs font-semibold text-blue-400">
              {data.minTemp}°C
            </span>
          </div>
        </div>
      </div>

      {/* Label */}
      <div className="text-center">
        <p className="text-xs text-text-secondary">Escala Térmica</p>
      </div>
    </div>
  );
}
