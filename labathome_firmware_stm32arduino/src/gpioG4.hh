#pragma once

#include <Arduino.h>
#undef B0
#undef B1
#undef B10
#undef B11
namespace GPIO
{
    enum class Pin
    {
        A0 = GPIOA_BASE + 0, A1 = GPIOA_BASE + 1, A2 = GPIOA_BASE + 2, A3 = GPIOA_BASE + 3, A4 = GPIOA_BASE + 4, A5 = GPIOA_BASE + 5, A6 = GPIOA_BASE + 6, A7 = GPIOA_BASE + 7, A8 = GPIOA_BASE + 8, A9 = GPIOA_BASE + 9, A10 = GPIOA_BASE + 10, A11 = GPIOA_BASE + 11, A12 = GPIOA_BASE + 12, A13 = GPIOA_BASE + 13, A14 = GPIOA_BASE + 14, A15 = GPIOA_BASE + 15,
        B0 = GPIOB_BASE + 0, B1 = GPIOB_BASE + 1, B2 = GPIOB_BASE + 2, B3 = GPIOB_BASE + 3, B4 = GPIOB_BASE + 4, B5 = GPIOB_BASE + 5, B6 = GPIOB_BASE + 6, B7 = GPIOB_BASE + 7, B8 = GPIOB_BASE + 8, B9 = GPIOB_BASE + 9, B10 = GPIOB_BASE + 10, B11 = GPIOB_BASE + 11, B12 = GPIOB_BASE + 12, B13 = GPIOB_BASE + 13, B14 = GPIOB_BASE + 14, B15 = GPIOB_BASE + 15,
        C0 = GPIOC_BASE + 0, C1 = GPIOC_BASE + 1, C2 = GPIOC_BASE + 2, C3 = GPIOC_BASE + 3, C4 = GPIOC_BASE + 4, C5 = GPIOC_BASE + 5, C6 = GPIOC_BASE + 6, C7 = GPIOC_BASE + 7, C8 = GPIOC_BASE + 8, C9 = GPIOC_BASE + 9, C10 = GPIOC_BASE + 10, C11 = GPIOC_BASE + 11, C12 = GPIOC_BASE + 12, C13 = GPIOC_BASE + 13, C14 = GPIOC_BASE + 14, C15 = GPIOC_BASE + 15,
        D0 = GPIOD_BASE + 0, D1 = GPIOD_BASE + 1, D2 = GPIOD_BASE + 2, D3 = GPIOD_BASE + 3, D4 = GPIOD_BASE + 4, D5 = GPIOD_BASE + 5, D6 = GPIOD_BASE + 6, D7 = GPIOD_BASE + 7, D8 = GPIOD_BASE + 8, D9 = GPIOD_BASE + 9, D10 = GPIOD_BASE + 10, D11 = GPIOD_BASE + 11, D12 = GPIOD_BASE + 12, D13 = GPIOD_BASE + 13, D14 = GPIOD_BASE + 14, D15 = GPIOD_BASE + 15,
        E0 = GPIOE_BASE + 0, E1 = GPIOE_BASE + 1, E2 = GPIOE_BASE + 2, E3 = GPIOE_BASE + 3, E4 = GPIOE_BASE + 4, E5 = GPIOE_BASE + 5, E6 = GPIOE_BASE + 6, E7 = GPIOE_BASE + 7, E8 = GPIOE_BASE + 8, E9 = GPIOE_BASE + 9, E10 = GPIOE_BASE + 10, E11 = GPIOE_BASE + 11, E12 = GPIOE_BASE + 12, E13 = GPIOE_BASE + 13, E14 = GPIOE_BASE + 14, E15 = GPIOE_BASE + 15,
        F0 = GPIOF_BASE + 0, F1 = GPIOF_BASE + 1, F2 = GPIOF_BASE + 2, F3 = GPIOF_BASE + 3, F4 = GPIOF_BASE + 4, F5 = GPIOF_BASE + 5, F6 = GPIOF_BASE + 6, F7 = GPIOF_BASE + 7, F8 = GPIOF_BASE + 8, F9 = GPIOF_BASE + 9, F10 = GPIOF_BASE + 10, F11 = GPIOF_BASE + 11, F12 = GPIOF_BASE + 12, F13 = GPIOF_BASE + 13, F14 = GPIOF_BASE + 14, F15 = GPIOF_BASE + 15,
        G0 = GPIOG_BASE + 0, G1 = GPIOG_BASE + 1, G2 = GPIOG_BASE + 2, G3 = GPIOG_BASE + 3, G4 = GPIOG_BASE + 4, G5 = GPIOG_BASE + 5, G6 = GPIOG_BASE + 6, G7 = GPIOG_BASE + 7, G8 = GPIOG_BASE + 8, G9 = GPIOG_BASE + 9, G10 = GPIOG_BASE + 10, G11 = GPIOG_BASE + 11, G12 = GPIOG_BASE + 12, G13 = GPIOG_BASE + 13, G14 = GPIOG_BASE + 14, G15 = GPIOG_BASE + 15,
        #ifdef GPIOH_BASE
        H0 = GPIOH_BASE + 0, H1 = GPIOH_BASE + 1, H2 = GPIOH_BASE + 2, H3 = GPIOH_BASE + 3, H4 = GPIOH_BASE + 4, H5 = GPIOH_BASE + 5, H6 = GPIOH_BASE + 6, H7 = GPIOH_BASE + 7, H8 = GPIOH_BASE + 8, H9 = GPIOH_BASE + 9, H10 = GPIOH_BASE + 10, H11 = GPIOH_BASE + 11, H12 = GPIOH_BASE + 12, H13 = GPIOH_BASE + 13, H14 = GPIOH_BASE + 14, H15 = GPIOH_BASE + 15,
        #endif
    };

