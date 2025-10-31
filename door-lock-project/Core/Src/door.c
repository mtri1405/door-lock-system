/*
 * door.c
 *
 *  Created on: Oct 31, 2025
 *      Author: MinhTri
 */

#include "door.h"


typedef enum {
    DOOR_LOCKED,
    DOOR_UNLOCKED,
    DOOR_WAIT_TO_CLOSE,
    DOOR_ALARM
} DoorState;

/*
 * Lock state
 */
#define LOCK 0
#define OPEN 1

// TimeOut for Closing 20 Tick
#define TIMEOUT 20
/*
 Nếu cửa mở state = 1
 Nếu cửa đóng state = 0
 */

DoorState door_state = DOOR_LOCKED;
int open_timer = 0;

// Kiểm tra xem cửa có đóng không
int isDoorClose(){
	return (HAL_GPIO_ReadPin(DOOR_SENSOR_PORT, DOOR_SENSOR_PIN) == GPIO_PIN_SET);
}

int isPasswordCorrect() {
	// TODO Kiểm tra mật khẩu có đúng không
	return 1;
}
// Bật báo động
void aleart() {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);	// Giả sử đây là còi cảnh báo
}
// Tắt báo động
void stop_aleart() {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}


void update_led() {
    switch (door_state) {
    case DOOR_LOCKED:
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);   // LED đỏ
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET); // LED xanh
        break;
    case DOOR_UNLOCKED:
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
        break;
    case DOOR_ALARM:
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0); // nháy đỏ
        break;
    }
}

void door_fsm_run(void){
	switch(door_state){
	case DOOR_LOCKED:
		if (isOpenPress() && isPasswordCorrect()){
			door_state = DOOR_UNLOCKED;
			open_timer = TIMEOUT;
			stop_aleart();
			update_led();
		}
		break;
	case DOOR_UNLOCKED:
		if (open_timer > 0){
			open_timer--;

		} else {
			if (!isDoorClose()){
				door_state = DOOR_ALARM;
				aleart();
			} else{
				door_state = DOOR_LOCKED;
				stop_aleart();
			}
		}
		update_led();
		break;
	case DOOR_ALARM:
		if (isDoorClose()){
			door_state = DOOR_LOCKED;
			stop_aleart();
		}
		update_led();
		break;
	default:
		door_state = DOOR_LOCKED;
		break;
	}
}
