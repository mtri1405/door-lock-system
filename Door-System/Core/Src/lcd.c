/*
 * lcd.c
 *
 *  Created on: Nov 29, 2025
 *      Author: Gentle Croissant
 */


#include "lcd.h"


#define LCD_RS_PORT   LCD_RS_GPIO_Port
#define LCD_RS_PIN    LCD_RS_Pin

#define LCD_EN_PORT   LCD_EN_GPIO_Port
#define LCD_EN_PIN    LCD_EN_Pin

#define LCD_D4_PORT   LCD_D4_GPIO_Port
#define LCD_D4_PIN    LCD_D4_Pin

#define LCD_D5_PORT   LCD_D5_GPIO_Port
#define LCD_D5_PIN    LCD_D5_Pin

#define LCD_D6_PORT   LCD_D6_GPIO_Port
#define LCD_D6_PIN    LCD_D6_Pin

#define LCD_D7_PORT   LCD_D7_GPIO_Port
#define LCD_D7_PIN    LCD_D7_Pin

/* ====== Các lệnh cơ bản của HD44780 ====== */

#define LCD_CMD_CLEAR_DISPLAY    0x01
#define LCD_CMD_RETURN_HOME      0x02
#define LCD_CMD_ENTRY_MODE_SET   0x04
#define LCD_CMD_DISPLAY_CONTROL  0x08
#define LCD_CMD_FUNCTION_SET     0x20
#define LCD_CMD_SET_DDRAM_ADDR   0x80

/* Entry mode flags */
#define LCD_ENTRY_INCREMENT      0x02
#define LCD_ENTRY_SHIFT_OFF      0x00

/* Display control flags */
#define LCD_DISPLAY_ON           0x04
#define LCD_CURSOR_OFF           0x00
#define LCD_BLINK_OFF            0x00

/* Function set flags */
#define LCD_4BIT_MODE            0x00
#define LCD_2LINE                0x08
#define LCD_5x8DOTS              0x00

/* ====== Static helper ====== */

static void LCD_PulseEnable(void);
static void LCD_Write4Bits(uint8_t data);
static void LCD_Send(uint8_t value, uint8_t rs);

/* ====== Implementation ====== */

void LCD_Init(void) {
    // Đợi nguồn ổn định
    HAL_Delay(50);

    // Đảm bảo RS, EN = 0
    HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_EN_PORT, LCD_EN_PIN, GPIO_PIN_RESET);

    // Khởi tạo chuẩn 4-bit theo trình tự recommended
    // Gửi 0x33, 0x32 để thiết lập 4-bit mode
    LCD_SendCommand(0x33);
    LCD_SendCommand(0x32);

    // Function set: 4-bit, 2 line, 5x8 dots
    LCD_SendCommand(LCD_CMD_FUNCTION_SET | LCD_4BIT_MODE | LCD_2LINE | LCD_5x8DOTS);

    // Display control: Display ON, Cursor OFF, Blink OFF
    LCD_SendCommand(LCD_CMD_DISPLAY_CONTROL | LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF);

    // Entry mode: tăng địa chỉ, không dịch màn hình
    LCD_SendCommand(LCD_CMD_ENTRY_MODE_SET | LCD_ENTRY_INCREMENT | LCD_ENTRY_SHIFT_OFF);

    // Clear display
    LCD_Clear();
}

void LCD_Clear(void) {
    LCD_SendCommand(LCD_CMD_CLEAR_DISPLAY);
    HAL_Delay(2); // lệnh clear cần >1.52ms
}

void LCD_SetCursor(uint8_t row, uint8_t col) {
    uint8_t addr = 0;

    // Địa chỉ DDRAM cho LCD 16x2:
    // Hàng 0: 0x00..0x0F
    // Hàng 1: 0x40..0x4F
    if (row == 0) {
        addr = 0x00 + col;
    } else {
        addr = 0x40 + col;
    }

    LCD_SendCommand(LCD_CMD_SET_DDRAM_ADDR | addr);
}

void LCD_Print(const char *str) {
    while (*str) {
        LCD_PutChar(*str++);
    }
}

void LCD_PutChar(char c) {
    LCD_SendData((uint8_t)c);
}

/* ====== Static helper functions ====== */

static void LCD_PulseEnable(void) {
    HAL_GPIO_WritePin(LCD_EN_PORT, LCD_EN_PIN, GPIO_PIN_RESET);
    // delay ngắn, ~1 us
    HAL_Delay(1);
    HAL_GPIO_WritePin(LCD_EN_PORT, LCD_EN_PIN, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(LCD_EN_PORT, LCD_EN_PIN, GPIO_PIN_RESET);
    HAL_Delay(1);
}

static void LCD_Write4Bits(uint8_t data) {
    // D4
    HAL_GPIO_WritePin(LCD_D4_PORT, LCD_D4_PIN,
                      (data & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    // D5
    HAL_GPIO_WritePin(LCD_D5_PORT, LCD_D5_PIN,
                      (data & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    // D6
    HAL_GPIO_WritePin(LCD_D6_PORT, LCD_D6_PIN,
                      (data & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    // D7
    HAL_GPIO_WritePin(LCD_D7_PORT, LCD_D7_PIN,
                      (data & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    LCD_PulseEnable();
}

static void LCD_Send(uint8_t value, uint8_t rs) {
    // rs = 0: command, rs = 1: data
    HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN,
                      rs ? GPIO_PIN_SET : GPIO_PIN_RESET);

    // Gửi high nibble (4 bit cao)
    LCD_Write4Bits(value >> 4);
    // Gửi low nibble (4 bit thấp)
    LCD_Write4Bits(value & 0x0F);
}

void LCD_SendCommand(uint8_t cmd) {
    LCD_Send(cmd, 0);
    HAL_Delay(2); // đảm bảo đủ thời gian xử lý
}

void LCD_SendData(uint8_t data) {
    LCD_Send(data, 1);
    HAL_Delay(1);
}
