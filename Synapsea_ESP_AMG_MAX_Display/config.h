#pragma once

// ─── Bibliotecas comuns ────────────────────────────────────────────────────
#include "DFRobot_GDL.h"
#include <Wire.h>
#include <Adafruit_AMG88xx.h>
#include <MAX30105.h>
#include <heartRate.h>
#include <spo2_algorithm.h>

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
#define MODE_INTERPOLATION 2
#define MINTEMP 10
#define MAXTEMP 39

// ─── Display ECG ──────────────────────────────────────────────────────────
#define ECG_W  220
#define ECG_Y  228
#define ECG_H   80

// ─── Botão ────────────────────────────────────────────────────────────────
#define BTN_PIN 27

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
extern long  rrBuf[RR_BUF_SIZE];
extern int   rrBufIdx;
extern int   rrBufCount;
extern int   hrvRMSSD;

// ─── Controle de tela e botão ─────────────────────────────────────────────
extern int  telaAtual;
extern bool btnAnterior;
extern unsigned long ultimoDebounce;
extern const unsigned long debounceDelay;
extern bool telaPrecisaRedesenhar;
