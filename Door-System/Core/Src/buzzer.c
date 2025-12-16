/*
* buzzer.c
*
*  Created on: Nov 8, 2025
*      Author: thait
*/
#include "buzzer.h"

/* ====== Buzzer Configuration ====== */
#define BUZZER_CTRL_PORT    BUZZER_CTRL_GPIO_Port
#define BUZZER_CTRL_PIN     BUZZER_CTRL_Pin

#define MUTE_BTN_PORT       MUTE_BUTTON_GPIO_Port
#define MUTE_BTN_PIN        MUTE_BUTTON_Pin

/* ====== Private Variables ====== */
static int buzzer_active = 0;          // Cờ buzzer đang hoạt động
static int buzzer_muted = 0;           // Cờ buzzer bị mute
static int mute_button_pressed = 0;    // Debounce mute button

/* ====== Private Functions ====== */

static void buzzer_turn_on(void) {
    // Buzzer active HIGH
    HAL_GPIO_WritePin(BUZZER_CTRL_PORT, BUZZER_CTRL_PIN, GPIO_PIN_SET);
}

static void buzzer_turn_off(void) {
    // Buzzer off
    HAL_GPIO_WritePin(BUZZER_CTRL_PORT, BUZZER_CTRL_PIN, GPIO_PIN_RESET);
}

static int is_mute_button_pressed(void) {
    // Mute button active LOW (pull-up)
    return (HAL_GPIO_ReadPin(MUTE_BTN_PORT, MUTE_BTN_PIN) == GPIO_PIN_RESET);
}

/* ====== Public Functions ====== */

/**
 * @brief Khởi tạo buzzer module
 */
void Buzzer_Init(void) {
    buzzer_active = 0;
    buzzer_muted = 0;
    mute_button_pressed = 0;
    buzzer_turn_off();
}

/**
 * @brief Kích hoạt buzzer (được gọi từ door FSM)
 */
void Buzzer_Activate(void) {
    buzzer_active = 1;
    buzzer_muted = 0;  // Reset mute flag khi có alarm mới
}

/**
 * @brief Tắt buzzer (được gọi từ door FSM)
 */
void Buzzer_Deactivate(void) {
    buzzer_active = 0;
    buzzer_muted = 0;
    buzzer_turn_off();
}

/**
 * @brief Kiểm tra buzzer có đang hoạt động không
 * @return 1 nếu active, 0 nếu không
 */
int Buzzer_IsActive(void) {
    return buzzer_active;
}

/**
 * @brief Hàm chạy logic chính của buzzer
 * Gọi liên tục trong main loop
 * Xử lý:
 * - Bật/tắt buzzer dựa trên buzzer_active flag
 * - Xử lý nút mute
 */
void Buzzer_Run(void) {
    // Xử lý nút mute với debounce đơn giản
    if (is_mute_button_pressed()) {
        if (!mute_button_pressed) {
            // Sườn lên của nút mute
            mute_button_pressed = 1;
            
            if (buzzer_active) {
                // Mute buzzer
                buzzer_muted = 1;
            }
        }
    } else {
        mute_button_pressed = 0;
    }
    
    // Điều khiển buzzer output
    if (buzzer_active && !buzzer_muted) {
        // Buzzer kêu liên tục
        buzzer_turn_on();
    } else {
        // Buzzer tắt
        buzzer_turn_off();
    }
}
