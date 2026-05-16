// ─── display.cpp ──────────────────────────────────────────────────────────
// Interface visual do display ILI9341 — Synapsea
// Telas: 0=Home, 1=Temperatura, 2=Heart, 3=Pulse, 4=Analysis, 5=Summary
// ──────────────────────────────────────────────────────────────────────────
#include "display.h"
#include "sensorAMG.h"
#include <math.h>

// ─── Cores do projeto ─────────────────────────────────────────────────────
#define COR_FUNDO       0x0000
#define COR_CARD        0x0841
#define COR_CIANO       0x07FF
#define COR_VERDE       0x07E0
#define COR_VERMELHO    0xF800
#define COR_AMARELO     0xFFE0
#define COR_ROXO        0xA11F
#define COR_LARANJA     0xFD20
#define COR_BRANCO      0xFFFF
#define COR_CINZA       0x4208
#define COR_CINZA_ESC   0x2104

// ─── Buffers PPG e ECG ────────────────────────────────────────────────────
#define PPG_W 218
#define PPG_AREA_X  11
#define PPG_AREA_Y  52
#define PPG_AREA_W  218
#define PPG_AREA_H  98

static int  ppgBuf[PPG_W]  = {0};
static int  ppgPos          = 0;
static long ppgPrevIR       = 0;
static int  ecgBuf2[ECG_W] = {0};
static int  ecgPos2         = 0;
static long prevIR2         = 0;

// ─── Relógio simulado ─────────────────────────────────────────────────────
static int horaSimulada    = 14;
static int minutoSimulado  = 32;

// ─── Variáveis externas do .ino ───────────────────────────────────────────
extern int   telaAtual;
extern bool  btnAnterior;
extern unsigned long ultimoDebounce;
extern const unsigned long debounceDelay;
extern bool  telaPrecisaRedesenhar;

// ══════════════════════════════════════════════════════════════════════════
// FUNÇÕES AUXILIARES
// ══════════════════════════════════════════════════════════════════════════

void desenharCabecalho(const char* titulo, uint16_t corLinha) {
  tft.fillRect(0, 0, 240, 32, COR_CARD);
  tft.setTextColor(COR_CIANO);
  tft.setTextSize(1);
  tft.setCursor(6, 6);
  tft.print("Synapsea");
  tft.setTextColor(COR_BRANCO);
  tft.setTextSize(2);
  int len = strlen(titulo);
  int cx  = (240 - len * 12) / 2;
  tft.setCursor(cx, 10);
  tft.print(titulo);
  tft.fillRect(0, 32, 240, 2, corLinha);
}

void desenharIconeBateria(int x, int y) {
  tft.drawRect(x, y, 20, 10, COR_CINZA);
  tft.fillRect(x + 20, y + 3, 3, 4, COR_CINZA);
  tft.fillRect(x + 2, y + 2, 14, 6, COR_VERDE);
}

void desenharIndicadorPagina(int telaAtualLocal, int total) {
  int dotW   = 8;
  int gap    = 5;
  int totalW = total * dotW + (total - 1) * gap;
  int startX = (240 - totalW) / 2;
  int y      = 312;
  for (int i = 0; i < total; i++) {
    int xi = startX + i * (dotW + gap);
    if (i == telaAtualLocal) tft.fillCircle(xi + 4, y, 4, COR_CIANO);
    else                     tft.drawCircle(xi + 4, y, 3, COR_CINZA);
  }
}

void desenharCard(int x, int y, int w, int h, uint16_t corBorda) {
  tft.fillRoundRect(x, y, w, h, 6, COR_CARD);
  tft.drawRoundRect(x, y, w, h, 6, corBorda);
}

void desenharStatus(int x, int y, int w, int h, const char* texto, uint16_t corFundo, uint16_t corTexto) {
  tft.fillRoundRect(x, y, w, h, 5, corFundo);
  tft.setTextColor(corTexto);
  tft.setTextSize(2);
  int len = strlen(texto);
  int cx  = x + (w - len * 12) / 2;
  tft.setCursor(cx, y + (h - 16) / 2);
  tft.print(texto);
}

// ══════════════════════════════════════════════════════════════════════════
// AMG — colorbar e drawpixels (mantidos, ajuste de offset Y para caber layout)
// ══════════════════════════════════════════════════════════════════════════

void colorbar() {
  // barra de cores abaixo da imagem térmica (y=277, imagem termina em y=276)
  tft.fillRect(0, 277, 240, 26, COR_FUNDO);
  for (int i = 0; i < 240; i++) {
    int colorIndex = constrain((int)(i * 1.05), 0, maxIndex);
    tft.fillRect(i, 277, 1, 26, camColors[colorIndex]);
  }
  tft.setTextColor(COR_BRANCO);
  tft.setTextSize(1);
  tft.setCursor(2, 281);
  tft.print(MINTEMP); tft.print("C");
  tft.setCursor(210, 281);
  tft.print(MAXTEMP); tft.print("C");
}

