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
    // Đợi nguồn ổn định - giảm delay để tránh Proteus crash
    HAL_Delay(20);

    // Đảm bảo RS, EN = 0
    HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_EN_PORT, LCD_EN_PIN, GPIO_PIN_RESET);
    HAL_Delay(2);

    // Khởi tạo chuẩn 4-bit theo datasheet HD44780
    // Bước 1: Gửi 0x03 ba lần (8-bit mode reset)
    LCD_Write4Bits(0x03);
    HAL_Delay(1);
    LCD_Write4Bits(0x03);
    HAL_Delay(1);
    LCD_Write4Bits(0x03);
    HAL_Delay(1);

    // Bước 2: Chuyển sang 4-bit mode
    LCD_Write4Bits(0x02);
    HAL_Delay(1);

    // Function set: 4-bit, 2 line, 5x8 dots
    LCD_SendCommand(0x28);

    // Display control: Display ON, Cursor OFF, Blink OFF
    LCD_SendCommand(0x0C);

    // Entry mode: tăng địa chỉ, không dịch màn hình
    LCD_SendCommand(0x06);

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
    HAL_GPIO_WritePin(LCD_EN_PORT, LCD_EN_PIN, GPIO_PIN_SET);
    // Small delay using NOP instructions instead of HAL_Delay
    for(volatile int i = 0; i < 10; i++); // ~1us delay
    HAL_GPIO_WritePin(LCD_EN_PORT, LCD_EN_PIN, GPIO_PIN_RESET);
    for(volatile int i = 0; i < 10; i++); // ~1us delay
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
    // HAL_Delay(2); // Removed to prevent Proteus crash
}

void LCD_SendData(uint8_t data) {
    LCD_Send(data, 1);
    // HAL_Delay(1); // Removed to prevent Proteus crash
}

