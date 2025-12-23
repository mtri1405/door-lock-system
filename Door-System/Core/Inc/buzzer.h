/*
 * buzzer.h
 *
 *  Created on: Nov 8, 2025
 *      Author: thait
 */

#ifndef INC_BUZZER_H_
#define INC_BUZZER_H_

#include "global.h"

/* Public API */
void Buzzer_Init(void);
void Buzzer_Run(void);
void Buzzer_Activate(void);
void Buzzer_Deactivate(void);
int Buzzer_IsActive(void);  // Kiểm tra buzzer có đang active không

#endif /* INC_BUZZER_H_ */
