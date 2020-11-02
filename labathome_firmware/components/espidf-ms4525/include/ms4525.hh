#pragma once
 
#include <stdio.h>
#include "driver/i2c.h"
#include "driver/gpio.h"
 
/* Register address */
constexpr uint8_t  ADDR_READ_MR=0x00;    /* write to this address to start conversion */
 
// MS4525D sensor full scale range and units
constexpr int16_t MS4525FullScaleRange = 1;  // 1 psi
//const int16_t MS4525FullScaleRange = 0.0689476;  // 1 psi in Bar
//const int16_t MS4525FullScaleRange = 6895;  // 1 psi in Pascal
//const int16_t MS4525FullScaleRange = 2;  // 2 psi
//const int16_t MS4525FullScaleRange = 5;  // 5 psi
 
// MS4525D Sensor type (A or B) comment out the wrong type assignments
// Type A (10% to 90%)
constexpr int16_t MS4525MinScaleCounts = 1638;
constexpr int16_t MS4525FullScaleCounts = 14746;
 // Type B (5% to 95%)
//const int16_t MS4525MinScaleCounts = 819;
//const int16_t MS4525FullScaleCounts = 15563;

constexpr int16_t MS4525Span=MS4525FullScaleCounts-MS4525MinScaleCounts;
 
//MS4525D sensor pressure style, differential or otherwise. Comment out the wrong one.
//Differential
constexpr int16_t MS4525ZeroCounts=(MS4525MinScaleCounts+MS4525FullScaleCounts)/2;
 
 enum class MS4525_Status{
     NormalOperation=0b00,
     Reserved=0b01,
     StaleData=0b10,
     Fault=0b11,
 };

 enum class MS4523_Adress{
     I=0x28,
     J=0x36,
     K=0x46,
     DIGIT_0=0x48,
     DIGIT_1=0x49,
     DIGIT_2=0x4A,
     DIGIT_3=0x4B,
     DIGIT_4=0x4C,
     DIGIT_5=0x4D,
     DIGIT_6=0x4E,
     DIGIT_7=0x4F,
     DIGIT_8=0x50,
     DIGIT_9=0x51,
 };

 /** airspeed scaling factors; out = (in * Vscale) + offset */
struct airspeed_scale {
    float   offset_pa;
    float   scale;
};
 
class MS4525DO
{
    public:

        MS4525DO(i2c_port_t i2c_port, MS4523_Adress address);
        ~MS4525DO();
        esp_err_t Init(void);
        esp_err_t Read(void);              // returns status of measurement
        float GetPSI(void);             // returns the PSI of last measurement
        float GetTemperature(void);     // returns temperature of last measurement
        float GetAirSpeedMetersPerSecond(void);        // calculates and returns the airspeed
        MS4525_Status GetStatus();
    private:
        i2c_port_t i2c_port;
        uint8_t address;
        MS4525_Status _status= MS4525_Status::Reserved;
        uint16_t    P_dat;  // 14 bit pressure data
        uint16_t    T_dat;  // 11 bit temperature data
}; 
 
 

 