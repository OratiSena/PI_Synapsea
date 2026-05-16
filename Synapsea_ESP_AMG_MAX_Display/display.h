#pragma once
#include "config.h"

// ─── Funções do sensor AMG (mantidas) ─────────────────────────────────────
void colorbar();
void drawpixels(float *p, uint8_t rows, uint8_t cols, uint8_t boxW, uint8_t boxH, boolean showVal);

// ─── Auxiliares visuais ───────────────────────────────────────────────────
void desenharCabecalho(const char* titulo, uint16_t corLinha = 0x07FF);
void desenharIconeBateria(int x, int y);
void desenharIndicadorPagina(int telaAtual, int total);
void desenharCard(int x, int y, int w, int h, uint16_t corBorda);
void desenharStatus(int x, int y, int w, int h, const char* texto, uint16_t corFundo, uint16_t corTexto);

// ─── Telas ────────────────────────────────────────────────────────────────
void desenharTelaHome();
void atualizarTelaHome();
void desenharTelaTemperatura();
void desenharTelaHeart();
void atualizarTelaHeart();
void desenharTelaPulse();
void atualizarTelaPulse();
void desenharTelaAnalysis();
void atualizarTelaAnalysis();
void desenharTelaSummary();
void atualizarTelaSummary();

// ─── ECG/PPG ──────────────────────────────────────────────────────────────
void atualizarECG();
void atualizarPPGPulse();

// ─── Navegação ────────────────────────────────────────────────────────────
void verificarBotaoTrocaTela();
