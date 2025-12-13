/*
 * software_timer.h
 *
 *  Created on: Nov 8, 2025
 *      Author: thait
 */

#ifndef INC_SOFTWARE_TIMER_H_
#define INC_SOFTWARE_TIMER_H_

#include "global.h"

extern int timer1_flag;
extern int timer_debounce_flag;

void setTimer1(int duration);
void setTimerDebounce(int duration);
void timerRun(void);

#endif /* INC_SOFTWARE_TIMER_H_ */
