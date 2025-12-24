/*
 * password.c
 *
 *  Created on: Nov 10, 2025
 *      Author: Gentle Croissant
 */

#include "password.h"
#include "lcd.h"
#include <string.h>

/* ====== Cấu hình chung ====== */

#define PASSWORD_LENGTH        4
#define MAX_WRONG_ATTEMPTS     3
#define INITIAL_LOCK_TIME      6000   // 60s = 6000 * 10ms

typedef enum {
    PW_STATE_INPUT = 0,   // Đang chờ nhập 4 ký tự
    PW_STATE_CHECK,       // Đã đủ 4 ký tự, chuẩn bị kiểm tra
    PW_STATE_LOCKED       // Bị khóa vì sai nhiều lần
} PasswordState;

typedef enum {
    PW_MODE_NORMAL = 0,       // Nhập/mở cửa bình thường
    PW_MODE_CHANGE_OLD,       // Đang nhập mật khẩu cũ để đổi pass
    PW_MODE_CHANGE_NEW1,      // Nhập mật khẩu mới lần 1
    PW_MODE_CHANGE_NEW2       // Nhập mật khẩu mới lần 2 (xác nhận)
} PasswordMode;

typedef enum {
    PW_DISPLAY_NONE = 0,
    PW_DISPLAY_CHANGE_OLD_WRONG,
    PW_DISPLAY_CHANGE_SUCCESS,
    PW_DISPLAY_CHANGE_NOT_MATCH
} PasswordDisplayState;

/* ====== Biến trạng thái password ====== */

// Mật khẩu hiện tại (chỉ dùng số 0–9)
static uint8_t current_pw[PASSWORD_LENGTH] = {1, 2, 3, 4};

// Dãy người dùng đang nhập
static uint8_t input_pw[PASSWORD_LENGTH]   = {0, 0, 0, 0};
static uint8_t digits_entered              = 0;

// Lưu mật khẩu mới trong quá trình đổi
static uint8_t new_pw_candidate[PASSWORD_LENGTH] = {0, 0, 0, 0};

static PasswordState pw_state  = PW_STATE_INPUT;
static PasswordMode  pw_mode   = PW_MODE_NORMAL;
static PasswordDisplayState pw_display_state = PW_DISPLAY_NONE;

static uint8_t  wrong_attempts       = 0;
static int      current_lock_time    = INITIAL_LOCK_TIME;

// Sự kiện “vừa nhập đúng mật khẩu” (cho door FSM)
static int pw_correct_flag       = 0;
// Cờ nghi ngờ xâm nhập (sai >= 3 lần, đang LOCKED)
static int intruder_alarm_flag   = 0;// Flag enable/disable password FSM
static int enable_password_fsm   = 1;

/* ====== Cấu hình keypad 3x4 (PHONE) ======
 *
 * Layout:
 *      C1   C2   C3
 * R1    1    2    3
 * R2    4    5    6
 * R3    7    8    9
 * R4    *    0    #
 *
 * Bạn phải đặt label trong .ioc:
 *   - KPD_R1, KPD_R2, KPD_R3, KPD_R4  (output)
 *   - KPD_C1, KPD_C2, KPD_C3          (input pull-up)
 */

#define ROW1_GPIO_Port   KPD_R1_GPIO_Port
#define ROW1_Pin         KPD_R1_Pin

#define ROW2_GPIO_Port   KPD_R2_GPIO_Port
#define ROW2_Pin         KPD_R2_Pin

#define ROW3_GPIO_Port   KPD_R3_GPIO_Port
#define ROW3_Pin         KPD_R3_Pin

#define ROW4_GPIO_Port   KPD_R4_GPIO_Port
#define ROW4_Pin         KPD_R4_Pin

#define COL1_GPIO_Port   KPD_C1_GPIO_Port
#define COL1_Pin         KPD_C1_Pin

#define COL2_GPIO_Port   KPD_C2_GPIO_Port
#define COL2_Pin         KPD_C2_Pin

#define COL3_GPIO_Port   KPD_C3_GPIO_Port
#define COL3_Pin         KPD_C3_Pin


