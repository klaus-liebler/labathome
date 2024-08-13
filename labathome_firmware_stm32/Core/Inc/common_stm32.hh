#pragma once
#include "stm32g4xx_ll_gpio.h"
#include "stm32g4xx_hal.h"

typedef int64_t time_t;

namespace GPIO
{
    enum class Pin : uint8_t
    {
        PA00,
        PA01,
        PA02,
        PA03,
        PA04,
        PA05,
        PA06,
        PA07,
        PA08,
        PA09,
        PA10,
        PA11,
        PA12,
        PA13,
        PA14,
        PA15,
        PB00,
        PB01,
        PB02,
        PB03,
        PB04,
        PB05,
        PB06,
        PB07,
        PB08,
        PB09,
        PB10,
        PB11,
        PB12,
        PB13,
        PB14,
        PB15,
        PC00,
        PC01,
        PC02,
        PC03,
        PC04,
        PC05,
        PC06,
        PC07,
        PC08,
        PC09,
        PC10,
        PC11,
        PC12,
        PC13,
        PC14,
        PC15,
        PD00,
        PD01,
        PD02,
        PD03,
        PD04,
        PD05,
        PD06,
        PD07,
        PD08,
        PD09,
        PD10,
        PD11,
        PD12,
        PD13,
        PD14,
        PD15,
        NO_PIN = UINT8_MAX
    };

    constexpr uint32_t CRL = 0;
    constexpr uint32_t CRH = 1;
    constexpr uint32_t IDR = 2;
    constexpr uint32_t ODR = 3;
    constexpr uint32_t BSSR = 4;
    constexpr uint32_t BRR = 5;

    uint8_t pin2port(Pin pin)
    {
        return ((uint8_t)pin) >> 4;
    }

    uint8_t pin2localPin(Pin pin)
    {
        return ((uint8_t)pin) & 0x0F;
    }

    uint32_t *pin2portBase(Pin pin)
    {
        return (uint32_t *)(GPIOA_BASE + (GPIOB_BASE - GPIOA_BASE) * (pin2port(pin)));
    }

    uint8_t pin2CRx(Pin pin)
    {
        return (((uint8_t)pin) & 0x08) >> 3;
    }

    uint8_t pin2CRxOffset(Pin pin)
    {
        return 4 * (((uint8_t)pin) & 0x07);
    }

    void Set (Pin pin, bool value) {
        if(pin==Pin::NO_PIN) return;
        uint32_t* gpiox_BSSR = pin2portBase(pin)+BSSR;
        uint8_t localPin = pin2localPin(pin);
        uint32_t bit2set= 1 << (localPin + 16 * !value);
        *gpiox_BSSR = bit2set;
    }

    bool Get(Pin pin) {
        if(pin==Pin::NO_PIN) return false;
        uint32_t* gpiox_IDR32 = pin2portBase(pin)+IDR;
        uint16_t* gpiox_IDR = (uint16_t*)gpiox_IDR32;
        return * gpiox_IDR & (1 << pin2localPin(pin));
    }

}