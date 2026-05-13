#include "DFRobot_GDL.h"
#include <Wire.h>
#include <Adafruit_AMG88xx.h>
#include <MAX30105.h>       // SparkFun MAX3010x library (compativel com MAX30102)
#include <heartRate.h>      // Calculo de BPM (checkForBeat)
#include <spo2_algorithm.h> // Calculo de SpO2 (maxim_heart_rate_and_oxygen_saturation)


// Pinos do display
#define TFT_DC 2
#define TFT_CS 5
#define TFT_RST 4

DFRobot_ST7789_240x320_HW_SPI tft(TFT_DC, TFT_CS, TFT_RST);

// Sensor térmico
Adafruit_AMG88xx amg;
#define AMG_COLS 8
#define AMG_ROWS 8
float pixels[AMG_COLS * AMG_ROWS];
float pixels2[AMG_COLS * AMG_ROWS];

// Tamanho da imagem interpolada
#define INTERPOLATED_COLS 30 /* Em 35 ele diminui a tela!! */
#define INTERPOLATED_ROWS 30 /* Em 35 ele diminui a tela!! */


// /* WIFI - DESATIVADO TEMPORARIAMENTE */
// #include <WiFi.h>
// #include <HTTPClient.h>
// #include "base64.h"
// const char *ssid = "iPhone de Vitor";
// const char *password = "vitor.sena";


// Paleta de cores
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


#define MINTEMP 10
#define MAXTEMP 39

float pix_max = 0, pos_x = 0, pos_y = 0;
float dest_2d[INTERPOLATED_ROWS * INTERPOLATED_COLS];


// /* ENVIO PARA O BANCO DE DADOS - DESATIVADO TEMPORARIAMENTE */
// unsigned long ultimoEnvio = 0;
// const unsigned long intervaloEnvio = 1000;

// ─── MAX30102 ──────────────────────────────────────────────────────────────
TwoWire I2C_MAX = TwoWire(1); // Segundo barramento I2C exclusivo para o MAX30102 (GPIO25/GPIO26)
MAX30105 particleSensor;

bool     maxOK         = false;
bool     dedoDetectado = false;
long     irValue       = 0;
long     redValue      = 0;

// ─── PPG / ECG scrolling ──────────────────────────────────────────────────
#define ECG_W   220
#define ECG_Y   228
#define ECG_H    80
int  ecgBuf[ECG_W]   = {0};
int  ecgPos          = 0;
long prevIR          = 0;

// ─── Cache de display para evitar redesenho desnecessário ─────────────────
int   prevBPM_d      = -999;
int   prevSpo2_d     = -999;
bool  prevDedo_d     = false;
bool  prevMaxOK_d    = false;
float prevPI_d       = -1.0f;
int   prevQual_d     = -1;
int   prevHRV_d      = -999;

// ─── Cálculo real de BPM ──────────────────────────────────────────────────
#define BPM_BUF_SIZE 8           // Mais amostras = média mais estável
float         bpmBuffer[BPM_BUF_SIZE] = {0};
int           bpmBufIdx               = 0;
int           bpmBufCount             = 0;
unsigned long ultimoBeat              = 0;
unsigned long ultimaVezComDedo        = 0; // histerese: ultima vez que irValue > 10000
int           bpmReal                 = 0;

// ─── SpO2 e PI ────────────────────────────────────────────────────────────
int   spo2Real = 0;
float piVal    = 0.0f;

// Buffers de suavização para SpO2 e PI
#define SPO2_BUF_SIZE 5
int   spo2Buffer[SPO2_BUF_SIZE] = {0};
int   spo2BufIdx   = 0;
int   spo2BufCount = 0;

#define PI_BUF_SIZE 5
float piBuffer[PI_BUF_SIZE] = {0.0f};
int   piBufIdx   = 0;
int   piBufCount = 0;

// ─── Filtro EMA (Exponential Moving Average) — suavização final para exibição ──
// Alpha: 0.0 = sem resposta (congela), 1.0 = sem suavização (bruto)
// 0.30 = suave e responsivo (~6 amostras equiv.); aumente para 0.5 se quiser mais responsividade
#define EMA_ALPHA 0.30f
float emaBPM  = 0.0f;  bool emaBPMReady  = false;
float emaSpo2 = 0.0f;  bool emaSpo2Ready = false;
float emaPI   = 0.0f;  bool emaPIReady   = false;

