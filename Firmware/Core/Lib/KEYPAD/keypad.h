#ifndef INC_KEYPAD_H_
#define INC_KEYPAD_H_

#include "stm32f1xx_hal.h"

// Định nghĩa chân (Giữ nguyên như bạn đã làm)
#define R1_PORT GPIOB
#define R1_PIN  GPIO_PIN_12
#define R2_PORT GPIOB
#define R2_PIN  GPIO_PIN_13
#define R3_PORT GPIOB
#define R3_PIN  GPIO_PIN_14
#define R4_PORT GPIOB
#define R4_PIN  GPIO_PIN_15

#define C1_PORT GPIOA
#define C1_PIN  GPIO_PIN_8
#define C2_PORT GPIOA
#define C2_PIN  GPIO_PIN_9
#define C3_PORT GPIOA
#define C3_PIN  GPIO_PIN_10
#define C4_PORT GPIOA
#define C4_PIN  GPIO_PIN_11

// Sửa tên hàm này cho khớp với main.c
char Keypad_GetKey(void);

#endif
