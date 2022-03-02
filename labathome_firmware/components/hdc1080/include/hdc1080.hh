#pragma once
#include <stdint.h>
#include <i2c_sensor.hh>

namespace hdc1080{
    constexpr uint16_t I2C_ADDRESS=0x40;
    constexpr uint8_t TEMPERATURE_OFFSET = 0x00;
    constexpr uint8_t HUMIDITY_OFFSET = 0x01;
    constexpr uint8_t CONFIG_OFFSET = 0x02;
    enum class HUMRESOLUTION{
        __8bit=0b10,
        _11bit=0b01,
        _14bit=0b00,
    };
    enum class TEMPRESOLUTION{
        _14bit=0b0,
        _11bit=0b1,
    };
    enum class MODE{
        TorH=0b0,
        TandH=0b1,
    };
    enum class HEATER{
        DISABLE = 0b0,
        ENABLE  = 0b1,
    };

    typedef union {
	    uint8_t rawData;
        struct {
            uint8_t HumidityMeasurementResolution : 2;
            uint8_t TemperatureMeasurementResolution : 1;
            uint8_t BatteryStatus : 1;
            uint8_t ModeOfAcquisition : 1;
            uint8_t Heater : 1;
            uint8_t ReservedAgain : 1;
            uint8_t SoftwareReset : 1;
        };
    } ConfigRegister;
    
    
    class M:public I2CSensor{
        public:
            M(i2c_port_t i2c_num):I2CSensor(i2c_num, I2C_ADDRESS){}
            ErrorCode Reconfigure(TEMPRESOLUTION tempRes, HUMRESOLUTION humRes, HEATER heater){
                this->tempRes=tempRes;
                this->humRes=humRes;
                this->heater=heater;
                ReInit();
                return ErrorCode::OK;
            }
        private:
           
           TEMPRESOLUTION tempRes{TEMPRESOLUTION::_14bit};
           HUMRESOLUTION humRes{HUMRESOLUTION::_14bit};
           HEATER heater{HEATER::DISABLE};
           float t{0.0};
           float h{0.0};
        protected:
        
        esp_err_t Initialize(int64_t& wait) override{
            ConfigRegister config;
            I2C::ReadReg(i2c_num, I2C_ADDRESS, CONFIG_OFFSET, &config.rawData, 1);
            config.HumidityMeasurementResolution= (uint8_t)humRes;
            config.TemperatureMeasurementResolution= (uint8_t)tempRes;
            config.Heater= (uint8_t)heater;
            uint8_t dummy[2] = {config.rawData, 0x00};
            return I2C::WriteReg(i2c_num, I2C_ADDRESS, CONFIG_OFFSET, dummy, 2);
        }
        esp_err_t Trigger(int64_t& wait) override{
            wait=100;
            return I2C::WriteReg(i2c_num, I2C_ADDRESS, TEMPERATURE_OFFSET, NULL, 0);
        }

        esp_err_t Readout(int64_t& wait) override{
            wait=100;
            uint16_t rawT{0};
            if(ESP_OK!=I2C::ReadReg(i2c_num, I2C_ADDRESS, TEMPERATURE_OFFSET, (uint8_t*)&rawT, 2)){
                return ESP_FAIL;
            }
	        t= (rawT / pow(2, 16)) * 165.0 - 40.0;
            uint16_t rawH{0};
            if(ESP_OK!=I2C::ReadReg(i2c_num, I2C_ADDRESS, HUMIDITY_OFFSET, (uint8_t*)&rawH, 2)){
                return ESP_FAIL;
            }
	        h = (rawH / pow(2, 16)) * 100.0;
            return ESP_OK;
        }
    };

}
