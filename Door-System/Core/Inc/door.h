/*
 * door.h
 *
 *  Created on: Oct 31, 2025
 *      Author: MinhTri
 */

#ifndef INC_DOOR_H_
#define INC_DOOR_H_

#include "global.h"

#define DOOR_SENSOR_PORT DOOR_SENSOR_GPIO_Port
#define DOOR_SENSOR_PIN  DOOR_SENSOR_Pin
#define DOOR_RED_LED_PORT DOOR_RED_LED_GPIO_Port
#define DOOR_RED_LED_PIN  DOOR_RED_LED_Pin
#define DOOR_GREEN_LED_PORT DOOR_GREEN_LED_GPIO_Port
#define DOOR_GREEN_LED_PIN  DOOR_GREEN_LED_Pin
#define DOOR_SOLENOID_PORT DOOR_SOLENOID_GPIO_Port
#define DOOR_SOLENOID_PIN  DOOR_SOLENOID_Pin

void door_init(void);
void door_fsm_run(void);
int door_is_unlocked_or_open(void);
void door_enable_fsm(void);   // Bật door FSM
void door_disable_fsm(void);  // Tắt door FSM


#endif /* INC_DOOR_H_ */
