/*
 * password.h
 *
 *  Created on: Nov 10, 2025
 *      Author: Gentle Croissant
 */

#ifndef INC_PASSWORD_H_
#define INC_PASSWORD_H_

#include <stdint.h>
#include "global.h"

/**
 * @brief Initialize password module
 * Must be called once during system startup
 */
void password_init(void);

/**
 * @brief Run password FSM
 * Should be called in main loop
 */
void password_fsm_run(void);

/**
 * @brief Check if password is correct
 * Returns 1 if correct password was just entered, 0 otherwise
 * Clears the flag after reading
 */
int password_is_correct_event(void);

/**
 * @brief Check if system is locked due to too many wrong attempts
 */
int password_is_locked(void);

/**
 * @brief Check if intruder alarm flag is set
 */
int password_is_intruder_alarm(void);

void password_stop_alarm_signal(void);

/**
 * @brief Hiển thị màn hình nhập password
 * Gọi từ door module khi cần quay về trạng thái nhập password
 */
void password_show_input_screen(void);

/**
 * @brief Bật password FSM
 */
void password_enable_fsm(void);

/**
 * @brief Tắt password FSM
 */
void password_disable_fsm(void);

#endif /* INC_PASSWORD_H_ */
