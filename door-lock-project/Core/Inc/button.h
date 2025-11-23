/*
 * button.h
 *
 *  Created on: Oct 31, 2025
 *      Author: MinhTri
 */

#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_
#include "global.h"

#define NO_BUTTON 1
#define PRESS_STATE SET
#define NORMAL_STATE RESET
#define timeOutForKeyPress 50
// Khai bao nut bam
#define OPEN 0

typedef struct {
	int keyReg0;
	int keyReg1;
	int keyReg2;
	int keyReg3;
	int timeLongPress;
	int flag;
} keyInput;

int isButtonPress(int);
int isOpenPress();

void getKeyInput();
#endif /* INC_BUTTON_H_ */
