#pragma once

#include <stdio.h>
#include "driver/i2c.h"
#include "driver/gpio.h"

enum class ads1115_register_addresses_t
{ // register address
    ADS1115_CONVERSION_REGISTER_ADDR = 0,
    ADS1115_CONFIG_REGISTER_ADDR,
    ADS1115_LO_THRESH_REGISTER_ADDR,
    ADS1115_HI_THRESH_REGISTER_ADDR,
    ADS1115_MAX_REGISTER_ADDR
};

enum class ads1115_mux_t
{ // multiplex options
    ADS1115_MUX_0_1 = 0,
    ADS1115_MUX_0_3,
    ADS1115_MUX_1_3,
    ADS1115_MUX_2_3,
    ADS1115_MUX_0_GND,
    ADS1115_MUX_1_GND,
    ADS1115_MUX_2_GND,
    ADS1115_MUX_3_GND,
};

enum class ads1115_fsr_t
{ // full-scale resolution options
    ADS1115_FSR_6_144 = 0,
    ADS1115_FSR_4_096,
    ADS1115_FSR_2_048,
    ADS1115_FSR_1_024,
    ADS1115_FSR_0_512,
    ADS1115_FSR_0_256,
};

enum class ads1115_sps_t
{ // samples per second
    ADS1115_SPS_8 = 0,
    ADS1115_SPS_16,
    ADS1115_SPS_32,
    ADS1115_SPS_64,
    ADS1115_SPS_128,
    ADS1115_SPS_250,
    ADS1115_SPS_475,
    ADS1115_SPS_860
};

enum class ads1115_mode_t
{
    ADS1115_MODE_CONTINUOUS = 0,
    ADS1115_MODE_SINGLE
};

typedef union
{ // configuration register
    struct
    {
        uint16_t COMP_QUE : 2;  // bits 0..  1  Comparator queue and disable
        uint16_t COMP_LAT : 1;  // bit  2       Latching Comparator
        uint16_t COMP_POL : 1;  // bit  3       Comparator Polarity
        uint16_t COMP_MODE : 1; // bit  4       Comparator Mode
        uint16_t DR : 3;        // bits 5..  7  Data rate
        uint16_t MODE : 1;      // bit  8       Device operating mode
        uint16_t PGA : 3;       // bits 9..  11 Programmable gain amplifier configuration
        uint16_t MUX : 3;       // bits 12.. 14 Input multiplexer configuration
        uint16_t OS : 1;        // bit  15      Operational status or single-shot conversion start
    } bit;
    uint16_t reg;
} ADS1115_CONFIG_REGISTER_Type;




class ADS1115
{
private:
    ADS1115_CONFIG_REGISTER_Type config;
    i2c_port_t i2c_port;
    int address;
    esp_err_t write_register(ads1115_register_addresses_t reg, uint16_t data);
    esp_err_t read_register(ads1115_register_addresses_t reg, uint8_t *data, uint8_t len);
public:
    ADS1115(const i2c_port_t i2c_port, const uint8_t address);
    ~ADS1115() {}
    // set configuration
    esp_err_t Init(ads1115_sps_t dr, TickType_t *howLongToWaitForResult);
    esp_err_t TriggerMeasurement(ads1115_mux_t mux);
    esp_err_t GetRaw(int16_t *val);    // get voltage in bits
    esp_err_t GetVoltage(float *val); // get voltage in volts
};