void drawpixels(float *p, uint8_t rows, uint8_t cols, uint8_t boxW, uint8_t boxH, boolean showVal) {
  // offsetY = 36 para caber abaixo do cabeçalho da tela de temperatura
  for (int y = 0; y < rows; y++) {
    for (int x = 0; x < cols; x++) {
      float val = get_point(p, rows, cols, x, y);
      int idx = map(val, MINTEMP, MAXTEMP, 0, 239);
      idx = constrain(idx, 0, 239);
      tft.fillRect(x * boxW, y * boxH + 36, boxW, boxH, camColors[idx]);
      if (val > pix_max) {
        pix_max = val;
        pos_x   = x * boxW;
        pos_y   = y * boxH + 36;
      }
      if (showVal) {
        tft.setCursor(x * boxW + boxW / 2 - 12, y * boxH + 36 + boxH / 2 + 4);
        tft.setTextColor(COR_BRANCO);
        tft.setTextSize(1);
        tft.print(val, 1);
      }
    }
  }
}

// ══════════════════════════════════════════════════════════════════════════
// TELA 0 — HOME / CLOCK
// ══════════════════════════════════════════════════════════════════════════

void desenharTelaHome() {
  tft.fillScreen(COR_FUNDO);

  // Cabeçalho
  tft.fillRect(0, 0, 240, 42, COR_CARD);
  tft.setTextColor(COR_BRANCO);
  tft.setTextSize(2);
  tft.setCursor(56, 10);
  tft.print("Synapsea");
  desenharIconeBateria(210, 14);
  tft.fillRect(0, 42, 240, 2, COR_CIANO);

  // Subtítulo
  tft.setTextColor(COR_CIANO);
  tft.setTextSize(1);
  tft.setCursor(44, 52);
  tft.print("Monitoramento inteligente");

  // Relógio grande
  tft.setTextColor(COR_BRANCO);
  tft.setTextSize(6);
  char horaBuf[6];
  snprintf(horaBuf, sizeof(horaBuf), "%02d:%02d", horaSimulada, minutoSimulado);
  tft.setCursor(10, 70);
  tft.print(horaBuf);

  // Linha decorativa
  tft.fillRect(20, 148, 200, 1, COR_CIANO);
  tft.setTextColor(COR_CINZA);
  tft.setTextSize(1);
  tft.setCursor(50, 155);
  tft.print("Qui  Sao Paulo");
  tft.fillRect(20, 168, 200, 1, COR_CINZA_ESC);

  // Card TEMP
  desenharCard(6, 176, 108, 62, COR_CIANO);
  tft.fillCircle(22, 216, 8, COR_VERMELHO);
  tft.fillRect(19, 190, 6, 28, COR_VERMELHO);
  tft.setTextColor(COR_CIANO);
  tft.setTextSize(1);
  tft.setCursor(34, 181);
  tft.print("TEMP");
  {
    bool tOK = (pix_max > 25.0f && pix_max < 42.0f);
    uint16_t cT = (pix_max < 0.5f) ? COR_CINZA : (tOK ? COR_VERDE : COR_VERMELHO);
    tft.setTextColor(cT); tft.setTextSize(2); tft.setCursor(34, 195);
    if (pix_max > 0.5f) {
      char tBuf[6]; dtostrf(pix_max, 4, 1, tBuf); tft.print(tBuf);
      tft.setTextSize(1); tft.setCursor(82, 201); tft.print("C");
    } else {
      tft.print("--");
    }
  }

  // Card VITAIS
  desenharCard(126, 176, 108, 62, COR_CIANO);
  tft.fillCircle(140, 198, 6, COR_VERMELHO);
  tft.fillCircle(147, 198, 6, COR_VERMELHO);
  tft.fillTriangle(136, 202, 151, 202, 143, 212, COR_VERMELHO);
  tft.setTextColor(COR_CIANO);
  tft.setTextSize(1);
  tft.setCursor(156, 184);
  tft.print("VITAIS");
  bool vitaisOK = (maxOK && dedoDetectado && bpmReal > 40 && spo2Real > 85);
  tft.setTextColor(vitaisOK ? COR_VERDE : (maxOK ? COR_AMARELO : COR_VERMELHO));
  tft.setTextSize(2);
  tft.setCursor(156, 204);
  tft.print(vitaisOK ? "OK" : "--");

  desenharIndicadorPagina(0, 6);
}

