#include <inttypes.h>
#include <i2c.hh>
#include "aht_sensor.hh"
#include <esp_log.h>
#include <string.h>
#include <common.hh>
#define TAG "AHT"

namespace AHT
{
    constexpr uint8_t eSensorCalibrateCmd[]{0xE1, 0x08, 0x00};
    constexpr uint8_t eSensorNormalCmd[]{0xA8, 0x00, 0x00};
    constexpr uint8_t eSensorMeasureCmd[]{0xAC, 0x33, 0x00};
    constexpr uint8_t eSensorResetCmd{0xBA};
    constexpr bool GetRHumidityCmd{true};
    constexpr bool GetTempCmd{false};

    M::M(i2c_port_t i2c_num, AHT::ADDRESS slaveaddr) : I2CSensor(i2c_num, (uint8_t)slaveaddr)
    {
    }
    esp_err_t M::Initialize(int64_t &waitTillFirstTrigger)
    {
        if (I2C::IsAvailable(this->i2c_num, (uint8_t)this->address_7bit) != ESP_OK)
        {
            ESP_LOGW(TAG, "AHTxy not found");
        }
        if (I2C::Write(this->i2c_num, (uint8_t)this->address_7bit, eSensorCalibrateCmd, sizeof(eSensorCalibrateCmd)) != ESP_OK)
        {
            ESP_LOGE(TAG, "Could not write calibration commands.");
            return ESP_FAIL;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
        if ((readStatus() & 0x68) != 0x08)
        {
            ESP_LOGE(TAG, "Incorrect status after calibration");
            return ESP_FAIL;
        }
        ESP_LOGI(TAG, "AHTxy successfully initialized");
        return ESP_OK;
    }
    esp_err_t M::Readout(int64_t &waitTillNextTrigger)
    {
        uint8_t temp[6];
        waitTillNextTrigger=500;
        I2C::Read(this->i2c_num, (uint8_t)this->address_7bit, temp, 6);
        this->humid = ((temp[1] << 16) | (temp[2] << 8) | temp[3]) >> 4;
        this->temp = ((temp[3] & 0x0F) << 16) | (temp[4] << 8) | temp[5];
        ESP_LOGD(TAG, "Readout H=%d T=%d", this->humid, this->temp);
        return ESP_OK;
    }
    esp_err_t M::Trigger(int64_t &waitTillReadout)
    {
        waitTillReadout = 500;
        ESP_LOGD(TAG, "Trigger!!!!");
        return I2C::Write(this->i2c_num, (uint8_t)this->address_7bit, eSensorMeasureCmd, sizeof(eSensorMeasureCmd));
    }

    esp_err_t M::Read(float &humidity, float &temperature)
    {
        humidity = this->humid*100.0f/1048576.0f;
        temperature = (this->temp*200.0f/1048576.0f)-50.0f;
        return ESP_OK;
    }

    esp_err_t M::Reset(){
        return I2C::Write(this->i2c_num, (uint8_t)this->address_7bit, &eSensorResetCmd, 1);
    }

    uint8_t M::readStatus(){
        uint8_t result = 0;
        I2C::Read(this->i2c_num, (uint8_t)this->address_7bit, &result, 1);
        return result;
    }
}