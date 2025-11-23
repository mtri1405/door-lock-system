# Quick Reference - Pin Mapping After Reorganization

## Pin Assignment Summary

```
╔════════════════════════════════════════════════════════╗
║      STM32F103C8 - DOOR LOCK SYSTEM PIN MAP          ║
╚════════════════════════════════════════════════════════╝

┌─── INPUT PINS (PA0-PA2) ───────────────────────────────┐
│                                                        │
│  PA0  ◄─ OPEN_BUTTON_Pin          (Push = LOW)         │
│  PA1  ◄─ DOOR_SENSOR_Pin          (Closed = HIGH)      │
│  PA2  ◄─ MUTE_BUTTON_Pin          (Press = LOW)        │
│                                                        │
└────────────────────────────────────────────────────────┘

┌─── OUTPUT PINS (PA3-PA6) ───────────────────────────────┐
│                                                         │
│  PA3  ──► RED_LED_Pin             (On = HIGH)           │
│  PA4  ──► GREEN_LED_Pin           (On = HIGH)           │
│  PA5  ──► SOLENOID_LOCK_Pin       (Unlock = HIGH)       │
│  PA6  ──► BUZZER_CTRL_Pin         (On = HIGH)           │
│                                                         │
└─────────────────────────────────────────────────────────┘

┌─── DEBUG PINS ─────────────────────────────────────────┐
│                                                        │
│  PA13 ──► SWD_JTMS (SWDIO)                             │
│  PA14 ──► SWD_JTCK (SWCLK)                             │
│                                                        │
└────────────────────────────────────────────────────────┘

┌─── RESERVED FOR PASSWORD MODULE ───────────────────────┐
│                                                        │
│  PA7-PA11: 5 pins available                            │
│    Suggested: KEY_COL0/1/2, UART_RX, UART_TX           │
│                                                        │
│  PB0-PB2: 3 pins available                             │
│    Suggested: KEY_ROW0/1/2                             │
│                                                        │
│  PC0+: Full Port C available                           │
│    Suggested: I2C, SPI, or future expansion            │
│                                                        │
└────────────────────────────────────────────────────────┘
```

## Code Usage

### Accessing Pins in Code

```c
// In main.h (all definitions centralized):
#define OPEN_BUTTON_Pin GPIO_PIN_0
#define OPEN_BUTTON_GPIO_Port GPIOA
#define DOOR_SENSOR_Pin GPIO_PIN_1
#define DOOR_SENSOR_GPIO_Port GPIOA
// ... etc

// In door.c (using defines):
void unlock_door() {
    HAL_GPIO_WritePin(SOLENOID_LOCK_GPIO_Port, SOLENOID_LOCK_Pin, GPIO_PIN_SET);
}

void lock_door() {
    HAL_GPIO_WritePin(SOLENOID_LOCK_GPIO_Port, SOLENOID_LOCK_Pin, GPIO_PIN_RESET);
}

int isDoorClose() {
    return (HAL_GPIO_ReadPin(DOOR_SENSOR_GPIO_Port, DOOR_SENSOR_Pin) == GPIO_PIN_SET);
}

// In main.c (GPIO initialization):
void MX_GPIO_Init(void) {
    // All outputs init together
    HAL_GPIO_WritePin(GPIOA, RED_LED_Pin|GREEN_LED_Pin|SOLENOID_LOCK_Pin|BUZZER_CTRL_Pin, GPIO_PIN_RESET);
    
    // All inputs init together
    GPIO_InitStruct.Pin = OPEN_BUTTON_Pin|DOOR_SENSOR_Pin|MUTE_BUTTON_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
```

## Proteus Simulation Setup

When setting up Proteus simulation, map to hardware pins:

| Function | STM32 Pin | Proteus Component |
|----------|-----------|-------------------|
| OPEN_BUTTON | PA0 | Push Button (PB) |
| DOOR_SENSOR | PA1 | Magnetic/Reed Switch |
| MUTE_BUTTON | PA2 | Push Button (PB) |
| RED_LED | PA3 | Red LED + Resistor |
| GREEN_LED | PA4 | Green LED + Resistor |
| SOLENOID_LOCK | PA5 | Relay/FET + Solenoid |
| BUZZER_CTRL | PA6 | Buzzer/Piezo Speaker |

## State Machine LED Indicators

| State | Red LED | Green LED | Buzzer | Action |
|-------|---------|-----------|--------|--------|
| **LOCKED** | ON | OFF | OFF | Door locked, waiting for button |
| **UNLOCKED** | OFF | ON | OFF | Solenoid energized for 200ms |
| **ALARM** | BLINK | OFF | ON | Timeout reached, door still open |

## Timing Configuration

Located in `main.c` TIM2 init:

```c
htim2.Init.Prescaler = 7999;    // Clock divider
htim2.Init.Period = 9;           // Reload value
// Result: 1 ms per tick (1 kHz interrupt frequency)

// In door.c:
#define TIMEOUT 20  // 20 ticks = 200 ms
```

## Adding Password Module (Template)

### Step 1: Define new pins in main.h

```c
/* Password Module Pins */
#define KEY_COL0_Pin GPIO_PIN_7
#define KEY_COL0_GPIO_Port GPIOA
#define KEY_COL1_Pin GPIO_PIN_8
#define KEY_COL1_GPIO_Port GPIOA
#define KEY_COL2_Pin GPIO_PIN_9
#define KEY_COL2_GPIO_Port GPIOA

#define KEY_ROW0_Pin GPIO_PIN_0
#define KEY_ROW0_GPIO_Port GPIOB
#define KEY_ROW1_Pin GPIO_PIN_1
#define KEY_ROW1_GPIO_Port GPIOB
#define KEY_ROW2_Pin GPIO_PIN_2
#define KEY_ROW2_GPIO_Port GPIOB
```

### Step 2: Create password.c

```c
#include "password.h"

void password_init(void) {
    // Initialize keypad GPIO
}

int get_password(void) {
    // Read keypad input
    return 0; // or scanned digit
}

int verify_password(int input_pwd) {
    int correct_pwd = 1234;  // Example
    return (input_pwd == correct_pwd);
}
```

### Step 3: Integrate into main

```c
// In main loop:
while (1) {
    getKeyInput();
    door_fsm_run();
    Buzzer_Run();
    
    // New: Check password module when needed
    if (isOpenPress()) {
        if (get_password() == CORRECT_PASSWORD) {
            // FSM will unlock
        }
    }
    
    HAL_Delay(10);
}
```

## Migration Path

### Phase 1: Door Lock Only (Current)
- PA0-PA6: Door lock system ✅
- isPasswordCorrect() always returns 1

### Phase 2: Add Password Module
- PA0-PA6: Door lock (unchanged)
- PA7-PA11: Password keypad pins
- Update isPasswordCorrect() to call password verification

### Phase 3: Add Debug/Monitoring
- PC0+: UART for debug logging
- Optional: I2C for EEPROM (user management)

## Troubleshooting

If recompiling shows errors:

1. **Undefined reference to GPIO_PIN_X**: Ensure you defined in main.h
2. **GPIO_WritePin with wrong port**: Check `_GPIO_Port` suffix
3. **Pin conflicts**: Verify no duplicate PA0-PA6 assignments
4. **.ioc file issues**: Open in STM32CubeMX, verify pin config matches main.h

## Files Modified

- ✅ `Core/Inc/main.h` - Pin definitions
- ✅ `Core/Src/door.c` - GPIO operations
- ✅ `Core/Src/button.c` - Button port
- ✅ `Core/Src/main.c` - GPIO initialization
- ✅ `door-lock-project.ioc` - CubeMX configuration
- 📄 `PIN_REORGANIZATION.md` - Full documentation

---

**All changes maintain same functionality with cleaner organization!** ✨