// ─── Filtros IIR + detecção de batimento (baseado em codigo_da_internet.ino) ──
// Fs efetiva do MAX30102 com setup() padrão: 400sps / avg4 = 100 sps
const float kSampFreq   = 100.0f;
const float kEdgeThresh = -500.0f; // limiar do diferenciador para confirmar batida
float kLPF_a0, kLPF_b1;           // coeficientes LPF @ 5 Hz
float kHPF_a0, kHPF_a1, kHPF_b1;  // coeficientes HPF @ 0.5 Hz

bool  filtrosInit  = false; // filtros prontos para uso
float lpf_red, lpf_ir;     // estado do LPF
float hpf_raw, hpf_out;    // estado do HPF
float dif_prev;             // estado do diferenciador

float         zc_lastDiff  = 0.0f;
bool          zc_crossed   = false;
unsigned long zc_crossedAt = 0;

// MinMax por ciclo entre batidas (para SpO2 e PI)
float mm_red_min, mm_red_max, mm_red_sum;  int mm_red_n = 0;
float mm_ir_min,  mm_ir_max,  mm_ir_sum;   int mm_ir_n  = 0;

// ─── Qualidade do sinal: 0=sem dedo, 1=fraco, 2=bom ──────────────────────
int   qualSinal        = 0;

// ─── HRV (RMSSD) ─────────────────────────────────────────────────────────
#define RR_BUF_SIZE 15
long  rrBuf[RR_BUF_SIZE] = {0};
int   rrBufIdx            = 0;
int   rrBufCount          = 0;
int   hrvRMSSD            = 0; // ms

// ─── Botão e controle de tela ──────────────────────────────────────────────
#define BTN_PIN 27
int  telaAtual                              = 0;
bool btnAnterior                            = HIGH;
unsigned long ultimoDebounce                = 0;
const unsigned long debounceDelay           = 300;
unsigned long ultimaAtualizacaoMAX          = 0;
const unsigned long intervaloAtualizacaoMAX = 500;
unsigned long ultimoDebugMAX                = 0;







// Declarações antecipadas
void verificarBotaoTrocaTela();
void iniciarMAX30102();
void scannerI2CMAX();
void initFiltros();
void lerMAX30102();
void desenharTelaMAX();
void desenharCabecalhoMAX();
void desenharValoresMAX(bool forcar);
void atualizarECG();
void colorbar();
void drawpixels(float *p, uint8_t rows, uint8_t cols, uint8_t boxW, uint8_t boxH, boolean showVal);
float get_point(float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y);
void set_point(float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y, float f);
void interpolate_image(float *src, uint8_t src_rows, uint8_t src_cols, float *dest, uint8_t dest_rows, uint8_t dest_cols);
void interpolate_linear_image(float *src, uint8_t src_rows, uint8_t src_cols, float *dest, uint8_t dest_rows, uint8_t dest_cols);

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  tft.begin();
  tft.invertDisplay(false);  // DISPLAY TAVA COM CORES INVERTIDAS!!
  tft.setRotation(2);        /* DISPLAY AO CONTRÁRIO */
  tft.fillScreen(0x0000);    /* Fundo do display */
  tft.setTextColor(0xFFFF);
  tft.setTextSize(2);

  if (!amg.begin(0x68)) {
    Serial.println("Sensor AMG8833 nao encontrado no endereco 0x68");
    // Alternativa: amg.begin(0x69)
    tft.setCursor(10, 10);
    tft.print("Sensor NAO OK");
    while (1)
      ;
  } else {
    Serial.println("Sensor AMG8833 encontrado no endereco 0x68");
  }

  colorbar();

  // Botão de troca de tela
  pinMode(BTN_PIN, INPUT_PULLUP);

  // Inicializar MAX30102 no segundo barramento I2C
  I2C_MAX.begin(25, 26);
  scannerI2CMAX();
  iniciarMAX30102();
}





#define OFFSET_TEMP 5.0  // +5.0 elevava todos os valores (ex: 22C = 27C)
#define MODE_INTERPOLATION 2
// 0 = sem interpolação (imagem crua 8x8)
// 1 = bilinear (nitidez)
// 2 = bicubica (fluidez)


