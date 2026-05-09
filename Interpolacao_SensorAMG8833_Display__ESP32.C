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
MAX30105 particleSensor;

#define BUFFER_LENGTH 100
uint32_t irBuffer[BUFFER_LENGTH];
uint32_t redBuffer[BUFFER_LENGTH];
int32_t  spo2_val         = 0;
int8_t   spo2Valid        = 0;
int32_t  bpm_val          = 0;
int8_t   bpmValid         = 0;
bool     maxOK            = false;
bool     dedoDetectado    = false;
bool     bufferPreenchido = false;
int      bufIdx           = 0;
int      prevBPM_disp     = -1;
int      prevSpO2_disp    = -1;
String   prevStatus_disp  = "";

// ─── Botão e controle de tela ──────────────────────────────────────────────
#define BTN_PIN 27
int  telaAtual                              = 0;
bool btnAnterior                            = HIGH;
unsigned long ultimoDebounce                = 0;
const unsigned long debounceDelay           = 300;
unsigned long ultimaAtualizacaoMAX          = 0;
const unsigned long intervaloAtualizacaoMAX = 1000;







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



  // /* BANCO DE DADOS - DESATIVADO TEMPORARIAMENTE */
  // WiFi.begin(ssid, password);
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(1000);
  //   Serial.println("Conectando ao WiFi...");
  // }
  // Serial.println("WiFi conectado.");
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

  //smooth3x3(dest_2d, INTERPOLATED_ROWS, INTERPOLATED_COLS); /* DESATIVAR CASO: tiver pouca nitidez */


  uint16_t boxSize = min(tft.width() / INTERPOLATED_COLS, 240 / INTERPOLATED_ROWS);
  drawpixels(dest_2d, INTERPOLATED_ROWS, INTERPOLATED_COLS, boxSize, boxSize, false);

  tft.fillRect(0, 250, 240, 30, 0x0000);  // limpa fundo
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



  // /* BANCO DE DADOS - DESATIVADO TEMPORARIAMENTE */
  // float soma = 0;
  // float minTemp = 1000;
  // float maxTemp = -1000;
  // for (int i = 0; i < INTERPOLATED_ROWS * INTERPOLATED_COLS; i++) {
  //   float val = dest_2d[i];
  //   soma += val;
  //   if (val < minTemp) minTemp = val;
  //   if (val > maxTemp) maxTemp = val;
  // }
  // float mediaTemp = soma / (INTERPOLATED_ROWS * INTERPOLATED_COLS);
  // if (millis() - ultimoEnvio >= intervaloEnvio) {
  //   enviarDadosParaServidor(maxTemp, dest_2d, INTERPOLATED_ROWS, INTERPOLATED_COLS);
  //   ultimoEnvio = millis();
  // }
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
// void enviarDadosParaServidor(float maxTemp, float *matriz, int linhas, int colunas) {
//   if (WiFi.status() == WL_CONNECTED) {
//     HTTPClient http;
//     http.begin("https://thermotrack.ganhemaishoje.com.br/inserirdados.php");
//     http.addHeader("Content-Type", "application/json");
//     String json = "{\"dispositivo_id\":1";
//     json += ",\"temperatura_max\":" + String(maxTemp, 2);
//     json += ",\"imagem_base64\":\"data:image/png;base64," + imagem_base64_fake + "\"}";
//     int httpResponseCode = http.POST(json);
//     Serial.print("Código resposta: ");
//     Serial.println(httpResponseCode);
//     Serial.println("Resposta: " + http.getString());
//     http.end();
//   }
// }