 #pragma once
 #include <cstdint>
  namespace PIN{
    constexpr uint32_t HEATER{PC4};
    constexpr uint32_t BL_ENABLE{PC6};
    constexpr uint32_t UART4_TX{PC10};
    constexpr uint32_t UART4_RX{PC11};
    constexpr uint32_t BTN_YELLOW{PC13};
    constexpr uint32_t MOVEMENT{PC14};
    constexpr uint32_t BL_HALL2{PB0};
    constexpr uint32_t BL_HALL1{PB1};
    constexpr uint32_t BRIGHTNESS{PB2};
    constexpr uint32_t LED_INFO{PB3};
    constexpr uint32_t USBPD_CC2{PB4};
    constexpr uint32_t BL_HALL3{PB5};
    constexpr uint32_t USBPD_CC1{PB6};
    constexpr uint32_t SDA{PB7};
    constexpr uint32_t BTN_RED{PB8};
    constexpr uint32_t LED_WHITE_P{PB9};
    constexpr uint32_t NC{PB10};
    constexpr uint32_t RELAY{PB11};
    constexpr uint32_t BL_RESET{PB12};
    constexpr uint32_t BL_FAULT{PB13};
    constexpr uint32_t ADC_0{PB14};
    constexpr uint32_t ADC_1{PB15};
    constexpr uint32_t ROT_A{PA0};
    constexpr uint32_t ROT_B{PA1};
    constexpr uint32_t Servo0{PA2};
    constexpr uint32_t Servo1{PA3};
    constexpr uint32_t DAC_0{PA4};
    constexpr uint32_t DAC_1{PA5};
    constexpr uint32_t Servo2{PA6};
    constexpr uint32_t FAN{PA7};
    constexpr uint32_t BL_DRV1{PA8};
    constexpr uint32_t BL_DRV2{PA9};
    constexpr uint32_t BL_DRV3{PA10};
    constexpr uint32_t USB_M{PA11};
    constexpr uint32_t USB_P{PA12};
    constexpr uint32_t SWDIO{PA13};
    constexpr uint32_t SWCLK{PA14};
    constexpr uint32_t SCL{PA15};
}
