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

// ─── ECG fake ─────────────────────────────────────────────────────────────
#define ECG_W   220
#define ECG_Y   255
#define ECG_H    40
int  ecgBuf[ECG_W]   = {0};
int  ecgPos          = 0;
long prevIR          = 0;

// ─── Cache de display para evitar redesenho desnecessário ─────────────────
int   prevBPM_d      = -999;
int   prevSpo2_d     = -999;
bool  prevDedo_d     = false;
bool  prevMaxOK_d    = false;

// ─── Botão e controle de tela ──────────────────────────────────────────────
#define BTN_PIN 27
int  telaAtual                              = 0;
bool btnAnterior                            = HIGH;
unsigned long ultimoDebounce                = 0;
const unsigned long debounceDelay           = 300;
unsigned long ultimaAtualizacaoMAX          = 0;
const unsigned long intervaloAtualizacaoMAX = 300;







// Declarações antecipadas
void verificarBotaoTrocaTela();
void iniciarMAX30102();
void scannerI2CMAX();
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
    if (millis() - ultimaAtualizacaoMAX >= intervaloAtualizacaoMAX) {
      lerMAX30102();
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
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x1F);
  particleSensor.setPulseAmplitudeIR(0x1F);
  particleSensor.setPulseAmplitudeGreen(0);
  maxOK = true;
  Serial.println("MAX30102 encontrado e inicializado em GPIO25/GPIO26");
}

// ─── MAX30102: leitura raw de IR e RED (modo depuração) ──────────────────
void lerMAX30102() {
  if (!maxOK) return;

  particleSensor.check(); // atualiza buffer interno sem bloquear
  if (particleSensor.available()) {
    irValue  = particleSensor.getIR();
    redValue = particleSensor.getRed();
    particleSensor.nextSample();
  }

  // Limiar baixo para validar detecção: IR > 10000
  dedoDetectado = (irValue > 10000);

  Serial.print("IR: "); Serial.print(irValue);
  Serial.print("  RED: "); Serial.print(redValue);
  Serial.print("  Status: ");
  Serial.println(dedoDetectado ? "Dedo detectado" : "Aguardando dedo");
}

// ─── MAX30102: desenhar tela estilo monitor médico ────────────────────────

// Desenha o cabeçalho fixo (chamado apenas uma vez ao entrar na tela)
void desenharCabecalhoMAX() {
  tft.fillScreen(0x0000);

  // Fundo do cabeçalho
  tft.fillRect(0, 0, 240, 38, 0x0841); // cinza escuro quase preto

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

  // Linha divisória vertical central
  tft.fillRect(119, 42, 2, 110, 0x2104);

  // Rótulo STATUS
  tft.setTextColor(0x8410);
  tft.setTextSize(1);
  tft.setCursor(8, 165);
  tft.print("STATUS");

  // Rótulo ECG
  tft.setTextColor(0x8410);
  tft.setCursor(8, 213);
  tft.print("ECG");

  // Borda ECG
  tft.drawRect(0, 222, 240, 58, 0x2104);
}

void desenharValoresMAX(bool forcar) {
  // ── Status / dedo ──────────────────────────────────────────────────────
  bool mudouEstado = (dedoDetectado != prevDedo_d) || (maxOK != prevMaxOK_d) || forcar;

  if (mudouEstado) {
    tft.fillRect(0, 172, 240, 38, 0x0000);

    if (!maxOK) {
      // Fundo vermelho escuro
      tft.fillRoundRect(6, 173, 228, 33, 5, 0x2000);
      tft.setTextColor(0xF800);
      tft.setTextSize(2);
      tft.setCursor(52, 181);
      tft.print("SEM SENSOR");
    } else if (!dedoDetectado) {
      // Fundo amarelo escuro
      tft.fillRoundRect(6, 173, 228, 33, 5, 0x2200);
      tft.setTextColor(0xFFE0);
      tft.setTextSize(2);
      tft.setCursor(42, 181);
      tft.print("SEM LEITURA");
    } else {
      // Fundo verde escuro
      tft.fillRoundRect(6, 173, 228, 33, 5, 0x0240);
      tft.setTextColor(0x07E0);
      tft.setTextSize(2);
      tft.setCursor(76, 181);
      tft.print("NORMAL");
    }
    prevDedo_d  = dedoDetectado;
    prevMaxOK_d = maxOK;
  }

  // ── BPM ────────────────────────────────────────────────────────────────
  // Lê BPM das variáveis globais (placeholder por enquanto)
  int bpmDisp = dedoDetectado ? (int)((irValue / 1000) % 60 + 60) : 0; // placeholder visual
  // (será substituído por bpm_val real na próxima etapa)

  if (bpmDisp != prevBPM_d || forcar) {
    tft.fillRect(6, 58, 108, 56, 0x0000);
    if (dedoDetectado) {
      tft.setTextColor(0xF800);
      tft.setTextSize(1);
      tft.setCursor(14, 60);
      // Ícone coração simplificado
      tft.print("\3"); // char coração não suportado: usamos número direto
      tft.setTextColor(0xF800);
      tft.setTextSize(5);
      // Centraliza número de 2-3 dígitos
      int bx = (bpmDisp < 100) ? 22 : 8;
      tft.setCursor(bx, 68);
      tft.print(bpmDisp);
    } else {
      tft.setTextColor(0x4208);
      tft.setTextSize(4);
      tft.setCursor(22, 72);
      tft.print("--");
    }
    prevBPM_d = bpmDisp;
  }

  // ── SpO2 ───────────────────────────────────────────────────────────────
  int spo2Disp = dedoDetectado ? 98 : 0; // placeholder visual

  if (spo2Disp != prevSpo2_d || forcar) {
    tft.fillRect(122, 58, 118, 56, 0x0000);
    if (dedoDetectado) {
      tft.setTextColor(0x07FF);
      tft.setTextSize(4);
      tft.setCursor(128, 68);
      tft.print(spo2Disp);
      tft.setTextSize(2);
      tft.setCursor(196, 78);
      tft.print("%");
    } else {
      tft.setTextColor(0x4208);
      tft.setTextSize(4);
      tft.setCursor(140, 72);
      tft.print("--");
    }
    prevSpo2_d = spo2Disp;
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
  }

  desenharValoresMAX(false);
  atualizarECG();
}