/*
 * lcd.h
 *
 *  Created on: Nov 29, 2025
 *      Author: Gentle Croissant
 */

#ifndef INC_LCD_H_
#define INC_LCD_H_

#include <stdint.h>
#include "global.h"
/* ====== LCD Pin Definitions ====== */
/* Configure these based on your STM32 board */
#define LCD_RS_Pin        GPIO_PIN_7      // RS pin
#define LCD_RS_GPIO_Port  GPIOA

#define LCD_EN_Pin        GPIO_PIN_8      // Enable pin
#define LCD_EN_GPIO_Port  GPIOA

#define LCD_D4_Pin        GPIO_PIN_9      // Data 4
#define LCD_D4_GPIO_Port  GPIOA

#define LCD_D5_Pin        GPIO_PIN_10     // Data 5
#define LCD_D5_GPIO_Port  GPIOA

#define LCD_D6_Pin        GPIO_PIN_11     // Data 6
#define LCD_D6_GPIO_Port  GPIOA

#define LCD_D7_Pin        GPIO_PIN_12     // Data 7
#define LCD_D7_GPIO_Port  GPIOA

/* ====== LCD Functions ====== */

/**
 * @brief Initialize LCD in 4-bit mode
 */
void LCD_Init(void);

/**
 * @brief Clear LCD display
 */
void LCD_Clear(void);

/**
 * @brief Set cursor position
 * @param row: 0 for first line, 1 for second line
 * @param col: 0-15 for 16-char display
 */
void LCD_SetCursor(uint8_t row, uint8_t col);

/**
 * @brief Print string on LCD
 */
void LCD_Print(const char *str);

/**
 * @brief Print single character on LCD
 */
void LCD_PutChar(char c);

#endif /* INC_LCD_H_ */
