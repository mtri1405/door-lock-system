/*
 * button.c
 *
 *  Created on: Oct 31, 2025
 *      Author: MinhTri
 */

#include "button.h"
// Khai báo button
#define open_button_pin GPIO_PIN_1

uint16_t pin_of_buttons[NO_BUTTON] = {
open_button_pin,
};

keyInput buttons[NO_BUTTON];

int isButtonPress(int idx) {
	if (idx < 0 || idx > NO_BUTTON) {
		return -1;
	}
	if (buttons[idx].flag == 1) {
		buttons[idx].flag = 0;
		return 1;
	}
	return 0;
}

int isOpenPress() {
	return isButtonPress(OPEN);
}

void getKeyInput() {
	for (int i = 0; i < NO_BUTTON; i++) {
		buttons[i].keyReg2 = buttons[i].keyReg1;
		buttons[i].keyReg1 = buttons[i].keyReg0;
		buttons[i].keyReg0 = HAL_GPIO_ReadPin(GPIOB, pin_of_buttons[i]);
		if (buttons[i].keyReg1 == buttons[i].keyReg2
				&& buttons[i].keyReg1 == buttons[i].keyReg0) {
			if (buttons[i].keyReg2 != buttons[i].keyReg3) {
				buttons[i].keyReg3 = buttons[i].keyReg2;
				if (buttons[i].keyReg3 == PRESS_STATE) {
					buttons[i].flag = 1;
				}
			} else {
				buttons[i].timeLongPress--;
				if (buttons[i].timeLongPress <= 0) {
					buttons[i].timeLongPress = timeOutForKeyPress;
					if (buttons[i].keyReg3 = PRESS_STATE) {
						buttons[i].flag = 1;
					}
				}
			}
		}
	}
}
