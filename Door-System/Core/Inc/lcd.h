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


#endif /* INC_LCD_H_ */