void atualizarTelaHome() {
  // Relógio (a cada minuto)
  static unsigned long ultimaAtualHora = 0;
  if (millis() - ultimaAtualHora >= 60000UL) {
    ultimaAtualHora = millis();
    minutoSimulado++;
    if (minutoSimulado >= 60) { minutoSimulado = 0; horaSimulada = (horaSimulada + 1) % 24; }
    tft.fillRect(10, 70, 220, 74, COR_FUNDO);
    tft.setTextColor(COR_BRANCO);
    tft.setTextSize(6);
    char horaBuf[6];
    snprintf(horaBuf, sizeof(horaBuf), "%02d:%02d", horaSimulada, minutoSimulado);
    tft.setCursor(10, 70);
    tft.print(horaBuf);
  }
  // Card TEMP — atualiza valor de temperatura
  static float prevTempVal = -1.0f;
  if (fabsf(pix_max - prevTempVal) > 0.3f) {
    tft.fillRect(34, 192, 74, 22, COR_CARD);
    bool tOK = (pix_max > 25.0f && pix_max < 42.0f);
    uint16_t cT = (pix_max < 0.5f) ? COR_CINZA : (tOK ? COR_VERDE : COR_VERMELHO);
    tft.setTextColor(cT); tft.setTextSize(2); tft.setCursor(34, 195);
    if (pix_max > 0.5f) {
      char tBuf[6]; dtostrf(pix_max, 4, 1, tBuf); tft.print(tBuf);
      tft.setTextSize(1); tft.setCursor(82, 201); tft.print("C");
    } else tft.print("--");
    prevTempVal = pix_max;
  }
  // Card VITAIS
  static bool prevVitOK = false;
  bool vitaisOK = (maxOK && dedoDetectado && bpmReal > 40 && spo2Real > 85);
  if (vitaisOK != prevVitOK) {
    tft.fillRect(156, 200, 72, 20, COR_CARD);
    tft.setTextColor(vitaisOK ? COR_VERDE : (maxOK ? COR_AMARELO : COR_VERMELHO));
    tft.setTextSize(2);
    tft.setCursor(156, 204);
    tft.print(vitaisOK ? "OK" : "--");
    prevVitOK = vitaisOK;
  }
}

// ══════════════════════════════════════════════════════════════════════════
// TELA 1 — TEMPERATURA / AMG8833
// ══════════════════════════════════════════════════════════════════════════

void desenharTelaTemperatura() {
  tft.fillScreen(COR_FUNDO);
  desenharCabecalho("Temperature", COR_CIANO);
  // Ícone termômetro no cabeçalho (direita)
  tft.fillCircle(228, 24, 6, COR_VERMELHO);
  tft.fillRect(225, 8, 6, 18, COR_VERMELHO);

  // Label rodapé
  tft.setTextColor(COR_CINZA);
  tft.setTextSize(1);
  tft.setCursor(4, 306);
  tft.print("AMG8833  8x8");

  colorbar();
  desenharIndicadorPagina(1, 6);
  // Overlay MAX/MIN é redesenhado pelo loop a cada frame
}

// ══════════════════════════════════════════════════════════════════════════
// TELA 2 — HEART
// ══════════════════════════════════════════════════════════════════════════

void desenharTelaHeart() {
  tft.fillScreen(COR_FUNDO);
  desenharCabecalho("Heart", COR_VERMELHO);
  desenharIconeBateria(215, 6);

  // Coração decorativo (forma melhorada)
  int hx = 62, hy = 110;
  tft.fillCircle(hx - 12, hy - 4, 15, COR_VERMELHO);
  tft.fillCircle(hx + 12, hy - 4, 15, COR_VERMELHO);
  tft.fillTriangle(hx - 28, hy + 8, hx + 28, hy + 8, hx, hy + 38, COR_VERMELHO);
  tft.fillRect(hx - 28, hy + 1, 56, 9, COR_VERMELHO);
  // Linha ECG
  int lx = hx - 26, ly = hy + 5;
  tft.drawLine(lx,      ly,      lx + 10, ly,      COR_BRANCO);
  tft.drawLine(lx + 10, ly,      lx + 13, ly - 16, COR_BRANCO);
  tft.drawLine(lx + 13, ly - 16, lx + 16, ly + 14, COR_BRANCO);
  tft.drawLine(lx + 16, ly + 14, lx + 19, ly,      COR_BRANCO);
  tft.drawLine(lx + 19, ly,      lx + 30, ly,      COR_BRANCO);

  // BPM
  tft.setTextColor(COR_BRANCO);
  tft.setTextSize(6);
  tft.setCursor(132, 82);
  if (dedoDetectado && bpmReal > 0) tft.print(bpmReal);
  else tft.print("--");
  tft.setTextColor(COR_VERMELHO);
  tft.setTextSize(2);
  tft.setCursor(142, 144);
  tft.print("BPM");

  tft.fillRect(0, 162, 240, 1, COR_CINZA_ESC);

  // Card SpO2
  desenharCard(6, 168, 108, 58, COR_CIANO);
  tft.setTextColor(COR_CIANO);
  tft.setTextSize(1);
  tft.setCursor(14, 175);
  tft.print("SpO2");
  tft.fillCircle(95, 186, 8, COR_CIANO);
  tft.fillTriangle(87, 183, 103, 183, 95, 169, COR_CIANO);
  tft.setTextColor(COR_BRANCO);
  tft.setTextSize(3);
  tft.setCursor(14, 192);
  if (dedoDetectado && spo2Real > 0) {
    tft.print(spo2Real);
    tft.setTextSize(1);
    tft.setCursor(72, 202);
    tft.print("%");
  } else tft.print("--");

  // Card HRV
  desenharCard(126, 168, 108, 58, COR_VERDE);
  tft.setTextColor(COR_VERDE);
  tft.setTextSize(1);
  tft.setCursor(134, 175);
  tft.print("HRV");
  tft.setTextColor(COR_BRANCO);
  tft.setTextSize(3);
  tft.setCursor(134, 192);
  if (dedoDetectado && hrvRMSSD > 0) {
    tft.print(hrvRMSSD);
    tft.setTextSize(1);
    tft.setCursor(198, 202);
    tft.print("ms");
  } else tft.print("--");

  // Status
  if (!maxOK)                                desenharStatus(6, 234, 228, 34, "SEM SENSOR",  0x2000, COR_VERMELHO);
  else if (!dedoDetectado || qualSinal == 0) desenharStatus(6, 234, 228, 34, "SEM LEITURA", 0x2200, COR_AMARELO);
  else if (qualSinal == 1)                   desenharStatus(6, 234, 228, 34, "SINAL FRACO", 0x4200, COR_LARANJA);
  else                                       desenharStatus(6, 234, 228, 34, "NORMAL",      0x0240, COR_VERDE);

  desenharIndicadorPagina(2, 6);
}

