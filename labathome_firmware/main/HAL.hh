#pragma once

#include <stdio.h>
#include "sdkconfig.h"
#include "esp_system.h"
#include "errorcodes.hh"

enum class LED : uint8_t
{
    LED_RED,
    LED_YELLOW,
    LED_GREEN,
    LED_3,
    LED_4,
    LED_5,
    LED_6,
    LED_7,
};



enum class Servo : uint8_t
{
    Servo1 = 0,
    Servo2 = 1,
};

class HAL
{
    public:
        virtual ErrorCode Init()=0;
        virtual ErrorCode HardwareTest()=0;
        virtual void SensorLoop_ForInternalUseOnly()=0;
        virtual ErrorCode StartBuzzer(double freqHz)=0;
        virtual ErrorCode EndBuzzer()=0;
        virtual ErrorCode ColorizeLed(LED led, uint32_t color)=0;
        virtual ErrorCode UnColorizeAllLed()=0;
        virtual ErrorCode SetRelayState(bool state)=0;
        virtual ErrorCode SetHeaterState(float dutyInPercent)=0;
        virtual float GetHeaterState()=0;
        virtual ErrorCode SetFan1State(float dutyInPercent)=0;
        virtual float GetFan1State()=0;
        virtual ErrorCode SetFan2State(float dutyInPercent)=0;
        virtual float GetFan2State()=0;
        virtual ErrorCode SetLedPowerWhiteState(uint8_t dutyInpercent)=0;
        virtual bool GetButtonRedIsPressed()=0;
        virtual bool GetButtonEncoderIsPressed()=0;
        virtual ErrorCode GetEncoderValue(int16_t *value);
        virtual bool GetButtonGreenIsPressed()=0;
        virtual bool IsMovementDetected()=0;
        virtual ErrorCode SetServo1Position(uint32_t angle_0_to_180)=0;
        virtual ErrorCode SetServo2Position(uint32_t angle_0_to_180)=0;
        virtual ErrorCode BeforeLoop()=0;
        virtual ErrorCode AfterLoop()=0;
        virtual ErrorCode GetHeaterTemperature(float *degreesCelcius);
        virtual ErrorCode GetAmbientBrightness(float *lux);
        virtual ErrorCode GetAirTemperature(float *degreesCelcius);
        virtual ErrorCode GetAirPressure(float *pa);
        virtual ErrorCode GetAirRelHumidity(float *percent);
        virtual ErrorCode GetAirSpeed(float *speedMetersPerSecond);
        virtual ErrorCode GetADCValues(float **voltages);
        virtual ErrorCode PlaySong(uint32_t songNumber);
        virtual int64_t GetMicros()=0;
        virtual uint32_t GetMillis()=0; 
};