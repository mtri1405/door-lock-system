# Các Sửa Chữa Áp Dụng cho Hệ Thống Khóa Điện Tử

## Tóm Tắt
Đã sửa chữa các lỗi quan trọng để hệ thống có thể chạy thử trên Proteus. Hiện tại hệ thống chờ module xác thực mật khẩu được thêm vào.

---

## 1. **MAIN.C** - Thêm Task Vào Loop

### ✅ Sửa: Gọi đủ các task trong main loop

**Trước:**
```c
while (1) {
    Buzzer_Run();           // Chỉ có buzzer
    HAL_Delay(10);
}
```

**Sau:**
```c
while (1) {
    getKeyInput();          // Đọc nút bấm
    door_fsm_run();         // Chạy state machine cửa
    Buzzer_Run();           // Chạy logic buzzer
    HAL_Delay(10);
}
```

**Tác dụng**: Giờ đây:
- Button được đọc liên tục 
- Door FSM được cập nhật trạng thái
- Buzzer hoạt động bình thường

---

## 2. **DOOR.C** - Sửa Lỗi Logic

### ✅ Sửa: Lỗi logic kiểm tra cửa đóng/mở (Line 84)

**Trước:**
```c
case DOOR_UNLOCKED:
    if (open_timer > 0) {
        open_timer--;
    } else {
        if (!isDoorClose()) {           // SAI: ! (NOT)
            door_state = DOOR_ALARM;
            aleart();
        } else {
            door_state = DOOR_LOCKED;
        }
    }
```

**Sau:**
```c
case DOOR_UNLOCKED:
    if (open_timer > 0) {
        open_timer--;
    } else {
        if (isDoorClose()) {            // ĐÚNG: không có !
            door_state = DOOR_LOCKED;
            lock_door();
            stop_aleart();
        } else {                        // Cửa vẫn mở → ALARM
            door_state = DOOR_ALARM;
            aleart();
        }
    }
```

**Tác dụng**: Báo động sẽ kích hoạt khi cửa không đóng trong thời gian quy định

### ✅ Thêm: Hàm điều khiển khóa solenoid

**Thêm mới:**
```c
// Mở khóa cửa (điều khiển solenoid)
void unlock_door() {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}

// Khóa cửa
void lock_door() {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
}
```

**Tác dụng**: 
- Điều khiển solenoid khoá điện thực tế
- Sử dụng PA4 (GPIO PIN 4) để điều khiển
- Tự động khóa khi vào trạng thái LOCKED

### ✅ Sửa: FSM gọi unlock/lock tại các điểm thích hợp

**Các thay đổi:**
```c
case DOOR_LOCKED:
    lock_door();  // Luôn giữ trạng thái khóa
    if (isOpenPress() && isPasswordCorrect()) {
        door_state = DOOR_UNLOCKED;
        unlock_door();  // Mở khóa khi cấp phép
        ...
    }
    break;

case DOOR_UNLOCKED:
    if (...) {
        if (isDoorClose()) {
            lock_door();  // Tự động khóa khi cửa đóng
            ...
        }
    }
    break;
```

---

## 3. **BUTTON.C** - Ghi Chú Mapping

### 📝 Ghi Chú: Button Pin Mapping

```c
// Mapping button (using OPEN_BUTTON_Pin for single button setup)
uint16_t BUTTON_PIN[NO_OF_BUTTONS] = { 
    OPEN_BUTTON_Pin, OPEN_BUTTON_Pin, OPEN_BUTTON_Pin 
};  
// TODO: Update when more buttons are connected
```

**Hiện tại**: Tất cả button dùng OPEN_BUTTON_Pin (single button configuration)

**Khi thêm button**: Thay đổi mapping để trỏ đến các pin GPIO khác nhau

---

## 4. **BUZZER.C** - Sửa State Sync

### ✅ Sửa: Cập nhật trạng thái khi mute

**Trước:**
```c
if (HAL_GPIO_ReadPin(MUTE_BUTTON_GPIO_Port, MUTE_BUTTON_Pin) == 0) {
    HAL_GPIO_WritePin(BUZZER_CTRL_GPIO_Port, BUZZER_CTRL_Pin, GPIO_PIN_SET);
    timer1_flag = 0;
    // isBuzzerActive không được cập nhật → state mất đồng bộ
}
```