void atualizarTelaHeart() {
  if (bpmReal != prevBPM_d || (bool)dedoDetectado != (bool)prevDedo_d) {
    tft.fillRect(132, 82, 100, 60, COR_FUNDO);
    tft.setTextColor(COR_BRANCO);
    tft.setTextSize(6);
    tft.setCursor(132, 82);
    if (dedoDetectado && bpmReal > 0) tft.print(bpmReal);
    else tft.print("--");
    tft.setTextColor(COR_VERMELHO);
    tft.setTextSize(2);
    tft.setCursor(142, 144);
    tft.print("BPM");
    prevBPM_d = bpmReal;
  }
  if (spo2Real != prevSpo2_d) {
    tft.fillRect(14, 190, 88, 28, COR_CARD);
    tft.setTextColor(COR_BRANCO);
    tft.setTextSize(3);
    tft.setCursor(14, 192);
    if (dedoDetectado && spo2Real > 0) {
      tft.print(spo2Real);
      tft.setTextSize(1); tft.setCursor(72, 202); tft.print("%");
    } else tft.print("--");
    prevSpo2_d = spo2Real;
  }
  if (hrvRMSSD != prevHRV_d) {
    tft.fillRect(134, 190, 88, 28, COR_CARD);
    tft.setTextColor(COR_BRANCO);
    tft.setTextSize(3);
    tft.setCursor(134, 192);
    if (dedoDetectado && hrvRMSSD > 0) {
      tft.print(hrvRMSSD);
      tft.setTextSize(1); tft.setCursor(198, 202); tft.print("ms");
    } else tft.print("--");
    prevHRV_d = hrvRMSSD;
  }
  if (qualSinal != prevQual_d || maxOK != prevMaxOK_d || (bool)dedoDetectado != (bool)prevDedo_d) {
    if (!maxOK)                                desenharStatus(6, 234, 228, 34, "SEM SENSOR",  0x2000, COR_VERMELHO);
    else if (!dedoDetectado || qualSinal == 0) desenharStatus(6, 234, 228, 34, "SEM LEITURA", 0x2200, COR_AMARELO);
    else if (qualSinal == 1)                   desenharStatus(6, 234, 228, 34, "SINAL FRACO", 0x4200, COR_LARANJA);
    else                                       desenharStatus(6, 234, 228, 34, "NORMAL",      0x0240, COR_VERDE);
    prevQual_d  = qualSinal;
    prevMaxOK_d = maxOK;
    prevDedo_d  = dedoDetectado;
  }
}

// ══════════════════════════════════════════════════════════════════════════
// TELA 3 — PULSE (PPG Waveform)
// ══════════════════════════════════════════════════════════════════════════

