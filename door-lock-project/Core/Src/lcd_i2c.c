/*
 * lcd_i2c.c
 *
 *  Created on: Dec 5, 2025
 *      Author: Door Lock System
 *  
 *  I2C-based LCD driver for 16x2 LCD with PCF8574 I2C backpack
 *  Supports 4-bit LCD communication through I2C expander
 */

#include "lcd_i2c.h"
#include <stdio.h>

/* Static variables */
static I2C_HandleTypeDef *g_hi2c = NULL;
static uint8_t g_lcd_backlight_state = 1;  /* 1: ON, 0: OFF */
static uint8_t g_lcd_i2c_addr = 0x27;      /* I2C address - can be changed at runtime */

/* LCD Commands */
#define LCD_CMD_CLEAR_DISPLAY       0x01
#define LCD_CMD_RETURN_HOME         0x02
#define LCD_CMD_ENTRY_MODE_SET      0x04
#define LCD_CMD_DISPLAY_CONTROL     0x08
#define LCD_CMD_FUNCTION_SET        0x20
#define LCD_CMD_SET_DDRAM_ADDR      0x80

/* Entry Mode Flags */
#define LCD_ENTRY_INCREMENT         0x02
#define LCD_ENTRY_SHIFT_OFF         0x00

/* Display Control Flags */
#define LCD_DISPLAY_ON              0x04
#define LCD_CURSOR_ON               0x02
#define LCD_CURSOR_OFF              0x00
#define LCD_BLINK_ON                0x01
#define LCD_BLINK_OFF               0x00

/* Function Set Flags */
#define LCD_4BIT_MODE               0x00
#define LCD_2LINE                   0x08
#define LCD_5x8DOTS                 0x00

/* Static helper functions */
static void LCD_I2C_SendByte(uint8_t data);
static void LCD_I2C_WriteBits(uint8_t data, uint8_t en);
static void LCD_I2C_PulseEnable(uint8_t data);
static void LCD_I2C_SendCommand(uint8_t cmd);
static void LCD_I2C_SendData(uint8_t data);

/**
 * @brief Send a byte through I2C
 */
static void LCD_I2C_SendByte(uint8_t data) {
    if (g_hi2c == NULL) return;
    
    HAL_I2C_Master_Transmit(g_hi2c, 
                            (g_lcd_i2c_addr << 1), 
                            &data, 
                            1, 
                            LCD_I2C_TIMEOUT);
}

/**
 * @brief Set I2C address at runtime
 */
void LCD_I2C_SetAddress(uint8_t addr) {
    g_lcd_i2c_addr = addr;
}

/**
 * @brief Get current I2C address
 */
uint8_t LCD_I2C_GetAddress(void) {
    return g_lcd_i2c_addr;
}

/**
 * @brief Write 4 bits with enable control
 */
static void LCD_I2C_WriteBits(uint8_t data, uint8_t en) {
    uint8_t bits = data & 0xF0;  /* Keep high nibble */
    
    if (en)
        bits |= (1 << LCD_EN_PIN);
    else
        bits &= ~(1 << LCD_EN_PIN);
    
    if (g_lcd_backlight_state)
        bits |= (1 << LCD_BACKLIGHT_PIN);
    else
        bits &= ~(1 << LCD_BACKLIGHT_PIN);
    
    /* RW = 0 (write mode - ALWAYS write, never read) */
    bits &= ~(1 << LCD_RW_PIN);
    
    LCD_I2C_SendByte(bits);
}

/**
 * @brief Pulse enable pin
 */
static void LCD_I2C_PulseEnable(uint8_t data) {
    LCD_I2C_WriteBits(data, 1);
    HAL_Delay(1);
    LCD_I2C_WriteBits(data, 0);
    HAL_Delay(1);
}

/**
 * @brief Send command to LCD (4-bit mode)
 */
static void LCD_I2C_SendCommand(uint8_t cmd) {
    uint8_t high_nibble = (cmd >> 4) & 0x0F;
    uint8_t low_nibble = cmd & 0x0F;
    
    /* Set RS = 0 for command */
    uint8_t byte_high = (high_nibble << 4);
    uint8_t byte_low = (low_nibble << 4);
    
    /* Send high nibble */
    LCD_I2C_PulseEnable(byte_high);
    /* Send low nibble */
    LCD_I2C_PulseEnable(byte_low);
    
    HAL_Delay(2);  /* Wait for command execution */
}

/**
 * @brief Send data to LCD (4-bit mode)
 */
static void LCD_I2C_SendData(uint8_t data) {
    uint8_t high_nibble = (data >> 4) & 0x0F;
    uint8_t low_nibble = data & 0x0F;
    
    /* Set RS = 1 for data */
    uint8_t byte_high = ((high_nibble << 4) | (1 << LCD_RS_PIN));
    uint8_t byte_low = ((low_nibble << 4) | (1 << LCD_RS_PIN));
    
    /* Send high nibble */
    LCD_I2C_PulseEnable(byte_high);
    /* Send low nibble */
    LCD_I2C_PulseEnable(byte_low);
    
    HAL_Delay(1);
}

