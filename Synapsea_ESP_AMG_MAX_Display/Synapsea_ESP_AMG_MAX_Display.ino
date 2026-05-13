// ─── Arquivo principal: main.ino ──────────────────────────────────────────
// Contém: variáveis globais, setup() e loop()
// Demais funcionalidades estão em:
//   config.h      → defines, includes e declarações extern
//   sensorAMG.h/cpp → lógica do sensor térmico AMG8833
//   sensorMAX.h/cpp → lógica do sensor MAX30102 (BPM, SpO2, PI, HRV)
//   display.h/cpp   → tudo relacionado ao display ILI9341
// ──────────────────────────────────────────────────────────────────────────

#include "config.h"
#include "sensorAMG.h"
#include "sensorMAX.h"
#include "display.h"

// ─── Definição dos objetos globais ────────────────────────────────────────
DFRobot_ST7789_240x320_HW_SPI tft(TFT_DC, TFT_CS, TFT_RST);
Adafruit_AMG88xx amg;
TwoWire  I2C_MAX = TwoWire(1);
MAX30105 particleSensor;

// ─── Variáveis do sensor AMG8833 ──────────────────────────────────────────
float pixels[AMG_COLS * AMG_ROWS];
float pixels2[AMG_COLS * AMG_ROWS];
float dest_2d[INTERPOLATED_ROWS * INTERPOLATED_COLS];
float pix_max = 0, pos_x = 0, pos_y = 0;

const uint16_t camColors[] = {
  0x480F, 0x400F, 0x400F, 0x400F, 0x4010, 0x3810, 0x3810, 0x3810,
  0x3810, 0x3010, 0x3010, 0x3010, 0x2810, 0x2810, 0x2810, 0x2810,
  0x2010, 0x2010, 0x2010, 0x1810, 0x1810, 0x1811, 0x1811, 0x1011,
  0x1011, 0x1011, 0x0811, 0x0811, 0x0811, 0x0011, 0x0011, 0x0011,
  0x0011, 0x0011, 0x0031, 0x0031, 0x0051, 0x0072, 0x0072, 0x0092,
  0x00B2, 0x00B2, 0x00D2, 0x00F2, 0x00F2, 0x0112, 0x0132, 0x0152,
  0x0152, 0x0172, 0x0192, 0x0192, 0x01B2, 0x01D2, 0x01F3, 0x01F3,
  0x0213, 0x0233, 0x0253, 0x0253, 0x0273, 0x0293, 0x02B3, 0x02D3,
  0x02D3, 0x02F3, 0x0313, 0x0333, 0x0333, 0x0353, 0x0373, 0x0394,
  0x03B4, 0x03D4, 0x03D4, 0x03F4, 0x0414, 0x0434, 0x0454, 0x0474,
  0x0474, 0x0494, 0x04B4, 0x04D4, 0x04F4, 0x0514, 0x0534, 0x0534,
  0x0554, 0x0554, 0x0574, 0x0574, 0x0573, 0x0573, 0x0573, 0x0572,
  0x0572, 0x0572, 0x0571, 0x0591, 0x0591, 0x0590, 0x0590, 0x058F,
  0x058F, 0x058F, 0x058E, 0x05AE, 0x05AE, 0x05AD, 0x05AD, 0x05AD,
  0x05AC, 0x05AC, 0x05AB, 0x05CB, 0x05CB, 0x05CA, 0x05CA, 0x05CA,
  0x05C9, 0x05C9, 0x05C8, 0x05E8, 0x05E8, 0x05E7, 0x05E7, 0x05E6,
  0x05E6, 0x05E6, 0x05E5, 0x05E5, 0x0604, 0x0604, 0x0604, 0x0603,
  0x0603, 0x0602, 0x0602, 0x0601, 0x0621, 0x0621, 0x0620, 0x0620,
  0x0620, 0x0620, 0x0E20, 0x0E20, 0x0E40, 0x1640, 0x1640, 0x1E40,
  0x1E40, 0x2640, 0x2640, 0x2E40, 0x2E60, 0x3660, 0x3660, 0x3E60,
  0x3E60, 0x3E60, 0x4660, 0x4660, 0x4E60, 0x4E80, 0x5680, 0x5680,
  0x5E80, 0x5E80, 0x6680, 0x6680, 0x6E80, 0x6EA0, 0x76A0, 0x76A0,
  0x7EA0, 0x7EA0, 0x86A0, 0x86A0, 0x8EA0, 0x8EC0, 0x96C0, 0x96C0,
  0x9EC0, 0x9EC0, 0xA6C0, 0xAEC0, 0xAEC0, 0xB6E0, 0xB6E0, 0xBEE0,
  0xBEE0, 0xC6E0, 0xC6E0, 0xCEE0, 0xCEE0, 0xD6E0, 0xD700, 0xDF00,
  0xDEE0, 0xDEC0, 0xDEA0, 0xDE80, 0xDE80, 0xE660, 0xE640, 0xE620,
  0xE600, 0xE5E0, 0xE5C0, 0xE5A0, 0xE580, 0xE560, 0xE540, 0xE520,
  0xE500, 0xE4E0, 0xE4C0, 0xE4A0, 0xE480, 0xE460, 0xEC40, 0xEC20,
  0xEC00, 0xEBE0, 0xEBC0, 0xEBA0, 0xEB80, 0xEB60, 0xEB40, 0xEB20,
  0xEB00, 0xEAE0, 0xEAC0, 0xEAA0, 0xEA80, 0xEA60, 0xEA40, 0xF220,
  0xF200, 0xF1E0, 0xF1C0, 0xF1A0, 0xF180, 0xF160, 0xF140, 0xF100,
  0xF0E0, 0xF0C0, 0xF0A0, 0xF080, 0xF060, 0xF040, 0xF020, 0xF800,
};