// Interpolação bilinear: mais rápida e mais nítida que a bicúbica
void interpolate_linear_image(float *src, uint8_t src_rows, uint8_t src_cols,
                              float *dest, uint8_t dest_rows, uint8_t dest_cols) {
  float mu_x = (src_cols - 1.0) / (dest_cols - 1.0);
  float mu_y = (src_rows - 1.0) / (dest_rows - 1.0);

  for (int y = 0; y < dest_rows; y++) {
    for (int x = 0; x < dest_cols; x++) {
      float gx = x * mu_x;
      float gy = y * mu_y;

      int gxi = (int)gx;
      int gyi = (int)gy;
      float c00 = get_point(src, src_rows, src_cols, gxi, gyi);
      float c10 = get_point(src, src_rows, src_cols, gxi + 1, gyi);
      float c01 = get_point(src, src_rows, src_cols, gxi, gyi + 1);
      float c11 = get_point(src, src_rows, src_cols, gxi + 1, gyi + 1);

      float tx = gx - gxi;
      float ty = gy - gyi;

      float val = (1 - tx) * (1 - ty) * c00 + tx * (1 - ty) * c10 + (1 - tx) * ty * c01 + tx * ty * c11;

      set_point(dest, dest_rows, dest_cols, x, y, val);
    }
  }
}



void loop() {
  verificarBotaoTrocaTela();

  if (telaAtual == 0) {
    // ── Tela térmica AMG8833 (lógica original intacta) ──────────────────────
    amg.readPixels(pixels2);
    for (int i = 0; i < 64; i++)
      pixels[i] = pixels2[(((int)(i / 8) * 8) + 7 - (i % 8))] + OFFSET_TEMP;

    if (MODE_INTERPOLATION == 2) {
      interpolate_image(pixels, AMG_ROWS, AMG_COLS, dest_2d, INTERPOLATED_ROWS, INTERPOLATED_COLS);
    } else if (MODE_INTERPOLATION == 1) {
      interpolate_linear_image(pixels, AMG_ROWS, AMG_COLS, dest_2d, INTERPOLATED_ROWS, INTERPOLATED_COLS);
    } else {
      for (int y = 0; y < INTERPOLATED_ROWS; y++) {
        for (int x = 0; x < INTERPOLATED_COLS; x++) {
          int origX = map(x, 0, INTERPOLATED_COLS - 1, 0, AMG_COLS - 1);
          int origY = map(y, 0, INTERPOLATED_ROWS - 1, 0, AMG_ROWS - 1);
          float val = get_point(pixels, AMG_ROWS, AMG_COLS, origX, origY);
          set_point(dest_2d, INTERPOLATED_ROWS, INTERPOLATED_COLS, x, y, val);
        }
      }
    }

    //smooth3x3(dest_2d, INTERPOLATED_ROWS, INTERPOLATED_COLS);

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
    // ── Tela do MAX30102 ────────────────────────────────────────────────────
    // Lê o sensor em CADA iteração do loop para não deixar o FIFO transbordar
    // (FIFO = 32 amostras @ ~100sps → overflow em ~320ms; display a cada 500ms causava perda de dados)
    lerMAX30102();
    if (millis() - ultimaAtualizacaoMAX >= intervaloAtualizacaoMAX) {
      desenharTelaMAX();
      ultimaAtualizacaoMAX = millis();
    }
  }
}




void drawpixels(float *p, uint8_t rows, uint8_t cols, uint8_t boxW, uint8_t boxH, boolean showVal) {
  // percorre todos os pontos interpolados
  for (int y = 0; y < rows; y++) {
    for (int x = 0; x < cols; x++) {
      float val = get_point(p, rows, cols, x, y);

      // mapeia val ∈ [MINTEMP, MAXTEMP] para idx ∈ [0, maxIndex]
      int idx = map(val, MINTEMP, MAXTEMP, 0, 239);  // Sem multiplicar por 10
      idx = constrain(idx, 0, 239);


      // **AQUI**: só este fillRect usa camColors[idx]
      tft.fillRect(x * boxW, y * boxH, boxW, boxH, camColors[idx]);

      // rastreia o máximo para desenhar o cruz
      if (val > pix_max) {
        pix_max = val;
        pos_x = x * boxW;
        pos_y = y * boxH;
      }

      // (opcional) sobrepõe valor numérico
      if (showVal) {
        tft.setCursor(x * boxW + boxW / 2 - 12, y * boxH + boxH / 2 + 4);
        tft.setTextColor(COLOR_RGB565_WHITE);
        tft.setTextSize(1);
        tft.print(val, 1);
      }
    }
  }
}


