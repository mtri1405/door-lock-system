/*
 * door.h
 *
 *  Created on: Oct 31, 2025
 *      Author: MinhTri
 */

#ifndef INC_DOOR_H_
#define INC_DOOR_H_

#include "global.h"
// Khai báo sensor cửa
#define DOOR_SENSOR_PIN GPIO_PIN_3
#define DOOR_SENSOR_PORT GPIOB

int isDoorClose();
int isPasswordCorrect();
void aleart();
void stop_aleart();
void update_led();
void door_fsm_run(void);

#endif /* INC_DOOR_H_ */
