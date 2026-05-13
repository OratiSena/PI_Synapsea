#include "display.h"
#include "sensorAMG.h"

void colorbar() {
  tft.fillRect(0, 280, 240, 24, 0x0000);

  for (int i = 0; i < 240; i++) {
    int colorIndex = constrain((int)(i * 1.05), 0, maxIndex);
    tft.fillRect(i, 280, 1, 24, camColors[colorIndex]);
  }

  tft.setTextColor(0xFFFF);
  tft.setTextSize(2);
  tft.setCursor(0, 285);
  tft.print(MINTEMP);
  tft.print("C");

  tft.setCursor(202, 285);
  tft.print(MAXTEMP);
  tft.print("C");
}

void drawpixels(float *p, uint8_t rows, uint8_t cols, uint8_t boxW, uint8_t boxH, boolean showVal) {
  for (int y = 0; y < rows; y++) {
    for (int x = 0; x < cols; x++) {
      float val = get_point(p, rows, cols, x, y);

      int idx = map(val, MINTEMP, MAXTEMP, 0, 239);
      idx = constrain(idx, 0, 239);

      tft.fillRect(x * boxW, y * boxH, boxW, boxH, camColors[idx]);

      if (val > pix_max) {
        pix_max = val;
        pos_x = x * boxW;
        pos_y = y * boxH;
      }

      if (showVal) {
        tft.setCursor(x * boxW + boxW / 2 - 12, y * boxH + boxH / 2 + 4);
        tft.setTextColor(COLOR_RGB565_WHITE);
        tft.setTextSize(1);
        tft.print(val, 1);
      }
    }
  }
}

void verificarBotaoTrocaTela() {
  bool btnAtual = digitalRead(BTN_PIN);
  if (btnAtual == LOW && btnAnterior == HIGH && (millis() - ultimoDebounce > debounceDelay)) {
    ultimoDebounce = millis();
    telaAtual = (telaAtual == 0) ? 1 : 0;
    tft.fillScreen(0x0000);
    if (telaAtual == 0) {
      colorbar();
    } else {
      prevBPM_d  = -999;
      prevSpo2_d = -999;
      prevPI_d   = -1.0f;
      prevQual_d = -1;
      prevHRV_d  = -999;
      prevDedo_d  = false;
      prevMaxOK_d = false;
      desenharCabecalhoMAX();
    }
  }
  btnAnterior = btnAtual;
}

void desenharCabecalhoMAX() {
  tft.fillScreen(0x0000);

  tft.fillRect(0, 0, 240, 38, 0x0841);

  tft.setTextColor(0x07FF);
  tft.setTextSize(2);
  tft.setCursor(52, 4);
  tft.print("SYNAPSEA");

  tft.setTextColor(0x8410);
  tft.setTextSize(1);
  tft.setCursor(72, 24);
  tft.print("Sinais Vitais");

  tft.fillRect(0, 38, 240, 2, 0x07FF);

  tft.setTextColor(0xF800);
  tft.setTextSize(1);
  tft.setCursor(14, 50);
  tft.print("BPM");

  tft.setTextColor(0x07FF);
  tft.setCursor(134, 50);
  tft.print("SpO2");

  tft.fillRect(119, 42, 2, 70, 0x2104);
  tft.fillRect(0, 112, 240, 2, 0x2104);

  tft.setTextColor(0xFD20);
  tft.setTextSize(1);
  tft.setCursor(10, 116);
  tft.print("PI");

  tft.setTextColor(0x9FF3);
  tft.setCursor(143, 116);
  tft.print("HRV");

  tft.fillRect(0, 143, 240, 2, 0x2104);

  tft.setTextColor(0x8410);
  tft.setCursor(8, 147);
  tft.print("STATUS");

  tft.fillRect(0, 194, 240, 2, 0x2104);

  tft.setTextColor(0x8410);
  tft.setCursor(8, 198);
  tft.print("PPG");

  tft.drawRect(0, 208, 240, 110, 0x2104);
}

