 #pragma once
 #include <cstdint>
  namespace PIN{
    constexpr uint32_t BRIGHTNESS{PF0};
    constexpr uint32_t BTN_YELLOW{PF1};
    constexpr uint32_t LCD_RESET{PD2};
    
    constexpr uint32_t Servo0{PC0};//TIM1.1
    constexpr uint32_t Servo1{PC1};//TIM1.2
    constexpr uint32_t Servo2{PC2};//TIM1.3
    constexpr uint32_t Servo3{PC3};//TIM1.4
    
    constexpr uint32_t ADC_0{PC4};
    constexpr uint32_t ADC_1{PC5};

    constexpr uint32_t BL_DRV1{PC6};//TIM8.1
    constexpr uint32_t BL_DRV2{PC7};//TIM8.2
    constexpr uint32_t BL_DRV3{PC8};//TIM8.3
    constexpr uint32_t BL_FAULT{PC9};
    constexpr uint32_t UART4_TX{PC10};
    constexpr uint32_t UART4_RX{PC11};
    constexpr uint32_t LED_INFO{PC12};
    constexpr uint32_t HEATER{PC13};
    constexpr uint32_t MOVEMENT{PC14};
    constexpr uint32_t RELAY_STEPPER_EN{PC15};
   
  
    constexpr uint32_t ROT_A{PA0};//TIM2.1
    constexpr uint32_t ROT_B{PA1};//TIM2.2
    constexpr uint32_t LED_WHITE_P_SERVO5_STEPPER3_STEP{PA2};//TIM15.2
    constexpr uint32_t FAN_SERVO4_STEPPER3_DIR{PA3};//TIM15.1

   
    constexpr uint32_t DAC_0{PA4};
    constexpr uint32_t DAC_1{PA5};
    constexpr uint32_t BL_HALL3{PA6};//TIM3.1
    constexpr uint32_t BL_HALL2{PA7};//TIM3.2
    constexpr uint32_t BL_SLEEP{PA8};

    constexpr uint32_t USBPD_CC1_A{PA9};
    constexpr uint32_t USBPD_CC2_A{PA10};
    constexpr uint32_t USB_M{PA11};
    constexpr uint32_t USB_P{PA12};
    constexpr uint32_t SWDIO{PA13};
    constexpr uint32_t SWCLK{PA14};
    constexpr uint32_t SCL{PA15};//I2C1

    constexpr uint32_t BL_HALL1{PB0};//TIM3.3
    constexpr uint32_t OPAMP3_Q{PB1};
    constexpr uint32_t OPAMP3_M{PB2};
    constexpr uint32_t BL_RESET{PB3};
    constexpr uint32_t USBPD_CC2_B{PB4};
    constexpr uint32_t ADC_2_24V{PB5};
    constexpr uint32_t USBPD_CC1_B{PB6};

    constexpr uint32_t BL_ENABLE{PB7};
    constexpr uint32_t BTN_RED{PB8};
    constexpr uint32_t SDA{PB9};//I2C1

    constexpr uint32_t UART3_TX{PB10};
    constexpr uint32_t UART3_RX{PB11};
    constexpr uint32_t PIN_PB12{PB12};
    constexpr uint32_t OPAMP3_P{PB13};
    constexpr uint32_t BL_ISENSE{PB14};
    constexpr uint32_t SUPPLY_24V_SENSE{PB15};
}