void desenharTelaPulse() {
  tft.fillScreen(COR_FUNDO);
  desenharCabecalho("Pulse", COR_CIANO);
  desenharIconeBateria(215, 6);

  // BPM no canto da área PPG
  tft.setTextColor(COR_BRANCO);
  tft.setTextSize(3);
  tft.setCursor(168, 38);
  if (dedoDetectado && bpmReal > 0) tft.print(bpmReal);
  else tft.print("--");
  tft.setTextColor(COR_VERMELHO);
  tft.setTextSize(1);
  tft.setCursor(214, 42);
  tft.print("bpm");

  // Área da onda PPG
  tft.drawRect(PPG_AREA_X, PPG_AREA_Y, PPG_AREA_W, PPG_AREA_H, COR_CINZA_ESC);
  for (int gx = PPG_AREA_X; gx < PPG_AREA_X + PPG_AREA_W; gx += 40) {
    tft.drawLine(gx, PPG_AREA_Y, gx, PPG_AREA_Y + PPG_AREA_H, COR_CINZA_ESC);
  }
  tft.drawLine(PPG_AREA_X, PPG_AREA_Y + PPG_AREA_H / 2, PPG_AREA_X + PPG_AREA_W, PPG_AREA_Y + PPG_AREA_H / 2, COR_CINZA_ESC);

  tft.setTextColor(COR_CINZA);
  tft.setTextSize(1);
  tft.setCursor(PPG_AREA_X + 2, PPG_AREA_Y + PPG_AREA_H + 4);
  tft.print("PPG Waveform");

  tft.fillRect(0, 160, 240, 1, COR_CINZA_ESC);

  // Card PI
  desenharCard(6, 164, 72, 64, COR_AMARELO);
  tft.setTextColor(COR_AMARELO);
  tft.setTextSize(1);
  tft.setCursor(14, 172);
  tft.print("PI");
  tft.setTextColor(COR_BRANCO);
  tft.setTextSize(2);
  tft.setCursor(10, 190);
  if (dedoDetectado) {
    char piBuf[8];
    dtostrf(piVal, 3, 1, piBuf);
    tft.print(piBuf);
    tft.setTextSize(1); tft.setCursor(52, 196); tft.print("%");
    int piBarW = constrain((int)(piVal * 4), 0, 58);
    tft.fillRect(10, 216, 58, 5, COR_CINZA_ESC);
    tft.fillRect(10, 216, piBarW, 5, COR_AMARELO);
  } else tft.print("--");

  // Card Resp (placeholder)
  desenharCard(84, 164, 72, 64, COR_ROXO);
  tft.setTextColor(COR_ROXO);
  tft.setTextSize(1);
  tft.setCursor(92, 172);
  tft.print("Resp.");
  tft.setTextColor(COR_BRANCO);
  tft.setTextSize(2);
  tft.setCursor(90, 190);
  tft.print("--");
  tft.setTextColor(COR_ROXO);
  tft.setTextSize(1);
  tft.setCursor(92, 214);
  tft.print("rpm");

  // Card Sinal
  uint16_t corSin = qualSinal >= 2 ? COR_VERDE : (qualSinal == 1 ? COR_LARANJA : COR_CINZA);
  desenharCard(162, 164, 72, 64, corSin);
  tft.setTextColor(COR_CINZA);
  tft.setTextSize(1);
  tft.setCursor(170, 172);
  tft.print("Sinal");
  if (!maxOK || !dedoDetectado || qualSinal == 0) {
    tft.setTextColor(COR_CINZA); tft.setTextSize(2); tft.setCursor(174, 190); tft.print("--");
  } else if (qualSinal == 1) {
    tft.setTextColor(COR_LARANJA); tft.setTextSize(1); tft.setCursor(168, 188); tft.print("Fraco");
  } else {
    tft.setTextColor(COR_VERDE); tft.setTextSize(2); tft.setCursor(170, 190); tft.print("Bom");
  }

  desenharIndicadorPagina(3, 6);

  // Reinicia buffer PPG
  memset(ppgBuf, 0, sizeof(ppgBuf));
  ppgPos    = 0;
  ppgPrevIR = irValue;
}

void atualizarTelaPulse() {
  if (bpmReal != prevBPM_d) {
    tft.fillRect(168, 36, 66, 22, COR_FUNDO);
    tft.setTextColor(COR_BRANCO); tft.setTextSize(3); tft.setCursor(168, 38);
    if (dedoDetectado && bpmReal > 0) tft.print(bpmReal); else tft.print("--");
    tft.setTextColor(COR_VERMELHO); tft.setTextSize(1); tft.setCursor(214, 42); tft.print("bpm");
    prevBPM_d = bpmReal;
  }
  if (fabsf(piVal - prevPI_d) > 0.09f || (bool)dedoDetectado != (bool)prevDedo_d) {
    tft.fillRect(10, 187, 62, 34, COR_CARD);
    tft.setTextColor(COR_BRANCO); tft.setTextSize(2); tft.setCursor(10, 190);
    if (dedoDetectado) {
      char piBuf[8]; dtostrf(piVal, 3, 1, piBuf); tft.print(piBuf);
      tft.setTextSize(1); tft.setCursor(52, 196); tft.print("%");
      int piBarW = constrain((int)(piVal * 4), 0, 58);
      tft.fillRect(10, 216, 58, 5, COR_CINZA_ESC);
      tft.fillRect(10, 216, piBarW, 5, COR_AMARELO);
    } else tft.print("--");
    prevPI_d   = piVal;
    prevDedo_d = dedoDetectado;
  }
  atualizarPPGPulse();
}

