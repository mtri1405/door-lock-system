///*
// * buzzer.c
// *
// *  Created on: Nov 8, 2025
// *      Author: thait
// */
//#include "buzzer.h"
//static bool isDoorOpen = false;
//static bool isBuzzerActive = false;
//
///* Định nghĩa private */
//#define DOOR_TIMEOUT_TICKS 300 // 3s
//
///**
// * @brief Hàm chạy logic chính của còi báo động.
// * Hãy gọi hàm này liên tục trong vòng lặp while(1) của main.c
// */
//void Buzzer_Run(void) {
//	//input pullup
//	uint8_t doorState = HAL_GPIO_ReadPin(DOOR_SENSOR_GPIO_Port,
//			DOOR_SENSOR_Pin);
//
//	if (doorState == GPIO_PIN_RESET) // khi nhan thi input = 0, khi nhan tuong duong voi mo cua
//			{
//
//		if (isDoorOpen == false) {
//
//			isDoorOpen = true;
//			setTimer1(DOOR_TIMEOUT_TICKS);
//		}
//
//		if (timer1_flag == 1) {
//
//			isBuzzerActive = true;
//
//			HAL_GPIO_WritePin(BUZZER_CTRL_GPIO_Port, BUZZER_CTRL_Pin,
//					GPIO_PIN_RESET);
//		}
//
//	} else // cua dongg
//	{
//
//		isDoorOpen = false;
//		setTimer1(0);      // Dừng bộ đếm
//		timer1_flag = 0;
//
//		// Nếu buzzer đang kêu thì tắt
//		if (isBuzzerActive == true) {
//			isBuzzerActive = false;
//
//			HAL_GPIO_WritePin(BUZZER_CTRL_GPIO_Port, BUZZER_CTRL_Pin,
//					GPIO_PIN_SET);
//		}
//	}
//	if (HAL_GPIO_ReadPin(MUTE_BUTTON_GPIO_Port, MUTE_BUTTON_Pin) == 0) {
//		HAL_GPIO_WritePin(BUZZER_CTRL_GPIO_Port, BUZZER_CTRL_Pin, GPIO_PIN_SET);
//		timer1_flag = 0;
//		isBuzzerActive = false;
//	}
//
//}
