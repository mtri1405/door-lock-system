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
    DOOR_STATE_UNLOCKED,        // Cửa đã mở khóa, đèn xanh sáng
    DOOR_STATE_WAITING_CLOSE,   // Đợi người dùng đóng cửa
    DOOR_STATE_ALARM            // Báo động xâm nhập
} DoorState;

/* ====== Configuration ====== */
#define DOOR_UNLOCK_TIME        500    // 5 giây = 500 * 10ms
#define DOOR_OPEN_TIMEOUT       1000   // 10 giây = 1000 * 10ms
#define DOOR_ALARM_DURATION     3000   // 30 giây = 3000 * 10ms

/* ====== Private Variables ====== */
static DoorState door_state = DOOR_STATE_LOCKED;

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
    
    if (now - last_toggle > 500) { // Blink mỗi 200ms
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
    
    // Test LEDs - removed HAL_Delay to prevent Proteus crash
    HAL_GPIO_WritePin(DOOR_RED_LED_PORT, DOOR_RED_LED_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(DOOR_GREEN_LED_PORT, DOOR_GREEN_LED_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(DOOR_GREEN_LED_PORT, DOOR_GREEN_LED_PIN, GPIO_PIN_RESET);
}

// Hàm helper để cập nhật giao diện nhanh
void update_lcd_lock_screen(void) {
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_Print("Enter Password:");
    LCD_SetCursor(1, 0);
    LCD_Print("PASS: ____");
}

void door_fsm_run(void) {
    
    // Dừng door FSM khi password bị khóa do nhập sai quá nhiều lần
    if (password_is_locked()) {
        // Giữ nguyên trạng thái hiện tại, không xử lý gì
        // Khi hết thời gian khóa, FSM sẽ hoạt động lại bình thường
        return;
    }
    
    switch (door_state) {
        
    case DOOR_STATE_LOCKED:
        lock_door(); // Đảm bảo chốt luôn đóng
        
        // 1. Cửa bị cạy (đang khóa mà cảm biến báo mở)
        if (is_door_open()) {
            door_state = DOOR_STATE_ALARM;
            setTimerDoor(DOOR_ALARM_DURATION);
            activate_buzzer_alarm();
            LCD_Clear();
            LCD_Print("SECURITY BREACH!");
            break;
        }

        // 2. Nhập sai quá nhiều lần
        if (password_is_intruder_alarm()) {
            door_state = DOOR_STATE_ALARM;
            setTimerDoor(DOOR_ALARM_DURATION);
            activate_buzzer_alarm();
            LCD_Clear();
            LCD_Print("INTRUDER ALERT!");
            break;
        }

        // 3. Mở khóa thành công
        if (password_is_correct_event()) {
            door_state = DOOR_STATE_UNLOCKED;
            unlock_door();
            setTimerDoor(DOOR_UNLOCK_TIME);
            LCD_Clear();
            LCD_Print("Door Unlocked");
            LCD_SetCursor(1, 0);
            LCD_Print("Welcome!");
        }
        break;
        
    case DOOR_STATE_UNLOCKED:
        // Đợi người dùng mở cửa và vào nhà
        if (timer_door_flag) {
            timer_door_flag = 0;
            if (is_door_open()) {
                // Người dùng đã mở cửa nhưng chưa đóng
                door_state = DOOR_STATE_WAITING_CLOSE;
                setTimerDoor(DOOR_OPEN_TIMEOUT); // Chờ thêm một khoảng trước khi báo động
            } else {
                // Hết thời gian mà cửa vẫn đóng (không ai vào) -> Khóa lại
                door_state = DOOR_STATE_LOCKED;
                update_lcd_lock_screen();
            }
        }
        break;
        
    case DOOR_STATE_WAITING_CLOSE:
        // Cửa đang mở, chốt có thể đã đóng (nhưng không cản trở vì cửa đang mở)
        lock_door(); 

        if (!is_door_open()) {
            door_state = DOOR_STATE_LOCKED;
            update_lcd_lock_screen();
        } else if (timer_door_flag) {
            // Đợi quá lâu không đóng -> Báo động
            door_state = DOOR_STATE_ALARM;
            setTimerDoor(DOOR_ALARM_DURATION);
            activate_buzzer_alarm();
        }
        break;
        
    case DOOR_STATE_ALARM:
        led_blink_red();
        activate_buzzer_alarm();

        // ĐIỀU KIỆN THOÁT 1: Cửa đóng (Ưu tiên nhất)
        if (!is_door_open()) {
            deactivate_buzzer_alarm();
            door_state = DOOR_STATE_LOCKED;
            update_lcd_lock_screen();
            timer_door_flag = 0;
        } 
        // ĐIỀU KIỆN THOÁT 2: Hết thời gian báo động (nhưng cửa vẫn mở)
        else if (timer_door_flag) {
            timer_door_flag = 0;
            deactivate_buzzer_alarm(); 
            // KHÔNG về LOCKED, quay lại trạng thái chờ đóng cửa
            door_state = DOOR_STATE_WAITING_CLOSE; 
            LCD_Clear();
            LCD_Print("Close the Door!");
        }
        break;

    default:
        door_state = DOOR_STATE_LOCKED;
        break;
    }
}