/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

//Khác
#include <stdio.h>

//Thư viện ngoại vi
#include "..\Lib\Storage\flash_storage.h"
#include "..\Lib\RC522\rc522.h"
#include "../Lib/DFPlayer/dfplayer_mini.h"
#include "../Lib/KEYPAD/keypad.h"
#include "../Lib/LCD I2C/i2c-lcd.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define RELAY_PIN GPIO_PIN_1
#define RELAY_PORT GPIOB
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
// System Data
SystemData sysData;
uint8_t ADMIN_UID[4] = {51, 194, 201, 27}; // <-- THAY UID ADMIN VÀO ĐÂY
uint8_t str[MFRC522_MAX_LEN];
uint8_t sNum[4];

// Logic Variables
typedef enum {
    STATE_IDLE, STATE_INPUT_PASS, STATE_ACCESS_GRANTED, STATE_ACCESS_DENIED,
    STATE_LOCKED, STATE_ADMIN_MENU, STATE_ADMIN_SET_PASS_1, STATE_ADMIN_SET_PASS_2,
    STATE_ADMIN_EDIT_CARD, STATE_ADMIN_VIEW_ALL
} State;

State currState = STATE_IDLE;
char pass_buf[6];
char temp_pass[6];
uint8_t pass_idx = 0;
uint8_t wrong_cnt = 0;
uint32_t timer_relay = 0;
uint32_t timer_lock = 0;
uint32_t lock_duration = 0;
int view_scroll = 0;

// Biến xử lý phím (Quan trọng cho việc chống rung/lặp)
volatile char key_input = 0;     // Phím đã được xử lý (sạch)
char last_key_state = 0;         // Trạng thái phím lần quét trước
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void Show_Idle() {
    LCD_Clear();
    LCD_SetCursor(0, 0); LCD_WriteString("Nhap Mat Khau");
    LCD_SetCursor(1, 0); LCD_WriteString("Hoac Quet The");
}

int Find_Card(uint8_t *uid) {
    for (int i = 0; i < sysData.card_count; i++) {
        if (memcmp(sysData.uids[i], uid, 4) == 0) return i;
    }
    return -1;
}

