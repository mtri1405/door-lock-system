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

// Timer for door FSM
int timer_door_counter = 0;
int timer_door_flag = 0;

// Timer for password FSM (lockout)
int timer_password_counter = 0;
int timer_password_flag = 0;

// Timer for LCD display messages
int timer_lcd_display_counter = 0;
int timer_lcd_display_flag = 0;

void setTimer1(int duration){
	timer1_counter = duration;
	timer1_flag = 0;
}

void setTimerDebounce(int duration){
	timer_debounce_counter = duration;
	timer_debounce_flag = 0;
}

void setTimerDoor(int duration){
	timer_door_counter = duration;
	timer_door_flag = 0;
}

void setTimerPassword(int duration){
	timer_password_counter = duration;
	timer_password_flag = 0;
}

void setTimerLcdDisplay(int duration){
	timer_lcd_display_counter = duration;
	timer_lcd_display_flag = 0;
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
	
	// Door Timer
	if (timer_door_counter > 0){
		timer_door_counter--;
		if (timer_door_counter <= 0){
			timer_door_flag = 1;
		}
	}
	
	// Password Timer
	if (timer_password_counter > 0){
		timer_password_counter--;
		if (timer_password_counter <= 0){
			timer_password_flag = 1;
		}
	}
	
	// LCD Display Timer
	if (timer_lcd_display_counter > 0){
		timer_lcd_display_counter--;
		if (timer_lcd_display_counter <= 0){
			timer_lcd_display_flag = 1;
		}
	}
}