/**
 * @brief Initialize I2C LCD
 */
HAL_StatusTypeDef LCD_I2C_Init(I2C_HandleTypeDef *hi2c) {
    if (hi2c == NULL) {
        return HAL_ERROR;
    }
    
    g_hi2c = hi2c;
    g_lcd_backlight_state = 1;
    
    /* Wait for power to stabilize - critical! */
    HAL_Delay(100);
    
    /* Send initialization byte with backlight on */
    uint8_t init_byte = (1 << LCD_BACKLIGHT_PIN);
    LCD_I2C_SendByte(init_byte);
    HAL_Delay(50);
    
    /* 4-bit initialization sequence - must do 3 times */
    for (int attempt = 0; attempt < 3; attempt++) {
        uint8_t byte = (0x3 << 4) | (1 << LCD_BACKLIGHT_PIN);
        LCD_I2C_WriteBits(byte, 1);  /* EN high */
        HAL_Delay(5);
        LCD_I2C_WriteBits(byte, 0);  /* EN low */
        HAL_Delay(5);
    }
    
    /* Set 4-bit mode */
    uint8_t byte = (0x2 << 4) | (1 << LCD_BACKLIGHT_PIN);
    LCD_I2C_WriteBits(byte, 1);
    HAL_Delay(5);
    LCD_I2C_WriteBits(byte, 0);
    HAL_Delay(5);
    
    /* Function Set: 4-bit, 2 lines, 5x8 dots */
    LCD_I2C_SendCommand(LCD_CMD_FUNCTION_SET | LCD_4BIT_MODE | LCD_2LINE | LCD_5x8DOTS);
    HAL_Delay(5);
    
    /* Display Control: ON, Cursor OFF, Blink OFF */
    LCD_I2C_SendCommand(LCD_CMD_DISPLAY_CONTROL | LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF);
    HAL_Delay(5);
    
    /* Entry Mode: Increment, No Shift */
    LCD_I2C_SendCommand(LCD_CMD_ENTRY_MODE_SET | LCD_ENTRY_INCREMENT | LCD_ENTRY_SHIFT_OFF);
    HAL_Delay(5);
    
    /* Clear Display */
    LCD_I2C_Clear();
    HAL_Delay(10);
    
    return HAL_OK;
}

/**
 * @brief Clear LCD display
 */
void LCD_I2C_Clear(void) {
    LCD_I2C_SendCommand(LCD_CMD_CLEAR_DISPLAY);
    HAL_Delay(2);  /* Clear command needs >1.52ms */
}

/**
 * @brief Set cursor position
 */
void LCD_I2C_SetCursor(uint8_t row, uint8_t col) {
    uint8_t addr;
    
    if (row == 0) {
        addr = 0x00 + col;  /* First line: 0x00-0x0F */
    } else {
        addr = 0x40 + col;  /* Second line: 0x40-0x4F */
    }
    
    LCD_I2C_SendCommand(LCD_CMD_SET_DDRAM_ADDR | addr);
}

/**
 * @brief Print string on LCD
 */
void LCD_I2C_Print(const char *str) {
    if (str == NULL) return;
    
    while (*str) {
        LCD_I2C_SendData((uint8_t)*str++);
    }
}

/**
 * @brief Print single character on LCD
 */
void LCD_I2C_PutChar(char c) {
    LCD_I2C_SendData((uint8_t)c);
}

/**
 * @brief Print integer on LCD
 */
void LCD_I2C_PrintInt(int num) {
    char buffer[16];
    sprintf(buffer, "%d", num);
    LCD_I2C_Print(buffer);
}

/**
 * @brief Turn backlight on
 */
void LCD_I2C_BacklightOn(void) {
    g_lcd_backlight_state = 1;
    uint8_t byte = (1 << LCD_BACKLIGHT_PIN);
    LCD_I2C_SendByte(byte);
}

/**
 * @brief Turn backlight off
 */
void LCD_I2C_BacklightOff(void) {
    g_lcd_backlight_state = 0;
    uint8_t byte = 0;
    LCD_I2C_SendByte(byte);
}

/**
 * @brief Toggle backlight
 */
void LCD_I2C_BacklightToggle(void) {
    g_lcd_backlight_state = !g_lcd_backlight_state;
    
    if (g_lcd_backlight_state) {
        LCD_I2C_BacklightOn();
    } else {
        LCD_I2C_BacklightOff();
    }
}

/**
 * @brief Enable cursor display
 */
void LCD_I2C_CursorOn(void) {
    LCD_I2C_SendCommand(LCD_CMD_DISPLAY_CONTROL | LCD_DISPLAY_ON | LCD_CURSOR_ON | LCD_BLINK_OFF);
}

/**
 * @brief Disable cursor display
 */
void LCD_I2C_CursorOff(void) {
    LCD_I2C_SendCommand(LCD_CMD_DISPLAY_CONTROL | LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF);
}
