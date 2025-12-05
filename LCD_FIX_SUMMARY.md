# Các Sửa Chữa LCD I2C 1602 - Tóm Tắt

## 🔧 Những Gì Đã Sửa

### 1. **main.c** - I2C Address Auto-Scan
- ✅ Tự động quét địa chỉ I2C (0x20-0x27, 0x38-0x3F)
- ✅ Hiển thị địa chỉ tìm được trên LCD
- ✅ Fallback nếu không tìm thấy

### 2. **lcd_i2c.c** - Cải Thiện Driver
- ✅ **Initialization sequence**: Từ 50ms → 100ms + 3x retry
- ✅ **RW Pin Fix**: Đổi từ HIGH (read) → LOW (write) - **QUAN TRỌNG!**
- ✅ **Runtime address change**: Hàm `LCD_I2C_SetAddress(addr)`
- ✅ **Better delays**: Thêm delays giữa các commands

### 3. **lcd_i2c.h** - API Mới
- ✅ `void LCD_I2C_SetAddress(uint8_t addr)` - Set địa chỉ động
- ✅ `uint8_t LCD_I2C_GetAddress()` - Lấy địa chỉ hiện tại

---

## 🚀 Để Fix LCD Không Hiển Thị

### **Bước 1: Nạp Code**
Compile và download code cải tiến vừa nhận

### **Bước 2: Quan Sát**
Khi khởi động, LCD sẽ:
- **Nếu tốt**: Hiển thị "LCD Found! 0x??" trong 2 giây
- **Nếu lỗi**: Hiển thị "LCD Not Found! Check I2C wire"

### **Bước 3: Xử Lý**

#### ✅ **Nếu thấy "LCD Found! 0x??"**
1. Ghi nhớ địa chỉ (ví dụ: 0x3F, 0x27)
2. LCD hiện "Door Lock Ready" → **Thành công!**
3. Có thể bỏ code scan hoặc để nguyên

#### ❌ **Nếu thấy "LCD Not Found!"**
Kiểm tra:
- [ ] PB6 (SCL), PB7 (SDA) nối đúng?
- [ ] Pull-up resistor 4.7kΩ có không?
- [ ] Nguồn 5V ổn định?
- [ ] Backpack/LCD có tín hiệu không?

Thử fix:
```c
// Giảm I2C tốc độ trong MX_I2C1_Init():
hi2c1.Init.ClockSpeed = 50000;  // Từ 100000

// Hoặc tăng delays trong LCD_I2C_Init():
HAL_Delay(150);  // Từ 100
```

---

## 📝 Code Changes详细

### main.c (khối USER CODE 2)
```c
// Scan I2C và tìm LCD
uint8_t found_address = 0;

// Thử PCF8574 (0x20-0x27)
for(uint8_t addr = 0x20; addr <= 0x27; addr++) {
    if(HAL_I2C_IsDeviceReady(&hi2c1, (addr << 1), 1, 100) == HAL_OK) {
        found_address = addr;
        break;
    }
}

// Thử PCF8574A (0x38-0x3F)
if(found_address == 0) {
    for(uint8_t addr = 0x38; addr <= 0x3F; addr++) {
        if(HAL_I2C_IsDeviceReady(&hi2c1, (addr << 1), 1, 100) == HAL_OK) {
            found_address = addr;
            break;
        }
    }
}

// Set địa chỉ và init
if(found_address != 0) {
    LCD_I2C_SetAddress(found_address);
    LCD_I2C_Init(&hi2c1);
    // ... hiển thị kết quả
}
```

### lcd_i2c.c - RW Pin Fix (QUAN TRỌNG!)
```c
// CŨ (sai):
bits |= (1 << LCD_RW_PIN);  // RW = 1 (read mode)

// MỚI (đúng):
bits &= ~(1 << LCD_RW_PIN);  // RW = 0 (write mode)
```

### lcd_i2c.c - Improved Initialization
```c
HAL_Delay(100);  // Power stabilization
uint8_t init_byte = (1 << LCD_BACKLIGHT_PIN);
LCD_I2C_SendByte(init_byte);
HAL_Delay(50);

// 3x retry 4-bit initialization
for (int attempt = 0; attempt < 3; attempt++) {
    // ... send 0x33, 0x32
}
```

---

## 🎯 Lịch Sử Vấn Đề

| Vấn Đề | Nguyên Nhân | Sửa Chữa |
|--------|-----------|---------|
| LCD không hiển thị | Địa chỉ I2C sai | I2C auto-scan |
| Ngay cả khi scan OK | RW pin sai | Đổi RW = 0 (write) |
| Init không ổn định | Timing quá nhanh | Tăng delays |
| Một số LCD hoạt động | Có thể khác loại chip | Runtime address change |

---

## ✅ Test Checklist

Sau khi nạp code:
- [ ] LCD backlight sáng
- [ ] Hiển thị "LCD Found" hoặc "LCD Not Found"
- [ ] Nếu found: ghi nhớ địa chỉ
- [ ] LCD hiển thị "Door Lock Ready"
- [ ] Khi nhập password: hiển thị "*"
- [ ] Backlight có thể bật/tắt

---

## 📚 File Liên Quan

- **LCD_DEBUG_GUIDE.md** - Hướng dẫn debug chi tiết
- **LCD_SETUP_GUIDE.md** - Cấu hình hệ thống
- **LCD_TROUBLESHOOTING.md** - Khắc phục sự cố
- **Core/Src/main.c** - Khởi động với I2C scan
- **Core/Src/lcd_i2c.c** - Driver I2C cải tiến
- **Core/Inc/lcd_i2c.h** - API LCD

---

## 💡 Tip Thêm

1. **Nếu LCD tìm được nhưng text lạ**:
   - Xoay nút potentiometer điều chỉnh contrast
   - Hoặc giảm I2C clock speed

2. **Để optimize startup**:
   - Khi tìm được địa chỉ, update `LCD_I2C_ADDR` trong lcd_i2c.h
   - Bỏ code scan để boot nhanh hơn

3. **Debug I2C khác**:
   - Dùng HAL_I2C_IsDeviceReady() để test
   - Kiểm tra SCL/SDA có xung không (scope)

---

**Nạp code, reboot board, và quan sát kết quả LCD!**
