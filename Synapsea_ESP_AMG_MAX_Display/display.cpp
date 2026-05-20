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

static int   ppgBuf[PPG_W]  = {0};
static int   ppgPos         = 0;
static long  ppgPrevIR      = 0;
static float ppgPeakAbs     = 500.0f;  // pico adaptativo do sinal PPG
static int  ecgBuf2[ECG_W] = {0};
static int  ecgPos2         = 0;
static long prevIR2         = 0;
static bool histPopupVisible = false;  // popup de histórico BPM/SpO2 ativo

// ─── Relógio simulado ─────────────────────────────────────────────────────
static int horaSimulada    = 14;
static int minutoSimulado  = 32;

// ─── Variáveis externas do .ino ───────────────────────────────────────────
extern int   telaAtual;
extern bool  btnAnterior;
extern unsigned long ultimoDebounce;
extern const unsigned long debounceDelay;
extern bool  telaPrecisaRedesenhar;

// ─── Forward declarations (funções auxiliares de touch) ───────────────────
static void _redesenharZoom();
static void _tratarTap(int x, int y);
extern int   respRPM;

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
      float tDisp = tempUnitCelsius ? pix_max : (pix_max * 9.0f / 5.0f + 32.0f);
      char tBuf[7]; dtostrf(tDisp, 4, 1, tBuf); tft.print(tBuf);
      tft.setTextSize(1); tft.setCursor(82, 201); tft.print(tempUnitCelsius ? "C" : "F");
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
      float tDisp = tempUnitCelsius ? pix_max : (pix_max * 9.0f / 5.0f + 32.0f);
      char tBuf[7]; dtostrf(tDisp, 4, 1, tBuf); tft.print(tBuf);
      tft.setTextSize(1); tft.setCursor(82, 201); tft.print(tempUnitCelsius ? "C" : "F");
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
  if (histPopupVisible) return;  // não sobrescreve popup de histórico
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

  // Botões de zoom (–  valor  +) na faixa y=34..53, à esquerda do BPM
  _redesenharZoom();

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

  // Card Resp
  desenharCard(84, 164, 72, 64, COR_ROXO);
  tft.setTextColor(COR_ROXO);
  tft.setTextSize(1);
  tft.setCursor(92, 172);
  tft.print("Resp.");
  tft.setTextColor(COR_BRANCO);
  tft.setTextSize(2);
  tft.setCursor(90, 190);
  if (respRPM > 0 && dedoDetectado) tft.print(respRPM); else tft.print("--");
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
  static int prevResp_d = -1;
  if (respRPM != prevResp_d) {
    tft.fillRect(90, 187, 62, 28, COR_CARD);
    tft.setTextColor(COR_BRANCO); tft.setTextSize(2); tft.setCursor(90, 190);
    if (respRPM > 0 && dedoDetectado) tft.print(respRPM); else tft.print("--");
    prevResp_d = respRPM;
  }
  // Card Sinal (atualiza quando qualidade ou estado do dedo muda)
  static int prevQualPulse_d = -1;
  if (qualSinal != prevQualPulse_d) {
    uint16_t corSin = qualSinal >= 2 ? COR_VERDE : (qualSinal == 1 ? COR_LARANJA : COR_CINZA);
    desenharCard(162, 164, 72, 64, corSin);
    tft.setTextColor(COR_CINZA); tft.setTextSize(1); tft.setCursor(170, 172); tft.print("Sinal");
    tft.fillRect(166, 182, 62, 40, COR_CARD);  // limpa texto anterior
    if (!maxOK || !dedoDetectado || qualSinal == 0) {
      tft.setTextColor(COR_CINZA); tft.setTextSize(2); tft.setCursor(174, 190); tft.print("--");
    } else if (qualSinal == 1) {
      tft.setTextColor(COR_LARANJA); tft.setTextSize(1); tft.setCursor(168, 188); tft.print("Fraco");
    } else {
      tft.setTextColor(COR_VERDE); tft.setTextSize(2); tft.setCursor(170, 190); tft.print("Bom");
    }
    prevQualPulse_d = qualSinal;
  }
  atualizarPPGPulse();
}

