#include "keypad.h"

// Hàm quét phím có chống rung và KHÔNG DÙNG while chặn (Non-blocking)
char Keypad_GetKey(void) {
    char keys[4][4] = { {'4','5','6','B'},
    					{'1','2','3','A'},
						{'7','8','9','C'},
						{'*','0','#','D'}};

    // Đảm bảo tất cả hàng ở mức CAO trước khi quét
    HAL_GPIO_WritePin(R1_PORT, R1_PIN, 1);
    HAL_GPIO_WritePin(R2_PORT, R2_PIN, 1);
    HAL_GPIO_WritePin(R3_PORT, R3_PIN, 1);
    HAL_GPIO_WritePin(R4_PORT, R4_PIN, 1);

    for (int r = 0; r < 4; r++) {
        // Kéo hàng hiện tại xuống THẤP
        if (r==0) HAL_GPIO_WritePin(R1_PORT, R1_PIN, 0);
        if (r==1) HAL_GPIO_WritePin(R2_PORT, R2_PIN, 0);
        if (r==2) HAL_GPIO_WritePin(R3_PORT, R3_PIN, 0);
        if (r==3) HAL_GPIO_WritePin(R4_PORT, R4_PIN, 0);

        // Kiểm tra các cột
        if (HAL_GPIO_ReadPin(C1_PORT, C1_PIN) == 0) {
            HAL_Delay(20); // Chống rung 20ms
            if (HAL_GPIO_ReadPin(C1_PORT, C1_PIN) == 0) return keys[r][0];
        }
        if (HAL_GPIO_ReadPin(C2_PORT, C2_PIN) == 0) {
            HAL_Delay(20);
            if (HAL_GPIO_ReadPin(C2_PORT, C2_PIN) == 0) return keys[r][1];
        }
        if (HAL_GPIO_ReadPin(C3_PORT, C3_PIN) == 0) {
            HAL_Delay(20);
            if (HAL_GPIO_ReadPin(C3_PORT, C3_PIN) == 0) return keys[r][2];
        }
        if (HAL_GPIO_ReadPin(C4_PORT, C4_PIN) == 0) {
            HAL_Delay(20);
            if (HAL_GPIO_ReadPin(C4_PORT, C4_PIN) == 0) return keys[r][3];
        }

        // Trả hàng về mức CAO
        if (r==0) HAL_GPIO_WritePin(R1_PORT, R1_PIN, 1);
        if (r==1) HAL_GPIO_WritePin(R2_PORT, R2_PIN, 1);
        if (r==2) HAL_GPIO_WritePin(R3_PORT, R3_PIN, 1);
        if (r==3) HAL_GPIO_WritePin(R4_PORT, R4_PIN, 1);
    }
    return 0; // Không phím nào được nhấn
}
