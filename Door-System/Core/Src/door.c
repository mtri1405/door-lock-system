/*
* door.c
*
* Created on: Oct 31, 2025
* Author: MinhTri
*/

#include "door.h"
#include "password.h"
#include "buzzer.h"
#include "lcd.h"
#include "software_timer.h"

/* ====== Door FSM States ====== */
typedef enum {
    DOOR_STATE_LOCKED = 0,      // Cửa khóa
    DOOR_STATE_UNLOCKED,        // Cửa đã mở khóa, chờ người dùng mở
    DOOR_STATE_DOOR_OPENED,     // Cửa đang mở
    DOOR_STATE_DOOR_CLOSED_WAIT,// Cửa vừa đóng, hiện "DOOR CLOSED" 1s
    DOOR_STATE_WAITING_CLOSE,   // Đợi người dùng đóng cửa (timeout)
    DOOR_STATE_ALARM            // Báo động xâm nhập
} DoorState;

/* ====== Configuration ====== */
#define DOOR_UNLOCK_TIME        500    // 5 giây
#define DOOR_OPEN_TIMEOUT       1000   // 10 giây
#define DOOR_ALARM_DURATION     3000   // 30 giây
#define DOOR_CLOSED_DISPLAY     100    // 1 giây hiển thị "DOOR CLOSED"

/* ====== Private Variables ====== */
static DoorState door_state = DOOR_STATE_LOCKED;
static int unlocked_displayed = 0;  // Flag hiển thị DOOR UNLOCK
static int enable_door_fsm = 1;     // Flag enable/disable door FSM

/* ====== Private Functions ====== */

static int is_door_open(void) {
    // Sensor active LOW (cửa mở = RESET)
    return (HAL_GPIO_ReadPin(DOOR_SENSOR_PORT, DOOR_SENSOR_PIN) == GPIO_PIN_RESET);
}

static void lock_door(void) {
    HAL_GPIO_WritePin(DOOR_SOLENOID_PORT, DOOR_SOLENOID_PIN, GPIO_PIN_RESET); // Khóa cửa
    HAL_GPIO_WritePin(DOOR_RED_LED_PORT, DOOR_RED_LED_PIN, GPIO_PIN_SET);     // Đèn đỏ ON
    HAL_GPIO_WritePin(DOOR_GREEN_LED_PORT, DOOR_GREEN_LED_PIN, GPIO_PIN_RESET); // Đèn xanh OFF
}

static void unlock_door(void) {
    HAL_GPIO_WritePin(DOOR_SOLENOID_PORT, DOOR_SOLENOID_PIN, GPIO_PIN_SET);   // Mở khóa
    HAL_GPIO_WritePin(DOOR_RED_LED_PORT, DOOR_RED_LED_PIN, GPIO_PIN_RESET);   // Đèn đỏ OFF
    HAL_GPIO_WritePin(DOOR_GREEN_LED_PORT, DOOR_GREEN_LED_PIN, GPIO_PIN_SET); // Đèn xanh ON
}

static void led_blink_red(void) {
    static uint32_t last_toggle = 0;
    uint32_t now = HAL_GetTick();

    if (now - last_toggle > 200) {
        HAL_GPIO_TogglePin(DOOR_RED_LED_PORT, DOOR_RED_LED_PIN);
        last_toggle = now;
    }
}

/* ====== Public Functions ====== */

