#include "rc522.h"

extern SPI_HandleTypeDef hspi1;

// Gửi/Nhận 1 byte qua SPI
uint8_t SPI1_Transfer(uint8_t data) {
    uint8_t rx_data;
    HAL_SPI_TransmitReceive(RC522_SPI_HANDLE, &data, &rx_data, 1, 100);
    return rx_data;
}

// Ghi vào thanh ghi RC522
void Write_MFRC522(uint8_t addr, uint8_t val) {
    HAL_GPIO_WritePin(RC522_CS_GPIO, RC522_CS_PIN, GPIO_PIN_RESET); // CS Low
    SPI1_Transfer((addr << 1) & 0x7E); // Địa chỉ ghi
    SPI1_Transfer(val);
    HAL_GPIO_WritePin(RC522_CS_GPIO, RC522_CS_PIN, GPIO_PIN_SET);   // CS High
}

// Đọc từ thanh ghi RC522
uint8_t Read_MFRC522(uint8_t addr) {
    uint8_t val;
    HAL_GPIO_WritePin(RC522_CS_GPIO, RC522_CS_PIN, GPIO_PIN_RESET); // CS Low
    SPI1_Transfer(((addr << 1) & 0x7E) | 0x80); // Địa chỉ đọc
    val = SPI1_Transfer(0x00); // Gửi dummy để nhận dữ liệu
    HAL_GPIO_WritePin(RC522_CS_GPIO, RC522_CS_PIN, GPIO_PIN_SET);   // CS High
    return val;
}

// Bật bit trong thanh ghi
void SetBitMask(uint8_t reg, uint8_t mask) {
    uint8_t tmp = Read_MFRC522(reg);
    Write_MFRC522(reg, tmp | mask);
}

// Tắt bit trong thanh ghi
void ClearBitMask(uint8_t reg, uint8_t mask) {
    uint8_t tmp = Read_MFRC522(reg);
    Write_MFRC522(reg, tmp & (~mask));
}

void AntennaOn(void) {
    uint8_t temp = Read_MFRC522(TxControlReg);
    if (!(temp & 0x03)) SetBitMask(TxControlReg, 0x03);
}

void MFRC522_Reset(void) {
    Write_MFRC522(CommandReg, PCD_RESETPHASE);
}

void MFRC522_Init(void) {
    HAL_GPIO_WritePin(RC522_CS_GPIO, RC522_CS_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(RC522_RST_GPIO, RC522_RST_PIN, GPIO_PIN_SET);

    MFRC522_Reset();

    Write_MFRC522(TModeReg, 0x8D);
    Write_MFRC522(TPrescalerReg, 0x3E);
    Write_MFRC522(TReloadRegL, 30);
    Write_MFRC522(TReloadRegH, 0);
    Write_MFRC522(TxAutoReg, 0x40);
    Write_MFRC522(ModeReg, 0x3D);

    AntennaOn();
}

uint8_t MFRC522_ToCard(uint8_t command, uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint16_t *backLen) {
    uint8_t status = MI_ERR;
    uint8_t irqEn = 0x00;
    uint8_t waitIRq = 0x00;
    uint8_t lastBits;
    uint8_t n;
    uint16_t i;

    switch (command) {
        case PCD_AUTHENT:
            irqEn = 0x12; waitIRq = 0x10; break;
        case PCD_TRANSCEIVE:
            irqEn = 0x77; waitIRq = 0x30; break;
        default: break;
    }

    Write_MFRC522(CommIEnReg, irqEn | 0x80);
    ClearBitMask(CommIrqReg, 0x80);
    SetBitMask(FIFOLevelReg, 0x80);
    Write_MFRC522(CommandReg, PCD_IDLE);

    for (i = 0; i < sendLen; i++) Write_MFRC522(FIFODataReg, sendData[i]);

    Write_MFRC522(CommandReg, command);
    if (command == PCD_TRANSCEIVE) SetBitMask(BitFramingReg, 0x80);

    i = 2000;
    do {
        n = Read_MFRC522(CommIrqReg);
        i--;
    } while ((i != 0) && !(n & 0x01) && !(n & waitIRq));

    ClearBitMask(BitFramingReg, 0x80);

    if (i != 0) {
        if (!(Read_MFRC522(ErrorReg) & 0x1B)) {
            status = MI_OK;
            if (n & irqEn & 0x01) status = MI_NOTAGERR;

            if (command == PCD_TRANSCEIVE) {
                n = Read_MFRC522(FIFOLevelReg);
                lastBits = Read_MFRC522(ControlReg) & 0x07;
                if (lastBits) *backLen = (n - 1) * 8 + lastBits;
                else *backLen = n * 8;

                if (n == 0) n = 1;
                if (n > MFRC522_MAX_LEN) n = MFRC522_MAX_LEN;

                for (i = 0; i < n; i++) backData[i] = Read_MFRC522(FIFODataReg);
            }
        } else status = MI_ERR;
    }
    return status;
}

uint8_t MFRC522_Request(uint8_t reqMode, uint8_t *TagType) {
    uint8_t status;
    uint16_t backBits;
    Write_MFRC522(BitFramingReg, 0x07);
    TagType[0] = reqMode;
    status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);
    if ((status != MI_OK) || (backBits != 0x10)) status = MI_ERR;
    return status;
}

uint8_t MFRC522_Anticoll(uint8_t *serNum) {
    uint8_t status;
    uint8_t i;
    uint8_t serNumCheck = 0;
    uint16_t unLen;

    Write_MFRC522(BitFramingReg, 0x00);
    serNum[0] = PICC_ANTICOLL;
    serNum[1] = 0x20;
    status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

    if (status == MI_OK) {
        for (i = 0; i < 4; i++) serNumCheck ^= serNum[i];
        if (serNumCheck != serNum[4]) status = MI_ERR;
    }
    return status;
}

void CalulateCRC(uint8_t *pIndata, uint8_t len, uint8_t *pOutData) {
    uint8_t i, n;
    ClearBitMask(DivIrqReg, 0x04);
    SetBitMask(FIFOLevelReg, 0x80);
    for (i = 0; i < len; i++) Write_MFRC522(FIFODataReg, *(pIndata + i));
    Write_MFRC522(CommandReg, PCD_CALCCRC);
    i = 0xFF;
    do {
        n = Read_MFRC522(DivIrqReg);
        i--;
    } while ((i != 0) && !(n & 0x04));
    pOutData[0] = Read_MFRC522(CRCResultRegL);
    pOutData[1] = Read_MFRC522(CRCResultRegH);
}

void MFRC522_Halt(void) {
    uint16_t unLen;
    uint8_t buff[4];
    buff[0] = PICC_HALT;
    buff[1] = 0;
    CalulateCRC(buff, 2, &buff[2]);
    MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &unLen);
}
