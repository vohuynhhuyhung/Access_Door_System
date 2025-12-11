#include "i2c-lcd.h"

extern I2C_HandleTypeDef hi2c2;
#define SLAVE_ADDRESS_LCD 0x4E // 0x27 << 1

// Hàm nội bộ (static) để gửi lệnh/dữ liệu
static void LCD_SendCmd(char cmd) {
  char data_u, data_l;
  uint8_t data_t[4];
  data_u = (cmd&0xf0);
  data_l = ((cmd<<4)&0xf0);
  data_t[0] = data_u|0x0C;  //en=1, rs=0
  data_t[1] = data_u|0x08;  //en=0, rs=0
  data_t[2] = data_l|0x0C;  //en=1, rs=0
  data_t[3] = data_l|0x08;  //en=0, rs=0
  HAL_I2C_Master_Transmit(&hi2c2, SLAVE_ADDRESS_LCD,(uint8_t *) data_t, 4, 100);
}

static void LCD_SendData(char data) {
  char data_u, data_l;
  uint8_t data_t[4];
  data_u = (data&0xf0);
  data_l = ((data<<4)&0xf0);
  data_t[0] = data_u|0x0D;  //en=1, rs=1
  data_t[1] = data_u|0x09;  //en=0, rs=1
  data_t[2] = data_l|0x0D;  //en=1, rs=1
  data_t[3] = data_l|0x09;  //en=0, rs=1
  HAL_I2C_Master_Transmit(&hi2c2, SLAVE_ADDRESS_LCD,(uint8_t *) data_t, 4, 100);
}

// --- CÁC HÀM API CHÍNH (Đã sửa tên cho khớp main.c) ---

void LCD_Init(I2C_HandleTypeDef *hi2c) {
    // (Lưu ý: trong code cũ bạn dùng biến toàn cục hi2c2,
    // nhưng để chuẩn hàm Init nên nhận tham số, dù ở đây ta hardcode hi2c2 cũng được)
    HAL_Delay(50);
    LCD_SendCmd(0x30); HAL_Delay(5);
    LCD_SendCmd(0x30); HAL_Delay(1);
    LCD_SendCmd(0x30); HAL_Delay(10);
    LCD_SendCmd(0x20); HAL_Delay(10);
    LCD_SendCmd(0x28); HAL_Delay(1);
    LCD_SendCmd(0x08); HAL_Delay(1);
    LCD_SendCmd(0x01); HAL_Delay(1);
    LCD_SendCmd(0x06); HAL_Delay(1);
    LCD_SendCmd(0x0C);
}

void LCD_Clear(void) {
    LCD_SendCmd(0x80);
    for (int i=0; i<70; i++) LCD_SendData(' ');
}

void LCD_SetCursor(uint8_t row, uint8_t col) {
    switch (row) {
        case 0: col |= 0x80; break;
        case 1: col |= 0xC0; break;
    }
    LCD_SendCmd(col);
}

void LCD_WriteString(char *str) {
    while (*str) LCD_SendData(*str++);
}

void LCD_WriteChar(char c) {
    LCD_SendData(c);
}
