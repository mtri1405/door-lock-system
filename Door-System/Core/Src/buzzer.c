/*
 * buzzer.c
 * - Hardware: Buzzer nối nguồn 3.3V (Active LOW: 0=Kêu, 1=Tắt)
 * - Logic: Nhận lệnh Activate/Deactivate từ hệ thống Door
 */
#include "buzzer.h"
#include "main.h"
#include <stdbool.h> // Sửa lỗi 'bool', 'true', 'false'

// Biến trạng thái nội bộ
static bool isAlarmActive = false; // Cờ báo động từ hệ thống
static bool isMuted = false;       // Cờ ngắt tiếng tạm thời
static bool muteBtnWasPressed = false; // Debounce cho nút MUTE

/* ====== API KHỞI TẠO & ĐIỀU KHIỂN ====== */

void Buzzer_Init(void) {
    // Khởi tạo trạng thái tắt (Active LOW -> Xuất mức 1 để tắt)
    HAL_GPIO_WritePin(BUZZER_CTRL_GPIO_Port, BUZZER_CTRL_Pin, GPIO_PIN_SET);
    isAlarmActive = false;
    isMuted = false;
    muteBtnWasPressed = false;
}

// Hàm này được gọi từ door.c khi có sự cố (cạy cửa/quên đóng)
void Buzzer_Activate(void) {
    isAlarmActive = true;
    isMuted = false; // Có báo động mới thì hủy chế độ im lặng cũ
}

// Hàm này được gọi từ door.c khi sự cố đã được xử lý
void Buzzer_Deactivate(void) {
    isAlarmActive = false;
    isMuted = false;
    // Tắt còi ngay lập tức
    HAL_GPIO_WritePin(BUZZER_CTRL_GPIO_Port, BUZZER_CTRL_Pin, GPIO_PIN_SET);
}

// Kiểm tra buzzer có đang active không
int Buzzer_IsActive(void) {
    return isAlarmActive ? 1 : 0;
}

/* ====== HÀM CHẠY TRONG MAIN LOOP ====== */
void Buzzer_Run(void)
{
    // Logic nút Mute với DEBOUNCE (phát hiện sườn lên) - NÚT TOGGLE
    // Pull-down: Nhấn nút = 1 (SET)
    bool btnCurrentState = (HAL_GPIO_ReadPin(MUTE_BUTTON_GPIO_Port, MUTE_BUTTON_Pin) == GPIO_PIN_SET);
    
    if (btnCurrentState && !muteBtnWasPressed) {
        // SƯỜN LÊN - nút vừa được nhấn
        muteBtnWasPressed = true;
        
        if (isAlarmActive) {
            // TOGGLE trạng thái mute (bật/tắt buzzer)
            isMuted = !isMuted;
            
            // Cập nhật trạng thái buzzer NGAY LẬP TỨC
            if (isMuted) {
                // Tắt buzzer
                HAL_GPIO_WritePin(BUZZER_CTRL_GPIO_Port, BUZZER_CTRL_Pin, GPIO_PIN_SET);
            } else {
                // Bật buzzer
                HAL_GPIO_WritePin(BUZZER_CTRL_GPIO_Port, BUZZER_CTRL_Pin, GPIO_PIN_RESET);
            }
        }
    }
    else if (!btnCurrentState) {
        // Nút đã được thả ra
        muteBtnWasPressed = false;
    }

    // ĐIỀU KHIỂN CÒI THỰC TẾ
    // Chỉ kêu khi: Có báo động VÀ Chưa bị tắt tiếng
    if (isAlarmActive && !isMuted) {
        // BẬT CÒI (Active LOW -> Xuất mức 0)
        HAL_GPIO_WritePin(BUZZER_CTRL_GPIO_Port, BUZZER_CTRL_Pin, GPIO_PIN_RESET);
    }
    else {
        // TẮT CÒI (Active LOW -> Xuất mức 1)
        HAL_GPIO_WritePin(BUZZER_CTRL_GPIO_Port, BUZZER_CTRL_Pin, GPIO_PIN_SET);
    }
}