**Sau:**
```c
if (HAL_GPIO_ReadPin(MUTE_BUTTON_GPIO_Port, MUTE_BUTTON_Pin) == 0) {
    HAL_GPIO_WritePin(BUZZER_CTRL_GPIO_Port, BUZZER_CTRL_Pin, GPIO_PIN_SET);
    timer1_flag = 0;
    isBuzzerActive = false;  // Cập nhật state
}
```

**Tác dụng**: State machine của buzzer luôn đồng bộ

---

## 5. **SOFTWARE_TIMER.C** - Hỗ Trợ Multi-Timer

### ✅ Thêm: Timer thứ hai cho debounce button

**Thêm mới:**
```c
// Timer for button debounce
int timer_debounce_counter = 0;
int timer_debounce_flag = 0;

void setTimerDebounce(int duration){
    timer_debounce_counter = duration;
    timer_debounce_flag = 0;
}
```

**Sửa: timerRun() để hỗ trợ cả hai timer**
```c
void timerRun(){
    // Timer 1 (buzzer/door)
    if (timer1_counter > 0){
        timer1_counter--;
        if (timer1_counter <= 0){
            timer1_flag = 1;
        }
    }
    
    // Debounce Timer
    if (timer_debounce_counter > 0){
        timer_debounce_counter--;
        if (timer_debounce_counter <= 0){
            timer_debounce_flag = 1;
        }
    }
}
```

**Tác dụng**: Hệ thống không bị xung đột timer resource

---

## 6. **SOFTWARE_TIMER.H** - Cập Nhật Header

### ✅ Thêm: Export timer_debounce_flag

```c
extern int timer1_flag;
extern int timer_debounce_flag;  // Mới

void setTimer1(int duration);
void setTimerDebounce(int duration);  // Mới
void timerRun(void);
```

---

## 7. **DOOR.H** - Cập Nhật Interface

### ✅ Thêm: Khai báo hàm unlock/lock

```c
int isDoorClose();
int isPasswordCorrect();
void unlock_door();      // Mới
void lock_door();        // Mới
void aleart();
void stop_aleart();
void update_led();
void door_fsm_run(void);
```

---

## Trạng Thái Hiện Tại

✅ **Hệ thống có thể chạy thử trên Proteus** với các tính năng:
- Button đọc được
- Door FSM cập nhật trạng thái
- Solenoid lock được điều khiển
- Buzzer hoạt động
- Cảnh báo LED

⏳ **Chờ thêm:**
- Module xác thực mật khẩu (isPasswordCorrect())
- UART debug/logging (tùy chọn)
- Timeout tuning theo phần cứng thực tế

---

## Cấu Hình GPIO Hiện Tại

| Chức Năng | GPIO | Pin | Ghi Chú |
|-----------|------|-----|---------|
| Door Sensor | PB3 | Input | Pull-up |
| Open Button | PB0 | Input | Pull-up |
| Solenoid Lock | PA4 | Output | PWM/Digital |
| Buzzer Control | PA5 | Output | Digital |
| Red LED | PA0 | Output | Digital |
| Green LED | PA1 | Output | Digital |
| Mute Button | PA2 | Input | Pull-up |

---

## Hướng Dẫn Test Proteus

1. **Mở schematic** và kiểm tra connection đúng với GPIO config
2. **Biên dịch project** 
3. **Tải firmware** vào STM32F103C8
4. **Test workflow**:
   - Press OPEN_BUTTON → LED xanh sáng, solenoid mở
   - Wait 20 ticks (200ms) → Cửa sẽ tự động khóa nếu đóng
   - Nếu cửa mở quá lâu → Buzzer kêu, LED đỏ nháy
   - Press MUTE_BUTTON → Buzzer tắt

5. **Sau khi password module thêm vào**:
   - Sửa isPasswordCorrect() để gọi password validation
   - System sẽ chỉ mở khi password đúng

---

## Lưu Ý
- Các hardcoded GPIO pin có thể cần điều chỉnh theo schematic thực tế
- Timer frequency hiện là 1kHz (dựa trên TIM2 config)
- Password module sẽ được thêm vào khi hoàn thành