/* ====== LCD helper ====== */

static void lcd_show_message(const char *line1, const char *line2) {
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_Print(line1);
    LCD_SetCursor(1, 0);
    LCD_Print(line2);
}

// Hiển thị dạng PASS: **** trên dòng 2
static void lcd_show_password_mask(void) {
    char buf[17];
    memset(buf, 0, sizeof(buf));

    // Ví dụ: "PASS: ****" hoặc "PASS: 12**"
    strcpy(buf, "PASS: ");
    for (int i = 0; i < PASSWORD_LENGTH; i++) {
        if (i < digits_entered) {
            buf[6 + i] = '*';   // ẩn số thật, chỉ hiện *
        } else {
            buf[6 + i] = '_';   // chưa nhập
        }
    }
    buf[6 + PASSWORD_LENGTH] = '\0';

    LCD_SetCursor(1, 0);
    LCD_Print(buf);
}

/* ====== Password helper ====== */

static void pw_clear_input(void) {
    for (int i = 0; i < PASSWORD_LENGTH; i++) {
        input_pw[i] = 0;
    }
    digits_entered = 0;
    lcd_show_password_mask();
}

static void pw_shift_and_append(uint8_t digit) {
    // Dịch trái và thêm digit mới vào bên phải
    input_pw[0] = input_pw[1];
    input_pw[1] = input_pw[2];
    input_pw[2] = input_pw[3];
    input_pw[3] = digit;

    if (digits_entered < PASSWORD_LENGTH) {
        digits_entered++;
    }
    lcd_show_password_mask();
}

static int pw_compare(const uint8_t *a, const uint8_t *b) {
    for (int i = 0; i < PASSWORD_LENGTH; i++) {
        if (a[i] != b[i]) return 0;
    }
    return 1;
}

/* ====== Keypad helper ======
 * Quy ước trả về của keypad_read_raw_key():
 *   0..9 : phím số
 *   -2   : '*'
 *   -3   : '#'
 *   -1   : không có phím
 */


