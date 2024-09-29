#pragma once
#include <cstdint>
namespace I2C_SETUP{
    constexpr uint8_t STM32_I2C_ADDRESS{126};
}

struct __attribute__((packed)) S2E_t
{
    uint8_t Status;
    uint8_t ButtonRed:1;
    uint8_t ButtonYellow:1;
    uint8_t Movement:1;
    uint8_t Blfault:1;
    uint16_t Rotenc;
    uint16_t Brightness;
    uint16_t UsbpdVoltage_mv;
    uint16_t Adc0;
    uint16_t Adc1;
};

struct __attribute__((packed)) E2S_t{
    uint8_t AddressPointer;
    uint8_t Relay:1;
    uint8_t Blreset:1;
    uint8_t Servo[4];
    uint8_t Fan[2];
    uint16_t UsbpdVoltage_mv;
    uint16_t Dac0;
    uint16_t Dac1;
    uint8_t Heater;
    uint8_t LedPower;
    uint8_t ThreephaseMode;
    uint8_t ThreephaseP1;
    uint8_t ThreephaseP2;
    uint8_t ThreephaseP3;
};

constexpr int E2S_s = sizeof(E2S_t);
constexpr int S2E_s = sizeof(S2E_t);
