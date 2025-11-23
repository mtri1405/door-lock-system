/*
 * software_timer.c
 *
 *  Created on: Nov 8, 2025
 *      Author: thait
 */
#include "software_timer.h"

// Timer for buzzer/door
int timer1_counter = 0;
int timer1_flag = 0;

// Timer for button debounce
int timer_debounce_counter = 0;
int timer_debounce_flag = 0;

void setTimer1(int duration){
	timer1_counter = duration;
	timer1_flag = 0;
}

void setTimerDebounce(int duration){
	timer_debounce_counter = duration;
	timer_debounce_flag = 0;
}

void timerRun(){
	// Timer 1
	if (timer1_counter > 0){
		timer1_counter--;
		if (timer1_counter <= 0){
			timer1_flag = 1;
		}
	}
	
	// Debounce Timer
	if (timer_debounce_counter > 0){
		timer_debounce_counter--;
		if (timer_debounce_counter <= 0){
			timer_debounce_flag = 1;
		}
	}
}
