#ifndef INC_LCD_H_
#define INC_LCD_H_

/* Includes ------------------------------------------------------------------*/
#include "global.h"


/* Public Functions ----------------------------------------------------------*/
void LCD_Init(void);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_Print(const char *str);
void LCD_PutChar(char c);
void LCD_SendCommand(uint8_t cmd);
void LCD_SendData(uint8_t data);

//void lcd_init (void);   // initialize lcd
//
//void lcd_send_cmd (char cmd);  // send command to the lcd
//
//void lcd_send_data (char data);  // send data to the lcd
//
//void lcd_send_string (char *str);  // send string to the lcd
//
//void lcd_clear_display (void);	//clear display lcd
//
//void lcd_goto_XY (int row, int col); //set proper location on screen
#endif
