#pragma once

#include <stdio.h>
#include "sdkconfig.h"
#include "esp_system.h"
#include "errorcodes.hh"
#include "crgb.hh"

enum class Servo : uint8_t
{
    Servo1 = 0,
    Servo2 = 1,
};

class HAL
{
    public:
        virtual ErrorCode HardwareTest(){return ErrorCode::OK;};
        virtual ErrorCode InitAndRun()=0;
        virtual ErrorCode StartBuzzer(float freqHz)=0;
        virtual ErrorCode EndBuzzer()=0;
        virtual ErrorCode ColorizeLed(uint8_t ledIndex, CRGB color)=0;
        ErrorCode ColorizeLed(uint8_t ledIndex, uint32_t color){return ColorizeLed(ledIndex, CRGB(color));}
        virtual ErrorCode UnColorizeAllLed()=0;
        virtual ErrorCode SetRelayState(bool state)=0;
        virtual ErrorCode SetHeaterDuty(float dutyInPercent)=0;
        virtual float GetHeaterState()=0;
        virtual ErrorCode SetFan1Duty(float dutyInPercent)=0;
        virtual float GetFan1State()=0;
        virtual ErrorCode SetFan2Duty(float dutyInPercent)=0;
        virtual float GetFan2State()=0;
        virtual ErrorCode SetLedPowerWhiteDuty(float dutyInpercent)=0;
        virtual bool GetButtonRedIsPressed()=0;
        virtual bool GetButtonEncoderIsPressed()=0;
        virtual ErrorCode GetEncoderValue(int *value)=0;
        virtual bool GetButtonGreenIsPressed()=0;
        virtual bool IsMovementDetected()=0;
        virtual ErrorCode SetServo1Position(float angle_0_to_180)=0;
        virtual ErrorCode SetServo2Position(float angle_0_to_180)=0;
        virtual ErrorCode SetAnalogOutput(float volts)=0;
        virtual ErrorCode BeforeLoop()=0;
        virtual ErrorCode AfterLoop()=0;
        virtual float GetUSBCVoltage()=0;
        virtual ErrorCode GetCO2PPM(float *co2PPM)=0;
        virtual ErrorCode GetHeaterTemperature(float *degreesCelcius)=0;
        virtual ErrorCode GetAmbientBrightness(float *lux)=0;
        virtual ErrorCode GetAirTemperature(float *degreesCelcius)=0;
        virtual ErrorCode GetAirPressure(float *pa)=0;
        virtual ErrorCode GetAirRelHumidity(float *percent)=0;
        virtual ErrorCode GetAirQuality(float *qualityPercent)=0;
        virtual ErrorCode GetAirSpeed(float *speedMetersPerSecond)=0;
        virtual ErrorCode GetAnalogInputs(float **voltages)=0;
        virtual ErrorCode SetSound(int32_t songNumber)=0;
        virtual ErrorCode GetSound(int32_t *songNumber)=0;
        virtual ErrorCode GetFan1Rpm(float* rpm)=0;
        virtual ErrorCode GetWifiRssiDb(float *db)=0;
        virtual int64_t GetMicros()=0;
        virtual uint32_t GetMillis()=0; 
        virtual ErrorCode GetFFT64(float *magnitudes64)=0;
        virtual ErrorCode UpdatePinConfiguration(uint8_t* configMessage, size_t configMessagelen)=0;
        virtual ErrorCode OutputOneLineStatus()=0;
        virtual ErrorCode GreetUserOnStartup()=0;
};