float get_point(float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y) {
  x = constrain(x, 0, cols - 1);
  y = constrain(y, 0, rows - 1);
  return p[y * cols + x];
}

void set_point(float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y, float f) {
  if (x >= 0 && x < cols && y >= 0 && y < rows) {
    p[y * cols + x] = f;
  }
}

float cubicInterpolate(float p[], float x) {
  return p[1] + 0.5 * x * (p[2] - p[0] + x * (2.0 * p[0] - 5.0 * p[1] + 4.0 * p[2] - p[3] + x * (3.0 * (p[1] - p[2]) + p[3] - p[0])));
}

float bicubicInterpolate(float p[], float x, float y) {
  float arr[4] = {
    cubicInterpolate(p + 0, x),
    cubicInterpolate(p + 4, x),
    cubicInterpolate(p + 8, x),
    cubicInterpolate(p + 12, x)
  };
  return cubicInterpolate(arr, y);
}

void get_adjacents_2d(float *src, float *dest, uint8_t rows, uint8_t cols, int8_t x, int8_t y) {
  for (int8_t dy = -1; dy < 3; dy++) {
    for (int8_t dx = -1; dx < 3; dx++) {
      dest[(dy + 1) * 4 + (dx + 1)] = get_point(src, rows, cols, x + dx, y + dy);
    }
  }
}

void interpolate_image(float *src, uint8_t src_rows, uint8_t src_cols, float *dest, uint8_t dest_rows, uint8_t dest_cols) {
  float mu_x = (src_cols - 1.0) / (dest_cols - 1.0);
  float mu_y = (src_rows - 1.0) / (dest_rows - 1.0);
  float adj_2d[16];

  for (uint8_t y_idx = 0; y_idx < dest_rows; y_idx++) {
    for (uint8_t x_idx = 0; x_idx < dest_cols; x_idx++) {
      float x = x_idx * mu_x;
      float y = y_idx * mu_y;
      get_adjacents_2d(src, adj_2d, src_rows, src_cols, x, y);
      float fx = x - (int)x;
      float fy = y - (int)y;
      float out = bicubicInterpolate(adj_2d, fx, fy);
      set_point(dest, dest_rows, dest_cols, x_idx, y_idx, out);
    }
  }
}





// faz um passe simples de média 3×3
void smooth3x3(float *arr, uint8_t R, uint8_t C) {
  static float buf[INTERPOLATED_ROWS * INTERPOLATED_COLS];
  memcpy(buf, arr, sizeof(buf));
  for (int y = 0; y < R; y++) {
    for (int x = 0; x < C; x++) {
      float s = 0, count = 0;
      for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
          int yy = y + dy, xx = x + dx;
          if (yy >= 0 && yy < R && xx >= 0 && xx < C) {
            s += buf[yy * C + xx];
            count += 1;
          }
        }
      }
      arr[y * C + x] = s / count;
    }
  }
}





void colorbar() {
  tft.fillRect(0, 280, 240, 24, 0x0000);  // Fundo preto da barra

  for (int i = 0; i < 240; i++) {
    int maxIndex = sizeof(camColors) / 2 - 1;
    int colorIndex = constrain((int)(i * 1.05), 0, maxIndex);  // <- ordem azul até vermelho
    tft.fillRect(i, 280, 1, 24, camColors[colorIndex]);
  }

  // Textos
  tft.setTextColor(0xFFFF);  // Branco
  tft.setTextSize(2);
  tft.setCursor(0, 285);
  tft.print(MINTEMP);
  tft.print("C");

  tft.setCursor(202, 285);
  tft.print(MAXTEMP);
  tft.print("C");
}



// /* BANCO DE DADOS - DESATIVADO TEMPORARIAMENTE */
// void enviarDadosParaServidor(float maxTemp, float *matriz, int linhas, int colunas) { ... }

