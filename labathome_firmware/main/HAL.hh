#pragma once

#include <stdio.h>
#include "sdkconfig.h"
#include "esp_system.h"
#include "WS2812.hh"
#include "labathomeerror.hh"

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
        virtual LabAtHomeErrorCode Init()=0;
        virtual void SensorLoop_ForInternalUseOnly()=0;
        virtual LabAtHomeErrorCode StartBuzzer(double freqHz)=0;
        virtual LabAtHomeErrorCode EndBuzzer()=0;
        virtual LabAtHomeErrorCode ColorizeLed(LED led, uint32_t color)=0;
        virtual LabAtHomeErrorCode UnColorizeAllLed()=0;
        virtual LabAtHomeErrorCode SetRelayState(bool state)=0;
        virtual LabAtHomeErrorCode SetHeaterState(float dutyInPercent)=0;
        virtual float GetHeaterState()=0;
        virtual LabAtHomeErrorCode SetFan1State(float dutyInPercent)=0;
        virtual float GetFan1State()=0;
        virtual LabAtHomeErrorCode SetFan2State(float dutyInPercent)=0;
        virtual float GetFan2State()=0;
        virtual LabAtHomeErrorCode SetLedPowerWhiteState(uint8_t dutyInpercent)=0;
        virtual bool GetButtonRedIsPressed()=0;
        virtual bool GetButtonEncoderIsPressed()=0;
        virtual bool GetButtonGreenIsPressed()=0;
        virtual bool IsMovementDetected()=0;
        virtual LabAtHomeErrorCode SetServoPosition(Servo servo, uint32_t angle_0_to_180)=0;
        virtual LabAtHomeErrorCode BeforeLoop()=0;
        virtual LabAtHomeErrorCode AfterLoop()=0;
        virtual LabAtHomeErrorCode GetHeaterTemperature(float *degreesCelcius);
        virtual LabAtHomeErrorCode GetAmbientBrightness(float *lux);
        virtual LabAtHomeErrorCode GetAirTemperature(float *degreesCelcius);
        virtual LabAtHomeErrorCode GetAirPressure(float *pa);
        virtual LabAtHomeErrorCode GetAirRelHumidity(float *percent);
        virtual LabAtHomeErrorCode GetADCValues(float **voltages);
        virtual int64_t GetMicros()=0;
        virtual uint32_t GetMillis()=0; 
};