void atualizarPPGPulse() {
  long delta  = irValue - ppgPrevIR;
  ppgPrevIR   = irValue;

  // Peak-hold com decaimento lento: ataque instantâneo, decai ~0,05% por amostra.
  float af = fabsf((float)delta);
  if (af > ppgPeakAbs) ppgPeakAbs = af;                            // ataque instantâneo
  else                 ppgPeakAbs = ppgPeakAbs * 0.9995f + af * 0.0005f;  // decaimento lento
  ppgPeakAbs = max(ppgPeakAbs, 10.0f);

  // Zoom: escala geométrica — cada nível é ~2× maior que o anterior.
  // zoom=1 → pico ~15% da altura  (sinal muito comprimido)
  // zoom=2 → pico ~30%            (default "pequeno")
  // zoom=3 → pico ~60%            (confortável — recomendado)
  // zoom=4 → pico ~100%           (cheio, começa a clipar nos picos)
  // zoom=5 → pico ~200%           (amplificado, clipado — para sinais fracos)
  static const float zoomTable[5] = {0.15f, 0.30f, 0.60f, 1.00f, 2.00f};
  float halfH = (float)(PPG_AREA_H / 2 - 4);
  float scale = (halfH * zoomTable[ppgZoom - 1]) / ppgPeakAbs;
  int amostra = constrain((int)((float)delta * scale), -(int)halfH, (int)halfH);
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

// Score composto de estresse 0-100 (maior = mais estresse)
// Pesos: HRV 40pts, BPM 25pts, PI 20pts, Resp 15pts
// Retorna -1 se sem leitura válida
static int calcStressScore() {
  if (!dedoDetectado || !maxOK) return -1;
  // Aguarda leituras estabilizarem (igual ao SpO2/HRV que levam alguns segundos)
  if (!emaBPMReady || !emaSpo2Ready || rrBufCount < 3) return -1;
  int s = 0;
  // HRV (40 pts)
  if      (hrvRMSSD == 0)  s += 25;
  else if (hrvRMSSD > 60)  s += 0;
  else if (hrvRMSSD > 40)  s += 10;
  else if (hrvRMSSD > 25)  s += 20;
  else if (hrvRMSSD > 10)  s += 30;
  else                     s += 40;
  // BPM (25 pts)
  if      (bpmReal == 0)   s += 12;
  else if (bpmReal < 65)   s += 0;
  else if (bpmReal < 80)   s += 8;
  else if (bpmReal < 95)   s += 16;
  else                     s += 25;
  // PI (20 pts)
  if      (piVal <= 0.0f)  s += 10;
  else if (piVal > 5.0f)   s += 0;
  else if (piVal > 2.0f)   s += 7;
  else if (piVal > 0.5f)   s += 14;
  else                     s += 20;
  // Resp (15 pts)
  if      (respRPM == 0)                        s += 7;
  else if (respRPM >= 12 && respRPM <= 20)      s += 0;
  else if (respRPM <= 26)                       s += 8;
  else                                          s += 15;
  return s;  // 0-20=Baixo, 21-50=Medio, 51+=Alto
}

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

  int score = calcStressScore();
  const char* textoEstresse;
  uint16_t    corEstresse;
  int         angGauge;
  if (score < 0)       { textoEstresse = "--";    corEstresse = COR_CINZA;    angGauge = 270; }
  else if (score <= 20){ textoEstresse = "Baixo"; corEstresse = COR_VERDE;    angGauge = constrain(185 + (score * 170) / 100, 185, 355); }
  else if (score <= 50){ textoEstresse = "Medio"; corEstresse = COR_AMARELO;  angGauge = constrain(185 + (score * 170) / 100, 185, 355); }
  else                 { textoEstresse = "Alto";  corEstresse = COR_VERMELHO; angGauge = constrain(185 + (score * 170) / 100, 185, 355); }

  _desenharGauge(120, 175, 78, angGauge, corEstresse);

  tft.setTextColor(corEstresse);
  tft.setTextSize(3);
  int len = strlen(textoEstresse);
  tft.setCursor(120 - len * 9, 165);
  tft.print(textoEstresse);

  // Emoji
  if (score >= 0) {
    int ex = 120, ey = 200;
    tft.drawCircle(ex, ey, 14, corEstresse);
    tft.fillCircle(ex - 5, ey - 4, 2, corEstresse);
    tft.fillCircle(ex + 5, ey - 4, 2, corEstresse);
    if (corEstresse == COR_VERDE) {
      tft.drawLine(ex - 6, ey + 4, ex, ey + 8, corEstresse);
      tft.drawLine(ex, ey + 8, ex + 6, ey + 4, corEstresse);
    } else if (corEstresse == COR_VERMELHO) {
      tft.drawLine(ex - 6, ey + 8, ex, ey + 4, corEstresse);
      tft.drawLine(ex, ey + 4, ex + 6, ey + 8, corEstresse);
    } else {
      tft.drawLine(ex - 6, ey + 6, ex + 6, ey + 6, corEstresse);
    }
  }

  // Divisor + card de respiração
  tft.fillRect(0, 218, 240, 2, COR_CINZA_ESC);
  desenharCard(6, 222, 228, 74, COR_ROXO);
  tft.setTextColor(COR_ROXO); tft.setTextSize(1); tft.setCursor(14, 230); tft.print("RESPIRACAO");
  if (respRPM > 0 && dedoDetectado) {
    uint16_t corR = (respRPM >= 12 && respRPM <= 20) ? COR_VERDE : (respRPM <= 26 ? COR_AMARELO : COR_VERMELHO);
    tft.setTextColor(corR); tft.setTextSize(3); tft.setCursor(14, 244); tft.print(respRPM);
    tft.setTextColor(COR_CINZA); tft.setTextSize(1); tft.setCursor(56, 254); tft.print("rpm");
    const char* rStatus = (respRPM >= 12 && respRPM <= 20) ? "Normal" : (respRPM <= 26 ? "Elevada" : "Alta");
    tft.setTextColor(corR); tft.setTextSize(2); tft.setCursor(110, 244); tft.print(rStatus);
    tft.setTextColor(COR_CINZA); tft.setTextSize(1); tft.setCursor(110, 268); tft.print("12-20 rpm");
  } else {
    tft.setTextColor(COR_CINZA); tft.setTextSize(2); tft.setCursor(14, 250); tft.print("--");
    tft.setTextColor(COR_CINZA); tft.setTextSize(1); tft.setCursor(14, 274); tft.print("Aguardando leitura...");
  }

  desenharIndicadorPagina(4, 6);
}

