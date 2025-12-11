#ifndef INC_FLASH_STORAGE_H_
#define INC_FLASH_STORAGE_H_

#include "stm32f1xx_hal.h"
#include <string.h>

#define FLASH_ADDR 0x0800FC00 // Page cuối STM32F103C8T6
#define MAX_CARDS 200

typedef struct {
    char password[6];
    uint8_t card_count;
    uint8_t uids[MAX_CARDS][4];
    uint32_t magic;
} SystemData;

void Flash_Load(SystemData *data);
void Flash_Save(SystemData *data);

#endif
