## ⚡ Quick Test Guide for Proteus

### 🎯 System Flow (Quy Trình Hoạt Động)

```
[DOOR LOCKED] (Red LED ON)
    ↓
[Press OPEN_BUTTON] → isPasswordCorrect() = 1 (currently always true)
    ↓
[DOOR UNLOCKED] (Green LED ON) + Solenoid opens (PA4 = SET)
    ↓
[Wait 20 ticks = 200ms]
    ↓
    ├─ If door is CLOSED (DOOR_SENSOR = SET)
    │  └─ → [DOOR LOCKED] (auto-close)
    │
    └─ If door is still OPEN (DOOR_SENSOR = RESET)
       └─ → [DOOR ALARM] (Red LED blink) + Buzzer ON
            ↓
           [Press MUTE_BUTTON or close DOOR_SENSOR]
            ↓
           [DOOR LOCKED] (back to normal)
```

### 🔧 Components to Simulate in Proteus

| Component | Pin | Type | Initial State |
|-----------|-----|------|----------------|
| **OPEN Button** | PB0 | Digital Input | HIGH (release) |
| **MUTE Button** | PA2 | Digital Input | HIGH (release) |
| **Door Sensor** | PB3 | Digital Input | HIGH (closed) |
| **Solenoid** | PA4 | Digital Output | LOW (locked) |
| **Buzzer** | PA5 | Digital Output | HIGH (off) |
| **Red LED** | PA0 | Digital Output | LOW (off) |
| **Green LED** | PA1 | Digital Output | LOW (off) |

### ✅ Test Cases

**Test 1: Basic Button Press**
- Input: Press OPEN_BUTTON (PB0 goes LOW)
- Expected Output:
  - PA1 (Green LED) = HIGH → Green LED lights
  - PA4 (Solenoid) = SET → Door unlocks
  - Status: DOOR_UNLOCKED state

**Test 2: Auto Lock on Door Close**
- Precondition: Door in UNLOCKED state (from Test 1)
- Wait: 200ms (20 ticks × 10ms delay)
- Input: Set PB3 (Door Sensor) = SET (door closes)
- Expected Output:
  - PA4 (Solenoid) = RESET → Door locks
  - PA0 (Red LED) = HIGH → Red LED lights
  - PA1 (Green LED) = LOW → Green LED off
  - Status: DOOR_LOCKED state

**Test 3: Alarm on Timeout**
- Precondition: Door in UNLOCKED state
- Wait: 200ms timeout
- Input: Keep PB3 (Door Sensor) = RESET (door stays open)
- Expected Output:
  - PA5 (Buzzer) = SET → Buzzer sounds
  - PA0 (Red LED) = TOGGLE → Red LED blinks
  - Status: DOOR_ALARM state

**Test 4: Mute Alarm**
- Precondition: Door in ALARM state (from Test 3)
- Input: Press MUTE_BUTTON (PA2 = LOW)
- Expected Output:
  - PA5 (Buzzer) = RESET → Buzzer stops
  - isBuzzerActive = false
  - LED blink stops

**Test 5: Recover from Alarm**
- Precondition: Door in ALARM state (from Test 3)
- Input: Set PB3 (Door Sensor) = SET (close door)
- Expected Output:
  - PA5 (Buzzer) = RESET → Buzzer stops
  - PA0 (Red LED) = SET → Red LED on (no blink)
  - PA4 (Solenoid) = RESET → Door locked
  - Status: DOOR_LOCKED state

### 📊 Main Loop Execution (mỗi 10ms)

```c
while(1) {
    1. getKeyInput()      // Read & debounce buttons
    2. door_fsm_run()     // Update door state machine
    3. Buzzer_Run()       // Handle buzzer logic
    4. HAL_Delay(10)      // Wait 10ms
}
```

### 🔄 Timer Interrupt (mỗi 1ms)

```c
void HAL_TIM_PeriodElapsedCallback() {
    timerRun()  // Decrement timers
    // - timer1_counter (for buzzer/door timeout)
    // - timer_debounce_counter (for button debounce)
}
```

### 📝 Debug Output (nếu có UART)

```
Current State: DOOR_LOCKED
Button Status: NOT_PRESSED
Timer1: 0
---
Current State: DOOR_UNLOCKED
Button Status: PRESSED
Timer1: 200
---
Current State: DOOR_ALARM
Buzzer: ON
Timer1: TIMEOUT
```

### ⚠️ Known Limitations (Giới Hạn Hiện Tại)

1. **isPasswordCorrect()** - Hiện luôn trả về 1 (đúng)
   - Cần thêm module xác thực mật khẩu thực tế

2. **Solenoid Control** - Sử dụng GPIO digital
   - Có thể cần PWM nếu solenoid yêu cầu

3. **LED Blink Timing** - Phụ thuộc vào HAL_Delay(10)
   - Nếu cần blink ổn định, thêm timer riêng

4. **Button Debounce** - Giờ chỉ detect rising edge
   - Có thể cần detect falling edge cho long press

### 🚀 Next Steps (Bước Tiếp Theo)

1. ✅ Compile & load firmware vào STM32F103C8
2. ✅ Simulate in Proteus with test cases above
3. ⏳ Add password module when ready
4. ⏳ Integrate UART for debug logging
5. ⏳ Fine-tune timings for production hardware

### 🔗 File Changes Summary

- ✅ `main.c`: Added getKeyInput() + door_fsm_run() to loop
- ✅ `door.c`: Fixed logic, added unlock_door()/lock_door()
- ✅ `buzzer.c`: Fixed state sync on mute
- ✅ `software_timer.c`: Added debounce timer support
- ✅ `button.c`: Added TODO for multi-button mapping
- ✅ `door.h`: Added unlock_door()/lock_door() declarations

All files are ready for Proteus simulation! 🎉
