#ifndef INC_LCD_H_
#define INC_LCD_H_

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif
