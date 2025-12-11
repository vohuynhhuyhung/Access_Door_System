#ifndef INC_I2C_LCD_H_
#define INC_I2C_LCD_H_

#include "stm32f1xx_hal.h"

void LCD_Init(I2C_HandleTypeDef *hi2c);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_WriteString(char *str);
void LCD_WriteChar(char c); // Thêm hàm này vì main.c có dùng

#endif
