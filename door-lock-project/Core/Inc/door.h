/*
 * door.h
 *
 *  Created on: Oct 31, 2025
 *      Author: MinhTri
 */

#ifndef INC_DOOR_H_
#define INC_DOOR_H_

#include "global.h"

int isDoorClose();
int isPasswordCorrect();
void unlock_door();
void lock_door();
void alert();
void stop_alert();
void update_led();
void door_fsm_run(void);

#endif /* INC_DOOR_H_ */
