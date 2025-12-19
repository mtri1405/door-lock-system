/*
 * software_timer.h
 *
 *  Created on: Nov 8, 2025
 *      Author: thait
 */

#ifndef INC_SOFTWARE_TIMER_H_
#define INC_SOFTWARE_TIMER_H_

#include "global.h"

// Timer flags
extern int timer1_flag;
extern int timer_debounce_flag;
extern int timer_door_flag;
extern int timer_password_flag;
extern int timer_lcd_display_flag;

// Timer setters
void setTimer1(int duration);
void setTimerDebounce(int duration);
void setTimerDoor(int duration);
void setTimerPassword(int duration);
void setTimerLcdDisplay(int duration);
void timerRun(void);

#endif /* INC_SOFTWARE_TIMER_H_ */
