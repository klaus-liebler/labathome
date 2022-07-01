#pragma once

#include <stdio.h>
#include "sdkconfig.h"
#include "esp_system.h"
#include "errorcodes.hh"

enum class LED : uint8_t
{
    LED_RED=0,
    LED_YELLOW=1,
    LED_GREEN=2,
    LED_3=3,
    LED_4=4,
    LED_5=5,
    LED_6=6,
    LED_7=7,
};



enum class Servo : uint8_t
{
    Servo1 = 0,
    Servo2 = 1,
};

class HAL
{
    public:
        virtual ErrorCode InitAndRun()=0;
        virtual ErrorCode HardwareTest()=0;
        virtual ErrorCode StartBuzzer(float freqHz)=0;
        virtual ErrorCode EndBuzzer()=0;
        virtual ErrorCode ColorizeLed(LED led, uint32_t color)=0;
        virtual ErrorCode UnColorizeAllLed()=0;
        virtual ErrorCode SetRelayState(bool state)=0;
        virtual ErrorCode SetHeaterState(float dutyInPercent)=0;
        virtual float GetHeaterState()=0;
        virtual ErrorCode SetFan1Duty(float dutyInPercent)=0;
        virtual float GetFan1State()=0;
        virtual ErrorCode SetFan2State(float dutyInPercent)=0;
        virtual float GetFan2State()=0;
        virtual ErrorCode SetLedPowerWhiteState(float dutyInpercent)=0;
        virtual bool GetButtonRedIsPressed()=0;
        virtual bool GetButtonEncoderIsPressed()=0;
        virtual ErrorCode GetEncoderValue(int *value);
        virtual bool GetButtonGreenIsPressed()=0;
        virtual bool IsMovementDetected()=0;
        virtual ErrorCode SetServo1Position(float angle_0_to_180)=0;
        virtual ErrorCode SetServo2Position(float angle_0_to_180)=0;
        virtual ErrorCode SetAnalogOutput(float volts)=0;
        virtual ErrorCode BeforeLoop()=0;
        virtual ErrorCode AfterLoop()=0;
        virtual float GetUSBCVoltage();
        virtual ErrorCode GetCO2PPM(float *co2PPM);
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
        virtual ErrorCode GetFFT64(float *magnitudes64);
        virtual ErrorCode UpdatePinConfiguration(uint8_t* configMessage, size_t configMessagelen);
};