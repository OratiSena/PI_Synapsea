#pragma once
#include "config.h"

void colorbar();
void drawpixels(float *p, uint8_t rows, uint8_t cols, uint8_t boxW, uint8_t boxH, boolean showVal);
void desenharCabecalhoMAX();
void desenharValoresMAX(bool forcar);
void atualizarECG();
void desenharTelaMAX();
void verificarBotaoTrocaTela();
