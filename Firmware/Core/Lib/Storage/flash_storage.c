#include "flash_storage.h"

void Flash_Save(SystemData *data) {
    HAL_FLASH_Unlock();
    FLASH_EraseInitTypeDef Erase;
    Erase.TypeErase = FLASH_TYPEERASE_PAGES;
    Erase.PageAddress = FLASH_ADDR;
    Erase.NbPages = 1;
    uint32_t PageError;
    HAL_FLASHEx_Erase(&Erase, &PageError);

    uint32_t *ptr = (uint32_t *)data;
    for (int i = 0; i < sizeof(SystemData); i += 4) {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_ADDR + i, *ptr++);
    }
    HAL_FLASH_Lock();
}

void Flash_Load(SystemData *data) {
    SystemData *flash = (SystemData *)FLASH_ADDR;
    if (flash->magic == 0xDEADBEEF) {
        memcpy(data, flash, sizeof(SystemData));
    } else {
        strcpy(data->password, "12345");
        data->card_count = 0;
        data->magic = 0xDEADBEEF;
        Flash_Save(data);
    }
}