static int keypad_read_raw_key(void) {
    int key = -1;

    // Mặc định: tất cả ROW = HIGH
    HAL_GPIO_WritePin(ROW1_GPIO_Port, ROW1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ROW2_GPIO_Port, ROW2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ROW3_GPIO_Port, ROW3_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ROW4_GPIO_Port, ROW4_Pin, GPIO_PIN_SET);


    for (int row = 0; row < 4; row++) {
        // Set tất cả hàng = HIGH
        HAL_GPIO_WritePin(ROW1_GPIO_Port, ROW1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(ROW2_GPIO_Port, ROW2_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(ROW3_GPIO_Port, ROW3_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(ROW4_GPIO_Port, ROW4_Pin, GPIO_PIN_SET);

        // Kéo hàng hiện tại xuống LOW
        switch (row) {
        case 0:
            HAL_GPIO_WritePin(ROW1_GPIO_Port, ROW1_Pin, GPIO_PIN_RESET);
            break;
        case 1:
            HAL_GPIO_WritePin(ROW2_GPIO_Port, ROW2_Pin, GPIO_PIN_RESET);
            break;
        case 2:
            HAL_GPIO_WritePin(ROW3_GPIO_Port, ROW3_Pin, GPIO_PIN_RESET);
            break;
        case 3:
            HAL_GPIO_WritePin(ROW4_GPIO_Port, ROW4_Pin, GPIO_PIN_RESET);
            break;
        }

        // Delay nhỏ để tín hiệu ổn định trước khi đọc
        for(volatile int i = 0; i < 100; i++);

        // Đọc các cột (active-low)
        int c1 = (HAL_GPIO_ReadPin(COL1_GPIO_Port, COL1_Pin) == GPIO_PIN_RESET);
        int c2 = (HAL_GPIO_ReadPin(COL2_GPIO_Port, COL2_Pin) == GPIO_PIN_RESET);
        int c3 = (HAL_GPIO_ReadPin(COL3_GPIO_Port, COL3_Pin) == GPIO_PIN_RESET);

        if (c1 || c2 || c3) {
            int col = -1;
            if (c1) col = 0;
            else if (c2) col = 1;
            else if (c3) col = 2;

            // Map (row, col) -> key
            // row 0: 1 2 3
            // row 1: 4 5 6
            // row 2: 7 8 9
            // row 3: * 0 #
            if (row == 0) {
                key = col + 1;          // 1..3
            } else if (row == 1) {
                key = col + 4;          // 4..6
            } else if (row == 2) {
                key = col + 7;          // 7..9
            } else { // row == 3
                if (col == 0) {
                    key = -2;           // '*'
                } else if (col == 1) {
                    key = 0;            // '0'
                } else if (col == 2) {
                    key = -3;           // '#'
                }
            }

            // QUAN TRỌNG: Set lại tất cả hàng về HIGH TRƯỚC KHI break
            // Tránh để hàng bị stuck ở LOW
            HAL_GPIO_WritePin(ROW1_GPIO_Port, ROW1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(ROW2_GPIO_Port, ROW2_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(ROW3_GPIO_Port, ROW3_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(ROW4_GPIO_Port, ROW4_Pin, GPIO_PIN_SET);

            break;
        }
    }

    // Set lại tất cả hàng = HIGH
    HAL_GPIO_WritePin(ROW1_GPIO_Port, ROW1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ROW2_GPIO_Port, ROW2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ROW3_GPIO_Port, ROW3_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ROW4_GPIO_Port, ROW4_Pin, GPIO_PIN_SET);

    return key;
}

// Biến để detect “nhấn mới”
static int keypad_was_pressed   = 0;
static uint8_t last_digit_event = 0;
static int clear_event_pending  = 0;
static int change_mode_request  = 0;

// Poll keypad, phát sinh event số & clear & change-mode
static int keypad_poll_new_digit(void) {
    int k = keypad_read_raw_key();  // 0..9, -2(*), -3(#), -1

    if (k != -1) {
        if (!keypad_was_pressed) {
            // sườn lên: nhấn mới
            keypad_was_pressed = 1;

            if (k >= 0 && k <= 9) {
                last_digit_event = (uint8_t)k;
                return 1;   // có digit mới
            } else if (k == -2) {
                // '*': CLEAR
                clear_event_pending = 1;
            } else if (k == -3) {
                // '#': yêu cầu đổi mật khẩu
                change_mode_request = 1;
            }
        }
    } else {
        keypad_was_pressed = 0;
    }

    return 0;   // không có digit mới
}

static int is_enter_pressed(void) {
    return keypad_poll_new_digit();
}

static uint8_t read_digit_from_input(void) {
    return last_digit_event;
}

static int is_clear_pressed(void) {
    if (clear_event_pending) {
        clear_event_pending = 0;
        return 1;
    }
    return 0;
}

static int is_change_mode_requested(void) {
    if (change_mode_request) {
        change_mode_request = 0;
        return 1;
    }
    return 0;
}

/* ====== API public ====== */

void password_init(void) {
    pw_state  = PW_STATE_INPUT;
    pw_mode   = PW_MODE_NORMAL;
    pw_display_state = PW_DISPLAY_NONE;

    wrong_attempts       = 0;
    current_lock_time    = INITIAL_LOCK_TIME;

    pw_correct_flag      = 0;
    intruder_alarm_flag  = 0;

    // Mật khẩu mặc định
    current_pw[0] = 1;
    current_pw[1] = 2;
    current_pw[2] = 3;
    current_pw[3] = 4;

    LCD_Clear();
    lcd_show_message("Enter password", "");
    pw_clear_input();
}

void password_fsm_run(void) {

    // Kiểm tra flag enable password FSM
    if (!enable_password_fsm) {
        return;  // FSM bị disable, không xử lý
    }

    // Xử lý hiển thị tạm thời (1 giây)
    if (pw_display_state != PW_DISPLAY_NONE) {
        if (timer_lcd_display_flag) {
            timer_lcd_display_flag = 0;
            // Hết 1 giây, chuyển về màn hình nhập password
            LCD_Clear();
            lcd_show_message("Enter password", "");
            pw_mode = PW_MODE_NORMAL;
            pw_state = PW_STATE_INPUT;
            pw_clear_input();
            pw_display_state = PW_DISPLAY_NONE;
        }
        return; // Đang hiển thị thông báo, không xử lý input
    }

    // Nếu đang LOCKED: xử lý riêng
    if (pw_state == PW_STATE_LOCKED) {
        if (timer_password_flag) {
            timer_password_flag = 0;
            // Hết thời gian khóa
            pw_state = PW_STATE_INPUT;
            wrong_attempts = 0;
            intruder_alarm_flag = 0; // hết báo xâm nhập
            LCD_Clear();
            lcd_show_message("Lockout ended", "Enter password");
            pw_clear_input();
        } else {
            // Đang bị khóa - không xử lý gì
            return;
        }
    }

    // KIỂM TRA: Nếu cửa đang unlocked/open thì chỉ xử lý phím '#' để đổi mật khẩu
    // Các phím số sẽ không được xử lý
    int door_is_open_now = door_is_unlocked_or_open();

    // Xử lý phím # để chuyển đổi mode hoặc hủy đổi password
    if (pw_state == PW_STATE_INPUT) {
        if (is_change_mode_requested()) {
            if (pw_mode == PW_MODE_NORMAL) {
                // Chuyển từ NORMAL sang đổi password
                pw_mode = PW_MODE_CHANGE_OLD;
                lcd_show_message("Change password", "Old pass:");
                pw_clear_input();
            } else {
                // Đang đổi password, bấm # để hủy và quay về NORMAL
                pw_mode = PW_MODE_NORMAL;
                lcd_show_message("Cancelled", "Enter password");
                pw_clear_input();
            }
        }
    }

    // Xử lý CLEAR ở mọi mode khi đang nhập (chỉ khi cửa không mở HOẶC đang đổi pass)
    if (pw_state == PW_STATE_INPUT) {
        if (is_clear_pressed()) {
            // Cho phép clear khi đang đổi mật khẩu hoặc cửa đóng
            if (pw_mode != PW_MODE_NORMAL || !door_is_open_now) {
                pw_clear_input();
            }
        }
    }

    // Xử lý nhập digit
    if (pw_state == PW_STATE_INPUT) {
        if (is_enter_pressed()) {
            uint8_t digit = read_digit_from_input();
            if (digit > 9) digit = 9;

            // Nếu đang đổi pass: luôn cho nhập
            if (pw_mode != PW_MODE_NORMAL) {
                pw_shift_and_append(digit);
                if (digits_entered >= PASSWORD_LENGTH) {
                    pw_state = PW_STATE_CHECK;
                }
            }
            // Nếu mode NORMAL: chỉ block khi cửa đang mở
            else if (!door_is_open_now) {
                pw_shift_and_append(digit);
                if (digits_entered >= PASSWORD_LENGTH) {
                    pw_state = PW_STATE_CHECK;
                }
            }
            // Nếu cửa đang mở và mode NORMAL: hiển thị thông báo
            else {
                // Hiển thị thông báo ngắn gọn trên dòng 2
                LCD_SetCursor(1, 0);
                LCD_Print("Door is open!  ");
            }
        }
    }

    if (pw_state == PW_STATE_CHECK) {
        switch (pw_mode) {
        case PW_MODE_NORMAL:
            // Kiểm tra mật khẩu để mở cửa
            if (pw_compare(input_pw, current_pw)) {
                // ĐÚNG MẬT KHẨU
                pw_correct_flag      = 1;
                wrong_attempts       = 0;
                current_lock_time    = INITIAL_LOCK_TIME;
                LCD_Clear();
                lcd_show_message("Password OK", "Access Granted");
                // Chờ door FSM xử lý mở cửa, sau đó mình quay về nhập bình thường
                pw_mode  = PW_MODE_NORMAL;
                pw_state = PW_STATE_INPUT;
                pw_clear_input();
            } else {
                // SAI MẬT KHẨU
                wrong_attempts++;
                pw_clear_input();

                if (wrong_attempts >= MAX_WRONG_ATTEMPTS) {
                    // Bị khóa
                    pw_state = PW_STATE_LOCKED;
                    setTimerPassword(current_lock_time);
                    intruder_alarm_flag = 1;   // báo xâm nhập

                    // Tăng thời gian lock cho lần sau (giới hạn tối đa 5 phút)
                    if (current_lock_time < 5 * 6000) {  // 5 phút = 30000 ticks
                        current_lock_time *= 2;
                    }

                    // Hiển thị thông báo bị khóa
                    LCD_Clear();
                    lcd_show_message("LOCKED!", "Too many tries");
                } else {
                    LCD_Clear();
                    lcd_show_message("Wrong password", "Try again");
                    pw_state = PW_STATE_INPUT;
                }
            }
            break;

        case PW_MODE_CHANGE_OLD:
            // Kiểm tra mật khẩu cũ
            if (pw_compare(input_pw, current_pw)) {
                // Mật khẩu cũ đúng -> nhập mật khẩu mới lần 1
                pw_mode  = PW_MODE_CHANGE_NEW1;
                pw_state = PW_STATE_INPUT;
                LCD_Clear();
                lcd_show_message("New password:", "");
                pw_clear_input();
            } else {
                LCD_Clear();
                lcd_show_message("Wrong old pass", "Cancel change");
                pw_display_state = PW_DISPLAY_CHANGE_OLD_WRONG;
                setTimerLcdDisplay(100); // 1 giây = 100 * 10ms
            }
            break;

        case PW_MODE_CHANGE_NEW1:
            // Lưu mật khẩu mới lần 1
            for (int i = 0; i < PASSWORD_LENGTH; i++) {
                new_pw_candidate[i] = input_pw[i];
            }
            pw_mode  = PW_MODE_CHANGE_NEW2;
            pw_state = PW_STATE_INPUT;
            LCD_Clear();
            lcd_show_message("Confirm new:", "");
            pw_clear_input();
            break;

        case PW_MODE_CHANGE_NEW2:
            // So sánh với lần 1
            if (pw_compare(input_pw, new_pw_candidate)) {
                // Đổi mật khẩu thành công
                for (int i = 0; i < PASSWORD_LENGTH; i++) {
                    current_pw[i] = new_pw_candidate[i];
                }
                LCD_Clear();
                lcd_show_message("Password", "Changed OK!");
                pw_display_state = PW_DISPLAY_CHANGE_SUCCESS;
                setTimerLcdDisplay(100); // 1 giây = 100 * 10ms
            } else {
                LCD_Clear();
                lcd_show_message("Not matched", "Pass unchanged");
                pw_display_state = PW_DISPLAY_CHANGE_NOT_MATCH;
                setTimerLcdDisplay(100); // 1 giây = 100 * 10ms
            }
            break;

        default:
            pw_mode  = PW_MODE_NORMAL;
            pw_state = PW_STATE_INPUT;
            pw_clear_input();
            break;
        }
    }
}

int password_is_correct_event(void) {
    if (pw_correct_flag) {
        pw_correct_flag = 0;
        return 1;
    }
    return 0;
}

int password_is_locked(void) {
    return (pw_state == PW_STATE_LOCKED);
}

int password_is_intruder_alarm(void) {
    return intruder_alarm_flag;
}

void password_stop_alarm_signal(void) {
    intruder_alarm_flag = 0; // Tắt tín hiệu báo động nhưng vẫn giữ khóa bàn phím
}

void password_show_input_screen(void) {
    // Chỉ hiển thị nếu không đang bị locked
    if (pw_state != PW_STATE_LOCKED) {
        LCD_Clear();
        lcd_show_message("Enter password", "");
        pw_clear_input();
    }
}

void password_enable_fsm(void) {
    enable_password_fsm = 1;
}

void password_disable_fsm(void) {
    enable_password_fsm = 0;
}
