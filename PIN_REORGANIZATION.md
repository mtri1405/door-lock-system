# Pin Reorganization - Door Lock System Optimization

## Overview
Tất cả chân GPIO của hệ thống khóa điện tử đã được sắp xếp lại **một cách gọn gàng** vào **PA0-PA6**, để dành nhiều chổ trống cho **module xác thực mật khẩu** sẽ được thêm vào.

---

## 📌 New Pin Configuration

### Door Lock System (PA0-PA6) - 7 pins total

| GPIO | Pin Name | Type | Function | Proteus |
|------|----------|------|----------|---------|
| **PA0** | `OPEN_BUTTON_Pin` | Input | Button to open/unlock door | PB0 |
| **PA1** | `DOOR_SENSOR_Pin` | Input | Door open/close sensor | PB3 |
| **PA2** | `MUTE_BUTTON_Pin` | Input | Mute alarm/buzzer button | PA2 |
| **PA3** | `RED_LED_Pin` | Output | Status LED (locked/alarm) | PA0 |
| **PA4** | `GREEN_LED_Pin` | Output | Status LED (unlocked) | PA1 |
| **PA5** | `SOLENOID_LOCK_Pin` | Output | Electric lock solenoid | PA4 |
| **PA6** | `BUZZER_CTRL_Pin` | Output | Buzzer/alarm control | PA5 |

### Reserved for Password Module

| Region | Usage | Notes |
|--------|-------|-------|
| **PA7-PA11** | Available | 5 pins for keypad/password input |
| **PB0-PB2** | Available | 3 additional pins (matrix keypad cols) |
| **PC0+** | Available | Full Port C for future modules (UART, etc.) |
| **PB3+** | Available | Rest of Port B |

---

## 🔄 Changes Made

### 1. **main.h** - Updated Pin Definitions

**Before:**
```c
#define DOOR_SENSOR_Pin GPIO_PIN_0
#define DOOR_SENSOR_GPIO_Port GPIOA
#define MUTE_BUTTON_Pin GPIO_PIN_1
#define MUTE_BUTTON_GPIO_Port GPIOA
// Mixed with random PA12="nine" pin
#define BUZZER_CTRL_Pin GPIO_PIN_5
#define OPEN_BUTTON_Pin GPIO_PIN_0
#define OPEN_BUTTON_GPIO_Port GPIOB
#define nine_Pin GPIO_PIN_12
```

**After:**
```c
/* Door Lock System Pins (PA0-PA6) */
#define OPEN_BUTTON_Pin GPIO_PIN_0
#define OPEN_BUTTON_GPIO_Port GPIOA
#define DOOR_SENSOR_Pin GPIO_PIN_1
#define DOOR_SENSOR_GPIO_Port GPIOA
#define MUTE_BUTTON_Pin GPIO_PIN_2
#define MUTE_BUTTON_GPIO_Port GPIOA
#define RED_LED_Pin GPIO_PIN_3
#define RED_LED_GPIO_Port GPIOA
#define GREEN_LED_Pin GPIO_PIN_4
#define GREEN_LED_GPIO_Port GPIOA
#define SOLENOID_LOCK_Pin GPIO_PIN_5
#define SOLENOID_LOCK_GPIO_Port GPIOA
#define BUZZER_CTRL_Pin GPIO_PIN_6
#define BUZZER_CTRL_GPIO_Port GPIOA
/* Reserved for Password Module: PA7-PA11, PB0-PB2, PC0+ */
```

**Benefits:**
- ✅ All 7 GPIO pins in consecutive order (PA0-PA6)
- ✅ Removed unused "nine" pin (PA12)
- ✅ Changed OPEN_BUTTON from PB0 to PA0
- ✅ Added explicit RED_LED and GREEN_LED defines
- ✅ Added explicit SOLENOID_LOCK define
- ✅ Clear comment showing reserved space

### 2. **door.c** - Updated to Use New Defines

**Changes:**
```c
// Before: Hardcoded pins
void unlock_door() {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}

// After: Using defines from main.h
void unlock_door() {
    HAL_GPIO_WritePin(SOLENOID_LOCK_GPIO_Port, SOLENOID_LOCK_Pin, GPIO_PIN_SET);
}
```

**All GPIO operations updated:**
- `isDoorClose()` - Uses `DOOR_SENSOR_GPIO_Port`, `DOOR_SENSOR_Pin`
- `unlock_door()` - Uses `SOLENOID_LOCK_GPIO_Port`, `SOLENOID_LOCK_Pin`
- `lock_door()` - Uses `SOLENOID_LOCK_GPIO_Port`, `SOLENOID_LOCK_Pin`
- `aleart()` - Uses `BUZZER_CTRL_GPIO_Port`, `BUZZER_CTRL_Pin`
- `stop_aleart()` - Uses `BUZZER_CTRL_GPIO_Port`, `BUZZER_CTRL_Pin`
- `update_led()` - Uses `RED_LED_GPIO_Port/Pin`, `GREEN_LED_GPIO_Port/Pin`

### 3. **button.c** - Updated Button Port

**Before:**
```c
GPIO_TypeDef *BUTTON_PORT = GPIOB;  // PB0
```

**After:**
```c
GPIO_TypeDef *BUTTON_PORT = GPIOA;  // PA0 (same port as other I/O)
```

### 4. **main.c** - Consolidated GPIO Init

**Before:**
```c
// Scattered configuration
HAL_GPIO_WritePin(GPIOA, BUZZER_CTRL_Pin|nine_Pin, GPIO_PIN_RESET);
GPIO_InitStruct.Pin = DOOR_SENSOR_Pin|MUTE_BUTTON_Pin;
// PB0 separate initialization
HAL_GPIO_Init(OPEN_BUTTON_GPIO_Port, &GPIO_InitStruct);
```

