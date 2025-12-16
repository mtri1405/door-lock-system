/*
* door.c
*
*  Created on: Oct 31, 2025
*      Author: MinhTri
*/

#include "door.h"
#include "password.h"
#include "buzzer.h"
#include "lcd.h"
#include "software_timer.h"

/* ====== Door FSM States ====== */
typedef enum {
    DOOR_STATE_LOCKED = 0,      // Cửa khóa, đèn đỏ sáng
    DOOR_STATE_UNLOCKING,       // Đang mở khóa (delay nhỏ)
    DOOR_STATE_UNLOCKED,        // Cửa đã mở khóa, đèn xanh sáng
    DOOR_STATE_WAITING_CLOSE,   // Đợi người dùng đóng cửa
    DOOR_STATE_ALARM            // Báo động xâm nhập
} DoorState;

/* ====== Configuration ====== */
#define DOOR_UNLOCK_TIME_MS     5000U    // 5 giây mở khóa tự động đóng
#define DOOR_OPEN_TIMEOUT_MS    10000U   // 10 giây cửa mở quá lâu -> báo động
#define DOOR_ALARM_DURATION_MS  30000U   // 30 giây báo động

/* ====== Private Variables ====== */
static DoorState door_state = DOOR_STATE_LOCKED;
static uint32_t door_timer_start = 0;

// Cờ buzzer alarm
static int buzzer_alarm_active = 0;

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
    // Nhấp nháy đèn đỏ khi alarm
    static uint32_t last_toggle = 0;
    uint32_t now = HAL_GetTick();
    
    if (now - last_toggle > 200) { // Blink mỗi 200ms
        HAL_GPIO_TogglePin(DOOR_RED_LED_PORT, DOOR_RED_LED_PIN);
        last_toggle = now;
    }
}

static void activate_buzzer_alarm(void) {
    buzzer_alarm_active = 1;
    Buzzer_Activate();  // Kích hoạt buzzer qua API
}

static void deactivate_buzzer_alarm(void) {
    buzzer_alarm_active = 0;
    Buzzer_Deactivate();  // Tắt buzzer qua API
}

/* ====== Public Functions ====== */

void door_init(void) {
    // Khởi tạo door ở trạng thái khóa
    door_state = DOOR_STATE_LOCKED;
    lock_door();
    buzzer_alarm_active = 0;
    
    // Khởi tạo buzzer
    Buzzer_Init();
    
    // Test LEDs
    HAL_GPIO_WritePin(DOOR_RED_LED_PORT, DOOR_RED_LED_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(DOOR_GREEN_LED_PORT, DOOR_GREEN_LED_PIN, GPIO_PIN_SET);
    HAL_Delay(200);
    HAL_GPIO_WritePin(DOOR_GREEN_LED_PORT, DOOR_GREEN_LED_PIN, GPIO_PIN_RESET);
}

void door_fsm_run(void) {
    uint32_t now = HAL_GetTick();
    
    switch (door_state) {
        
    case DOOR_STATE_LOCKED:
        // Trạng thái: Cửa khóa, chờ password đúng
        lock_door();
        
        // Kiểm tra xâm nhập (cửa bị mở mà không có password)
        if (is_door_open() && !password_is_locked()) {
            door_state = DOOR_STATE_ALARM;
            door_timer_start = now;
            activate_buzzer_alarm();
            LCD_Clear();
            LCD_SetCursor(0, 0);
            LCD_Print("INTRUSION ALERT!");
            LCD_SetCursor(1, 0);
            LCD_Print("Close door now!");
        }
        
        // Kiểm tra password đúng
        if (password_is_correct_event()) {
            door_state = DOOR_STATE_UNLOCKING;
            door_timer_start = now;
        }
        
        // Nếu system bị lock do sai password nhiều lần
        if (password_is_intruder_alarm()) {
            door_state = DOOR_STATE_ALARM;
            door_timer_start = now;
            activate_buzzer_alarm();
        }
        break;
        
    case DOOR_STATE_UNLOCKING:
        // Trạng thái: Đang mở khóa (có thể thêm animation)
        unlock_door();
        
        // Chuyển sang UNLOCKED ngay lập tức
        door_state = DOOR_STATE_UNLOCKED;
        door_timer_start = now;
        break;
        
    case DOOR_STATE_UNLOCKED:
        // Trạng thái: Cửa đã mở khóa, đợi người dùng vào
        unlock_door();
        
        // Kiểm tra timeout tự động khóa lại
        if (now - door_timer_start > DOOR_UNLOCK_TIME_MS) {
            if (is_door_open()) {
                // Cửa vẫn mở -> chuyển sang WAITING_CLOSE
                door_state = DOOR_STATE_WAITING_CLOSE;
                door_timer_start = now;
                LCD_Clear();
                LCD_SetCursor(0, 0);
                LCD_Print("Please close");
                LCD_SetCursor(1, 0);
                LCD_Print("the door");
            } else {
                // Cửa đã đóng -> khóa lại
                door_state = DOOR_STATE_LOCKED;
                LCD_Clear();
                LCD_SetCursor(0, 0);
                LCD_Print("Door locked");
                HAL_Delay(1000);
            }
        }
        break;
        
    case DOOR_STATE_WAITING_CLOSE:
        // Trạng thái: Đợi người dùng đóng cửa
        unlock_door(); // Vẫn mở khóa
        
        // Kiểm tra cửa đã đóng chưa
        if (!is_door_open()) {
            // Cửa đã đóng -> khóa lại
            door_state = DOOR_STATE_LOCKED;
            LCD_Clear();
            LCD_SetCursor(0, 0);
            LCD_Print("Door locked");
            HAL_Delay(1000);
            break;
        }
        
        // Kiểm tra timeout - cửa mở quá lâu
        if (now - door_timer_start > DOOR_OPEN_TIMEOUT_MS) {
            door_state = DOOR_STATE_ALARM;
            door_timer_start = now;
            activate_buzzer_alarm();
            LCD_Clear();
            LCD_SetCursor(0, 0);
            LCD_Print("Door open too");
            LCD_SetCursor(1, 0);
            LCD_Print("long! ALARM!");
        }
        break;
        
    case DOOR_STATE_ALARM:
        // Trạng thái: Báo động xâm nhập hoặc cửa mở quá lâu
        lock_door(); // Khóa cửa
        led_blink_red(); // Nhấp nháy đèn đỏ
        
        // Kiểm tra cửa đã đóng chưa
        if (!is_door_open()) {
            // Cửa đã đóng -> tắt alarm, quay về LOCKED
            door_state = DOOR_STATE_LOCKED;
            deactivate_buzzer_alarm();
            LCD_Clear();
            LCD_SetCursor(0, 0);
            LCD_Print("Alarm cleared");
            LCD_SetCursor(1, 0);
            LCD_Print("Door locked");
            HAL_Delay(1000);
        }
        
        // Timeout alarm (tự động tắt sau 30s)
        if (now - door_timer_start > DOOR_ALARM_DURATION_MS) {
            if (!is_door_open()) {
                door_state = DOOR_STATE_LOCKED;
                deactivate_buzzer_alarm();
            }
        }
        break;
        
    default:
        door_state = DOOR_STATE_LOCKED;
        break;
    }
}