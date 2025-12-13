///*
// * fsm.c
// *
// *  Created on: Dec 13, 2025
// *      Author: MinhTri
// */
//
//
//
///**
//  * @brief  Finite State Machine - Main Process
//  * @param  None
//  * @retval None
//  */
//#undef "fsm.c"
//static void FSM_Process(void)
//{
//    char key = Keypad_GetKey();
//    uint32_t currentTime = HAL_GetTick();
//
//    switch (currentState)
//    {
//        case STATE_IDLE:
//        case STATE_INPUT_PASSWORD:
//        {
//            if (key != KEY_NONE)
//            {
//                if (key == KEY_ENTER) // '#' - Submit password
//                {
//                    if (inputIndex > 0)
//                    {
//                        inputBuffer[inputIndex] = '\0';
//                        currentState = STATE_CHECK_PASSWORD;
//                    }
//                }
//                else if (key == KEY_CLEAR) // '*' - Clear input
//                {
//                    Reset_Input_Buffer();
//                    LCD_SetCursor(1, 0);
//                    LCD_WriteString("                "); // Clear line
//                    LCD_SetCursor(1, 0);
//                }
//                else if (inputIndex < PASSWORD_MAX_LENGTH) // Digit key
//                {
//                    inputBuffer[inputIndex++] = key;
//                    inputBuffer[inputIndex] = '\0';
//                    Display_Masked_Input();
//                    currentState = STATE_INPUT_PASSWORD;
//                }
//            }
//            break;
//        }
//
//        case STATE_CHECK_PASSWORD:
//        {
//            if (strcmp(inputBuffer, storedPassword) == 0)
//            {
//                // Correct password
//                wrongAttempts = 0;
//                LCD_Clear();
//                LCD_SetCursor(0, 0);
//                LCD_WriteString("  UNLOCKED!");
//                LCD_SetCursor(1, 0);
//                LCD_WriteString("Welcome!");
//
//                Lock_Control(0); // Unlock
//                Buzzer_Beep(1, 200); // Single beep
//
//                unlockTime = HAL_GetTick();
//                doorOpenTime = 0;
//                currentState = STATE_UNLOCKED;
//            }
//            else
//            {
//                // Wrong password
//                wrongAttempts++;
//                LCD_Clear();
//                LCD_SetCursor(0, 0);
//                LCD_WriteString("WRONG PASSWORD!");
//                LCD_SetCursor(1, 0);
//                char msg[16];
//                sprintf(msg, "Attempt %d of %d", wrongAttempts, MAX_WRONG_ATTEMPTS);
//                LCD_WriteString(msg);
//
//                Buzzer_Beep(2, 100); // Double beep
//                HAL_Delay(2000);
//
//                if (wrongAttempts >= MAX_WRONG_ATTEMPTS)
//                {
//                    // Lockout mode
//                    lockoutStartTime = HAL_GetTick();
//                    currentState = STATE_LOCKED_OUT;
//
//                    LCD_Clear();
//                    LCD_SetCursor(0, 0);
//                    LCD_WriteString("LOCKED OUT!");
//                    LCD_SetCursor(1, 0);
//                    LCD_WriteString("Wait 60 sec...");
//
//                    Buzzer_Beep(5, 100); // Alarm beeps
//                }
//                else
//                {
//                    Reset_Input_Buffer();
//                    LCD_Clear();
//                    LCD_SetCursor(0, 0);
//                    LCD_WriteString("Enter Password:");
//                    LCD_SetCursor(1, 0);
//                    currentState = STATE_IDLE;
//                }
//            }
//            break;
//        }
//
//        case STATE_UNLOCKED:
//        {
//            // Check door sensor
//            if (Door_IsOpen())
//            {
//                if (!isDoorOpen)
//                {
//                    isDoorOpen = 1;
//                    doorOpenTime = HAL_GetTick();
//                    LCD_Clear();
//                    LCD_SetCursor(0, 0);
//                    LCD_WriteString("Door Opened");
//                    LCD_SetCursor(1, 0);
//                    LCD_WriteString("Press * to lock");
//                }
//
//                // Check if door is open too long (30 seconds)
//                if ((HAL_GetTick() - doorOpenTime) > DOOR_OPEN_ALARM_MS)
//                {
//                    LCD_Clear();
//                    LCD_SetCursor(0, 0);
//                    LCD_WriteString("!!! ALARM !!!");
//                    LCD_SetCursor(1, 0);
//                    LCD_WriteString("Close the door!");
//                    Buzzer_Beep(3, 100);
//                    HAL_Delay(2000);
//                    doorOpenTime = HAL_GetTick(); // Reset to avoid continuous alarm
//                }
//            }
//            else // Door is closed
//            {
//                if (isDoorOpen)
//                {
//                    isDoorOpen = 0;
//                    doorCloseTime = HAL_GetTick();
//                    LCD_Clear();
//                    LCD_SetCursor(0, 0);
//                    LCD_WriteString("Door Closed");
//                    LCD_SetCursor(1, 0);
//                    LCD_WriteString("Locking in 5s");
//                }
//
//                // Auto-lock 5 seconds after door closed
//                if (doorCloseTime > 0 && (HAL_GetTick() - doorCloseTime) > DOOR_CLOSE_LOCK_MS)
//                {
//                    Lock_Control(1);
//                    LCD_Clear();
//                    LCD_SetCursor(0, 0);
//                    LCD_WriteString("Auto-Locked");
//                    Buzzer_Beep(1, 100);
//                    HAL_Delay(1500);
//
//                    Reset_Input_Buffer();
//                    LCD_Clear();
//                    LCD_SetCursor(0, 0);
//                    LCD_WriteString("Enter Password:");
//                    LCD_SetCursor(1, 0);
//                    currentState = STATE_IDLE;
//                    doorCloseTime = 0;
//                }
//            }
//
//            // Auto-lock if door not opened within 10 seconds
//            if (!isDoorOpen && (HAL_GetTick() - unlockTime) > AUTO_LOCK_TIME_MS)
//            {
//                Lock_Control(1);
//                LCD_Clear();
//                LCD_SetCursor(0, 0);
//                LCD_WriteString("Timeout!");
//                LCD_SetCursor(1, 0);
//                LCD_WriteString("Re-Locked");
//                Buzzer_Beep(1, 100);
//                HAL_Delay(1500);
//
//                Reset_Input_Buffer();
//                LCD_Clear();
//                LCD_SetCursor(0, 0);
//                LCD_WriteString("Enter Password:");
//                LCD_SetCursor(1, 0);
//                currentState = STATE_IDLE;
//            }
//
//            // Manual lock with '*' key
//            if (key == KEY_CLEAR)
//            {
//                Lock_Control(1);
//                LCD_Clear();
//                LCD_SetCursor(0, 0);
//                LCD_WriteString("Manual Lock");
//                Buzzer_Beep(1, 100);
//                HAL_Delay(1000);
//
//                Reset_Input_Buffer();
//                LCD_Clear();
//                LCD_SetCursor(0, 0);
//                LCD_WriteString("Enter Password:");
//                LCD_SetCursor(1, 0);
//                currentState = STATE_IDLE;
//            }
//
//            // Change password mode (press '0' three times)
//            static uint8_t zeroCount = 0;
//            static uint32_t lastZeroTime = 0;
//
//            if (key == '0')
//            {
//                if ((HAL_GetTick() - lastZeroTime) < 1000)
//                    zeroCount++;
//                else
//                    zeroCount = 1;
//
//                lastZeroTime = HAL_GetTick();
//
//                if (zeroCount >= 3)
//                {
//                    zeroCount = 0;
//                    Reset_Input_Buffer();
//                    LCD_Clear();
//                    LCD_SetCursor(0, 0);
//                    LCD_WriteString("Old Password:");
//                    LCD_SetCursor(1, 0);
//                    currentState = STATE_CHANGE_PASSWORD_OLD;
//                }
//            }
//            break;
//        }
//
//        case STATE_LOCKED_OUT:
//        {
//            uint32_t elapsed = HAL_GetTick() - lockoutStartTime;
//            uint32_t remaining = (LOCKOUT_TIME_MS - elapsed) / 1000;
//
//            if (elapsed >= LOCKOUT_TIME_MS)
//            {
//                // Lockout period expired
//                wrongAttempts = 0;
//                Reset_Input_Buffer();
//                LCD_Clear();
//                LCD_SetCursor(0, 0);
//                LCD_WriteString("Enter Password:");
//                LCD_SetCursor(1, 0);
//                currentState = STATE_IDLE;
//            }
//            else
//            {
//                // Update countdown every second
//                static uint32_t lastUpdate = 0;
//                if ((HAL_GetTick() - lastUpdate) >= 1000)
//                {
//                    LCD_SetCursor(1, 0);
//                    char msg[16];
//                    sprintf(msg, "Wait %ld sec...  ", remaining);
//                    LCD_WriteString(msg);
//                    lastUpdate = HAL_GetTick();
//                }
//            }
//            break;
//        }
//
//        case STATE_CHANGE_PASSWORD_OLD:
//        {
//            if (key != KEY_NONE)
//            {
//                if (key == KEY_ENTER)
//                {
//                    if (inputIndex > 0)
//                    {
//                        inputBuffer[inputIndex] = '\0';
//
//                        if (strcmp(inputBuffer, storedPassword) == 0)
//                        {
//                            // Old password correct
//                            Reset_Input_Buffer();
//                            LCD_Clear();
//                            LCD_SetCursor(0, 0);
//                            LCD_WriteString("New Password:");
//                            LCD_SetCursor(1, 0);
//                            currentState = STATE_CHANGE_PASSWORD_NEW;
//                        }
//                        else
//                        {
//                            // Wrong old password
//                            LCD_Clear();
//                            LCD_SetCursor(0, 0);
//                            LCD_WriteString("Wrong Password!");
//                            Buzzer_Beep(2, 100);
//                            HAL_Delay(2000);
//
//                            Lock_Control(1);
//                            Reset_Input_Buffer();
//                            LCD_Clear();
//                            LCD_SetCursor(0, 0);
//                            LCD_WriteString("Enter Password:");
//                            LCD_SetCursor(1, 0);
//                            currentState = STATE_IDLE;
//                        }
//                    }
//                }
//                else if (key == KEY_CLEAR)
//                {
//                    Reset_Input_Buffer();
//                    LCD_SetCursor(1, 0);
//                    LCD_WriteString("                ");
//                    LCD_SetCursor(1, 0);
//                }
//                else if (inputIndex < PASSWORD_MAX_LENGTH)
//                {
//                    inputBuffer[inputIndex++] = key;
//                    inputBuffer[inputIndex] = '\0';
//                    Display_Masked_Input();
//                }
//            }
//            break;
//        }
//
//        case STATE_CHANGE_PASSWORD_NEW:
//        {
//            if (key != KEY_NONE)
//            {
//                if (key == KEY_ENTER)
//                {
//                    if (inputIndex >= 4) // Minimum 4 digits
//                    {
//                        inputBuffer[inputIndex] = '\0';
//                        strcpy(newPassword, inputBuffer);
//                        Reset_Input_Buffer();
//
//                        LCD_Clear();
//                        LCD_SetCursor(0, 0);
//                        LCD_WriteString("Confirm Pass:");
//                        LCD_SetCursor(1, 0);
//                        currentState = STATE_CHANGE_PASSWORD_CONFIRM;
//                    }
//                    else
//                    {
//                        LCD_Clear();
//                        LCD_SetCursor(0, 0);
//                        LCD_WriteString("Min 4 digits!");
//                        HAL_Delay(1500);
//                        LCD_Clear();
//                        LCD_SetCursor(0, 0);
//                        LCD_WriteString("New Password:");
//                        LCD_SetCursor(1, 0);
//                        Display_Masked_Input();
//                    }
//                }
//                else if (key == KEY_CLEAR)
//                {
//                    Reset_Input_Buffer();
//                    LCD_SetCursor(1, 0);
//                    LCD_WriteString("                ");
//                    LCD_SetCursor(1, 0);
//                }
//                else if (inputIndex < PASSWORD_MAX_LENGTH)
//                {
//                    inputBuffer[inputIndex++] = key;
//                    inputBuffer[inputIndex] = '\0';
//                    Display_Masked_Input();
//                }
//            }
//            break;
//        }
//
//        case STATE_CHANGE_PASSWORD_CONFIRM:
//        {
//            if (key != KEY_NONE)
//            {
//                if (key == KEY_ENTER)
//                {
//                    if (inputIndex > 0)
//                    {
//                        inputBuffer[inputIndex] = '\0';
//
//                        if (strcmp(inputBuffer, newPassword) == 0)
//                        {
//                            // Passwords match - save to EEPROM
//                            strcpy(storedPassword, newPassword);
//                            Save_Password_To_EEPROM(storedPassword);
//
//                            LCD_Clear();
//                            LCD_SetCursor(0, 0);
//                            LCD_WriteString("Password Saved!");
//                            LCD_SetCursor(1, 0);
//                            LCD_WriteString("Success!");
//                            Buzzer_Beep(2, 200);
//                            HAL_Delay(2000);
//
//                            Lock_Control(1);
//                            Reset_Input_Buffer();
//                            LCD_Clear();
//                            LCD_SetCursor(0, 0);
//                            LCD_WriteString("Enter Password:");
//                            LCD_SetCursor(1, 0);
//                            currentState = STATE_IDLE;
//                        }
//                        else
//                        {
//                            // Passwords don't match
//                            LCD_Clear();
//                            LCD_SetCursor(0, 0);
//                            LCD_WriteString("Not Match!");
//                            Buzzer_Beep(3, 100);
//                            HAL_Delay(2000);
//
//                            Reset_Input_Buffer();
//                            LCD_Clear();
//                            LCD_SetCursor(0, 0);
//                            LCD_WriteString("New Password:");
//                            LCD_SetCursor(1, 0);
//                            currentState = STATE_CHANGE_PASSWORD_NEW;
//                        }
//                    }
//                }
//                else if (key == KEY_CLEAR)
//                {
//                    Reset_Input_Buffer();
//                    LCD_SetCursor(1, 0);
//                    LCD_WriteString("                ");
//                    LCD_SetCursor(1, 0);
//                }
//                else if (inputIndex < PASSWORD_MAX_LENGTH)
//                {
//                    inputBuffer[inputIndex++] = key;
//                    inputBuffer[inputIndex] = '\0';
//                    Display_Masked_Input();
//                }
//            }
//            break;
//        }
//    }
//}
//
///**
//  * @brief  Control lock relay
//  * @param  state: 1 = Locked, 0 = Unlocked
//  * @retval None
//  */
//static void Lock_Control(uint8_t state)
//{
//    if (state)
//    {
//        HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_RESET); // Lock
//        isLocked = 1;
//    }
//    else
//    {
//        HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_SET); // Unlock
//        isLocked = 0;
//    }
//}
//
///**
//  * @brief  Generate buzzer beeps
//  * @param  count: Number of beeps
//  * @param  duration: Duration of each beep in ms
//  * @retval None
//  */
//static void Buzzer_Beep(uint8_t count, uint16_t duration)
//{
//    for (uint8_t i = 0; i < count; i++)
//    {
//        HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
//        HAL_Delay(duration);
//        HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
//        if (i < count - 1)
//            HAL_Delay(duration);
//    }
//}
//
///**
//  * @brief  Check if door is open
//  * @param  None
//  * @retval 1 if open, 0 if closed
//  */
//static uint8_t Door_IsOpen(void)
//{
//    // Door sensor: LOW = Closed, HIGH = Open (with pull-up)
//    return (HAL_GPIO_ReadPin(DOOR_SENSOR_PORT, DOOR_SENSOR_PIN) == GPIO_PIN_SET) ? 1 : 0;
//}
//
///**
//  * @brief  Display masked password input
//  * @param  None
//  * @retval None
//  */
//static void Display_Masked_Input(void)
//{
//    LCD_SetCursor(1, 0);
//    for (uint8_t i = 0; i < inputIndex; i++)
//    {
//        LCD_WriteChar('*');
//    }
//}
//
///**
//  * @brief  Load password from EEPROM
//  * @param  None
//  * @retval None
//  */
//static void Load_Password_From_EEPROM(void)
//{
//    if (EEPROM_ReadPassword(&hi2c1, storedPassword) != HAL_OK)
//    {
//        storedPassword[0] = '\0'; // Empty on error
//    }
//}
//
///**
//  * @brief  Save password to EEPROM
//  * @param  password: Password string to save
//  * @retval None
//  */
//static void Save_Password_To_EEPROM(const char* password)
//{
//    EEPROM_WritePassword(&hi2c1, password);
//}
//
///**
//  * @brief  Reset input buffer
//  * @param  None
//  * @retval None
//  */
//static void Reset_Input_Buffer(void)
//{
//    memset(inputBuffer, 0, sizeof(inputBuffer));
//    inputIndex = 0;
//}
//
