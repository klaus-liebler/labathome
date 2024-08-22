#pragma once
#include <cstdint>
namespace I2C_SETUP{
    constexpr uint8_t STM32_I2C_ADDRESS{4};
}

//STM32 to ESP32
namespace S2E{
    constexpr size_t STM2ESP_SIZE{12};
    constexpr size_t STATUS_POS{0};//bit0==1->INITIALIZED, bit1->go to failsafe due to no communication, bit2 -> safety off because of overheat
    constexpr size_t BTN_MOVEMENT_BLFAULT_POS{1};
    constexpr size_t ROTENC_POS{2};
    constexpr size_t BRIGHTNESS_POS{4};
    constexpr size_t USBPD_VOLTAGE_IS_POS{6};
    constexpr size_t ADC0_POS{8};
    constexpr size_t ADC1_POS{10};
}
//ESP32 to STM32
namespace E2S{
    constexpr size_t ESP2STM_SIZE{20};
    constexpr size_t ADDRESS_POINTER_POS{0};
    constexpr size_t RELAY_BLRESET_POS{1};
    constexpr size_t SERVO0_POS{2};
    constexpr size_t SERVO1_POS{3};
    constexpr size_t SERVO2_POS{4};
    constexpr size_t SERVO3_POS{5};
    constexpr size_t FAN0_POS{6};
    constexpr size_t FAN1_POS{7};
    constexpr size_t USBPD_VOLTAGE_SHOULD_POS{8};
    constexpr size_t DAC0_POS{10};
    constexpr size_t DAC1_POS{12};
    constexpr size_t HEATER_POS{14};
    constexpr size_t LED_POWER_POS{15};
    constexpr size_t THREEPHASE_MODE_POS{16};
    constexpr size_t THREEPHASE_P1_DUTY_POS{17};
    constexpr size_t THREEPHASE_P2_DUTY_POS{18};
    constexpr size_t THREEPHASE_P3_DUTY_POS{19};
}