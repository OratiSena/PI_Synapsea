#include "sensorMAX.h"

void scannerI2CMAX() {
  Serial.println("Escaneando I2C_MAX (GPIO25/GPIO26)...");
  bool encontrou = false;
  for (uint8_t addr = 1; addr < 127; addr++) {
    I2C_MAX.beginTransmission(addr);
    if (I2C_MAX.endTransmission() == 0) {
      Serial.print("  Dispositivo encontrado: 0x");
      Serial.println(addr, HEX);
      encontrou = true;
    }
  }
  if (!encontrou) Serial.println("  Nenhum dispositivo encontrado no I2C_MAX!");
}

void iniciarMAX30102() {
  if (!particleSensor.begin(I2C_MAX, I2C_SPEED_STANDARD)) {
    Serial.println("MAX30102 NAO encontrado em GPIO25/GPIO26");
    maxOK = false;
    return;
  }
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x1F);
  particleSensor.setPulseAmplitudeIR(0x1F);
  particleSensor.setPulseAmplitudeGreen(0);
  maxOK = true;
  initFiltros();
  Serial.println("MAX30102 encontrado e inicializado em GPIO25/GPIO26");
}

void initFiltros() {
  const float pi2 = 6.28318530f;
  // LPF @ 5 Hz
  float lx = expf(-1.0f / (kSampFreq / (5.0f * pi2)));
  kLPF_a0 = 1.0f - lx;  kLPF_b1 = lx;
  // HPF @ 0.5 Hz
  float hx = expf(-1.0f / (kSampFreq / (0.5f * pi2)));
  kHPF_a0 = (1.0f + hx) / 2.0f;  kHPF_a1 = -kHPF_a0;  kHPF_b1 = hx;
  // Reset estados
  filtrosInit  = false;
  zc_crossed   = false;
  zc_crossedAt = 0;
  zc_lastDiff  = 0.0f;
  mm_red_n = 0;  mm_red_sum = 0.0f;
  mm_ir_n  = 0;  mm_ir_sum  = 0.0f;
  spo2BufCount = 0;  spo2BufIdx = 0;
  piBufCount   = 0;  piBufIdx   = 0;
  emaBPMReady  = false;  emaBPM  = 0.0f;
  emaSpo2Ready = false;  emaSpo2 = 0.0f;
  emaPIReady   = false;  emaPI   = 0.0f;
}

