/*
 * button.c
 *
 *  Created on: Oct 31, 2025
 *      Author: MinhTri
 */

#include "button.h"

#define NO_OF_BUTTONS 3
int KeyReg0[NO_OF_BUTTONS] = { NORMAL_STATE, NORMAL_STATE, NORMAL_STATE };
int KeyReg1[NO_OF_BUTTONS] = { NORMAL_STATE, NORMAL_STATE, NORMAL_STATE };
int KeyReg2[NO_OF_BUTTONS] = { NORMAL_STATE, NORMAL_STATE, NORMAL_STATE };
int KeyReg3[NO_OF_BUTTONS] = { NORMAL_STATE, NORMAL_STATE, NORMAL_STATE };
int button_flag[NO_OF_BUTTONS] = { 0, 0, 0 };

// Mapping button (using OPEN_BUTTON_Pin for single button setup)
GPIO_TypeDef *BUTTON_PORT = GPIOA;
uint16_t BUTTON_PIN[NO_OF_BUTTONS] = { OPEN_BUTTON_Pin, OPEN_BUTTON_Pin,
		OPEN_BUTTON_Pin };  // TODO: Update when more buttons are connected

int isButtonPress(int index) {
	if (button_flag[index] == 1) {
		button_flag[index] = 0;
		return 1;
	}
	return 0;
}

int isOpenPress() {
	return isButtonPress(OPEN);
}

void getKeyInput() {
	for (int i = 0; i < NO_OF_BUTTONS; i++) {
		KeyReg0[i] = KeyReg1[i];
		KeyReg1[i] = KeyReg2[i];
		KeyReg2[i] = HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN[i]);

		if ((KeyReg0[i] == KeyReg1[i]) && (KeyReg1[i] == KeyReg2[i])) {
			if (KeyReg3[i] != KeyReg2[i]) {
				KeyReg3[i] = KeyReg2[i];
				if (KeyReg2[i] == PRESS_STATE) {
					button_flag[i] = 1;
				}
			}

		}
	}
}