void door_init(void) {
    door_state = DOOR_STATE_LOCKED;
    lock_door();
    Buzzer_Init();

    // Test LED nhanh
    HAL_GPIO_WritePin(DOOR_RED_LED_PORT, DOOR_RED_LED_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(DOOR_GREEN_LED_PORT, DOOR_GREEN_LED_PIN, GPIO_PIN_SET);
}

int door_is_unlocked_or_open(void) {
    return (door_state == DOOR_STATE_UNLOCKED || 
            door_state == DOOR_STATE_DOOR_OPENED ||
            door_state == DOOR_STATE_DOOR_CLOSED_WAIT);
}

void door_enable_fsm(void) {
    enable_door_fsm = 1;
}

void door_disable_fsm(void) {
    enable_door_fsm = 0;
}

void door_fsm_run(void) {

    // Kiểm tra flag enable door FSM
    if (!enable_door_fsm) {
        return;  // FSM bị disable, không xử lý
    }

    switch (door_state) {

    case DOOR_STATE_LOCKED:
        lock_door();

        // 1. Cửa bị cạy
        if (is_door_open()) {
            door_state = DOOR_STATE_ALARM;
            setTimerDoor(DOOR_ALARM_DURATION);
            Buzzer_Activate();
            LCD_Clear();
            LCD_Print("SECURITY BREACH!");
            LCD_SetCursor(1, 0);
            LCD_Print("Close the door!");
        }
        // 2. Nhập sai quá nhiều lần - KHÔNG hiển thị LCD, password module sẽ xử lý
        else if (password_is_intruder_alarm()) {
            door_state = DOOR_STATE_ALARM;
            setTimerDoor(DOOR_ALARM_DURATION);
            Buzzer_Activate();
            // Password module đã hiển thị LCD
        }
        // 3. Mở khóa thành công
        else if (password_is_correct_event()) {
            door_state = DOOR_STATE_UNLOCKED;
            unlock_door();
            setTimerDoor(DOOR_UNLOCK_TIME);
            // KHÔNG ghi đè LCD - giữ message "Password OK" từ password module
        }
        break;

    case DOOR_STATE_UNLOCKED:
        // Hiển thị "DOOR UNLOCK" khi vào state (sau message password)
        if (!unlocked_displayed) {
            unlocked_displayed = 1;
            LCD_Clear();
            LCD_Print("DOOR UNLOCK");
            LCD_SetCursor(1, 0);
            LCD_Print("Open the door");
        }
        
        // Chờ người dùng mở cửa hoặc timeout
        if (is_door_open()) {
            unlocked_displayed = 0;  // Reset flag
            door_state = DOOR_STATE_DOOR_OPENED;
            LCD_Clear();
            LCD_Print("DOOR OPENED");  // Hiển thị khi cửa mở
        } else if (timer_door_flag) {
            unlocked_displayed = 0;  // Reset flag
            // Timeout - người dùng không mở cửa
            timer_door_flag = 0;
            door_state = DOOR_STATE_LOCKED;
            password_show_input_screen();
        }
        break;

    case DOOR_STATE_DOOR_OPENED:
        // Cửa đang mở, chờ đóng
        if (!is_door_open()) {
            // Cửa vừa đóng
            door_state = DOOR_STATE_DOOR_CLOSED_WAIT;
            setTimerDoor(DOOR_CLOSED_DISPLAY);  // 1 giây
            LCD_Clear();
            LCD_Print("DOOR CLOSED");  // Hiển thị DOOR CLOSED
        }
        break;

    case DOOR_STATE_DOOR_CLOSED_WAIT:
        // Hiển thị "DOOR CLOSED" trong 1 giây
        if (timer_door_flag) {
            timer_door_flag = 0;
            door_state = DOOR_STATE_LOCKED;
            password_show_input_screen();  // Về màn hình nhập password
        }
        break;

    case DOOR_STATE_WAITING_CLOSE:
        // State này dùng khi cửa mở quá lâu (chưa implement full)
        lock_door();
        if (!is_door_open()) {
            door_state = DOOR_STATE_DOOR_CLOSED_WAIT;
            setTimerDoor(DOOR_CLOSED_DISPLAY);
            LCD_Clear();
            LCD_Print("DOOR CLOSED");
        } else if (timer_door_flag) {
            // Đợi quá lâu không đóng -> Báo động
            door_state = DOOR_STATE_ALARM;
            setTimerDoor(DOOR_ALARM_DURATION);
            Buzzer_Activate();
            LCD_Clear();
            LCD_Print("WARNING!");
            LCD_SetCursor(1,0);
            LCD_Print("Close Door!");
        }
        break;

    case DOOR_STATE_ALARM:
        led_blink_red();
        
        // // Đảm bảo buzzer đang active
        // if (!Buzzer_IsActive()) {
        //     Buzzer_Activate();
        // }

        // ĐIỀU KIỆN THOÁT 1: Hết thời gian báo động (30s)
        if (timer_door_flag) {
            timer_door_flag = 0;
            Buzzer_Deactivate();
            if (is_door_open()) {
                door_state = DOOR_STATE_WAITING_CLOSE;
                LCD_Clear();
                LCD_Print("Close the Door!");
            } else {
                door_state = DOOR_STATE_LOCKED;
                password_show_input_screen();
            }
        }
        // ĐIỀU KIỆN THOÁT 2: Cửa đóng (chỉ tắt nếu KHÔNG phải báo động sai pass)
        else if (!is_door_open() && !password_is_intruder_alarm()) {
            Buzzer_Deactivate();
            door_state = DOOR_STATE_LOCKED;
            password_show_input_screen();
            timer_door_flag = 0;
        }
        break;

    default:
        door_state = DOOR_STATE_LOCKED;
        break;
    }
}