void atualizarTelaAnalysis() {
  int score = calcStressScore();
  // cat: 0=sem leitura, 1=Baixo, 2=Medio, 3=Alto
  int cat = 0;
  if (score >= 0) {
    if      (score <= 20) cat = 1;
    else if (score <= 50) cat = 2;
    else                  cat = 3;
  }
  static int prevCat   = -1;
  static int prevResp  = -1;
  bool respChanged = (respRPM != prevResp);
  if (cat == prevCat && !respChanged) return;
  prevCat   = cat;
  prevResp  = respRPM;

  tft.fillRect(0, 36, 240, 182, COR_FUNDO);
  tft.setTextColor(COR_BRANCO); tft.setTextSize(2); tft.setCursor(52, 40); tft.print("Estresse est.");

  const char* textoEstresse; uint16_t corEstresse; int angGauge;
  if (cat == 0)      { textoEstresse = "--";    corEstresse = COR_CINZA;    angGauge = 270; }
  else if (cat == 1) { textoEstresse = "Baixo"; corEstresse = COR_VERDE;    angGauge = constrain(185 + (score * 170) / 100, 185, 355); }
  else if (cat == 2) { textoEstresse = "Medio"; corEstresse = COR_AMARELO;  angGauge = constrain(185 + (score * 170) / 100, 185, 355); }
  else               { textoEstresse = "Alto";  corEstresse = COR_VERMELHO; angGauge = constrain(185 + (score * 170) / 100, 185, 355); }

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

  // Atualizar card de respiração
  tft.fillRect(6, 222, 228, 74, COR_FUNDO);
  desenharCard(6, 222, 228, 74, COR_ROXO);
  tft.setTextColor(COR_ROXO); tft.setTextSize(1); tft.setCursor(14, 230); tft.print("RESPIRACAO");
  if (respRPM > 0 && dedoDetectado) {
    uint16_t corR = (respRPM >= 12 && respRPM <= 20) ? COR_VERDE : (respRPM <= 26 ? COR_AMARELO : COR_VERMELHO);
    tft.setTextColor(corR); tft.setTextSize(3); tft.setCursor(14, 244); tft.print(respRPM);
    tft.setTextColor(COR_CINZA); tft.setTextSize(1); tft.setCursor(56, 254); tft.print("rpm");
    const char* rStatus = (respRPM >= 12 && respRPM <= 20) ? "Normal" : (respRPM <= 26 ? "Elevada" : "Alta");
    tft.setTextColor(corR); tft.setTextSize(2); tft.setCursor(110, 244); tft.print(rStatus);
    tft.setTextColor(COR_CINZA); tft.setTextSize(1); tft.setCursor(110, 268); tft.print("12-20 rpm");
  } else {
    tft.setTextColor(COR_CINZA); tft.setTextSize(2); tft.setCursor(14, 250); tft.print("--");
    tft.setTextColor(COR_CINZA); tft.setTextSize(1); tft.setCursor(14, 274); tft.print("Aguardando leitura...");
  }
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

  float tDisp = tempUnitCelsius ? pix_max : (pix_max * 9.0f / 5.0f + 32.0f);
  dtostrf(tDisp, 4, 1, buf); strcat(buf, tempUnitCelsius ? "C" : "F");
  _cardSum(SUM_X1, SUM_Y1, "Temp",   buf,     COR_VERMELHO, COR_BRANCO, COR_VERMELHO);

  if (dedoDetectado && bpmReal > 0) snprintf(buf, sizeof(buf), "%d bpm", bpmReal); else strcpy(buf, "--");
  _cardSum(SUM_X2, SUM_Y1, "BPM",    buf,     COR_VERMELHO, COR_BRANCO, COR_VERMELHO);

  if (dedoDetectado && spo2Real > 0) snprintf(buf, sizeof(buf), "%d%%",  spo2Real); else strcpy(buf, "--");
  _cardSum(SUM_X1, SUM_Y2, "SpO2",   buf,     COR_CIANO,    COR_BRANCO, COR_CIANO);

  if (dedoDetectado && hrvRMSSD > 0) snprintf(buf, sizeof(buf), "%d ms", hrvRMSSD); else strcpy(buf, "--");
  _cardSum(SUM_X2, SUM_Y2, "HRV",    buf,     COR_VERDE,    COR_BRANCO, COR_VERDE);

  if (dedoDetectado) { dtostrf(piVal, 3, 1, buf); strcat(buf, " %"); } else strcpy(buf, "--");
  _cardSum(SUM_X1, SUM_Y3, "PI",     buf,     COR_LARANJA,  COR_BRANCO, COR_LARANJA);

  if (respRPM > 0 && dedoDetectado) snprintf(buf, sizeof(buf), "%d rpm", respRPM); else strcpy(buf, "--");
  _cardSum(SUM_X2, SUM_Y3, "Resp",   buf,     COR_ROXO,     COR_BRANCO, COR_ROXO);

  const char* stressTxt; uint16_t corStr;
  int scoreSum = calcStressScore();
  if (scoreSum < 0)        { stressTxt = "--";    corStr = COR_CINZA; }
  else if (scoreSum <= 20) { stressTxt = "Baixo"; corStr = COR_VERDE; }
  else if (scoreSum <= 50) { stressTxt = "Medio"; corStr = COR_AMARELO; }
  else                     { stressTxt = "Alto";  corStr = COR_VERMELHO; }
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
    mostrarPainelConfig   = false;
    telaPrecisaRedesenhar = true;
  }
  btnAnterior = btnAtual;
}