void atualizarPPGPulse() {
  long delta  = irValue - ppgPrevIR;
  ppgPrevIR   = irValue;
  int amostra = constrain((int)(delta / 600), -40, 40);
  ppgBuf[ppgPos] = amostra;

  int x    = PPG_AREA_X + ppgPos + 1;
  int midY = PPG_AREA_Y + PPG_AREA_H / 2;

  tft.fillRect(x, PPG_AREA_Y + 1, 2, PPG_AREA_H - 2, COR_FUNDO);
  int ahead = (x - PPG_AREA_X + 4) % PPG_AREA_W + PPG_AREA_X;
  tft.fillRect(ahead, PPG_AREA_Y + 1, 3, PPG_AREA_H - 2, COR_FUNDO);
  // repintar grade na coluna apagada
  tft.drawPixel(ahead, PPG_AREA_Y + PPG_AREA_H / 2, COR_CINZA_ESC);

  int y1 = midY - ppgBuf[(ppgPos + PPG_W - 1) % PPG_W];
  int y2 = midY - amostra;
  y1 = constrain(y1, PPG_AREA_Y + 2, PPG_AREA_Y + PPG_AREA_H - 3);
  y2 = constrain(y2, PPG_AREA_Y + 2, PPG_AREA_Y + PPG_AREA_H - 3);

  tft.drawLine(x - 1, y1, x, y2, dedoDetectado ? COR_CIANO : COR_CINZA_ESC);
  ppgPos = (ppgPos + 1) % PPG_W;
}

// ══════════════════════════════════════════════════════════════════════════
// TELA 4 — ANALYSIS
// ══════════════════════════════════════════════════════════════════════════

static void _desenharGauge(int cx, int cy, int r, int angFim, uint16_t cor) {
  for (int ang = 180; ang <= 360; ang += 3) {
    float rad = ang * 3.14159f / 180.0f;
    int x1 = cx + (int)((r - 14) * cosf(rad));
    int y1 = cy + (int)((r - 14) * sinf(rad));
    int x2 = cx + (int)(r * cosf(rad));
    int y2 = cy + (int)(r * sinf(rad));
    tft.drawLine(x1, y1, x2, y2, COR_CINZA_ESC);
  }
  for (int ang = 180; ang <= angFim; ang += 3) {
    float rad = ang * 3.14159f / 180.0f;
    int x1 = cx + (int)((r - 14) * cosf(rad));
    int y1 = cy + (int)((r - 14) * sinf(rad));
    int x2 = cx + (int)(r * cosf(rad));
    int y2 = cy + (int)(r * sinf(rad));
    tft.drawLine(x1, y1, x2, y2, cor);
  }
}

void desenharTelaAnalysis() {
  tft.fillScreen(COR_FUNDO);
  desenharCabecalho("Analysis", COR_CIANO);
  desenharIconeBateria(215, 6);

  tft.setTextColor(COR_BRANCO);
  tft.setTextSize(2);
  tft.setCursor(52, 40);
  tft.print("Estresse est.");

  const char* textoEstresse;
  uint16_t    corEstresse;
  int         angGauge;
  if (!dedoDetectado || hrvRMSSD == 0) { textoEstresse = "--";    corEstresse = COR_CINZA;    angGauge = 270; }
  else if (hrvRMSSD >= 50)             { textoEstresse = "Baixo"; corEstresse = COR_VERDE;    angGauge = 205; }
  else if (hrvRMSSD >= 20)             { textoEstresse = "Medio"; corEstresse = COR_AMARELO;  angGauge = 270; }
  else                                 { textoEstresse = "Alto";  corEstresse = COR_VERMELHO; angGauge = 335; }

  _desenharGauge(120, 175, 78, angGauge, corEstresse);

  tft.setTextColor(corEstresse);
  tft.setTextSize(3);
  int len = strlen(textoEstresse);
  tft.setCursor(120 - len * 9, 165);
  tft.print(textoEstresse);

  // Smile / emoji
  if (dedoDetectado && hrvRMSSD > 0) {
    int ex = 120, ey = 200;
    tft.drawCircle(ex, ey, 14, corEstresse);
    tft.fillCircle(ex - 5, ey - 4, 2, corEstresse);
    tft.fillCircle(ex + 5, ey - 4, 2, corEstresse);
    if (corEstresse == COR_VERDE) {
      tft.drawLine(ex - 6, ey + 4, ex,     ey + 8, corEstresse);
      tft.drawLine(ex,     ey + 8, ex + 6, ey + 4, corEstresse);
    } else if (corEstresse == COR_VERMELHO) {
      tft.drawLine(ex - 6, ey + 8, ex,     ey + 4, corEstresse);
      tft.drawLine(ex,     ey + 4, ex + 6, ey + 8, corEstresse);
    } else {
      tft.drawLine(ex - 6, ey + 6, ex + 6, ey + 6, corEstresse);
    }
  }

  tft.setTextColor(COR_CIANO);
  tft.setTextSize(1);
  tft.setCursor(20, 228);
  tft.print("Baseado em HRV");
  const char* estadoTxt = !dedoDetectado ? "Sem leitura" : (hrvRMSSD >= 20 ? "Estado estavel" : "Atencao");
  tft.setTextColor(COR_CINZA);
  tft.setCursor(20, 244);
  tft.print(estadoTxt);
  tft.setCursor(6, 260);
  tft.print("* Estimativa. Nao e diagnostico.");

  desenharIndicadorPagina(4, 6);
}

