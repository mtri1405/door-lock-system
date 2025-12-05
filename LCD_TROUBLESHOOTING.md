# Quick LCD Troubleshooting Checklist

## ✅ Hardware Verification

- [ ] LCD I2C backpack powered with 5V
- [ ] LCD module connected to backpack
- [ ] I2C pins connected: PB6 (SCL) and PB7 (SDA)
- [ ] Pull-up resistors present on SDA/SCL (4.7kΩ typical)
- [ ] No loose connections or cold solder joints
- [ ] Backpack potentiometer adjusted for contrast (if available)

## ✅ Software Verification

- [ ] I2C1 configured at 100 kHz in main.c
- [ ] LCD_I2C_Init(&hi2c1) called after MX_I2C1_Init()
- [ ] 100ms delay before LCD initialization (already in code)
- [ ] LCD_I2C_ADDR set to correct value (default 0x27)
- [ ] All LCD functions properly included from lcd_i2c.h

## ✅ Quick Debug Steps

### Step 1: Find I2C Address
If LCD not responding, scan for actual I2C address:
```c
// Add to main.c for debugging
uint8_t data = 0;
for(uint8_t addr = 0x20; addr < 0x28; addr++) {
    if(HAL_I2C_IsDeviceReady(&hi2c1, (addr << 1), 1, 100) == HAL_OK) {
        LCD_I2C_ADDR = addr;  // Update address
        // Device found at addr
    }
}
```

### Step 2: Test Backlight
If LCD doesn't respond but backlight works:
- Backlight: Responds to power
- No display: I2C communication issue

### Step 3: Check I2C Communication
Add I2C timeout error handling:
```c
if(HAL_I2C_Master_Transmit(&hi2c1, addr, &data, 1, 1000) != HAL_OK) {
    // I2C error occurred
    Error_Handler();
}
```

### Step 4: Timing Issues
If display shows garbled characters or no response:
- Try increasing delays in LCD_I2C_Init()
- Current delays: 50ms (power-on), 5ms (init), 2ms (command)
- Increase by 5-10ms if needed

## ✅ Common I2C Address Issues

| Device Type | Address Range | Default |
|-------------|---------------|---------|
| PCF8574     | 0x20 - 0x27   | 0x27    |
| PCF8574A    | 0x38 - 0x3F   | 0x3F    |

**How to change**: Edit `LCD_I2C_ADDR` in `lcd_i2c.h` line 19

## ✅ Current System Status

**File**: `main.c` lines 99-108
```c
// System now includes:
1. 100ms stabilization delay
2. LCD initialization
3. Test message display (2 seconds)
4. Password module init
```

The test message **"Door Lock Ready / Initializing..."** will display at startup for verification.

## ✅ If LCD Works

1. Replace test message in main.c with actual startup message
2. Comment out the 2-second delay after testing
3. Proceed with normal password display in password.c

## ✅ Signal Quality Check

If display flickers or is unstable:
1. Check I2C clock: 100kHz (currently correct)
2. Verify power supply stability (5V ±0.5V)
3. Check for electromagnetic interference near cables
4. Consider adding 0.1µF capacitors near backpack power pins

## Quick Implementation

To display custom message from password.c:
```c
LCD_I2C_Clear();
LCD_I2C_SetCursor(0, 0);
LCD_I2C_Print("Line 1");
LCD_I2C_SetCursor(1, 0);
LCD_I2C_Print("Line 2");
```

**Max characters per line**: 16

## Reference Documents
- See `LCD_SETUP_GUIDE.md` for complete configuration
- See password.c for LCD integration examples
