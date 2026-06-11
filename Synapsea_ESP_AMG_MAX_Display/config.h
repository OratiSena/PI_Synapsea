#pragma once

// ─── Bibliotecas comuns ────────────────────────────────────────────────────
#include "DFRobot_GDL.h"
#include <Wire.h>
#include <Adafruit_AMG88xx.h>
#include <MAX30105.h>
#include <heartRate.h>
#include <spo2_algorithm.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "secrets.h"

// ─── Pinos do display ─────────────────────────────────────────────────────
#define TFT_DC  2
#define TFT_CS  5
#define TFT_RST 4

// ─── Sensor térmico ───────────────────────────────────────────────────────
#define AMG_COLS 8
#define AMG_ROWS 8
#define INTERPOLATED_COLS 30
#define INTERPOLATED_ROWS 30
#define OFFSET_TEMP 5.0

// Calibração do AMG8833: ajuste fino experimental.
// O AMG8833 mede temperatura superficial por infravermelho e pode apresentar variação
// conforme distância, ambiente e emissividade. O offset permite ajuste fino experimental.
// Altere AMG_TEMP_OFFSET para calibrar: ex. 0.0, +1.0, +1.5, +2.0, -1.0
#define AMG_TEMP_OFFSET 0.0f

#define MODE_INTERPOLATION 2
#define MINTEMP 10
#define MAXTEMP 39

// Telemetria HTTPS via ngrok. Credenciais locais ficam em secrets.h.
#define API_BASE_URL "https://pupil-renderer-defile.ngrok-free.dev/api"
#define API_VITALS_URL API_BASE_URL "/vitals"
#define API_TEMPERATURE_URL API_BASE_URL "/temperature"
#define API_STATUS_URL API_BASE_URL "/system/status"
#define DEVICE_ID_MAX "max30102-001"
#define DEVICE_ID_AMG "amg8833-001"
#define PATIENT_ID "20000000-0000-4000-8000-000000000001"
#define TELEMETRY_UPLOAD_INTERVAL_MS 5000UL

#ifndef DEVICE_API_KEY
#define DEVICE_API_KEY ""
#endif

// ─── Display ECG ──────────────────────────────────────────────────────────
#define ECG_W  220
#define ECG_Y  228
#define ECG_H   80

// ─── Botão ────────────────────────────────────────────────────────────────
#define BTN_PIN 15

// ─── MAX30102 ─────────────────────────────────────────────────────────────
#define BPM_BUF_SIZE  8
#define SPO2_BUF_SIZE 5
#define PI_BUF_SIZE   5
#define RR_BUF_SIZE   15
#define EMA_ALPHA     0.30f

// ─── Objetos globais compartilhados ───────────────────────────────────────
extern DFRobot_ST7789_240x320_HW_SPI tft;
extern Adafruit_AMG88xx amg;
extern MAX30105 particleSensor;
extern TwoWire  I2C_MAX;

// ─── Variáveis do sensor AMG8833 ──────────────────────────────────────────
extern float pixels[AMG_COLS * AMG_ROWS];
extern float pixels2[AMG_COLS * AMG_ROWS];
extern float dest_2d[INTERPOLATED_ROWS * INTERPOLATED_COLS];
extern float pix_max, pos_x, pos_y;
extern const uint16_t camColors[];
extern const int maxIndex;

// ─── Variáveis do sensor MAX30102 ─────────────────────────────────────────
extern bool     maxOK;
extern bool     dedoDetectado;
extern long     irValue;
extern long     redValue;

// Cache de display
extern int   prevBPM_d;
extern int   prevSpo2_d;
extern bool  prevDedo_d;
extern bool  prevMaxOK_d;
extern float prevPI_d;
extern int   prevQual_d;
extern int   prevHRV_d;

// BPM
extern float         bpmBuffer[BPM_BUF_SIZE];
extern int           bpmBufIdx;
extern int           bpmBufCount;
extern unsigned long ultimoBeat;
extern unsigned long ultimaVezComDedo;
extern int           bpmReal;