void lerMAX30102() {
  if (!maxOK) return;

  particleSensor.check();

  while (particleSensor.available()) {
    long ir  = particleSensor.getIR();
    long red = particleSensor.getRed();
    particleSensor.nextSample();

    irValue  = ir;
    redValue = red;

    unsigned long agora = millis();

    // ── Histerese de dedo (1.5 s) ─────────────────────────────────────────
    if (ir > 10000) ultimaVezComDedo = agora;
    dedoDetectado = (agora - ultimaVezComDedo < 1500);

    // ── Reset completo após 3 s sem dedo ─────────────────────────────────
    if (!dedoDetectado && (agora - ultimaVezComDedo > 3000)) {
      initFiltros();
      bpmReal = 0;  bpmBufCount = 0;  bpmBufIdx = 0;
      spo2Real = 0; piVal = 0.0f;
      spo2BufCount = 0;  spo2BufIdx = 0;
      piBufCount   = 0;  piBufIdx   = 0;
      hrvRMSSD = 0; rrBufCount = 0;   rrBufIdx  = 0;
      ultimoBeat = 0;
      continue;
    }

    if (!dedoDetectado) continue;

    // ── Inicialização dos filtros na primeira amostra válida ──────────────
    if (!filtrosInit) {
      lpf_red = (float)red;  lpf_ir  = (float)ir;
      hpf_raw = lpf_red;     hpf_out = 0.0f;
      dif_prev = 0.0f;
      filtrosInit = true;
      continue;
    }

    // ── LOW-PASS FILTER (5 Hz) ────────────────────────────────────────────
    lpf_red = kLPF_a0 * (float)red + kLPF_b1 * lpf_red;
    lpf_ir  = kLPF_a0 * (float)ir  + kLPF_b1 * lpf_ir;

    // ── ESTATÍSTICA MIN/MAX por ciclo de batida ───────────────────────────
    if (mm_red_n == 0) { mm_red_min = mm_red_max = lpf_red; }
    else { mm_red_min = min(mm_red_min, lpf_red);  mm_red_max = max(mm_red_max, lpf_red); }
    mm_red_sum += lpf_red;  mm_red_n++;

    if (mm_ir_n == 0) { mm_ir_min = mm_ir_max = lpf_ir; }
    else { mm_ir_min = min(mm_ir_min, lpf_ir);  mm_ir_max = max(mm_ir_max, lpf_ir); }
    mm_ir_sum += lpf_ir;  mm_ir_n++;

    // ── HIGH-PASS FILTER (0.5 Hz) ─────────────────────────────────────────
    float red_hp = kHPF_a0 * lpf_red + kHPF_a1 * hpf_raw + kHPF_b1 * hpf_out;
    hpf_raw = lpf_red;
    hpf_out = red_hp;

    // ── DIFERENCIADOR ─────────────────────────────────────────────────────
    float curDiff = (red_hp - dif_prev) * kSampFreq;
    dif_prev = red_hp;

    // ── DETECÇÃO DE BATIMENTO: zero-crossing + limiar de queda ───────────
    if (zc_lastDiff > 0.0f && curDiff < 0.0f) {
      zc_crossed   = true;
      zc_crossedAt = agora;
    }
    if (curDiff > 0.0f) zc_crossed = false;

    if (zc_crossed && curDiff < kEdgeThresh) {
      long delta = (long)(zc_crossedAt - ultimoBeat);
      if (ultimoBeat > 0 && delta > 300 && delta < 2000) {

        // ── BPM ──────────────────────────────────────────────────────────
        float bpmInst = 60000.0f / (float)delta;
        if (bpmInst >= 40.0f && bpmInst <= 180.0f) {
          bool bpmValido = true;
          if (bpmBufCount >= 4) {
            float somaAtual = 0.0f;
            for (int i = 0; i < bpmBufCount; i++) somaAtual += bpmBuffer[i];
            float mediaAtual = somaAtual / bpmBufCount;
            if (fabsf(bpmInst - mediaAtual) > mediaAtual * 0.30f) bpmValido = false;
          }
          if (bpmValido) {
            bpmBuffer[bpmBufIdx] = bpmInst;
            bpmBufIdx = (bpmBufIdx + 1) % BPM_BUF_SIZE;
            if (bpmBufCount < BPM_BUF_SIZE) bpmBufCount++;
            if (bpmBufCount >= 4) {
              float soma = 0.0f;
              for (int i = 0; i < bpmBufCount; i++) soma += bpmBuffer[i];
              float mediaCalc = soma / bpmBufCount;
              if (!emaBPMReady) { emaBPM = mediaCalc; emaBPMReady = true; }
              else emaBPM = EMA_ALPHA * mediaCalc + (1.0f - EMA_ALPHA) * emaBPM;
              bpmReal = (int)(emaBPM + 0.5f);
            }
          }
        }

        // ── HRV (RMSSD) ──────────────────────────────────────────────────
        rrBuf[rrBufIdx] = delta;
        rrBufIdx = (rrBufIdx + 1) % RR_BUF_SIZE;
        if (rrBufCount < RR_BUF_SIZE) rrBufCount++;
        if (rrBufCount >= 2) {
          long sumSq = 0;  int pairs = rrBufCount - 1;
          for (int k = 0; k < pairs; k++) {
            int i0 = (rrBufIdx - rrBufCount + k + RR_BUF_SIZE) % RR_BUF_SIZE;
            int i1 = (i0 + 1) % RR_BUF_SIZE;
            long d = rrBuf[i1] - rrBuf[i0];
            sumSq += d * d;
          }
          hrvRMSSD = (int)sqrtf((float)sumSq / pairs);
        }

        // ── SpO2 e PI ────────────────────────────────────────────────────
        if (mm_red_n > 5 && mm_ir_n > 5) {
          float redAC = mm_red_max - mm_red_min;
          float redDC = mm_red_sum / (float)mm_red_n;
          float irAC  = mm_ir_max  - mm_ir_min;
          float irDC  = mm_ir_sum  / (float)mm_ir_n;

          if (irDC > 0.0f) {
            float piInst = constrain((irAC / irDC) * 100.0f, 0.0f, 20.0f);
            piBuffer[piBufIdx] = piInst;
            piBufIdx = (piBufIdx + 1) % PI_BUF_SIZE;
            if (piBufCount < PI_BUF_SIZE) piBufCount++;
            float somaPI = 0.0f;
            for (int i = 0; i < piBufCount; i++) somaPI += piBuffer[i];
            float rawPI = somaPI / piBufCount;
            if (!emaPIReady) { emaPI = rawPI; emaPIReady = true; }
            else emaPI = EMA_ALPHA * rawPI + (1.0f - EMA_ALPHA) * emaPI;
            piVal = emaPI;
          }
          if (irAC > 0.0f && irDC > 0.0f && redDC > 0.0f && redAC > 0.0f) {
            float R     = (redAC / redDC) / (irAC / irDC);
            float spo2f = 1.5958422f * R * R - 34.6596622f * R + 112.6898759f;
            int spo2Inst = constrain((int)(spo2f + 0.5f), 70, 100);
            spo2Buffer[spo2BufIdx] = spo2Inst;
            spo2BufIdx = (spo2BufIdx + 1) % SPO2_BUF_SIZE;
            if (spo2BufCount < SPO2_BUF_SIZE) spo2BufCount++;
            int somaS = 0;
            for (int i = 0; i < spo2BufCount; i++) somaS += spo2Buffer[i];
            float rawSpo2 = (float)(somaS / spo2BufCount);
            if (!emaSpo2Ready) { emaSpo2 = rawSpo2; emaSpo2Ready = true; }
            else emaSpo2 = EMA_ALPHA * rawSpo2 + (1.0f - EMA_ALPHA) * emaSpo2;
            spo2Real = (int)(emaSpo2 + 0.5f);
          }
        }
        // Reset MinMax para o próximo ciclo de batida
        mm_red_n = 0;  mm_red_sum = 0.0f;
        mm_ir_n  = 0;  mm_ir_sum  = 0.0f;
      }
      zc_crossed = false;
      ultimoBeat = zc_crossedAt;
    }

    zc_lastDiff = curDiff;
  }

  // ── Qualidade do sinal ────────────────────────────────────────────────
  if (!dedoDetectado)                    qualSinal = 0;
  else if (piVal < 0.3f || bpmReal == 0) qualSinal = 1;
  else                                   qualSinal = 2;

  // ── Debug a cada 5 s ─────────────────────────────────────────────────
  unsigned long agoraNow = millis();
  if (agoraNow - ultimoDebugMAX >= 5000) {
    ultimoDebugMAX = agoraNow;
    Serial.print("IR=");    Serial.print(irValue);
    Serial.print(" RED=");  Serial.print(redValue);
    Serial.print(" DEDO="); Serial.print(dedoDetectado);
    Serial.print(" BPM=");  Serial.print(bpmReal);
    Serial.print(" SpO2="); Serial.print(spo2Real);
    Serial.print(" PI=");   Serial.print(piVal, 1);
    Serial.print(" HRV=");
    if (hrvRMSSD > 0) Serial.println(hrvRMSSD); else Serial.println("--");
  }
}