// ─── Botão: troca de tela com debounce ────────────────────────────────────
void verificarBotaoTrocaTela() {
  bool btnAtual = digitalRead(BTN_PIN);
  if (btnAtual == LOW && btnAnterior == HIGH && (millis() - ultimoDebounce > debounceDelay)) {
    ultimoDebounce = millis();
    telaAtual = (telaAtual == 0) ? 1 : 0;
    tft.fillScreen(0x0000); // limpa tela ao trocar
    if (telaAtual == 0) {
      colorbar(); // restaura barra de cores na tela térmica
    } else {
      // Força redesenho completo ao entrar na tela do MAX
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

// ─── Scanner I2C no barramento do MAX30102 ───────────────────────────────
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

// ─── MAX30102: inicialização ───────────────────────────────────────────────
void iniciarMAX30102() {
  // I2C_MAX.begin() já foi chamado antes desta função
  if (!particleSensor.begin(I2C_MAX, I2C_SPEED_STANDARD)) {
    Serial.println("MAX30102 NAO encontrado em GPIO25/GPIO26");
    maxOK = false;
    return;
  }
  // Configuracao padrao SparkFun (powerLevel=0x1F ~6mA, avg=4, RED+IR+Green, 400sps, 411us, adcRange=16384)
  // Comprovadamente compativel com threshold 10000 para deteccao de dedo
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x1F);  // LED vermelho máximo
  particleSensor.setPulseAmplitudeIR(0x1F);   // LED IR máximo
  particleSensor.setPulseAmplitudeGreen(0);   // desliga LED verde
  maxOK = true;
  initFiltros();
  Serial.println("MAX30102 encontrado e inicializado em GPIO25/GPIO26");
}

// ─── Inicializa coeficientes e estado de todos os filtros ─────────────────
void initFiltros() {
  const float pi2 = 6.28318530f;
  // LPF @ 5 Hz
  float lx = expf(-1.0f / (kSampFreq / (5.0f  * pi2)));
  kLPF_a0 = 1.0f - lx;  kLPF_b1 = lx;
  // HPF @ 0.5 Hz
  float hx = expf(-1.0f / (kSampFreq / (0.5f  * pi2)));
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

// ─── MAX30102: leitura com BPM, SpO2, PI e HRV (filtros IIR) ──────────────
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

    // ── Histerese de dedo (1.5 s) ──────────────────────────────────────────
    if (ir > 10000) ultimaVezComDedo = agora;
    dedoDetectado = (agora - ultimaVezComDedo < 1500);

    // ── Reset completo após 3 s sem dedo ──────────────────────────────────
    if (!dedoDetectado && (agora - ultimaVezComDedo > 3000)) {
      initFiltros(); // já reseta EMA dentro de initFiltros()
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
      continue; // descarta primeira amostra (apenas inicialização)
    }

    // ── LOW-PASS FILTER (5 Hz) — remove ruído de alta frequência ─────────
    lpf_red = kLPF_a0 * (float)red + kLPF_b1 * lpf_red;
    lpf_ir  = kLPF_a0 * (float)ir  + kLPF_b1 * lpf_ir;

    // ── ESTATÍSTICA MIN/MAX por ciclo de batida (SpO2 / PI) ───────────────
    if (mm_red_n == 0) { mm_red_min = mm_red_max = lpf_red; }
    else { mm_red_min = min(mm_red_min, lpf_red);  mm_red_max = max(mm_red_max, lpf_red); }
    mm_red_sum += lpf_red;  mm_red_n++;

    if (mm_ir_n == 0) { mm_ir_min = mm_ir_max = lpf_ir; }
    else { mm_ir_min = min(mm_ir_min, lpf_ir);  mm_ir_max = max(mm_ir_max, lpf_ir); }
    mm_ir_sum += lpf_ir;  mm_ir_n++;

    // ── HIGH-PASS FILTER (0.5 Hz) — remove baseline DC ───────────────────
    float red_hp = kHPF_a0 * lpf_red + kHPF_a1 * hpf_raw + kHPF_b1 * hpf_out;
    hpf_raw = lpf_red;
    hpf_out = red_hp;

    // ── DIFERENCIADOR ─────────────────────────────────────────────────────
    float curDiff = (red_hp - dif_prev) * kSampFreq;
    dif_prev = red_hp;

    // ── DETECÇÃO DE BATIMENTO: zero-crossing + limiar de queda ────────────
    // Zero-crossing: slope positivo → negativo (pico do PPG invertido)
    if (zc_lastDiff > 0.0f && curDiff < 0.0f) {
      zc_crossed   = true;
      zc_crossedAt = agora;
    }
    if (curDiff > 0.0f) zc_crossed = false;

    // Confirma batida quando o diferenciador cai abaixo do limiar
    if (zc_crossed && curDiff < kEdgeThresh) {
      long delta = (long)(zc_crossedAt - ultimoBeat);
      if (ultimoBeat > 0 && delta > 300 && delta < 2000) {

        // ── BPM ────────────────────────────────────────────────────────
        float bpmInst = 60000.0f / (float)delta;
        if (bpmInst >= 40.0f && bpmInst <= 180.0f) {
          // Rejeita outliers: se já há ≥4 amostras e o valor desvia >30% da média atual, descarta
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
            // Só exibe BPM após 4 amostras válidas acumuladas (evita picos iniciais)
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

        // ── HRV (RMSSD) ────────────────────────────────────────────────
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

        // ── SpO2 e PI (usando min/max do ciclo de batida) ──────────────
        if (mm_red_n > 5 && mm_ir_n > 5) {
          float redAC = mm_red_max - mm_red_min;
          float redDC = mm_red_sum / (float)mm_red_n;
          float irAC  = mm_ir_max  - mm_ir_min;
          float irDC  = mm_ir_sum  / (float)mm_ir_n;
          // PI: limita a 20% (valores acima são artefatos de pressão/movimento)
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
            // Calibração SpO2 do Maxim AN6845 (mesma do codigo_da_internet.ino)
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

  // ── Qualidade do sinal ─────────────────────────────────────────────────
  if (!dedoDetectado)                    qualSinal = 0;
  else if (piVal < 0.3f || bpmReal == 0) qualSinal = 1;
  else                                   qualSinal = 2;

  // ── Debug a cada 5 s ──────────────────────────────────────────────────
  unsigned long agoraNow = millis();
  if (agoraNow - ultimoDebugMAX >= 5000) {
    ultimoDebugMAX = agoraNow;
    Serial.print("IR=");   Serial.print(irValue);
    Serial.print(" RED="); Serial.print(redValue);
    Serial.print(" DEDO="); Serial.print(dedoDetectado);
    Serial.print(" BPM="); Serial.print(bpmReal);
    Serial.print(" SpO2="); Serial.print(spo2Real);
    Serial.print(" PI=");  Serial.print(piVal, 1);
    Serial.print(" HRV=");
    if (hrvRMSSD > 0) Serial.println(hrvRMSSD); else Serial.println("--");
  }
}

// ─── MAX30102: desenhar tela estilo monitor médico ────────────────────────

// Desenha o cabeçalho fixo (chamado apenas uma vez ao entrar na tela)
void desenharCabecalhoMAX() {
  tft.fillScreen(0x0000);

  // Fundo do cabeçalho
  tft.fillRect(0, 0, 240, 38, 0x0841);

  // "SYNAPSEA" centralizado
  tft.setTextColor(0x07FF); // ciano
  tft.setTextSize(2);
  tft.setCursor(52, 4);
  tft.print("SYNAPSEA");

  // Subtítulo
  tft.setTextColor(0x8410); // cinza médio
  tft.setTextSize(1);
  tft.setCursor(72, 24);
  tft.print("Sinais Vitais");

  // Linha separadora ciano
  tft.fillRect(0, 38, 240, 2, 0x07FF);

  // Rótulos fixos BPM e SpO2
  tft.setTextColor(0xF800); // vermelho
  tft.setTextSize(1);
  tft.setCursor(14, 50);
  tft.print("BPM");

  tft.setTextColor(0x07FF); // ciano
  tft.setCursor(134, 50);
  tft.print("SpO2");

  // Linha divisória vertical (BPM | SpO2 e PI | Qualidade) → y=42 a y=112
  tft.fillRect(119, 42, 2, 70, 0x2104);

  // Linha horizontal separando BPM/SpO2 de PI/Qualidade
  tft.fillRect(0, 112, 240, 2, 0x2104);

  // Rótulos PI e QUALIDADE
  tft.setTextColor(0xFD20); // laranja
  tft.setTextSize(1);
  tft.setCursor(10, 116);
  tft.print("PI");

  tft.setTextColor(0x9FF3); // verde-azulado claro
  tft.setCursor(143, 116);
  tft.print("HRV");

  // Linha horizontal separando PI/Qualidade de Status
  tft.fillRect(0, 143, 240, 2, 0x2104);

  // Rótulo STATUS
  tft.setTextColor(0x8410);
  tft.setCursor(8, 147);
  tft.print("STATUS");

  // Linha horizontal separando Status de ECG
  tft.fillRect(0, 194, 240, 2, 0x2104);

  // Rótulo PPG
  tft.setTextColor(0x8410);
  tft.setCursor(8, 198);
  tft.print("PPG");

  // Borda área ECG
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
      tft.setTextColor(0xF800); // vermelho
      tft.setTextSize(5);
      int bx = (bpmReal < 100) ? 22 : 8;
      tft.setCursor(bx, 62);
      tft.print(bpmReal);
    } else {
      tft.setTextColor(0x4208); // cinza escuro
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
      tft.setTextColor(0x07FF); // ciano
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

  // ── PI e Qualidade do sinal ────────────────────────────────────────────
  if (mudouPI) {
    tft.fillRect(6, 124, 108, 20, 0x0000);   // limpa área PI
    tft.fillRect(122, 124, 118, 20, 0x0000); // limpa área Qualidade
    if (dedoDetectado) {
      // Valor PI
      tft.setTextColor(0xFD20); // laranja
      tft.setTextSize(2);
      char piBuf[8];
      dtostrf(piVal, 4, 1, piBuf);
      tft.setCursor(10, 126);
      tft.print(piBuf);
      tft.setTextSize(1);
      tft.setCursor(66, 132);
      tft.print("%");
      // HRV valor + cor por faixa
      tft.setTextSize(2);
      if (hrvRMSSD >= 50) {
        tft.setTextColor(0x07E0); // verde = relaxado
      } else if (hrvRMSSD >= 20) {
        tft.setTextColor(0xFFE0); // amarelo = normal
      } else {
        tft.setTextColor(0xF800); // vermelho = estresse
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
      tft.fillRoundRect(6, 156, 228, 35, 5, 0x2000); // vermelho escuro
      tft.setTextColor(0xF800);
      tft.setTextSize(2);
      tft.setCursor(52, 165);
      tft.print("SEM SENSOR");
    } else if (qualSinal == 0) {
      tft.fillRoundRect(6, 156, 228, 35, 5, 0x2200); // amarelo escuro
      tft.setTextColor(0xFFE0);
      tft.setTextSize(2);
      tft.setCursor(42, 165);
      tft.print("SEM LEITURA");
    } else if (qualSinal == 1) {
      tft.fillRoundRect(6, 156, 228, 35, 5, 0x4200); // laranja escuro
      tft.setTextColor(0xFD20);
      tft.setTextSize(2);
      tft.setCursor(46, 165);
      tft.print("SINAL FRACO");
    } else {
      tft.fillRoundRect(6, 156, 228, 35, 5, 0x0240); // verde escuro
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
  // Gera amostra ECG baseada na variação do IR
  long delta = irValue - prevIR;
  prevIR = irValue;

  // Normaliza delta para amplitude da área ECG (±20px)
  int amostra = constrain((int)(delta / 500), -18, 18);
  ecgBuf[ecgPos] = amostra;

  // Redesenha apenas a coluna atual e apaga a próxima (efeito scroll)
  int x = ecgPos + 10; // margem interna
  int midY = ECG_Y + ECG_H / 2;

  // Apaga coluna atual e próxima
  tft.fillRect(x, ECG_Y, 2, ECG_H, 0x0000);
  tft.fillRect((x + 4) % ECG_W + 10, ECG_Y, 3, ECG_H, 0x0000);

  // Desenha ponto ECG
  int y1 = midY - ecgBuf[(ecgPos + ECG_W - 1) % ECG_W];
  int y2 = midY - amostra;
  y1 = constrain(y1, ECG_Y + 2, ECG_Y + ECG_H - 3);
  y2 = constrain(y2, ECG_Y + 2, ECG_Y + ECG_H - 3);

  uint16_t cor = dedoDetectado ? 0x07E0 : 0x2104; // verde se dedo, cinza se não
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