/*
 * buzzer.c
 * Logic: Pull-down (Nhấn/Mở = SET, Thả/Đóng = RESET)
 */
#include "buzzer.h"

static bool isDoorOpen = false;
static bool isBuzzerActive = false;

/* Định nghĩa private */
#define DOOR_TIMEOUT_TICKS 300 // 3s

void Buzzer_Run(void)
{
    static uint8_t muted = 0;   // Trạng thái mute

    // Đọc trạng thái chân GPIO
    uint8_t doorState = HAL_GPIO_ReadPin(DOOR_SENSOR_GPIO_Port, DOOR_SENSOR_Pin);
    uint8_t muteState = HAL_GPIO_ReadPin(MUTE_BUTTON_GPIO_Port, MUTE_BUTTON_Pin);

    /* ================= XỬ LÝ NÚT MUTE (PULL-DOWN) ================= */
    // Pull-down: Nhấn nút thì chân lên 1 (SET)
    if (muteState == GPIO_PIN_SET && isBuzzerActive)
    {
        muted = 1;
        isBuzzerActive = false;
        timer1_flag = 0;
        HAL_GPIO_WritePin(BUZZER_CTRL_GPIO_Port, BUZZER_CTRL_Pin, GPIO_PIN_SET); // Tắt còi
    }

    /* ================= ĐÃ MUTE → KHÓA CÒI ================= */
    if (muted)
    {
        HAL_GPIO_WritePin(BUZZER_CTRL_GPIO_Port, BUZZER_CTRL_Pin, GPIO_PIN_SET); // Luôn tắt

        // Nếu cửa đóng (bằng 1 theo ý bạn), reset trạng thái Mute
        if (doorState == GPIO_PIN_SET)
        {
            muted = 0;
            isDoorOpen = false;
        }
        return; // Thoát hàm để không chạy logic bên dưới
    }

    /* ================= LOGIC CỬA (0 MỞ, 1 ĐÓNG) ================= */
    if (doorState == GPIO_PIN_RESET) // CỬA MỞ (BẰNG 0)
    {
        if (!isDoorOpen)
        {
            isDoorOpen = true;
            setTimer1(DOOR_TIMEOUT_TICKS); // Bắt đầu đếm 3 giây
        }

        if (timer1_flag) // Nếu đã hết 3 giây
        {
            isBuzzerActive = true;
            HAL_GPIO_WritePin(BUZZER_CTRL_GPIO_Port, BUZZER_CTRL_Pin, GPIO_PIN_RESET); // Bật còi (ON)
        }
    }
    else // CỬA ĐÓNG (BẰNG 1)
    {
        isDoorOpen = false;
        muted = 0;
        setTimer1(0);
        timer1_flag = 0;

        if (isBuzzerActive)
        {
            isBuzzerActive = false;
            HAL_GPIO_WritePin(BUZZER_CTRL_GPIO_Port, BUZZER_CTRL_Pin, GPIO_PIN_SET); // Tắt còi (OFF)
        }
    }
}
