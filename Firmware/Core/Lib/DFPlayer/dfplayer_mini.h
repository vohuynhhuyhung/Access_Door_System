#ifndef DFPLAYER_MINI_H
#define DFPLAYER_MINI_H

#include "stm32f1xx_hal.h"

void DF_Init(UART_HandleTypeDef *huart);
void DF_Play(uint16_t track);
void DF_SetVolume(uint8_t volume);

#endif // DFPLAYER_MINI_H
