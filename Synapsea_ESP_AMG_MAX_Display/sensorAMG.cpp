#include "sensorAMG.h"

void iniciarAMG8833() {
  if (!amg.begin(0x68)) {
    Serial.println("Sensor AMG8833 nao encontrado no endereco 0x68");
    tft.setCursor(10, 10);
    tft.print("Sensor NAO OK");
    while (1);
  } else {
    Serial.println("Sensor AMG8833 encontrado no endereco 0x68");
  }
}

void lerAMG8833() {
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
}

// ─── Funções de interpolação e utilitários ────────────────────────────────

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

static float cubicInterpolate(float p[], float x) {
  return p[1] + 0.5 * x * (p[2] - p[0] + x * (2.0 * p[0] - 5.0 * p[1] + 4.0 * p[2] - p[3] + x * (3.0 * (p[1] - p[2]) + p[3] - p[0])));
}

static float bicubicInterpolate(float p[], float x, float y) {
  float arr[4] = {
    cubicInterpolate(p + 0,  x),
    cubicInterpolate(p + 4,  x),
    cubicInterpolate(p + 8,  x),
    cubicInterpolate(p + 12, x)
  };
  return cubicInterpolate(arr, y);
}

static void get_adjacents_2d(float *src, float *dest, uint8_t rows, uint8_t cols, int8_t x, int8_t y) {
  for (int8_t dy = -1; dy < 3; dy++) {
    for (int8_t dx = -1; dx < 3; dx++) {
      dest[(dy + 1) * 4 + (dx + 1)] = get_point(src, rows, cols, x + dx, y + dy);
    }
  }
}

void interpolate_image(float *src, uint8_t src_rows, uint8_t src_cols,
                       float *dest, uint8_t dest_rows, uint8_t dest_cols) {
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
      float c00 = get_point(src, src_rows, src_cols, gxi,     gyi);
      float c10 = get_point(src, src_rows, src_cols, gxi + 1, gyi);
      float c01 = get_point(src, src_rows, src_cols, gxi,     gyi + 1);
      float c11 = get_point(src, src_rows, src_cols, gxi + 1, gyi + 1);
      float tx  = gx - gxi;
      float ty  = gy - gyi;
      float val = (1 - tx) * (1 - ty) * c00 + tx * (1 - ty) * c10
                + (1 - tx) * ty * c01 + tx * ty * c11;
      set_point(dest, dest_rows, dest_cols, x, y, val);
    }
  }
}

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