    enum class Mode{Input = 0x0, Output = 0x1, AlternateFunction = 0x2, Analog = 0x3};
    enum class OutputSpeed{Low = 0x0, Medium = 0x1, High = 0x2, VeryHigh = 0x3};
    enum class PullMode{NoPull = 0x0,PullUp = 0x1,PullDown = 0x2};
    enum class AlternateFunction{AF0, AF1, AF2, AF3, AF4, AF5, AF6, AF7, AF8, AF9, AF10, AF11, AF12, AF13, AF14, AF15};

    void SetPinMode(Pin pin, Mode mode){
        uint32_t pinNumber = static_cast<uint32_t>(pin) & 0xF;
        uint32_t portBase = static_cast<uint32_t>(pin) & 0xFFFFFFF0;
        volatile uint32_t *modeRegister = reinterpret_cast<volatile uint32_t *>(portBase + 0x00);

        *modeRegister &= ~(0x3 << (pinNumber * 2));
        *modeRegister |= (static_cast<uint32_t>(mode) << (pinNumber * 2));
    }

    void SetPinOutputType(Pin pin, bool isOpenDrain)
    {
        uint32_t pinNumber = static_cast<uint32_t>(pin) & 0xF;
        uint32_t portBase = static_cast<uint32_t>(pin) & 0xFFFFFFF0;
        volatile uint32_t *outputTypeRegister = reinterpret_cast<volatile uint32_t *>(portBase + 0x04);

        if (isOpenDrain)
        {
            *outputTypeRegister |= (1 << pinNumber);
        }
        else
        {
            *outputTypeRegister &= ~(1 << pinNumber);
        }
    }

    void SetPinOutputSpeed(Pin pin, OutputSpeed speed)
    {
        uint32_t pinNumber = static_cast<uint32_t>(pin) & 0xF;
        uint32_t portBase = static_cast<uint32_t>(pin) & 0xFFFFFFF0;
        volatile uint32_t *speedRegister = reinterpret_cast<volatile uint32_t *>(portBase + 0x08);

        *speedRegister &= ~(0x3 << (pinNumber * 2));
        *speedRegister |= (static_cast<uint32_t>(speed) << (pinNumber * 2));
    }

    void SetPinPullMode(Pin pin, PullMode pull)
    {
        uint32_t pinNumber = static_cast<uint32_t>(pin) & 0xF;
        uint32_t portBase = static_cast<uint32_t>(pin) & 0xFFFFFFF0;
        volatile uint32_t *pullRegister = reinterpret_cast<volatile uint32_t *>(portBase + 0x0C);

        *pullRegister &= ~(0x3 << (pinNumber * 2));
        *pullRegister |= (static_cast<uint32_t>(pull) << (pinNumber * 2));
    }

    void SetPinAlternateFunction(Pin pin, AlternateFunction function)
    {
        uint32_t pinNumber = static_cast<uint32_t>(pin) & 0xF;
        uint32_t portBase = static_cast<uint32_t>(pin) & 0xFFFFFFF0;
        volatile uint32_t *afrRegister = reinterpret_cast<volatile uint32_t *>(portBase + 0x20 + ((pinNumber > 7) ? 0x04 : 0x00));

        *afrRegister &= ~(0xF << ((pinNumber % 8) * 4));
        *afrRegister |= (static_cast<uint32_t>(function) << ((pinNumber % 8) * 4));
    }
    bool GetPinInput(Pin pin)
    {
        uint32_t pinNumber = static_cast<uint32_t>(pin) & 0xF;
        uint32_t portBase = static_cast<uint32_t>(pin) & 0xFFFFFFF0;
        volatile uint32_t *inputDataRegister = reinterpret_cast<volatile uint32_t *>(portBase + 0x10);

        return (*inputDataRegister & (1 << pinNumber)) != 0;
    }

    void SetPinOutput(Pin pin, bool value)
    {
        uint32_t pinNumber = static_cast<uint32_t>(pin) & 0xF;
        uint32_t portBase = static_cast<uint32_t>(pin) & 0xFFFFFFF0;
        volatile uint32_t *outputDataRegister = reinterpret_cast<volatile uint32_t *>(portBase + 0x14);

        if (value)
        {
            *outputDataRegister |= (1 << pinNumber);
        }
        else
        {
            *outputDataRegister &= ~(1 << pinNumber);
        }
    }
}