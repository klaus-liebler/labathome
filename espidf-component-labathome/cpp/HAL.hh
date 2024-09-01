#pragma once

#include <cstdio>
#include "sdkconfig.h"
#include "esp_system.h"
#include "errorcodes.hh"
#include "crgb.hh"


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
        virtual ErrorCode SetFanDuty(uint8_t fanIndex, float dutyInPercent)=0;
        virtual ErrorCode GetFanDuty(uint8_t fanIndex, float* dutyInPercent)=0;
        virtual ErrorCode SetLedPowerWhiteDuty(float dutyInpercent)=0;
        virtual bool GetButtonRedIsPressed()=0;
        virtual bool GetButtonEncoderIsPressed()=0;
        virtual ErrorCode GetEncoderValue(int *value)=0;
        virtual bool GetButtonGreenIsPressed()=0;
        virtual bool IsMovementDetected()=0;
        virtual ErrorCode SetServoPosition(uint8_t servoIndex, float angle_0_to_180)=0;
        virtual ErrorCode SetAnalogOutput(uint8_t outputIndex, float volts)=0;
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
        virtual ErrorCode GetWifiRssiDb(float *db)=0;
        virtual int64_t GetMicros()=0;
        virtual uint32_t GetMillis()=0; 
        virtual ErrorCode OutputOneLineStatus()=0;
        virtual ErrorCode GreetUserOnStartup()=0;

        virtual ErrorCode GetAmbientBrightnessAnalog(float *lux)=0;
        virtual ErrorCode GetAmbientBrightnessDigital(float *lux)=0;
        virtual ErrorCode GetAirTemperatureDS18B20(float *degreesCelcius)=0;
        virtual ErrorCode GetAirTemperatureAHT21(float *degreesCelcius)=0;
        virtual ErrorCode GetAirTemperatureBME280(float *degreesCelcius)=0;
        virtual ErrorCode GetAirRelHumidityAHT21(float *percent)=0;
        virtual ErrorCode GetAirRelHumidityBME280(float *percent)=0;
        virtual ErrorCode GetDistanceMillimeters(uint16_t *value)=0;
};