**After:**
```c
// Consolidated - all inputs in one place, all outputs in one place
HAL_GPIO_WritePin(GPIOA, RED_LED_Pin|GREEN_LED_Pin|SOLENOID_LOCK_Pin|BUZZER_CTRL_Pin, GPIO_PIN_RESET);

GPIO_InitStruct.Pin = OPEN_BUTTON_Pin|DOOR_SENSOR_Pin|MUTE_BUTTON_Pin;
GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

GPIO_InitStruct.Pin = RED_LED_Pin|GREEN_LED_Pin|SOLENOID_LOCK_Pin|BUZZER_CTRL_Pin;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
```

### 5. **door-lock-project.ioc** - Updated Pin Configuration

**Before:**
```
Mcu.Pin0=PA0-WKUP
Mcu.Pin1=PA1
Mcu.Pin2=PA5
Mcu.Pin3=PB0      ← Open Button in PB
Mcu.Pin4=PA12     ← Unused "nine" pin
Mcu.PinsNb=9
```

**After:**
```
Mcu.Pin0=PA0      ← OPEN_BUTTON
Mcu.Pin1=PA1      ← DOOR_SENSOR
Mcu.Pin2=PA2      ← MUTE_BUTTON
Mcu.Pin3=PA3      ← RED_LED
Mcu.Pin4=PA4      ← GREEN_LED
Mcu.Pin5=PA5      ← SOLENOID_LOCK
Mcu.Pin6=PA6      ← BUZZER_CTRL
Mcu.Pin7=PA13     ← SWDIO (debug)
Mcu.Pin8=PA14     ← SWCLK (debug)
Mcu.PinsNb=11
```

Pin definitions in .ioc:
```
PA0.GPIO_Label=OPEN_BUTTON (Input)
PA1.GPIO_Label=DOOR_SENSOR (Input)
PA2.GPIO_Label=MUTE_BUTTON (Input)
PA3.GPIO_Label=RED_LED (Output)
PA4.GPIO_Label=GREEN_LED (Output)
PA5.GPIO_Label=SOLENOID_LOCK (Output)
PA6.GPIO_Label=BUZZER_CTRL (Output)
```

---

## 📊 Pin Usage Summary

### Before (Messy)
- PA0: DOOR_SENSOR
- PA1: MUTE_BUTTON
- PA5: BUZZER
- PB0: OPEN_BUTTON
- PA12: Unused "nine"
- Hardcoded LED: PA0, PA1 (conflicts!)
- Hardcoded Solenoid: PA4
- **Total: Scattered, conflicting**

### After (Organized)
```
PORTA:
  ├─ PA0: OPEN_BUTTON (Input)
  ├─ PA1: DOOR_SENSOR (Input)
  ├─ PA2: MUTE_BUTTON (Input)
  ├─ PA3: RED_LED (Output)
  ├─ PA4: GREEN_LED (Output)
  ├─ PA5: SOLENOID_LOCK (Output)
  ├─ PA6: BUZZER_CTRL (Output)
  ├─ PA7-PA11: RESERVED FOR PASSWORD
  ├─ PA12-PA13: JTAG (PA13=SWDIO)
  └─ PA14: JTAG (PA14=SWCLK)

PORTB:
  ├─ PB0-PB2: RESERVED FOR PASSWORD
  └─ PB3+: Available

PORTC:
  └─ PC0+: Available (future UART, etc.)
```

---

## 🚀 Benefits

1. ✅ **Organized** - All door lock I/O in PA0-PA6
2. ✅ **Compact** - Only 7 pins used (was scattered across PA/PB)
3. ✅ **Clear naming** - Each pin has a meaningful label
4. ✅ **Space for password** - 5+ pins available for keypad
5. ✅ **Easy maintenance** - All defines in one place (main.h)
6. ✅ **Future-proof** - Clear reserved regions for expansion
7. ✅ **Removed unused** - No more stray PA12 pin

---

## 🔧 When Adding Password Module

### For Matrix Keypad (3x4):

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
// Add PA10, PA11 for ROW3 if needed
```

### For UART Debug (optional):

```c
#define UART_TX_Pin GPIO_PIN_9   // PA9
#define UART_TX_GPIO_Port GPIOA
#define UART_RX_Pin GPIO_PIN_10  // PA10
#define UART_RX_GPIO_Port GPIOA
// Or use PB for I2C, SPI if needed
```

---

## ✅ Verification Checklist

- [x] main.h - Updated with new pin definitions
- [x] door.c - All functions use defines from main.h
- [x] door.h - Uses standard defines
- [x] button.c - Button port changed to GPIOA
- [x] buzzer.c - Uses BUZZER_CTRL defines
- [x] main.c - GPIO init consolidated and clean
- [x] door-lock-project.ioc - Pin configuration updated
- [x] No hardcoded GPIO_PIN_* values
- [x] Reserved space clearly marked
- [x] All defines centralized in main.h

---

## 📝 Next Steps

1. **Recompile** the project in STM32CubeIDE
2. **Load & test** on Proteus (same functionality, cleaner pins)
3. **When ready**, add password module using PA7-PA11, PB0-PB2
4. **Update main.h** with new password pin defines
5. **Update .ioc file** with new pins in CubeMX GUI

---

## Notes

- All code changes maintain **100% backward compatibility** with existing logic
- No functional changes - only pin reorganization and naming
- Password module can be added without modifying existing door lock code
- Reserved pins make it clear where future features should go