void Handle_Access(uint8_t granted, uint8_t is_pass) {
    LCD_Clear();
    if (granted) {
        LCD_SetCursor(0, 0); LCD_WriteString("Xin Moi Vao");
        DF_Play(1);

        HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_SET);

        timer_relay = HAL_GetTick();
        wrong_cnt = 0;
        currState = STATE_ACCESS_GRANTED;
    } else {
        LCD_SetCursor(0, 0);
        LCD_WriteString(is_pass ? "Sai Mat Khau" : "The Sai");

        // 1. Phát âm thanh "Sai" trước
        DF_Play(2);

        wrong_cnt++;
        HAL_Delay(2000);
        if (wrong_cnt >= 5) {
            currState = STATE_LOCKED;
            timer_lock = HAL_GetTick();
            // Logic thời gian khóa
            if (wrong_cnt >= 9) lock_duration = 1200000; // 20p
            else if (wrong_cnt >= 7) lock_duration = 600000; // 10p
            else lock_duration = 300000; // 5p (300s)
//             2. Sau đó mới phát âm thanh "Hệ thống khóa"
            DF_Play(3);
        } else {
            // Nếu chưa bị khóa thì chờ 2s rồi quay lại màn hình chính
            HAL_Delay(2000);
            currState = STATE_IDLE;
            Show_Idle();
        }
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C2_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	MFRC522_Init();
	LCD_Init(&hi2c2);
	DF_Init(&huart1);
	Flash_Load(&sysData);
	DF_SetVolume(25);
	Show_Idle();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  // 1. LOGIC TIMER (Tắt Relay, Hết giờ khóa)
	        if (currState == STATE_ACCESS_GRANTED) {
	            if (HAL_GetTick() - timer_relay > 7000) { // 7 giây mở cửa
	                HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_RESET); // Relay OFF
	                currState = STATE_IDLE;
	                Show_Idle();
	            }
	        }

	        // 2. LOGIC ĐỌC PHÍM (CHỐNG LẶP/CHỐNG RUNG)
	        char raw_key = Keypad_GetKey();
	        key_input = 0; // Reset phím sạch

	        // Chỉ nhận phím khi trạng thái thay đổi và phím đó khác 0
	        if (raw_key != 0 && raw_key != last_key_state) {
	            key_input = raw_key; // Đây là phím hợp lệ để xử lý
	            // (Tùy chọn) Có thể thêm tiếng Bíp mỗi khi bấm phím tại đây
	        }
	        last_key_state = raw_key; // Cập nhật trạng thái cũ

	        // 3. MÁY TRẠNG THÁI (STATE MACHINE)
	        switch (currState) {
	            case STATE_IDLE:
	                // -- Xử lý Keypad --
	                if (key_input >= '0' && key_input <= '9') {
	                    currState = STATE_INPUT_PASS;
	                    pass_idx = 0;
	                    memset(pass_buf, 0, 6);
	                    pass_buf[pass_idx++] = key_input;
	                    LCD_Clear();
	                    LCD_SetCursor(0, 0); LCD_WriteString("Mat khau:");
	                    LCD_SetCursor(1, 0); LCD_WriteString("*");
	                }

	                // -- Xử lý RFID --
	                // (Đặt ngoài if(key) để luôn quét được thẻ)
	                if (MFRC522_Request(PICC_REQIDL, str) == MI_OK) {
	                    if (MFRC522_Anticoll(str) == MI_OK) {
	                        memcpy(sNum, str, 4);
	                        // Check Admin
	                        if (memcmp(sNum, ADMIN_UID, 4) == 0) {
	                            currState = STATE_ADMIN_MENU;
	                            LCD_Clear();
	                            LCD_SetCursor(0, 0); LCD_WriteString("1.Doi MK 2.The");
	                            LCD_SetCursor(1, 0); LCD_WriteString("9.Xem tat ca");
	                            DF_Play(4);
	                        }
	                        // Check User Card
	                        else if (Find_Card(sNum) != -1) {
	                            Handle_Access(1, 0);
	                        } else {
	                            Handle_Access(0, 0);
	                        }
	                        HAL_Delay(500); // Chống đọc thẻ liên tục quá nhanh
	                    }
	                }
	                break;

	            case STATE_INPUT_PASS:
	                if (key_input == 'A') { // Nút Xóa / Hủy
	                    if (pass_idx > 0) {
	                        pass_idx--;
	                        pass_buf[pass_idx] = 0;
	                        LCD_SetCursor(1, pass_idx); LCD_WriteString(" ");
	                    } else {
	                        currState = STATE_IDLE;
	                        Show_Idle();
	                    }
	                } else if (key_input == 'D') { // Nút Enter
	                    if (pass_idx == 5 || pass_idx >= 1) { // Cho phép mật khẩu ngắn hơn 5 nếu muốn
	                        if (strcmp(pass_buf, sysData.password) == 0) Handle_Access(1, 1);
	                        else Handle_Access(0, 1);
	                    }
	                } else if (key_input >= '0' && key_input <= '9' && pass_idx < 5) {
	                    pass_buf[pass_idx++] = key_input;
	                    LCD_SetCursor(1, pass_idx - 1); LCD_WriteString("*");
	                }
	                break;

	            case STATE_LOCKED: {
	                uint32_t elapsed = HAL_GetTick() - timer_lock;
	                if (elapsed >= lock_duration) {
	                    wrong_cnt = 0;
	                    currState = STATE_IDLE;
	                    Show_Idle();
	                } else {
	                    static uint32_t last_disp = 0;
	                    if (HAL_GetTick() - last_disp > 1000) {
	                        LCD_Clear();
	                        LCD_SetCursor(0, 0); LCD_WriteString("BAN BI KHOA!");
	                        char buf[16];
	                        sprintf(buf, "Con: %lu s", (lock_duration - elapsed) / 1000);
	                        LCD_SetCursor(1, 0); LCD_WriteString(buf);
	                        last_disp = HAL_GetTick();
	                    }
	                }
	                break;
	            }

	            case STATE_ADMIN_MENU:
	                if (key_input == '1') {
	                    currState = STATE_ADMIN_SET_PASS_1;
	                    pass_idx = 0; memset(pass_buf, 0, 6);
	                    LCD_Clear(); LCD_SetCursor(0,0); LCD_WriteString("Nhap MK Moi:");
	                } else if (key_input == '2') {
	                    currState = STATE_ADMIN_EDIT_CARD;
	                    LCD_Clear(); LCD_SetCursor(0,0); LCD_WriteString("Quet the de sua");
	                } else if (key_input == '9') {
	                    currState = STATE_ADMIN_VIEW_ALL;
	                    view_scroll = 0;
	                    LCD_Clear();
	                } else if (key_input == 'A') {
	                    currState = STATE_IDLE; Show_Idle();
	                }
	                break;

	            case STATE_ADMIN_SET_PASS_1:
	                if (key_input >= '0' && key_input <= '9' && pass_idx < 5) {
	                    pass_buf[pass_idx++] = key_input;
	                    LCD_SetCursor(1, pass_idx-1); LCD_WriteChar(key_input);
	                } else if (key_input == 'A') {
	                    currState = STATE_ADMIN_MENU;
	                    LCD_Clear(); LCD_SetCursor(0, 0); LCD_WriteString("1.Doi MK 2.The");
	                    LCD_SetCursor(1, 0); LCD_WriteString("9.Xem tat ca");
	                } else if (key_input == 'D' && pass_idx == 5) {
	                    strcpy(temp_pass, pass_buf);
	                    currState = STATE_ADMIN_SET_PASS_2;
	                    pass_idx = 0; memset(pass_buf, 0, 6);
	                    LCD_Clear(); LCD_SetCursor(0,0); LCD_WriteString("Nhap Lai:");
	                }
	                break;

	            case STATE_ADMIN_SET_PASS_2:
	                if (key_input >= '0' && key_input <= '9' && pass_idx < 5) {
	                    pass_buf[pass_idx++] = key_input;
	                    LCD_SetCursor(1, pass_idx-1); LCD_WriteChar(key_input);
	                } else if (key_input == 'D' && pass_idx == 5) {
	                    if (strcmp(pass_buf, temp_pass) == 0) {
	                        strcpy(sysData.password, pass_buf);
	                        Flash_Save(&sysData);
	                        LCD_Clear(); LCD_WriteString("Thanh Cong!"); HAL_Delay(1000);
	                        currState = STATE_IDLE; Show_Idle();
	                    } else {
	                        LCD_Clear(); LCD_WriteString("Khong Khop!"); HAL_Delay(1000);
	                        currState = STATE_ADMIN_SET_PASS_1;
	                        pass_idx = 0; memset(pass_buf, 0, 6);
	                        LCD_Clear(); LCD_SetCursor(0,0); LCD_WriteString("Nhap MK Moi:");
	                    }
	                }
	                break;

	            case STATE_ADMIN_EDIT_CARD:
	                if (key_input == 'A') { currState = STATE_ADMIN_MENU; break; }

	                // Logic RFID trong menu Edit
	                if (MFRC522_Request(PICC_REQIDL, str) == MI_OK) {
	                    if (MFRC522_Anticoll(str) == MI_OK) {
	                        memcpy(sNum, str, 4);
	                        int idx = Find_Card(sNum);
	                        char buf[16];
	                        sprintf(buf, "%02X%02X%02X%02X", sNum[0], sNum[1], sNum[2], sNum[3]);
	                        LCD_Clear(); LCD_SetCursor(0,0); LCD_WriteString(buf);

	                        if (idx != -1) { // Thẻ đã có -> Hỏi xóa
	                            LCD_SetCursor(1,0); LCD_WriteString("B:Xoa A:Thoat");
	                            while(1) {
	                                char k = Keypad_GetKey(); // Blocking tạm thời ở đây cũng được
	                                if (k=='B') {
	                                    // Xóa bằng cách ghi đè thẻ cuối lên vị trí này
	                                    memcpy(sysData.uids[idx], sysData.uids[sysData.card_count-1], 4);
	                                    sysData.card_count--;
	                                    Flash_Save(&sysData);
	                                    LCD_Clear(); LCD_WriteString("Da Xoa!"); HAL_Delay(1000);
	                                    currState = STATE_IDLE; Show_Idle(); break;
	                                } else if (k=='A') { currState = STATE_IDLE; Show_Idle(); break; }
	                            }
	                        } else { // Thẻ chưa có -> Hỏi thêm
	                            if (sysData.card_count >= MAX_CARDS) {
	                                LCD_SetCursor(1,0); LCD_WriteString("Bo Nho Day!");
	                            } else {
	                                LCD_SetCursor(1,0); LCD_WriteString("B:Them A:Thoat");
	                                while(1) {
	                                    char k = Keypad_GetKey();
	                                    if (k=='B') {
	                                        memcpy(sysData.uids[sysData.card_count], sNum, 4);
	                                        sysData.card_count++;
	                                        Flash_Save(&sysData);
	                                        LCD_Clear(); LCD_WriteString("Da Them!"); HAL_Delay(1000);
	                                        currState = STATE_IDLE; Show_Idle(); break;
	                                    } else if (k=='A') { currState = STATE_IDLE; Show_Idle(); break; }
	                                }
	                            }
	                        }
	                    }
	                }
	                break;

	            case STATE_ADMIN_VIEW_ALL:
	                // Chỉ vẽ lại khi cần (ví dụ khi mới vào hoặc khi bấm nút cuộn)
	                // (Code rút gọn để đỡ nháy màn hình liên tục)
	                if (key_input == '*' || key_input == '#' || view_scroll == 0 || key_input == 0) {
	                   // Logic hiển thị danh sách thẻ
	                   char buf[32];
	                   LCD_SetCursor(0,0);
	                   if (view_scroll < sysData.card_count) {
	                       sprintf(buf, "%d.%02X%02X%02X%02X ", view_scroll+1, sysData.uids[view_scroll][0], sysData.uids[view_scroll][1], sysData.uids[view_scroll][2], sysData.uids[view_scroll][3]);
	                       LCD_WriteString(buf);
	                   } else LCD_WriteString("                ");

	                   LCD_SetCursor(1,0);
	                   if (view_scroll+1 < sysData.card_count) {
	                       sprintf(buf, "%d.%02X%02X%02X%02X ", view_scroll+2, sysData.uids[view_scroll+1][0], sysData.uids[view_scroll+1][1], sysData.uids[view_scroll+1][2], sysData.uids[view_scroll+1][3]);
	                       LCD_WriteString(buf);
	                   } else LCD_WriteString("                ");
	                }

	                if (key_input == '*') { if(view_scroll>0) view_scroll--; }
	                else if (key_input == '#') { if(view_scroll < sysData.card_count-1) view_scroll++; }
	                else if (key_input == 'A') { currState = STATE_IDLE; Show_Idle(); }
	                break;

	            default: break;
	        }

	        HAL_Delay(10); // Nghỉ 10ms để giảm tải CPU
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
