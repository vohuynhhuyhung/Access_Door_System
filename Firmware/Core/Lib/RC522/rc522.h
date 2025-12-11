#ifndef INC_RC522_H_
#define INC_RC522_H_

#include "stm32f1xx_hal.h"

// --- CẤU HÌNH CHÂN PHẦN CỨNG (Sửa tại đây nếu đổi chân) ---
#define RC522_SPI_HANDLE    &hspi1
#define RC522_CS_GPIO       GPIOA
#define RC522_CS_PIN        GPIO_PIN_4
#define RC522_RST_GPIO      GPIOB
#define RC522_RST_PIN       GPIO_PIN_0

// --- RC522 COMMANDS ---
#define PCD_IDLE            0x00
#define PCD_AUTHENT         0x0E
#define PCD_RECEIVE         0x08
#define PCD_TRANSMIT        0x04
#define PCD_TRANSCEIVE      0x0C
#define PCD_RESETPHASE      0x0F
#define PCD_CALCCRC         0x03

// --- MIFARE COMMANDS ---
#define PICC_REQIDL         0x26
#define PICC_REQALL         0x52
#define PICC_ANTICOLL       0x93
#define PICC_SElECTTAG      0x93
#define PICC_AUTHENT1A      0x60
#define PICC_AUTHENT1B      0x61
#define PICC_READ           0x30
#define PICC_WRITE          0xA0
#define PICC_HALT           0x50

// --- STATUS CODES ---
#define MI_OK               0
#define MI_NOTAGERR         1
#define MI_ERR              2

// --- REGISTERS ---
#define CommandReg          0x01
#define CommIEnReg          0x02
#define DivlEnReg           0x03
#define CommIrqReg          0x04
#define DivIrqReg           0x05
#define ErrorReg            0x06
#define Status1Reg          0x07
#define Status2Reg          0x08
#define FIFODataReg         0x09
#define FIFOLevelReg        0x0A
#define WaterLevelReg       0x0B
#define ControlReg          0x0C
#define BitFramingReg       0x0D
#define CollReg             0x0E
#define ModeReg             0x11
#define TxModeReg           0x12
#define RxModeReg           0x13
#define TxControlReg        0x14
#define TxAutoReg           0x15
#define CRCResultRegH       0x21
#define CRCResultRegL       0x22
#define TModeReg            0x2A
#define TPrescalerReg       0x2B
#define TReloadRegH         0x2C
#define TReloadRegL         0x2D
#define VersionReg          0x37

#define MFRC522_MAX_LEN     16

// --- FUNCTION PROTOTYPES ---
void MFRC522_Init(void);
uint8_t MFRC522_Check(uint8_t *id);
uint8_t MFRC522_Request(uint8_t reqMode, uint8_t *TagType);
uint8_t MFRC522_Anticoll(uint8_t *serNum);
uint8_t MFRC522_SelectTag(uint8_t *serNum);
uint8_t MFRC522_Auth(uint8_t authMode, uint8_t BlockAddr, uint8_t *Sectorkey, uint8_t *serNum);
uint8_t MFRC522_Read(uint8_t blockAddr, uint8_t *recvData);
uint8_t MFRC522_Write(uint8_t blockAddr, uint8_t *writeData);
void MFRC522_Halt(void);

#endif /* INC_RC522_H_ */