// ══════════════════════════════════════════════════════════════════════════
// TOUCH — painel de configurações e gestos de swipe
// ══════════════════════════════════════════════════════════════════════════

// ─── Externos necessários ────────────────────────────────────────────────
extern XPT2046_Touchscreen ts;
extern bool mostrarPainelConfig;
extern bool tempUnitCelsius;
extern int  ppgZoom;

// ─── Conversão de coordenada raw do XPT2046 para pixels de tela ──────────
// Display em rotation=2 (180°) → eixos invertidos.
// Ajuste TOUCH_X/Y_MIN/MAX em config.h se o toque estiver descalibrado.
static inline int _tX(int raw) {
  return constrain(map(raw, TOUCH_X_MAX, TOUCH_X_MIN, 0, 239), 0, 239);
}
static inline int _tY(int raw) {
  return constrain(map(raw, TOUCH_Y_MAX, TOUCH_Y_MIN, 0, 319), 0, 319);
}

// ─── Redesenha botões de zoom no topo da área de waveform (Pulse) ────────
static void _redesenharZoom() {
  // Área: x=58..122  y=34..55
  tft.fillRect(58, 34, 66, 20, COR_FUNDO);
  // Botão  –
  tft.drawRect(58, 34, 18, 20, COR_CIANO);
  tft.setTextColor(COR_CIANO); tft.setTextSize(2);
  tft.setCursor(62, 36); tft.print("-");
  // Valor do zoom (ex: "3x")
  tft.setTextColor(COR_BRANCO); tft.setTextSize(2);
  tft.setCursor(80, 36); tft.print(ppgZoom); tft.print("x");
  // Botão  +
  tft.drawRect(104, 34, 18, 20, COR_CIANO);
  tft.setTextColor(COR_CIANO); tft.setTextSize(2);
  tft.setCursor(107, 36); tft.print("+");
}

