
#include "include/ads1115.hh"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "i2c.hh"


static const char *TAG = "ADS1115";


ADS1115::ADS1115(i2c_port_t i2c_port, uint8_t address) : i2c_port(i2c_port), address(address){}

constexpr TickType_t SPSindex2WaitingTime[] = {125/portTICK_PERIOD_MS, 63/portTICK_PERIOD_MS, 32/portTICK_PERIOD_MS,
16/portTICK_PERIOD_MS, 8/portTICK_PERIOD_MS, 4/portTICK_PERIOD_MS, 3/portTICK_PERIOD_MS, 2/portTICK_PERIOD_MS};

esp_err_t ADS1115::Init(ads1115_sps_t sps, TickType_t *howLongToWaitForResult)
{
    config.bit.OS = 0; // always start conversion
    config.bit.MUX = (uint16_t)ads1115_mux_t::ADS1115_MUX_0_GND;
    config.bit.PGA = (uint16_t)ads1115_fsr_t::ADS1115_FSR_4_096;
    config.bit.MODE = (uint16_t)ads1115_mode_t::ADS1115_MODE_SINGLE;
    config.bit.DR = (uint16_t)sps;
    config.bit.COMP_MODE = 0;
    config.bit.COMP_POL = 0;
    config.bit.COMP_LAT = 0;
    config.bit.COMP_QUE = 0b11;
    ESP_LOGI(TAG, "Initial content of config register will be 0x%04X", config.reg);
    *howLongToWaitForResult=SPSindex2WaitingTime[config.bit.DR];
    
	uint8_t out[2];
    out[0] = config.reg >> 8;   // get 8 greater bits
    out[1] = config.reg & 0xFF; // get 8 lower bits
    return I2C::WriteReg(this->i2c_port, this->address, (uint8_t)ads1115_register_addresses_t::ADS1115_CONFIG_REGISTER_ADDR, out, 2);
}



esp_err_t ADS1115::TriggerMeasurement(ads1115_mux_t mux)
{
    config.bit.MUX=(uint16_t)mux;
    config.bit.OS=1;
    uint8_t out[2];
    out[0] = config.reg >> 8;   // get 8 greater bits
    out[1] = config.reg & 0xFF; // get 8 lower bits
    return I2C::WriteReg(this->i2c_port, this->address, (uint8_t)ads1115_register_addresses_t::ADS1115_CONFIG_REGISTER_ADDR, out, 2);
}
esp_err_t ADS1115::GetRaw(int16_t *val)
{
    uint8_t data[2];
    esp_err_t err = I2C::ReadReg(this->i2c_port, this->address, (uint8_t)(ads1115_register_addresses_t::ADS1115_CONVERSION_REGISTER_ADDR), data, 2);
    *val= ((uint16_t)data[0] << 8) | (uint16_t)data[1];
    return err;
}

esp_err_t ADS1115::GetVoltage(float *val)
{
    const double fsr[] = {6.144, 4.096, 2.048, 1.024, 0.512, 0.256};
    const int16_t bits = (1L << 15) - 1;
    int16_t raw;

    esp_err_t ret = GetRaw(&raw);
    *val= (float)raw * fsr[config.bit.PGA] / (double)bits;
    return ret;
}