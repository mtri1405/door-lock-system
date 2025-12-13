///*
// * door.c
// *
// *  Created on: Oct 31, 2025
// *      Author: MinhTri
// */
//
//#include "door.h"
//#include "password.h"
//
//typedef enum {
//    DOOR_LOCKED,
//    DOOR_UNLOCKED,
//    DOOR_WAIT_TO_CLOSE,
//    DOOR_ALARM
//} DoorState;
//
///*
// * Lock state
// */
//#define STATE_LOCK 0
//#define STATE_OPEN 1
//
//// TimeOut for Closing 20 Tick
//#define TIMEOUT 20
///*
// Nếu cửa mở state = 1
// Nếu cửa đóng state = 0
// */
//
//DoorState door_state = DOOR_LOCKED;
//int open_timer = 0;
//
//// Kiểm tra xem cửa có đóng không
//int isDoorClose(){
//	return (HAL_GPIO_ReadPin(DOOR_SENSOR_GPIO_Port, DOOR_SENSOR_Pin) == GPIO_PIN_SET);
//}
//
//int isPasswordCorrect() {
//	// TODO Kiểm tra mật khẩu có đúng không
//	return password_is_correct_event();
//}
//
//// Mở khóa cửa (điều khiển solenoid)
//void unlock_door() {
//	HAL_GPIO_WritePin(SOLENOID_LOCK_GPIO_Port, SOLENOID_LOCK_Pin, GPIO_PIN_SET);
//}
//
//// Khóa cửa
//void lock_door() {
//	HAL_GPIO_WritePin(SOLENOID_LOCK_GPIO_Port, SOLENOID_LOCK_Pin, GPIO_PIN_RESET);
//}
//
//// Bật báo động
//void aleart() {
//	HAL_GPIO_WritePin(BUZZER_CTRL_GPIO_Port, BUZZER_CTRL_Pin, GPIO_PIN_SET);
//}
//
//// Tắt báo động
//void stop_aleart() {
//	HAL_GPIO_WritePin(BUZZER_CTRL_GPIO_Port, BUZZER_CTRL_Pin, GPIO_PIN_RESET);
//}
//
//
//void update_led() {
//    switch (door_state) {
//    case DOOR_LOCKED:
//        HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_SET);     // RED LED on
//        HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_RESET); // GREEN LED off
//        break;
//    case DOOR_UNLOCKED:
//        HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_RESET);
//        HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_SET);   // GREEN LED on
//        break;
//    case DOOR_ALARM:
//        HAL_GPIO_TogglePin(RED_LED_GPIO_Port, RED_LED_Pin);  // RED LED blink
//        break;
//    }
//}
//
//void door_fsm_run(void){
//	switch(door_state){
//	case DOOR_LOCKED:
//		lock_door();
//		if (isOpenPress() && isPasswordCorrect()){
//			door_state = DOOR_UNLOCKED;
//			unlock_door();
//			open_timer = TIMEOUT;
//			stop_aleart();
//			update_led();
//		}
//		break;
//	case DOOR_UNLOCKED:
//		if (open_timer > 0){
//			open_timer--;
//
//		} else {
//			if (isDoorClose()){  // Door is closed
//				door_state = DOOR_LOCKED;
//				lock_door();
//				stop_aleart();
//			} else {  // Door is open after timeout
//				door_state = DOOR_ALARM;
//				aleart();
//			}
//		}
//		update_led();
//		break;
//	case DOOR_ALARM:
//		if (isDoorClose()){
//			door_state = DOOR_LOCKED;
//			lock_door();
//			stop_aleart();
//		}
//		update_led();
//		break;
//	default:
//		door_state = DOOR_LOCKED;
//		lock_door();
//		break;
//	}
//}