// ─── Painel de configurações (sobreposto, y=196..319) ────────────────────
void desenharPainelConfig() {
  const int pY = 196;
  tft.fillRect(0, pY, 240, 124, 0x1082);            // fundo escuro semi-opaco
  tft.drawRect(0, pY, 240, 124, COR_CIANO);
  tft.drawLine(0, pY + 2, 240, pY + 2, COR_CIANO);

  // Título
  tft.setTextColor(COR_CIANO); tft.setTextSize(1);
  tft.setCursor(6, pY + 7); tft.print("CONFIGURACOES");

  // Botão fechar (X) no canto superior direito
  tft.drawRect(202, pY + 2, 36, 26, COR_VERMELHO);
  tft.setTextColor(COR_VERMELHO); tft.setTextSize(2);
  tft.setCursor(212, pY + 6); tft.print("X");

  // Label unidade de temperatura
  tft.setTextColor(COR_BRANCO); tft.setTextSize(1);
  tft.setCursor(6, pY + 24); tft.print("Unidade de Temperatura:");

  // Botão °C
  uint16_t corC = tempUnitCelsius ? COR_CIANO : COR_CINZA_ESC;
  if (tempUnitCelsius) tft.fillRect(7, pY + 38, 52, 28, 0x0841);
  tft.drawRect(6, pY + 37, 54, 30, corC);
  tft.setTextColor(corC); tft.setTextSize(2);
  tft.setCursor(18, pY + 44); tft.print((char)0xF8); tft.print("C");

  // Botão °F
  uint16_t corF = !tempUnitCelsius ? COR_CIANO : COR_CINZA_ESC;
  if (!tempUnitCelsius) tft.fillRect(73, pY + 38, 52, 28, 0x0841);
  tft.drawRect(72, pY + 37, 54, 30, corF);
  tft.setTextColor(corF); tft.setTextSize(2);
  tft.setCursor(84, pY + 44); tft.print((char)0xF8); tft.print("F");

  // Dica de fechamento
  tft.setTextColor(COR_CINZA); tft.setTextSize(1);
  tft.setCursor(6, pY + 92);  tft.print("Deslize para baixo p/ fechar");
  tft.setCursor(6, pY + 105); tft.print("Toque X para fechar");

  // Redesenha bolinhas de página por cima do painel
  desenharIndicadorPagina(telaAtual, 6);
}

