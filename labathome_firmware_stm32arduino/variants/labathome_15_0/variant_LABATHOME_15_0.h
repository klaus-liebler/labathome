
#pragma once
#include <stdint.h>

#define PA0                     PIN_A0
#define PA1                     PIN_A1
#define PA2                     PIN_A2
#define PA3                     PIN_A3
#define PA4                     PIN_A4
#define PA5                     PIN_A5
#define PA6                     PIN_A6
#define PA7                     PIN_A7
#define PA8                     8
#define PA9                     9
#define PA10                    10
#define PA11                    11
#define PA12                    12
#define PA13                    13
#define PA14                    14
#define PA15                    15
#define PB0                     PIN_A8
#define PB1                     PIN_A9
#define PB2                     PIN_A10
#define PB3                     19
#define PB4                     20
#define PB5                     21
#define PB6                     22
#define PB7                     23
#define PB8                     24
#define PB9                     25
#define PB10                    26
#define PB11                    PIN_A11
#define PB12                    PIN_A12
#define PB13                    29
#define PB14                    PIN_A13
#define PB15                    PIN_A14
#define PC4                     PIN_A15
#define PC6                     33
#define PC10                    34
#define PC11                    35
#define PC13                    36
#define PC14                    37
#define PC15                    38
#define PF0                     PIN_A16
#define PF1                     PIN_A17
#define PG10                    41

// Alternate pins number
#define PA0_ALT1                (PA0  | ALT1)
#define PA1_ALT1                (PA1  | ALT1)
#define PA2_ALT1                (PA2  | ALT1)
#define PA3_ALT1                (PA3  | ALT1)
#define PA4_ALT1                (PA4  | ALT1)
#define PA6_ALT1                (PA6  | ALT1)
#define PA7_ALT1                (PA7  | ALT1)
#define PA7_ALT2                (PA7  | ALT2)
#define PA7_ALT3                (PA7  | ALT3)
#define PA9_ALT1                (PA9  | ALT1)
#define PA10_ALT1               (PA10 | ALT1)
#define PA11_ALT1               (PA11 | ALT1)
#define PA11_ALT2               (PA11 | ALT2)
#define PA12_ALT1               (PA12 | ALT1)
#define PA12_ALT2               (PA12 | ALT2)
#define PA13_ALT1               (PA13 | ALT1)
#define PA15_ALT1               (PA15 | ALT1)
#define PB0_ALT1                (PB0  | ALT1)
#define PB0_ALT2                (PB0  | ALT2)
#define PB1_ALT1                (PB1  | ALT1)
#define PB1_ALT2                (PB1  | ALT2)
#define PB3_ALT1                (PB3  | ALT1)
#define PB4_ALT1                (PB4  | ALT1)
#define PB4_ALT2                (PB4  | ALT2)
#define PB5_ALT1                (PB5  | ALT1)
#define PB5_ALT2                (PB5  | ALT2)
#define PB6_ALT1                (PB6  | ALT1)
#define PB6_ALT2                (PB6  | ALT2)
#define PB7_ALT1                (PB7  | ALT1)
#define PB7_ALT2                (PB7  | ALT2)
#define PB8_ALT1                (PB8  | ALT1)
#define PB8_ALT2                (PB8  | ALT2)
#define PB9_ALT1                (PB9  | ALT1)
#define PB9_ALT2                (PB9  | ALT2)
#define PB9_ALT3                (PB9  | ALT3)
#define PB11_ALT1               (PB11 | ALT1)
#define PB13_ALT1               (PB13 | ALT1)
#define PB14_ALT1               (PB14 | ALT1)
#define PB15_ALT1               (PB15 | ALT1)
#define PB15_ALT2               (PB15 | ALT2)
#define PC6_ALT1                (PC6  | ALT1)
#define PC10_ALT1               (PC10 | ALT1)
#define PC11_ALT1               (PC11 | ALT1)
#define PC13_ALT1               (PC13 | ALT1)

#define NUM_DIGITAL_PINS        42
#define NUM_ANALOG_INPUTS       18

// On-board LED pin number
#ifndef LED_BUILTIN
  #define LED_BUILTIN           PNUM_NOT_DEFINED
#endif

// On-board user button
#ifndef USER_BTN
  #define USER_BTN              PNUM_NOT_DEFINED
#endif

// SPI definitions
#ifndef PIN_SPI_SS
  #define PIN_SPI_SS            PA4
#endif
#ifndef PIN_SPI_SS1
  #define PIN_SPI_SS1           PA15
#endif
#ifndef PIN_SPI_SS2
  #define PIN_SPI_SS2           PNUM_NOT_DEFINED
#endif
#ifndef PIN_SPI_SS3
  #define PIN_SPI_SS3           PNUM_NOT_DEFINED
#endif
#ifndef PIN_SPI_MOSI
  #define PIN_SPI_MOSI          PA7
#endif
#ifndef PIN_SPI_MISO
  #define PIN_SPI_MISO          PA6
#endif
#ifndef PIN_SPI_SCK
  #define PIN_SPI_SCK           PA5
#endif

// I2C definitions
#ifndef PIN_WIRE_SDA
  #define PIN_WIRE_SDA          PA8
#endif
#ifndef PIN_WIRE_SCL
  #define PIN_WIRE_SCL          PA9
#endif

// Timer Definitions
// Use TIM6/TIM7 when possible as servo and tone don't need GPIO output pin
#ifndef TIMER_TONE
  #define TIMER_TONE            TIM6
#endif
#ifndef TIMER_SERVO
  #define TIMER_SERVO           TIM7
#endif

// UART Definitions
#define SERIAL_UART_INSTANCE 4
#define PIN_CC1_ARDUINO PB6
#define PIN_CC2_ARDUINO PB4
#define PIN_SERIAL_TX PC10
#define PIN_SERIAL_RX PC11
#define DEBUG_UART_BAUDRATE 115200


// Extra HAL modules
#if !defined(HAL_DAC_MODULE_DISABLED)
  #define HAL_DAC_MODULE_ENABLED
#endif

/*----------------------------------------------------------------------------
 *        Arduino objects - C++ only
 *----------------------------------------------------------------------------*/

#ifdef __cplusplus
  // These serial port names are intended to allow libraries and architecture-neutral
  // sketches to automatically default to the correct port name for a particular type
  // of use.  For example, a GPS module would normally connect to SERIAL_PORT_HARDWARE_OPEN,
  // the first hardware serial port whose RX/TX pins are not dedicated to another use.
  //
  // SERIAL_PORT_MONITOR        Port which normally prints to the Arduino Serial Monitor
  //
  // SERIAL_PORT_USBVIRTUAL     Port which is USB virtual serial
  //
  // SERIAL_PORT_LINUXBRIDGE    Port which connects to a Linux system via Bridge library
  //
  // SERIAL_PORT_HARDWARE       Hardware serial port, physical RX & TX pins.
  //
  // SERIAL_PORT_HARDWARE_OPEN  Hardware serial ports which are open for use.  Their RX & TX
  //                            pins are NOT connected to anything by default.
  #ifndef SERIAL_PORT_MONITOR
    #define SERIAL_PORT_MONITOR   Serial
  #endif
  #ifndef SERIAL_PORT_HARDWARE
    #define SERIAL_PORT_HARDWARE  Serial
  #endif

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


#endif