// SpO2 e PI
extern int   spo2Real;
extern float piVal;
extern int   spo2Buffer[SPO2_BUF_SIZE];
extern int   spo2BufIdx;
extern int   spo2BufCount;
extern float piBuffer[PI_BUF_SIZE];
extern int   piBufIdx;
extern int   piBufCount;

// EMA
extern float emaBPM;   extern bool emaBPMReady;
extern float emaSpo2;  extern bool emaSpo2Ready;
extern float emaPI;    extern bool emaPIReady;

// Filtros IIR
extern const float kSampFreq;
extern const float kEdgeThresh;
extern float kLPF_a0, kLPF_b1;
extern float kHPF_a0, kHPF_a1, kHPF_b1;
extern bool  filtrosInit;
extern float lpf_red, lpf_ir;
extern float hpf_raw, hpf_out;
extern float dif_prev;
extern float zc_lastDiff;
extern bool  zc_crossed;
extern unsigned long zc_crossedAt;
extern float mm_red_min, mm_red_max, mm_red_sum; extern int mm_red_n;
extern float mm_ir_min,  mm_ir_max,  mm_ir_sum;  extern int mm_ir_n;

// Qualidade e HRV
extern int   qualSinal;
extern int   respRPM;
extern long  rrBuf[RR_BUF_SIZE];
extern int   rrBufIdx;
extern int   rrBufCount;
extern int   hrvRMSSD;

// Filtro de respiração (vars globais para reset correto)
extern float         br_lp1;
extern float         br_lp2;
extern float         br_prev_sig;
extern bool          br_amp_ema_ready;
extern unsigned long br_last_t;
extern long          br_ints[6];
extern int           br_idx;
extern int           br_cnt;
extern int           br_beat_cnt;

// ─── Controle de tela e botão ─────────────────────────────────────────────
extern int  telaAtual;
extern bool btnAnterior;
extern unsigned long ultimoDebounce;
extern const unsigned long debounceDelay;
extern bool telaPrecisaRedesenhar;

// ─── Touchscreen (XPT2046) ────────────────────────────────────────────────
// GPIO 34 e 35 são somente entrada no ESP32 → usados para T_DO e T_IRQ.
// GPIO 32, 13 e 12 são pinos de saída disponíveis → usados para CLK, CS e DIN.
//
//   Pinagem final:
//     T_CLK → GPIO 32   (saída — clock SPI)
//     T_CS  → GPIO 13   (saída — chip select)
//     T_DIN → GPIO 12   (saída — MOSI)
//     T_DO  → GPIO 35   (entrada apenas — MISO)
//     T_IRQ → GPIO 34   (entrada apenas — interrupção)
#include <XPT2046_Touchscreen.h>
#define T_CLK  32
#define T_CS   13
#define T_DIN  12
#define T_DO   35
#define T_IRQ  34

// Calibração raw do XPT2046 (ajuste se o toque estiver descalibrado)
#define TOUCH_X_MIN   340
#define TOUCH_X_MAX  3900
#define TOUCH_Y_MIN   250
#define TOUCH_Y_MAX  3800
#define TOUCH_SWIPE_MIN 45   // pixels mínimos para detectar swipe

extern XPT2046_Touchscreen ts;
extern bool mostrarPainelConfig;   // true = painel de config sobreposto
extern bool tempUnitCelsius;       // true=°C  false=°F
extern int  ppgZoom;               // 1-5, zoom da onda PPG (padrão=2)
extern bool amgFrozen;             // true = imagem térmica congelada (tap)
extern bool  amgCacheReset;        // true = força redesenho dos números na próxima atualização
extern bool  amgAutoScale;         // true = escala automática suavizada  false = fixo 22–38°C
extern float escalaMin;            // mínimo atual da escala de cores
extern float escalaMax;            // máximo atual da escala de cores