// ─── Popup: histórico de leituras BPM / SpO2 ─────────────────────────────
static void _mostrarHistorico() {
  tft.fillRoundRect(10, 55, 220, 218, 8, 0x0841);
  tft.drawRoundRect(10, 55, 220, 218, 8, COR_CIANO);

  // Título
  tft.setTextColor(COR_CIANO); tft.setTextSize(1);
  tft.setCursor(18, 63); tft.print("Historico de Leituras");
  tft.drawLine(10, 74, 230, 74, 0x2104);

  // ── BPM ──────────────────────────────────────────────────────────────
  tft.setTextColor(COR_VERMELHO); tft.setTextSize(1);
  tft.setCursor(18, 80); tft.print("BPM (ultimos 8):");
  int cnt = min(bpmBufCount, BPM_BUF_SIZE);
  int si  = (bpmBufIdx - cnt + BPM_BUF_SIZE) % BPM_BUF_SIZE;
  int cx = 18, cy = 93;
  for (int i = 0; i < cnt; i++) {
    int idx = (si + i) % BPM_BUF_SIZE;
    tft.setTextColor(COR_BRANCO); tft.setTextSize(2);
    tft.setCursor(cx, cy); tft.print((int)bpmBuffer[idx]);
    cx += 54;
    if (cx > 180) { cx = 18; cy += 22; }
  }
  if (cnt == 0) { tft.setTextColor(COR_CINZA); tft.setTextSize(1); tft.setCursor(18, 93); tft.print("Sem leituras ainda"); }

  cy = max(cy + 26, 140);
  tft.drawLine(10, cy, 230, cy, 0x2104);
  cy += 6;

  // ── SpO2 ─────────────────────────────────────────────────────────────
  tft.setTextColor(COR_CIANO); tft.setTextSize(1);
  tft.setCursor(18, cy); tft.print("SpO2 (ultimos 5):");
  cy += 13;
  int scnt = min(spo2BufCount, SPO2_BUF_SIZE);
  int ssi  = (spo2BufIdx - scnt + SPO2_BUF_SIZE) % SPO2_BUF_SIZE;
  cx = 18;
  for (int i = 0; i < scnt; i++) {
    int idx = (ssi + i) % SPO2_BUF_SIZE;
    tft.setTextColor(COR_BRANCO); tft.setTextSize(2);
    tft.setCursor(cx, cy); tft.print(spo2Buffer[idx]);
    tft.setTextColor(COR_CINZA); tft.setTextSize(1);
    tft.setCursor(cx + 28, cy + 10); tft.print("%");
    cx += 46;
  }
  if (scnt == 0) { tft.setTextColor(COR_CINZA); tft.setTextSize(1); tft.setCursor(18, cy); tft.print("Sem leituras ainda"); }

  tft.setTextColor(COR_CINZA); tft.setTextSize(1);
  tft.setCursor(28, 262); tft.print("Toque p/ fechar");
}