const int maxIndex = sizeof(camColors) / 2 - 1;

// ─── Variáveis do MAX30102 ────────────────────────────────────────────────
bool     maxOK         = false;
bool     dedoDetectado = false;
long     irValue       = 0;
long     redValue      = 0;

int   prevBPM_d      = -999;
int   prevSpo2_d     = -999;
bool  prevDedo_d     = false;
bool  prevMaxOK_d    = false;
float prevPI_d       = -1.0f;
int   prevQual_d     = -1;
int   prevHRV_d      = -999;

float         bpmBuffer[BPM_BUF_SIZE] = {0};
int           bpmBufIdx               = 0;
int           bpmBufCount             = 0;
unsigned long ultimoBeat              = 0;
unsigned long ultimaVezComDedo        = 0;
int           bpmReal                 = 0;

int   spo2Real = 0;
float piVal    = 0.0f;

int   spo2Buffer[SPO2_BUF_SIZE] = {0};
int   spo2BufIdx   = 0;
int   spo2BufCount = 0;

float piBuffer[PI_BUF_SIZE] = {0.0f};
int   piBufIdx   = 0;
int   piBufCount = 0;

float emaBPM  = 0.0f;  bool emaBPMReady  = false;
float emaSpo2 = 0.0f;  bool emaSpo2Ready = false;
float emaPI   = 0.0f;  bool emaPIReady   = false;

const float kSampFreq   = 100.0f;
const float kEdgeThresh = -500.0f;
float kLPF_a0, kLPF_b1;
float kHPF_a0, kHPF_a1, kHPF_b1;
bool  filtrosInit  = false;
float lpf_red, lpf_ir;
float hpf_raw, hpf_out;
float dif_prev;
float         zc_lastDiff  = 0.0f;
bool          zc_crossed   = false;
unsigned long zc_crossedAt = 0;
float mm_red_min, mm_red_max, mm_red_sum;  int mm_red_n = 0;
float mm_ir_min,  mm_ir_max,  mm_ir_sum;   int mm_ir_n  = 0;
int   qualSinal        = 0;

long  rrBuf[RR_BUF_SIZE] = {0};
int   rrBufIdx            = 0;
int   rrBufCount          = 0;
int   hrvRMSSD            = 0;

// ─── ECG scroll ───────────────────────────────────────────────────────────
int  ecgBuf[ECG_W]   = {0};
int  ecgPos          = 0;
long prevIR          = 0;

// ─── Controle de tela e botão ─────────────────────────────────────────────
int  telaAtual                              = 0;
bool btnAnterior                            = HIGH;
unsigned long ultimoDebounce                = 0;
const unsigned long debounceDelay           = 300;
unsigned long ultimaAtualizacaoMAX          = 0;
const unsigned long intervaloAtualizacaoMAX = 500;
unsigned long ultimoDebugMAX                = 0;

// ─────────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  tft.begin();
  tft.invertDisplay(false);
  tft.setRotation(2);
  tft.fillScreen(0x0000);
  tft.setTextColor(0xFFFF);
  tft.setTextSize(2);

  iniciarAMG8833();
  colorbar();

  pinMode(BTN_PIN, INPUT_PULLUP);

  I2C_MAX.begin(25, 26);
  scannerI2CMAX();
  iniciarMAX30102();
}

void loop() {
  verificarBotaoTrocaTela();

  if (telaAtual == 0) {
    // ── Tela térmica AMG8833 ───────────────────────────────────────────────
    lerAMG8833();

    uint16_t boxSize = min(tft.width() / INTERPOLATED_COLS, 240 / INTERPOLATED_ROWS);
    drawpixels(dest_2d, INTERPOLATED_ROWS, INTERPOLATED_COLS, boxSize, boxSize, false);

    tft.fillRect(0, 250, 240, 30, 0x0000);
    tft.setTextColor(0xFFFF);
    tft.setCursor(0, 250);
    tft.setTextSize(3);
    tft.print("MAX:");
    tft.print(pix_max);
    tft.print(" C");

    tft.drawCircle(pos_x, pos_y, 6, 0);
    tft.drawLine(pos_x - 3, pos_y, pos_x + 3, pos_y, 0);
    tft.drawLine(pos_x, pos_y - 3, pos_x, pos_y + 3, 0);
    pix_max = 0;
    delay(50);

  } else {
    // ── Tela do MAX30102 ──────────────────────────────────────────────────
    lerMAX30102();
    if (millis() - ultimaAtualizacaoMAX >= intervaloAtualizacaoMAX) {
      desenharTelaMAX();
      ultimaAtualizacaoMAX = millis();
    }
  }
}
