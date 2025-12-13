# Electronic Smart Lock System - STM32F103C6

## Project Overview
A complete firmware implementation for an intelligent electronic door lock system using STM32F103C6 microcontroller with password-based authentication, persistent storage, and advanced security features.

---

## 📋 Table of Contents
1. [Features](#features)
2. [Hardware Requirements](#hardware-requirements)
3. [Software Architecture](#software-architecture)
4. [File Structure](#file-structure)
5. [Setup Instructions](#setup-instructions)
6. [User Guide](#user-guide)
7. [Security Features](#security-features)
8. [Troubleshooting](#troubleshooting)
9. [Future Enhancements](#future-enhancements)

---

## ✨ Features

### Core Functionality
- ✅ **Password Authentication**: 4-8 digit password protection
- ✅ **Persistent Storage**: Password stored in I2C EEPROM (survives power loss)
- ✅ **LCD Display**: 16x2 character display with user-friendly interface
- ✅ **Matrix Keypad**: 3x4 keypad for password entry
- ✅ **Electronic Lock**: Relay/solenoid control for door locking mechanism
- ✅ **Audio Feedback**: Active buzzer for alerts and confirmations
- ✅ **Door Status**: Magnetic reed switch for door open/close detection

### Advanced Features
- 🔐 **Security Lockout**: Blocks access for 60 seconds after 3 wrong attempts
- ⏱️ **Auto-Lock**: Automatically locks after 10 seconds if door not opened
- 🚪 **Smart Door Monitoring**: Auto-locks 5 seconds after door closes
- 🚨 **Door Alarm**: Alerts if door remains open for more than 30 seconds
- 🔑 **Password Change**: In-system password modification capability
- 🎯 **Non-Blocking**: Uses HAL_GetTick() for timing (no blocking delays in FSM)

### User Interface
- Password masking with asterisks (*) during input
- Clear visual feedback for all operations
- Countdown timer display during lockout
- Intuitive keypad controls:
  - `#` key: Enter/Submit
  - `*` key: Clear/Cancel
  - `0` key (3x): Access password change mode when unlocked

---

## 🔧 Hardware Requirements

### Components List
| Component | Specification | Quantity | Notes |
|-----------|---------------|----------|-------|
| MCU | STM32F103C6 (32KB Flash, 10KB RAM) | 1 | ARM Cortex-M3, 72MHz |
| Display | LCD 16x2 (HD44780) | 1 | 4-bit mode, I2C optional |
| Keypad | 3x4 Matrix Keypad | 1 | Membrane or mechanical |
| EEPROM | AT24C04 (512B) or AT24C32 (4KB) | 1 | I2C interface |
| Lock | 12V Solenoid Lock or Relay Module | 1 | With driver circuit |
| Buzzer | Active Buzzer (3-5V) | 1 | Piezo or magnetic |
| Sensor | Magnetic Reed Switch | 1 | NO or NC type |
| Transistor | NPN (2N2222, BC547) | 2 | For relay and buzzer |
| Resistors | 4.7kΩ, 1kΩ, 100Ω, 220Ω | Various | Pull-ups and drivers |
| Diode | 1N4007 | 1 | Flyback protection |
| Power Supply | 12V/1A DC Adapter | 1 | With 5V and 3.3V regulators |
| Voltage Regulator | LM7805, AMS1117-3.3 | 2 | For 5V and 3.3V rails |
| Capacitors | 100nF, 10µF, 1000µF | Various | Decoupling and filtering |

### Pin Configuration Summary
```
I2C EEPROM:    PB6 (SCL), PB7 (SDA) + 4.7kΩ pull-ups
LCD 16x2:      PA0-PA5 (RS, EN, D4-D7)
Keypad Rows:   PB0, PB1, PB10, PB11 (Output)
Keypad Cols:   PC13, PC14, PC15 (Input with pull-up)
Relay/Lock:    PA6 (with transistor driver)
Buzzer:        PA7 (with optional transistor)
Door Sensor:   PA8 (with internal pull-up)
```

**⚠️ CRITICAL**: I2C lines (PB6, PB7) **MUST** have 4.7kΩ external pull-up resistors to VCC!

---

## 🏗️ Software Architecture

### State Machine Design
The firmware implements a **Finite State Machine (FSM)** with the following states:

```
┌─────────────┐
│   IDLE      │ ← Initial state: "Enter Password"
└──────┬──────┘
       │ (User enters digits)
       ▼
┌─────────────┐
│INPUT_PASSWORD│ ← Collecting password input
└──────┬──────┘
       │ (Press #)
       ▼
┌─────────────┐
│CHECK_PASSWORD│ ← Validate against EEPROM
└──┬───────┬──┘
   │       │
   │Correct│Wrong
   ▼       ▼
┌────────┐ ┌─────────────┐
│UNLOCKED│ │ LOCKED_OUT  │ (After 3 attempts)
└────┬───┘ └─────────────┘
     │
     │ (Auto/Manual/Door Close)
     ▼
   [IDLE]
     │
     │ (Press 0 three times)
     ▼
┌─────────────────────┐
│CHANGE_PASSWORD_OLD  │ → Verify old password
└──────────┬──────────┘
           ▼
┌─────────────────────┐
│CHANGE_PASSWORD_NEW  │ → Enter new password
└──────────┬──────────┘
           ▼
┌─────────────────────┐
│CHANGE_PASSWORD_CONF │ → Confirm new password
└─────────────────────┘
```

### Non-Blocking Timing
All timing operations use `HAL_GetTick()` to maintain system responsiveness:
- No `HAL_Delay()` calls in FSM logic
- Keypad scanning remains responsive during timeouts
- Door sensor monitoring is continuous
- Lockout countdown updates in real-time

### Module Organization
```
Door-System/
├── Core/
│   ├── Inc/
│   │   ├── main.h              (Main header with configuration)
│   │   ├── lcd16x2.h           (LCD driver interface)
│   │   ├── keypad.h            (Keypad driver interface)
│   │   └── eeprom_i2c.h        (EEPROM driver interface)
│   └── Src/
│       ├── main_complete.c     (Main application with FSM)
│       ├── lcd16x2.c           (LCD implementation)
│       ├── keypad.c            (Keypad implementation)
│       ├── eeprom_i2c.c        (EEPROM implementation)
│       └── stm32f1xx_it.c      (Interrupt handlers)
└── Drivers/                    (STM32 HAL drivers)
```

---

## 📂 File Structure

### Delivered Files
```
door-lock-system/
├── CUBEMX_CONFIGURATION_GUIDE.md   (Detailed CubeMX setup)
├── CONNECTION_DIAGRAM.md            (Hardware wiring guide)
├── README.md                        (This file)
└── Door-System/
    └── Core/
        ├── Inc/
        │   ├── lcd16x2.h
        │   ├── keypad.h
        │   └── eeprom_i2c.h
        └── Src/
            ├── main_complete.c      (⭐ Main application)
            ├── lcd16x2.c
            ├── keypad.c
            └── eeprom_i2c.c
```

### Key Files Description

#### `main_complete.c`
Complete firmware with FSM implementation. Includes:
- System initialization
- State machine logic
- Password management
- Lock control
- Door monitoring
- All timing logic

#### `lcd16x2.c/h`
LCD 16x2 driver (4-bit mode):
- Hardware initialization
- Text display functions
- Cursor control
- Character/string writing

#### `keypad.c/h`
3x4 matrix keypad driver:
- Row scanning algorithm
- Debouncing logic
- Key mapping
- Non-blocking key detection

#### `eeprom_i2c.c/h`
I2C EEPROM driver:
- Byte and buffer read/write
- Page write optimization
- Password storage functions
- Device detection

---

## 🚀 Setup Instructions

### 1. Hardware Assembly
Follow the **CONNECTION_DIAGRAM.md** for detailed wiring instructions. Key points:
- Install 4.7kΩ pull-up resistors on I2C lines (mandatory!)
- Use flyback diode (1N4007) across relay coil
- Connect LCD contrast potentiometer (10kΩ)
- Verify all power rail voltages before MCU connection

### 2. STM32CubeMX Configuration
Follow the **CUBEMX_CONFIGURATION_GUIDE.md** step-by-step:
1. Create new project for STM32F103C6
2. Configure system clock to 72MHz
3. Enable and configure I2C1 (PB6/PB7, 100kHz)
4. Set up GPIO pins as specified
5. Generate code for STM32CubeIDE

### 3. Firmware Integration
1. **Open Generated Project** in STM32CubeIDE
2. **Add Driver Files**:
   ```
   Copy to Core/Inc/:
   - lcd16x2.h
   - keypad.h
   - eeprom_i2c.h
   
   Copy to Core/Src/:
   - lcd16x2.c
   - keypad.c
   - eeprom_i2c.c
   ```

3. **Replace main.c**:
   - Backup original `Core/Src/main.c`
   - Copy `main_complete.c` to `Core/Src/main.c`
   - Or merge USER CODE sections if customized

4. **Update Includes** in main.c:
   ```c
   #include "lcd16x2.h"
   #include "keypad.h"
   #include "eeprom_i2c.h"
   ```

5. **Build Project**:
   - Right-click project → Build Project
   - Resolve any path or configuration issues
   - Verify 0 errors

6. **Flash Firmware**:
   - Connect ST-Link programmer
   - Right-click project → Debug As → STM32 C/C++ Application
   - Or use Flash command

### 4. First Boot Setup
1. **Power on the system**
2. LCD should display: `Smart Lock v1.0` → `Default Pass Set` → `Pass: 1234`
3. Default password **1234** is automatically stored in EEPROM
4. System enters IDLE state: `Enter Password:`

---

## 📖 User Guide

### Normal Operation

#### Unlocking the Door
1. **Enter Password**: Press digits (e.g., `1 2 3 4`)
   - Each digit displays as `*` (masked)
2. **Submit**: Press `#` key
3. **Success**: 
   - LCD shows "UNLOCKED! Welcome!"
   - Relay activates (door unlocks)
   - Single beep
   - Door can be opened within 10 seconds

#### Wrong Password
- LCD shows "WRONG PASSWORD! Attempt X of 3"
- Two short beeps
- After 3 wrong attempts: 60-second lockout
- During lockout: "LOCKED OUT! Wait X sec..."

#### Auto-Lock Behavior
- **Timeout**: If door not opened within 10 seconds → auto-lock
- **Door Close**: After door closes, auto-lock after 5 seconds
- **Door Open Alarm**: If door stays open > 30 seconds → alarm beeps

#### Manual Lock
- When door is unlocked, press `*` key to lock immediately

### Changing Password

#### Steps
1. **Unlock the door** (enter correct password)
2. **Enter Change Mode**: Press `0` key **three times** quickly (within 1 second each)
3. **Enter Old Password**:
   - LCD: "Old Password:"
   - Enter current password + `#`
4. **Enter New Password**:
   - LCD: "New Password:"
   - Enter new password (4-8 digits) + `#`
   - Minimum 4 digits required
5. **Confirm New Password**:
   - LCD: "Confirm Pass:"
   - Re-enter new password + `#`
6. **Success**:
   - LCD: "Password Saved! Success!"
   - Two long beeps
   - System returns to locked state

#### Password Change Notes
- Old password must be correct
- New password: 4-8 digits
- Confirmation must match exactly
- Password is immediately saved to EEPROM
- System locks after password change

### Keypad Controls Summary
| Key | Function | Context |
|-----|----------|---------|
| `0-9` | Enter digit | Password entry |
| `#` | Submit/Enter | Confirm password |
| `*` | Clear | Clear input or manual lock |
| `000` | Change Password | Press 0 three times when unlocked |

---

## 🔒 Security Features

### 1. Persistent Storage
- Password stored in external EEPROM via I2C
- Survives power loss and resets
- Uses AT24C04 (512B) or AT24C32 (4KB)
- Password location: Address 0x0000
- Format: [Length byte][Password bytes]

### 2. Brute Force Protection
- Maximum 3 consecutive wrong attempts
- 60-second lockout after 3rd failure
- Wrong attempt counter resets on correct password
- Countdown displayed during lockout
- Multiple alarm beeps on lockout

### 3. Password Masking
- Input characters displayed as `*`
- Prevents shoulder surfing
- Buffer cleared after use
- No password echo in any state

### 4. Timeout Protection
- Auto-lock if door not opened (10 seconds)
- Auto-lock after door closes (5 seconds)
- Prevents unauthorized access if user forgets

### 5. Door Monitoring
- Continuous door status checking
- Alarm if door left open > 30 seconds
- Prevents forced entry detection
- Automatic lock on door close

### 6. Physical Security
- Relay/solenoid control
- Fail-secure or fail-safe configuration
- Backup mechanical key option (hardware dependent)

---

## 🛠️ Troubleshooting

### LCD Issues

#### Problem: LCD is blank or displays garbage
**Solutions:**
- Adjust contrast potentiometer (LCD pin 3)
- Verify VDD = 5V and VSS = GND
- Check EN pulse timing (LCD_Init sequence)
- Verify all data pins (PA2-PA5) are connected
- Try increasing initialization delays

#### Problem: LCD shows blocks on first line only
**Cause:** LCD not properly initialized
**Solution:** Reset MCU or power cycle. Check LCD_Init() execution.

### Keypad Issues

#### Problem: No response to key presses
**Solutions:**
- Verify row pins (PB0, PB1, PB10, PB11) configured as Output
- Verify column pins (PC13-15) configured as Input with Pull-up
- Check keypad connections (use multimeter in continuity mode)
- Increase debounce delay in `keypad.h`

#### Problem: Keys register multiple times
**Cause:** Insufficient debouncing
**Solution:** Increase `KEYPAD_DEBOUNCE_MS` from 50 to 100

### I2C EEPROM Issues

#### Problem: EEPROM not detected / "EEPROM Error!" on startup
**Solutions:**
- **MOST COMMON**: Missing pull-up resistors on SCL/SDA
  - Measure voltage on PB6/PB7: should be 3.3V when idle
  - Add 4.7kΩ resistors from PB6 to VCC and PB7 to VCC
- Verify EEPROM power (Pin 8 = 3.3V, Pin 4 = GND)
- Check I2C address configuration (A0=A1=A2=GND → 0xA0)
- Test with I2C scanner code
- Verify I2C speed (should be 100kHz, not 400kHz)

#### Problem: Password not saved/loaded
**Solutions:**
- Check EEPROM write delay (5-10ms required)
- Verify I2C communication with oscilloscope
- Try increasing timeout in HAL_I2C functions
- Check for I2C bus errors in debugging

### Relay/Lock Issues

#### Problem: Relay clicks but lock doesn't move
**Solutions:**
- Verify power supply can provide relay coil current
- Check flyback diode polarity (cathode to +V)
- Ensure transistor is properly driven (1kΩ base resistor)
- Test relay independently with direct power
- Verify relay contacts are NO (Normally Open)

#### Problem: Lock activates randomly
**Cause:** Electrical noise or poor grounding
**Solutions:**
- Add 100nF capacitor across relay coil
- Improve grounding (star ground topology)
- Shield relay/solenoid wiring
- Separate power supplies for logic and load

### Door Sensor Issues

#### Problem: Door status always shows "Open" or "Closed"
**Solutions:**
- Verify PA8 configured as Input with Pull-up
- Test reed switch with multimeter (continuity test)
- Check magnet alignment (max gap: 10-15mm)
- Try inverting Door_IsOpen() logic if NC switch

### System Issues

#### Problem: System freezes or resets randomly
**Solutions:**
- Check power supply stability (use oscilloscope)
- Add decoupling capacitors (100nF near MCU)
- Verify stack size is sufficient
- Check for HAL_Delay() in ISRs (not allowed)
- Reduce I2C timeout values

#### Problem: Default password "1234" not working
**Solutions:**
- EEPROM might be corrupted
- Re-flash firmware to reinitialize
- Manually write password using ST-Link utility
- Check EEPROM with I2C scanner

---

## 📊 Technical Specifications

### Timing Parameters
| Parameter | Value | Notes |
|-----------|-------|-------|
| Auto-lock timeout | 10 seconds | If door not opened |
| Door close lock delay | 5 seconds | After door closes |
| Door open alarm | 30 seconds | Continuous open |
| Lockout duration | 60 seconds | After 3 wrong attempts |
| Keypad debounce | 50 ms | Adjustable |
| EEPROM write delay | 10 ms | Per page write |

### Memory Usage (Approximate)
- **Flash**: ~15-20 KB (out of 32 KB available)
- **SRAM**: ~2-3 KB (out of 10 KB available)
- **EEPROM**: 9-16 bytes per password (length + data)

### Power Consumption (Typical @ 3.3V)
- **MCU (active)**: 30-50 mA
- **LCD with backlight**: 20-30 mA
- **EEPROM**: 1 mA
- **Relay coil**: 30-80 mA (depends on type)
- **Solenoid lock**: 200-500 mA (short pulse)
- **Total idle**: ~100 mA
- **Total active (unlocking)**: 400-700 mA

---

## 🔮 Future Enhancements

### Hardware Improvements
1. **RFID/NFC Module**: Add contactless card reader
2. **Fingerprint Sensor**: Biometric authentication
3. **GSM Module**: SMS alerts and remote control
4. **Real-Time Clock**: Time-based access control
5. **Battery Backup**: Maintain operation during power loss
6. **Tamper Detection**: Detect physical attacks
7. **Camera Module**: Capture image on unlock attempts

### Software Enhancements
1. **Multiple Users**: Support 10-20 different passwords
2. **Access Logs**: Store last 100 entry attempts in EEPROM
3. **Admin Mode**: Master password with special privileges
4. **Temporary Codes**: Time-limited guest passwords
5. **OTP Support**: One-time passwords via algorithm
6. **Bluetooth/Wi-Fi**: Smartphone app control
7. **Low Power Mode**: Sleep mode when idle
8. **Firmware Update**: Over-the-air programming capability

### Code Optimizations
1. **RTOS Integration**: FreeRTOS for better multitasking
2. **DMA for I2C**: Faster EEPROM access
3. **Interrupts**: Use EXTI for door sensor and keypad
4. **State Persistence**: Save system state in EEPROM
5. **Encryption**: AES encryption for stored passwords
6. **Watchdog**: Auto-recovery from system hangs

---

## 📚 References

### Datasheets
- [STM32F103C6 Datasheet](https://www.st.com/resource/en/datasheet/stm32f103c6.pdf)
- [HD44780 LCD Controller](https://www.sparkfun.com/datasheets/LCD/HD44780.pdf)
- [AT24C04 EEPROM](https://www.microchip.com/en-us/product/AT24C04)
- [AT24C32 EEPROM](https://www.microchip.com/en-us/product/AT24C32)

### Tools & Resources
- [STM32CubeMX](https://www.st.com/en/development-tools/stm32cubemx.html) - Configuration tool
- [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) - Development environment
- [ST-Link Utility](https://www.st.com/en/development-tools/stsw-link004.html) - Programming tool

### Documentation
- `CUBEMX_CONFIGURATION_GUIDE.md` - Complete CubeMX setup
- `CONNECTION_DIAGRAM.md` - Hardware wiring diagrams
- Inline code comments in all source files

---

## 👨‍💻 Development Information

### Author
**Senior Embedded Software Engineer**

### Version History
- **v1.0** (December 2025) - Initial release
  - Complete FSM implementation
  - I2C EEPROM integration
  - All security features
  - Comprehensive documentation

### License
This project is provided for educational and commercial use.

### Support
For issues, questions, or contributions:
1. Review troubleshooting section
2. Check connection diagrams
3. Verify CubeMX configuration
4. Test individual modules separately

---

## 🎯 Quick Start Checklist

- [ ] Hardware assembled per CONNECTION_DIAGRAM.md
- [ ] 4.7kΩ pull-up resistors installed on I2C lines
- [ ] Power supply voltages verified (3.3V, 5V, 12V)
- [ ] STM32CubeMX configured per guide
- [ ] Driver files copied to project
- [ ] main_complete.c integrated
- [ ] Project builds with 0 errors
- [ ] Firmware flashed to MCU
- [ ] LCD displays startup message
- [ ] Keypad responsive
- [ ] EEPROM detected (no "EEPROM Error!")
- [ ] Default password 1234 works
- [ ] Lock mechanism operates
- [ ] Door sensor detects open/close
- [ ] All states tested

---

## 📝 Notes

### Important Safety Warnings
- ⚠️ This is an educational/DIY project
- ⚠️ Not certified for high-security applications
- ⚠️ Always have mechanical backup key
- ⚠️ Test thoroughly before deployment
- ⚠️ Ensure fail-safe behavior during power loss

### Best Practices
- Regular password changes
- Use long passwords (6-8 digits)
- Keep firmware updated
- Monitor system logs
- Test backup access methods
- Maintain mechanical key access

---

**End of README**

For detailed hardware connections, see `CONNECTION_DIAGRAM.md`  
For STM32CubeMX setup, see `CUBEMX_CONFIGURATION_GUIDE.md`

**Status**: ✅ Production Ready  
**Last Updated**: December 2025  
**Firmware Version**: 1.0
