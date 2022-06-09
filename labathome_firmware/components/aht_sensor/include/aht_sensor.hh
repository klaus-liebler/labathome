#pragma once
#include <stdint.h>
#include <driver/i2c.h>
#include <i2c_sensor.hh>

namespace AHT
{
     enum class ADDRESS
    {
        default_address = 0x38,
        Low = 0x38,
        High = 0x39,
    };
 class M:public I2CSensor
  {
  public:                                                                              
    M(i2c_port_t i2c_num, AHT::ADDRESS slaveaddr = AHT::ADDRESS::default_address); 
    esp_err_t Initialize(int64_t& waitTillFirstTrigger) override;                                                                                                       
    esp_err_t Trigger(int64_t& waitTillReadout) override;
    esp_err_t Readout(int64_t& waitTillNExtTrigger)override;
    esp_err_t Read(float &humidity, float &temperatur);
    esp_err_t Reset();
  private:
    uint32_t temp{0};
    uint32_t humid{0};
    uint8_t readStatus();
  };
}