void atualizarTelaAnalysis() {
  // Atualiza apenas quando a categoria de estresse muda (reduz pisca)
  int cat = 0;  // 0=sem leitura, 1=Baixo, 2=Medio, 3=Alto
  if (dedoDetectado && hrvRMSSD > 0) {
    if (hrvRMSSD >= 50)      cat = 1;
    else if (hrvRMSSD >= 20) cat = 2;
    else                     cat = 3;
  }
  static int prevCat = -1;
  if (cat == prevCat) return;
  prevCat    = cat;
  prevHRV_d  = hrvRMSSD;
  prevDedo_d = dedoDetectado;

  tft.fillRect(0, 36, 240, 238, COR_FUNDO);
  tft.setTextColor(COR_BRANCO); tft.setTextSize(2); tft.setCursor(52, 40); tft.print("Estresse est.");

  const char* textoEstresse; uint16_t corEstresse; int angGauge;
  if (cat == 0)      { textoEstresse = "--";    corEstresse = COR_CINZA;    angGauge = 270; }
  else if (cat == 1) { textoEstresse = "Baixo"; corEstresse = COR_VERDE;    angGauge = 205; }
  else if (cat == 2) { textoEstresse = "Medio"; corEstresse = COR_AMARELO;  angGauge = 270; }
  else               { textoEstresse = "Alto";  corEstresse = COR_VERMELHO; angGauge = 335; }

  _desenharGauge(120, 175, 78, angGauge, corEstresse);
  tft.setTextColor(corEstresse); tft.setTextSize(3);
  int len = strlen(textoEstresse);
  tft.setCursor(120 - len * 9, 165); tft.print(textoEstresse);

  if (cat > 0) {
    int ex = 120, ey = 200;
    tft.drawCircle(ex, ey, 14, corEstresse);
    tft.fillCircle(ex - 5, ey - 4, 2, corEstresse);
    tft.fillCircle(ex + 5, ey - 4, 2, corEstresse);
    if (cat == 1) { tft.drawLine(ex-6,ey+4,ex,ey+8,corEstresse); tft.drawLine(ex,ey+8,ex+6,ey+4,corEstresse); }
    else if (cat == 3) { tft.drawLine(ex-6,ey+8,ex,ey+4,corEstresse); tft.drawLine(ex,ey+4,ex+6,ey+8,corEstresse); }
    else tft.drawLine(ex-6,ey+6,ex+6,ey+6,corEstresse);
  }
  tft.setTextColor(COR_CIANO); tft.setTextSize(1); tft.setCursor(20, 228); tft.print("Baseado em HRV");
  const char* estadoTxt = (cat == 0) ? "Sem leitura" : (cat <= 2 ? "Estado estavel" : "Atencao");
  tft.setTextColor(COR_CINZA); tft.setCursor(20, 244); tft.print(estadoTxt);
  tft.setCursor(6, 260); tft.print("* Estimativa. Nao e diagnostico.");
}

// ══════════════════════════════════════════════════════════════════════════
// TELA 5 — SUMMARY
// ══════════════════════════════════════════════════════════════════════════

#define SUM_W   108
#define SUM_H    58
#define SUM_X1     6
#define SUM_X2   126
#define SUM_Y1    34
#define SUM_Y2    98
#define SUM_Y3   162
#define SUM_Y4   226

static void _cardSum(int x, int y, const char* label, const char* valor,
                     uint16_t corLabel, uint16_t corValor, uint16_t corBorda) {
  desenharCard(x, y, SUM_W, SUM_H, corBorda);
  tft.setTextColor(corLabel); tft.setTextSize(1); tft.setCursor(x + 8, y + 8); tft.print(label);
  tft.setTextColor(corValor); tft.setTextSize(2); tft.setCursor(x + 8, y + 28); tft.print(valor);
}

void desenharTelaSummary() {
  tft.fillScreen(COR_FUNDO);
  tft.fillRect(0, 0, 240, 28, COR_CARD);
  tft.setTextColor(COR_CIANO); tft.setTextSize(1); tft.setCursor(6, 6); tft.print("SYNAPSEA");
  tft.setTextColor(COR_BRANCO); tft.setTextSize(2); tft.setCursor(70, 4); tft.print("Resumo Geral");
  desenharIconeBateria(215, 8);
  tft.fillRect(0, 28, 240, 2, COR_CIANO);
  atualizarTelaSummary();
  desenharIndicadorPagina(5, 6);
}

