# LCD Display Configuration Guide

## Hardware Setup

### I2C LCD Backpack Connection
```
LCD I2C Backpack (PCF8574 or PCF8574A)
├── GND         → GND
├── VCC         → +5V
├── SDA (Data)  → PB7 (I2C1_SDA)
└── SCL (Clock) → PB6 (I2C1_SCL)
```

### Typical I2C Addresses
- **PCF8574**: 0x20 - 0x27 (default: 0x27)
- **PCF8574A**: 0x38 - 0x3F

Current setting in code: **0x27** (defined in `lcd_i2c.h`)

## Software Configuration

### I2C1 Settings (in main.c)
```
Clock Speed: 100 kHz (Standard I2C)
Addressing Mode: 7-bit
DutyCycle: 2 (50%)
```

### LCD I2C Driver Features

**File**: `Core/Src/lcd_i2c.c` and `Core/Inc/lcd_i2c.h`

#### Available Functions:
1. `LCD_I2C_Init()` - Initialize LCD
2. `LCD_I2C_Clear()` - Clear display
3. `LCD_I2C_SetCursor(row, col)` - Set cursor position (row: 0-1, col: 0-15)
4. `LCD_I2C_Print(str)` - Print string
5. `LCD_I2C_PutChar(c)` - Print single character
6. `LCD_I2C_PrintInt(num)` - Print integer
7. `LCD_I2C_BacklightOn()` - Turn backlight on
8. `LCD_I2C_BacklightOff()` - Turn backlight off
9. `LCD_I2C_BacklightToggle()` - Toggle backlight
10. `LCD_I2C_CursorOn()` - Show cursor
11. `LCD_I2C_CursorOff()` - Hide cursor

## Initialization Sequence

### In main.c:
```c
// 1. System clock and peripherals initialized
HAL_Init();
SystemClock_Config();

// 2. GPIO, Timer, and I2C initialized (MX functions)
MX_GPIO_Init();
MX_TIM2_Init();
MX_I2C1_Init();

// 3. Timer interrupt started
HAL_TIM_Base_Start_IT(&htim2);

// 4. System stabilization delay
HAL_Delay(100);

// 5. LCD initialized with test message
LCD_I2C_Init(&hi2c1);

// Test display (shows for 2 seconds)
LCD_I2C_Clear();
LCD_I2C_SetCursor(0, 0);
LCD_I2C_Print("Door Lock Ready");
LCD_I2C_SetCursor(1, 0);
LCD_I2C_Print("Initializing...");
HAL_Delay(2000);

// 6. Password module initialized
password_init();
```

## Troubleshooting

### LCD Not Displaying
1. **Check I2C Address**:
   - Use I2C scanner to find actual device address
   - Most common addresses: 0x27, 0x3F (for PCF8574A)
   - Update `LCD_I2C_ADDR` in `lcd_i2c.h` if needed

2. **Verify I2C Connection**:
   - Check wiring: PB6 (SCL), PB7 (SDA)
   - Ensure pull-up resistors (typically 4.7kΩ on SDA/SCL)
   - Check for loose connections

3. **Check Power**:
   - LCD module needs stable 5V supply
   - Backlight pin might draw significant current

4. **Verify Initialization**:
   - LCD needs 50ms power-on delay before first command
   - Already included in code: `HAL_Delay(50)` in `LCD_I2C_Init()`

5. **Common Issues**:
   - I2C address mismatch → Update `LCD_I2C_ADDR`
   - Missing pull-ups → Add external 4.7kΩ resistors
   - Slow I2C clock → Increase timeout or add delays
   - Display contrast → Adjust potentiometer on backpack (if available)

## Pin Mapping (PCF8574 Backpack)

| PCF8574 Pin | LCD Signal | Function |
|-------------|-----------|----------|
| P0 (bit 0)  | RS        | Register Select |
| P1 (bit 1)  | RW        | Read/Write (hardwired low) |
| P2 (bit 2)  | EN        | Enable (strobe) |
| P3 (bit 3)  | BL        | Backlight |
| P4 (bit 4)  | D4        | Data bit 4 |
| P5 (bit 5)  | D5        | Data bit 5 |
| P6 (bit 6)  | D6        | Data bit 6 |
| P7 (bit 7)  | D7        | Data bit 7 |

## Usage Example in Code

```c
// Clear and display message
LCD_I2C_Clear();
LCD_I2C_SetCursor(0, 0);
LCD_I2C_Print("Password: ");
LCD_I2C_SetCursor(1, 0);
LCD_I2C_Print("****");

// Control backlight
LCD_I2C_BacklightOn();
HAL_Delay(500);
LCD_I2C_BacklightOff();
```

## Note on Alternative LCD Driver

There's also a **direct GPIO-based LCD driver** (`lcd.c`/`lcd.h`) that uses GPIO pins directly:
- PA7 (RS), PA8 (EN), PA9-PA12 (D4-D7)
- Currently **not used** in main.c
- Switch to it by changing include and init function in main.c if needed

## Configuration Files

- **I2C Settings**: `.ioc` file (STM32CubeMX) - configured on PB6/PB7
- **LCD Driver**: `lcd_i2c.c` / `lcd_i2c.h`
- **Main Setup**: `main.c` (lines 96-110)
- **LCD I2C Address**: `lcd_i2c.h` line 19
