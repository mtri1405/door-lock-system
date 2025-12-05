/*
 * lcd_i2c.h
 *
 *  Created on: Dec 5, 2025
 *      Author: Door Lock System
 *  
 *  I2C-based LCD driver for 16x2 LCD with I2C backpack
 *  Connection: GND, VCC, SDA (PB7), SCL (PB6)
 *  Default I2C Address: 0x27 (can vary 0x20-0x27)
 */

#ifndef INC_LCD_I2C_H_
#define INC_LCD_I2C_H_

#include <stdint.h>
#include "main.h"

/* I2C Configuration */
#define LCD_I2C_ADDR        0x27     /* Default I2C address - can be 0x20-0x27 or 0x38-0x3F */
#define LCD_I2C_TIMEOUT     1000     /* I2C timeout in ms */

/* Function to set I2C address at runtime */
void LCD_I2C_SetAddress(uint8_t addr);

/* Function to get current I2C address */
uint8_t LCD_I2C_GetAddress(void);

/* LCD Backpack Pin Mapping (PCF8574) */
#define LCD_BACKLIGHT_PIN   3        /* Backlight on P3 */
#define LCD_EN_PIN          2        /* Enable (EN) on P2 */
#define LCD_RW_PIN          1        /* Read/Write on P1 */
#define LCD_RS_PIN          0        /* Register Select on P0 */
#define LCD_D7_PIN          7        /* Data D7 on P7 */
#define LCD_D6_PIN          6        /* Data D6 on P6 */
#define LCD_D5_PIN          5        /* Data D5 on P5 */
#define LCD_D4_PIN          4        /* Data D4 on P4 */

/* Function Prototypes */

/**
 * @brief Initialize I2C LCD
 * @param hi2c: Pointer to I2C handle
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef LCD_I2C_Init(I2C_HandleTypeDef *hi2c);

/**
 * @brief Clear LCD display
 */
void LCD_I2C_Clear(void);

/**
 * @brief Set cursor position
 * @param row: 0 for first line, 1 for second line
 * @param col: 0-15 for 16-char display
 */
void LCD_I2C_SetCursor(uint8_t row, uint8_t col);

/**
 * @brief Print string on LCD
 * @param str: String to print
 */
void LCD_I2C_Print(const char *str);

/**
 * @brief Print single character on LCD
 * @param c: Character to print
 */
void LCD_I2C_PutChar(char c);

/**
 * @brief Print integer on LCD
 * @param num: Integer to print
 */
void LCD_I2C_PrintInt(int num);

/**
 * @brief Turn backlight on
 */
void LCD_I2C_BacklightOn(void);

/**
 * @brief Turn backlight off
 */
void LCD_I2C_BacklightOff(void);

/**
 * @brief Toggle backlight
 */
void LCD_I2C_BacklightToggle(void);

/**
 * @brief Enable cursor
 */
void LCD_I2C_CursorOn(void);

/**
 * @brief Disable cursor
 */
void LCD_I2C_CursorOff(void);

#endif /* INC_LCD_I2C_H_ */