void atualizarTelaSummary() {
  char buf[16];

  dtostrf(pix_max, 4, 1, buf); strcat(buf, "C");
  _cardSum(SUM_X1, SUM_Y1, "Temp",   buf,     COR_VERMELHO, COR_BRANCO, COR_VERMELHO);

  if (dedoDetectado && bpmReal > 0) snprintf(buf, sizeof(buf), "%d bpm", bpmReal); else strcpy(buf, "--");
  _cardSum(SUM_X2, SUM_Y1, "BPM",    buf,     COR_VERMELHO, COR_BRANCO, COR_VERMELHO);

  if (dedoDetectado && spo2Real > 0) snprintf(buf, sizeof(buf), "%d%%",  spo2Real); else strcpy(buf, "--");
  _cardSum(SUM_X1, SUM_Y2, "SpO2",   buf,     COR_CIANO,    COR_BRANCO, COR_CIANO);

  if (dedoDetectado && hrvRMSSD > 0) snprintf(buf, sizeof(buf), "%d ms", hrvRMSSD); else strcpy(buf, "--");
  _cardSum(SUM_X2, SUM_Y2, "HRV",    buf,     COR_VERDE,    COR_BRANCO, COR_VERDE);

  if (dedoDetectado) { dtostrf(piVal, 3, 1, buf); strcat(buf, " %"); } else strcpy(buf, "--");
  _cardSum(SUM_X1, SUM_Y3, "PI",     buf,     COR_LARANJA,  COR_BRANCO, COR_LARANJA);

  _cardSum(SUM_X2, SUM_Y3, "Resp",   "-- rpm", COR_ROXO,   COR_BRANCO, COR_ROXO);

  const char* stressTxt; uint16_t corStr;
  if (!dedoDetectado || hrvRMSSD == 0) { stressTxt = "--";    corStr = COR_CINZA; }
  else if (hrvRMSSD >= 50)             { stressTxt = "Baixo"; corStr = COR_VERDE; }
  else if (hrvRMSSD >= 20)             { stressTxt = "Medio"; corStr = COR_AMARELO; }
  else                                 { stressTxt = "Alto";  corStr = COR_VERMELHO; }
  _cardSum(SUM_X1, SUM_Y4, "Stress", stressTxt, 0x9FFF, corStr, 0x9FFF);

  const char* statTxt; uint16_t corStat;
  if (!maxOK)             { statTxt = "S/Sensor"; corStat = COR_VERMELHO; }
  else if (!dedoDetectado){ statTxt = "S/ Dedo";  corStat = COR_CINZA; }
  else if (qualSinal >= 2){ statTxt = "Normal";   corStat = COR_VERDE; }
  else                    { statTxt = "Fraco";    corStat = COR_AMARELO; }
  _cardSum(SUM_X2, SUM_Y4, "Status", statTxt, COR_VERDE, corStat, COR_VERDE);
}

// ══════════════════════════════════════════════════════════════════════════
// ECG (função mantida por compatibilidade)
// ══════════════════════════════════════════════════════════════════════════

void atualizarECG() {
  long delta  = irValue - prevIR2;
  prevIR2     = irValue;
  int amostra = constrain((int)(delta / 500), -18, 18);
  ecgBuf2[ecgPos2] = amostra;

  int x    = ecgPos2 + 10;
  int midY = ECG_Y + ECG_H / 2;
  tft.fillRect(x, ECG_Y, 2, ECG_H, COR_FUNDO);
  tft.fillRect((x + 4) % ECG_W + 10, ECG_Y, 3, ECG_H, COR_FUNDO);

  int y1 = midY - ecgBuf2[(ecgPos2 + ECG_W - 1) % ECG_W];
  int y2 = midY - amostra;
  y1 = constrain(y1, ECG_Y + 2, ECG_Y + ECG_H - 3);
  y2 = constrain(y2, ECG_Y + 2, ECG_Y + ECG_H - 3);
  tft.drawLine(x - 1, y1, x, y2, dedoDetectado ? COR_VERDE : COR_CINZA_ESC);
  ecgPos2 = (ecgPos2 + 1) % ECG_W;
}

// ══════════════════════════════════════════════════════════════════════════
// NAVEGAÇÃO — botão físico GPIO27
// ══════════════════════════════════════════════════════════════════════════

void verificarBotaoTrocaTela() {
  bool btnAtual = digitalRead(BTN_PIN);
  if (btnAtual == LOW && btnAnterior == HIGH &&
      (millis() - ultimoDebounce > debounceDelay)) {
    ultimoDebounce       = millis();
    telaAtual            = (telaAtual + 1) % 6;
    telaPrecisaRedesenhar = true;
  }
  btnAnterior = btnAtual;
}