// ─── Tratamento de toque curto (tap) ─────────────────────────────────────
static void _tratarTap(int x, int y) {
  // ── Histórico visível: qualquer tap fecha ────────────────────────────
  if (histPopupVisible) {
    histPopupVisible      = false;
    telaPrecisaRedesenhar = true;
    return;
  }

  // ── Painel de configurações visível ──────────────────────────────────
  if (mostrarPainelConfig) {
    const int pY = 196;
    // Botão X (fechar): rect(202, pY+2, 36, 26)
    if (x >= 202 && x <= 238 && y >= pY + 2 && y <= pY + 28) {
      mostrarPainelConfig   = false;
      telaPrecisaRedesenhar = true;
      return;
    }
    // Botão °C: rect(6, pY+37, 54, 30)
    if (x >= 6 && x <= 60 && y >= pY + 37 && y <= pY + 67) {
      if (!tempUnitCelsius) {
        tempUnitCelsius = true;
        desenharPainelConfig();
        telaPrecisaRedesenhar = true;  // força redesenho p/ valor atualizado
      }
      return;
    }
    // Botão °F: rect(72, pY+37, 54, 30)
    if (x >= 72 && x <= 126 && y >= pY + 37 && y <= pY + 67) {
      if (tempUnitCelsius) {
        tempUnitCelsius = false;
        desenharPainelConfig();
        telaPrecisaRedesenhar = true;
      }
      return;
    }
    return;  // tap fora das opções: ignora
  }

  // ── Tela Temperatura (1): tap = congelar/descongelar imagem ──────────
  if (telaAtual == 1) {
    amgFrozen = !amgFrozen;
    if (!amgFrozen) {
      // Ao descongelar, força redesenho completo para limpar o badge PAUSED
      telaPrecisaRedesenhar = true;
    }
    return;
  }

  // ── Tela Heart (2): tap no BPM, SpO2 ou HRV = mostrar histórico ──────
  if (telaAtual == 2) {
    bool tapBPM = (x >= 120 && x <= 235 && y >= 75  && y <= 165);
    bool tapCard= (x >= 6   && x <= 235 && y >= 165 && y <= 230);
    if (tapBPM || tapCard) {
      histPopupVisible = true;
      _mostrarHistorico();
      return;
    }
  }

  // ── Tela Pulse (3): botões de zoom ───────────────────────────────────
  if (telaAtual == 3) {
    // Botão  –  (58..76, 34..54)
    if (x >= 56 && x <= 78 && y >= 32 && y <= 56) {
      ppgZoom = constrain(ppgZoom - 1, 1, 5);
      memset(ppgBuf, 0, sizeof(ppgBuf));  // buffer limpo → efeito imediato
      ppgPeakAbs = 500.0f;               // reset do peak para re-adaptar
      ppgPos = 0;
      _redesenharZoom();
      return;
    }
    // Botão  +  (104..122, 34..54)
    if (x >= 102 && x <= 124 && y >= 32 && y <= 56) {
      ppgZoom = constrain(ppgZoom + 1, 1, 5);
      memset(ppgBuf, 0, sizeof(ppgBuf));  // buffer limpo → efeito imediato
      ppgPeakAbs = 500.0f;               // reset do peak para re-adaptar
      ppgPos = 0;
      _redesenharZoom();
      return;
    }
  }
}

// ─── Função principal de detecção de gestos ──────────────────────────────
void verificarTouch() {
  static bool          wasPressed    = false;
  static int           startX        = 0, startY = 0;
  static int           lastX         = 0, lastY  = 0;
  static unsigned long pressedAt     = 0;

  bool touched = ts.touched();

  if (touched) {
    TS_Point p = ts.getPoint();
    int x = _tX(p.x);
    int y = _tY(p.y);

    if (!wasPressed) {
      startX    = x;
      startY    = y;
      pressedAt = millis();
      wasPressed = true;
    }
    lastX = x;
    lastY = y;

  } else if (wasPressed) {
    // Dedo levantou — analisar gesto
    int dx = lastX - startX;
    int dy = lastY - startY;
    unsigned long dt = millis() - pressedAt;
    wasPressed = false;

    if (dt > 800) return;  // toque muito longo: ignora

    bool hSwipe = (abs(dx) >= TOUCH_SWIPE_MIN) && (abs(dx) > abs(dy));
    bool vSwipe = (abs(dy) >= TOUCH_SWIPE_MIN) && (abs(dy) > abs(dx));
    bool isTap  = (abs(dx) < 15) && (abs(dy) < 15) && (dt < 400);

    if (hSwipe) {
      // ── Swipe horizontal: navegar telas ────────────────────────────
      mostrarPainelConfig = false;
      if (dx < 0) {
        // Desliza para a ESQUERDA → próxima tela
        telaAtual = (telaAtual + 1) % 6;
      } else {
        // Desliza para a DIREITA → tela anterior
        telaAtual = (telaAtual - 1 + 6) % 6;
      }
      telaPrecisaRedesenhar = true;

    } else if (vSwipe) {
      if (dy < 0) {
        // ── Swipe para CIMA → abrir painel de configurações ─────────
        if (!mostrarPainelConfig) {
          mostrarPainelConfig = true;
          desenharPainelConfig();
        }
      } else {
        // ── Swipe para BAIXO → fechar painel de configurações ───────
        if (mostrarPainelConfig) {
          mostrarPainelConfig   = false;
          telaPrecisaRedesenhar = true;
        }
      }

    } else if (isTap) {
      _tratarTap(lastX, lastY);
    }
  }
}

