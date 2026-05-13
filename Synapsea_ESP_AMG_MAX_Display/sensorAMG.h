#pragma once
#include "config.h"

void iniciarAMG8833();
void lerAMG8833();

float get_point(float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y);
void  set_point(float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y, float f);
void  interpolate_image(float *src, uint8_t src_rows, uint8_t src_cols,
                        float *dest, uint8_t dest_rows, uint8_t dest_cols);
void  interpolate_linear_image(float *src, uint8_t src_rows, uint8_t src_cols,
                               float *dest, uint8_t dest_rows, uint8_t dest_cols);
void  smooth3x3(float *arr, uint8_t R, uint8_t C);
