#include "dfplayer_mini.h"

#define DF_START_BYTE 0x7E
#define DF_VERSION_BYTE 0xFF
#define DF_COMMAND_LENGTH 0x06
#define DF_ACKNOWLEDGE 0x00
#define DF_END_BYTE 0xEF

static UART_HandleTypeDef *df_uart;

void send_command(uint8_t cmd, uint16_t arg) {
    uint8_t command_buffer[10];
    command_buffer[0] = DF_START_BYTE;
    command_buffer[1] = DF_VERSION_BYTE;
    command_buffer[2] = DF_COMMAND_LENGTH;
    command_buffer[3] = cmd;
    command_buffer[4] = DF_ACKNOWLEDGE;
    command_buffer[5] = (uint8_t)(arg >> 8);
    command_buffer[6] = (uint8_t)(arg);

    uint16_t checksum = 0;
    for (int i = 1; i < 7; i++) {
        checksum += command_buffer[i];
    }
    checksum = -checksum;

    command_buffer[7] = (uint8_t)(checksum >> 8);
    command_buffer[8] = (uint8_t)(checksum);
    command_buffer[9] = DF_END_BYTE;

    HAL_UART_Transmit(df_uart, command_buffer, 10, 100);
}

void DF_Init(UART_HandleTypeDef *huart) {
    df_uart = huart;
}

void DF_Play(uint16_t track) {
    send_command(0x03, track);
}

void DF_SetVolume(uint8_t volume) {
    if (volume > 30) volume = 30;
    send_command(0x06, volume);
}

