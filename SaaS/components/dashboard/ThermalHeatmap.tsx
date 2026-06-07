'use client';

import { useEffect, useRef } from 'react';
import { TemperatureData } from '@/lib/types';

interface HeatmapProps {
  data: TemperatureData;
  width?: number;
  height?: number;
}

export function ThermalHeatmap({ data, width = 600, height = 600 }: HeatmapProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null);

  useEffect(() => {
    if (!canvasRef.current) return;

    const canvas = canvasRef.current;
    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    const cellWidth = width / 8;
    const cellHeight = height / 8;

    // Limpar canvas
    ctx.fillStyle = '#050816';
    ctx.fillRect(0, 0, width, height);

    // Função para converter temperatura para cor (blue -> green -> yellow -> red)
    const getTempColor = (temp: number, min: number, max: number) => {
      const normalized = (temp - min) / (max - min);

      if (normalized < 0.25) {
        // Blue
        const t = normalized / 0.25;
        return `rgb(0, 100, 255)`;
      } else if (normalized < 0.5) {
        // Blue to Green
        const t = (normalized - 0.25) / 0.25;
        const r = 0;
        const g = Math.round(100 + t * 100);
        const b = Math.round(255 - t * 155);
        return `rgb(${r}, ${g}, ${b})`;
      } else if (normalized < 0.75) {
        // Green to Yellow
        const t = (normalized - 0.5) / 0.25;
        const r = Math.round(t * 255);
        const g = 200;
        const b = Math.round(100 - t * 100);
        return `rgb(${r}, ${g}, ${b})`;
      } else {
        // Yellow to Red
        const t = (normalized - 0.75) / 0.25;
        const r = 255;
        const g = Math.round(200 - t * 100);
        const b = 0;
        return `rgb(${r}, ${g}, ${b})`;
      }
    };

    // Desenhar células
    data.grid.forEach((row, i) => {
      row.forEach((temp, j) => {
        const x = j * cellWidth;
        const y = i * cellHeight;

        ctx.fillStyle = getTempColor(temp, data.minTemp, data.maxTemp);
        ctx.fillRect(x, y, cellWidth, cellHeight);

        // Border
        ctx.strokeStyle = 'rgba(255, 255, 255, 0.1)';
        ctx.lineWidth = 1;
        ctx.strokeRect(x, y, cellWidth, cellHeight);

        // Temperatura no centro
        ctx.fillStyle = 'rgba(255, 255, 255, 0.6)';
        ctx.font = '10px Inter';
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.fillText(temp.toFixed(1), x + cellWidth / 2, y + cellHeight / 2);
      });
    });

    // Marcador do ponto mais quente
    const hotspotX = data.hotspotX * cellWidth + cellWidth / 2;
    const hotspotY = data.hotspotY * cellHeight + cellHeight / 2;

    ctx.strokeStyle = '#ef4444';
    ctx.lineWidth = 3;
    ctx.beginPath();
    ctx.arc(hotspotX, hotspotY, 25, 0, Math.PI * 2);
    ctx.stroke();

    ctx.fillStyle = 'rgba(239, 68, 68, 0.2)';
    ctx.beginPath();
    ctx.arc(hotspotX, hotspotY, 25, 0, Math.PI * 2);
    ctx.fill();
  }, [data, width, height]);

  return (
    <canvas
      ref={canvasRef}
      width={width}
      height={height}
      className="w-full h-auto rounded-lg"
    />
  );
}
