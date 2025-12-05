# LCD I2C 1602 - Hướng Dẫn Debug Chi Tiết

## Vấn Đề: LCD Không Hiển Thị Gì

### Nguyên Nhân Phổ Biến (xếp theo mức độ có khả năng)

1. **❌ Địa chỉ I2C sai (NGUYÊN NHÂN #1)** 
   - Code mặc định: 0x27
   - Địa chỉ thực: có thể là 0x20-0x27 hoặc 0x38-0x3F

2. **❌ Pin RW sai (ngoại tuyến)**
   - Pin RW phải set LOW (write mode)
   - Đã sửa trong lcd_i2c.c (dòng ~76)

3. **❌ Initialization sequence không đúng**
   - Timing quá nhanh
   - Đã cải thiện delays

4. **❌ Kết nối phần cứng**
   - PB6, PB7 không nối đúng
   - Thiếu pull-up resistor
   - Lỏng lẻo, kém liên lạc

5. **❌ Điện áp không ổn định**
   - LCD cần 5V ổn định
   - Dòng điện quá cao làm rơi voltages

---

## 🔧 Cách Fix: Nạp Code Mới

Bạn vừa nhận được code cải tiến với:

✅ **I2C Address Scan**: Tự động tìm địa chỉ LCD  
✅ **Improved Init**: Delays tốt hơn, sequence chính xác  
✅ **RW Pin Fix**: Luôn set LOW (write mode)  
✅ **Runtime Address Change**: Có thể đổi địa chỉ khi chạy  

### Bước 1: Nạp Code
```
Compile & Download code mới vào STM32
```

### Bước 2: Quan Sát Kết Quả Nào?

#### **Trường hợp 1: LCD hiển thị "LCD Found! 0x??"**
✅ **THÀNH CÔNG!** 
- Ghi nhớ địa chỉ (ví dụ: 0x27, 0x3F, v.v.)
- LCD sẽ chuyển sang màn hình chính sau 2 giây

**Làm gì tiếp?**
- Xóa mã scan I2C nếu muốn (hoặc để nguyên để debug lần sau)
- Tại dòng mà `LCD_I2C_SetAddress(found_address);` thay bằng địa chỉ cố định

#### **Trường hợp 2: LCD hiển thị "LCD Not Found! Check I2C wire"**
❌ **I2C không giao tiếp được**

**Kiểm tra:**
1. Kết nối phần cứng:
   - [ ] PB6 → I2C SCL
   - [ ] PB7 → I2C SDA
   - [ ] GND → GND
   - [ ] VCC (5V) → VCC

2. Pull-up resistor:
   - [ ] PB6 có 4.7kΩ pull-up to 3.3V
   - [ ] PB7 có 4.7kΩ pull-up to 3.3V
   
3. Nguồn điện:
   - [ ] LCD module có 5V ổn định
   - [ ] Backpack I2C có 5V ổn định
   - [ ] GND nối chung

4. Mạch chuyển I2C:
   - [ ] Kiểm tra lại LED hiệu trạng trên mạch
   - [ ] Thử nạp code scan I2C khác để test

#### **Trường hợp 3: LCD không hiển thị gì cả**
❌ **I2C có thể OK nhưng LCD init sai**

**Thử:**
1. Giảm I2C tốc độ xuống 50kHz:
   ```c
   // Trong main.c, hàm MX_I2C1_Init()
   hi2c1.Init.ClockSpeed = 50000;  // Thay từ 100000
   ```

2. Tăng delays:
   ```c
   // Trong lcd_i2c.c hàm LCD_I2C_Init()
   HAL_Delay(150);  // Thay từ 100
   ```

3. Kiểm tra contrast LCD:
   - Nếu LCD có nút potentiometer ở dưới
   - Quay từ từ để điều chỉnh độ sáng
   - Có thể cần xoay hết để nhìn được

---

## 📋 Danh Sách Kiểm Tra Phần Cứng

### Kết Nối:
- [ ] PB6 (STM32) ← → SCL (Backpack)
- [ ] PB7 (STM32) ← → SDA (Backpack)
- [ ] GND (STM32) ← → GND (Backpack)
- [ ] 5V ← → VCC (Backpack + LCD)

### Điện Áp (đo bằng multimeter):
- [ ] VCC = 5.0V (±0.3V)
- [ ] GND = 0V
- [ ] SCL = ~3.3V (khi high)
- [ ] SDA = ~3.3V (khi high)

### LCD Module:
- [ ] LED sau lưng sáng (backlight)
- [ ] Không có dấu hiệu hư hỏng
- [ ] Có thể xoay contrast potentiometer

---

## 💡 I2C Addresses - Bảng Tham Khảo

| Mạch | Địa Chỉ Thường Gặp | Ghi Chú |
|------|-------------------|--------|
| PCF8574 | 0x20-0x27 | Thường 0x27 |
| PCF8574A | 0x38-0x3F | Thường 0x3F |
| Chip A0,A1,A2 pins | Phụ thuộc jumper | Xem hướng dẫn backpack |

**Cách xác định:**
- Lì chân A0, A1, A2 trên PCF8574 nối GND hay không → quyết định địa chỉ
- Nếu không biết: dùng mã scan I2C để tìm

---

## 🔍 Debug Nâng Cao

### Nếu vẫn không hoạt động, thử debug I2C:

```c
// Thêm vào main.c để debug
uint8_t test_byte = 0xFF;
HAL_StatusTypeDef ret;

for(uint8_t addr = 0x20; addr <= 0x3F; addr++) {
    ret = HAL_I2C_IsDeviceReady(&hi2c1, (addr << 1), 1, 100);
    if(ret == HAL_OK) {
        // Thiết bị tìm thấy tại addr
        // Ghi nhớ: addr là giá trị 7-bit
    }
}
```

### Kiểm tra Logic I2C (scope/analyzer):
- [ ] SCL có xung clock
- [ ] SDA có signal thay đổi
- [ ] Không bị kẹt (stuck low)

---

## ✅ Nếu Thành Công

**Sau khi LCD hiển thị được:**

1. **Tối ưu hóa code:**
   ```c
   // Ghi nhớ địa chỉ tìm được, ví dụ 0x3F
   // Thay dòng này:
   // if(found_address != 0) {
   //     LCD_I2C_SetAddress(found_address);
   
   // Thành:
   LCD_I2C_SetAddress(0x3F);  // Địa chỉ thực tế
   LCD_I2C_Init(&hi2c1);
   ```

2. **Xóa code scan** (nếu muốn):
   - Loại bỏ vòng lặp for scan
   - Tiết kiệm thời gian boot

3. **Kiểm tra password display:**
   - Thử nhập mật khẩu
   - LCD phải hiển thị dấu "*"

---

## 📞 Nếu Vẫn Lỗi

**Gửi thông tin để debug:**
1. Loại mạch backpack (PCF8574 hay PCF8574A?)
2. Địa chỉ I2C nào được tìm thấy (nếu có)
3. Ảnh kết nối phần cứng
4. Output của serial debug (nếu có)

---

## 🎯 Tóm Tắt Sửa Chữa

| Vấn Đề | Nguyên Nhân | Giải Pháp |
|--------|-----------|----------|
| LCD không hiển thị | Địa chỉ I2C sai | Dùng I2C scan, set đúng địa chỉ |
| LCD sáng nhưng mập mờ | Contrast sai | Xoay nút potentiometer |
| LCD không sáng | Backlight hỏng | Kiểm tra nguồn 5V |
| I2C không phản hồi | Kết nối phần cứng | Kiểm tra PB6, PB7 |
| Chữ khác lạ | Timing init | Tăng delays, giảm tốc độ I2C |

---

**Code mới đã có I2C scan tự động - chỉ cần nạp và quan sát kết quả!**