void desenharValoresMAX(bool forcar) {
  bool mudouEstado = (qualSinal  != prevQual_d)  || (maxOK != prevMaxOK_d) || forcar;
  bool mudouBPM    = (bpmReal    != prevBPM_d)   || forcar;
  bool mudouSpo2   = (spo2Real   != prevSpo2_d)  || forcar;
  bool mudouHRV    = (hrvRMSSD   != prevHRV_d)   || forcar;
  bool mudouPI     = ((piVal - prevPI_d) > 0.09f || (prevPI_d - piVal) > 0.09f)
                     || mudouHRV || mudouEstado || forcar;

  // ── BPM ────────────────────────────────────────────────────────────────
  if (mudouBPM) {
    tft.fillRect(6, 58, 108, 56, 0x0000);
    if (dedoDetectado && bpmReal > 0) {
      tft.setTextColor(0xF800);
      tft.setTextSize(5);
      int bx = (bpmReal < 100) ? 22 : 8;
      tft.setCursor(bx, 62);
      tft.print(bpmReal);
    } else {
      tft.setTextColor(0x4208);
      tft.setTextSize(4);
      tft.setCursor(22, 68);
      tft.print("--");
    }
    prevBPM_d = bpmReal;
  }

  // ── SpO2 ───────────────────────────────────────────────────────────────
  if (mudouSpo2) {
    tft.fillRect(122, 58, 118, 56, 0x0000);
    if (dedoDetectado && spo2Real > 0) {
      tft.setTextColor(0x07FF);
      tft.setTextSize(4);
      tft.setCursor(128, 62);
      tft.print(spo2Real);
      tft.setTextSize(2);
      tft.setCursor(196, 72);
      tft.print("%");
    } else {
      tft.setTextColor(0x4208);
      tft.setTextSize(4);
      tft.setCursor(140, 68);
      tft.print("--");
    }
    prevSpo2_d = spo2Real;
  }

  // ── PI e HRV ──────────────────────────────────────────────────────────
  if (mudouPI) {
    tft.fillRect(6, 124, 108, 20, 0x0000);
    tft.fillRect(122, 124, 118, 20, 0x0000);
    if (dedoDetectado) {
      tft.setTextColor(0xFD20);
      tft.setTextSize(2);
      char piBuf[8];
      dtostrf(piVal, 4, 1, piBuf);
      tft.setCursor(10, 126);
      tft.print(piBuf);
      tft.setTextSize(1);
      tft.setCursor(66, 132);
      tft.print("%");

      tft.setTextSize(2);
      if (hrvRMSSD >= 50) {
        tft.setTextColor(0x07E0);
      } else if (hrvRMSSD >= 20) {
        tft.setTextColor(0xFFE0);
      } else {
        tft.setTextColor(0xF800);
      }
      char hrvBuf[10];
      snprintf(hrvBuf, sizeof(hrvBuf), "%d ms", hrvRMSSD);
      tft.setCursor(124, 126);
      tft.print(hrvBuf);
    } else {
      tft.setTextColor(0x4208);
      tft.setTextSize(2);
      tft.setCursor(22, 126);
      tft.print("--");
      tft.setCursor(140, 126);
      tft.print("--");
    }
    prevPI_d  = piVal;
    prevHRV_d = hrvRMSSD;
  }

  // ── Barra de Status ────────────────────────────────────────────────────
  if (mudouEstado) {
    tft.fillRect(0, 155, 240, 40, 0x0000);
    if (!maxOK) {
      tft.fillRoundRect(6, 156, 228, 35, 5, 0x2000);
      tft.setTextColor(0xF800);
      tft.setTextSize(2);
      tft.setCursor(52, 165);
      tft.print("SEM SENSOR");
    } else if (qualSinal == 0) {
      tft.fillRoundRect(6, 156, 228, 35, 5, 0x2200);
      tft.setTextColor(0xFFE0);
      tft.setTextSize(2);
      tft.setCursor(42, 165);
      tft.print("SEM LEITURA");
    } else if (qualSinal == 1) {
      tft.fillRoundRect(6, 156, 228, 35, 5, 0x4200);
      tft.setTextColor(0xFD20);
      tft.setTextSize(2);
      tft.setCursor(46, 165);
      tft.print("SINAL FRACO");
    } else {
      tft.fillRoundRect(6, 156, 228, 35, 5, 0x0240);
      tft.setTextColor(0x07E0);
      tft.setTextSize(2);
      tft.setCursor(76, 165);
      tft.print("NORMAL");
    }
    prevQual_d  = qualSinal;
    prevMaxOK_d = maxOK;
    prevDedo_d  = dedoDetectado;
  }
}

void atualizarECG() {
  long delta = irValue - prevIR;
  prevIR = irValue;

  int amostra = constrain((int)(delta / 500), -18, 18);
  ecgBuf[ecgPos] = amostra;

  int x = ecgPos + 10;
  int midY = ECG_Y + ECG_H / 2;

  tft.fillRect(x, ECG_Y, 2, ECG_H, 0x0000);
  tft.fillRect((x + 4) % ECG_W + 10, ECG_Y, 3, ECG_H, 0x0000);

  int y1 = midY - ecgBuf[(ecgPos + ECG_W - 1) % ECG_W];
  int y2 = midY - amostra;
  y1 = constrain(y1, ECG_Y + 2, ECG_Y + ECG_H - 3);
  y2 = constrain(y2, ECG_Y + 2, ECG_Y + ECG_H - 3);

  uint16_t cor = dedoDetectado ? 0x07E0 : 0x2104;
  tft.drawLine(x - 1, y1, x, y2, cor);

  ecgPos = (ecgPos + 1) % ECG_W;
}

void desenharTelaMAX() {
  static bool primeiraVez = true;
  if (primeiraVez) {
    desenharCabecalhoMAX();
    primeiraVez = false;
    prevBPM_d  = -999;
    prevSpo2_d = -999;
    prevPI_d   = -1.0f;
    prevQual_d = -1;
    prevHRV_d  = -999;
  }

  desenharValoresMAX(false);
  atualizarECG();
}
