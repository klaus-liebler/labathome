#pragma once

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"
#include "onewire_bus.h"
#include "onewire_cmd.h"
#include "onewire_crc.h"
#include "ds18b20.h"

#define TAG "ds18b20"

constexpr uint8_t DS18B20_CMD_CONVERT_TEMP{0x44};
constexpr uint8_t DS18B20_CMD_WRITE_SCRATCHPAD{0x4E};
constexpr uint8_t DS18B20_CMD_READ_SCRATCHPAD{0xBE};

/**
 * @brief Structure of DS18B20's scratchpad
 */
typedef struct
{
    uint8_t temp_lsb;      /*!< lsb of temperature */
    uint8_t temp_msb;      /*!< msb of temperature */
    uint8_t th_user1;      /*!< th register or user byte 1 */
    uint8_t tl_user2;      /*!< tl register or user byte 2 */
    uint8_t configuration; /*!< resolution configuration register */
    uint8_t _reserved1;
    uint8_t _reserved2;
    uint8_t _reserved3;
    uint8_t crc_value; /*!< crc value of scratchpad data */
} __attribute__((packed)) ds18b20_scratchpad_t;

class DS18B20
{
private:
    onewire_bus_handle_t bus;
    onewire_device_address_t addr;
    uint8_t th_user1;
    uint8_t tl_user2;
    ds18b20_resolution_t resolution;

    esp_err_t SendCommand(uint8_t cmd)
    {
        // send command
        uint8_t tx_buffer[10] = {0};
        tx_buffer[0] = ONEWIRE_CMD_MATCH_ROM;
        memcpy(&tx_buffer[1], &addr, sizeof(addr));
        tx_buffer[sizeof(addr) + 1] = cmd;
        return onewire_bus_write_bytes(bus, tx_buffer, sizeof(tx_buffer));
    }

    DS18B20(onewire_bus_handle_t bus, onewire_device_address_t addr, ds18b20_resolution_t resolution):bus(bus), addr(addr), th_user1(0), tl_user2(0), resolution(resolution){}

public:
    static DS18B20* BuildFromOnewireDevice(onewire_device_t *device)
    {
        if(!device){
            return nullptr;
        }
        // check ROM ID, the family code of DS18B20 is 0x28
        if ((device->address & 0xFF) != 0x28)
        {
            ESP_LOGD(TAG, "%016llX is not a DS18B20 device", device->address);
            return nullptr;
        }
        return new DS18B20(device->bus, device->address, DS18B20_RESOLUTION_12B);// DS18B20 default resolution is 12 bits
    }

    esp_err_t SetResolution(ds18b20_resolution_t resolution)
    {

        ESP_RETURN_ON_ERROR(onewire_bus_reset(bus), TAG, "reset bus error");
        ESP_RETURN_ON_ERROR(SendCommand(DS18B20_CMD_WRITE_SCRATCHPAD), TAG, "send DS18B20_CMD_WRITE_SCRATCHPAD failed");

        // write new resolution to scratchpad
        const uint8_t resolution_data[] = {0x1F, 0x3F, 0x5F, 0x7F};
        uint8_t tx_buffer[3] = {0};
        tx_buffer[0] = th_user1;
        tx_buffer[1] = tl_user2;
        tx_buffer[2] = resolution_data[resolution];
        ESP_RETURN_ON_ERROR(onewire_bus_write_bytes(bus, tx_buffer, sizeof(tx_buffer)), TAG, "send new resolution failed");

        resolution = resolution;
        return ESP_OK;
    }

    esp_err_t TriggerTemperatureConversion()
    {
        ESP_RETURN_ON_ERROR(onewire_bus_reset(bus), TAG, "reset bus error");
        // send command: DS18B20_CMD_CONVERT_TEMP
        ESP_RETURN_ON_ERROR(SendCommand(DS18B20_CMD_CONVERT_TEMP), TAG, "send DS18B20_CMD_CONVERT_TEMP failed");
        return ESP_OK;
    }

    esp_err_t GetTemperature(float *ret_temperature)
    {
        // reset bus and check if the ds18b20 is present
        ESP_RETURN_ON_ERROR(onewire_bus_reset(bus), TAG, "reset bus error");

        // send command: DS18B20_CMD_READ_SCRATCHPAD
        ESP_RETURN_ON_ERROR(SendCommand(DS18B20_CMD_READ_SCRATCHPAD), TAG, "send DS18B20_CMD_READ_SCRATCHPAD failed");

        // read scratchpad data
        ds18b20_scratchpad_t scratchpad;
        ESP_RETURN_ON_ERROR(onewire_bus_read_bytes(bus, (uint8_t *)&scratchpad, sizeof(scratchpad)),
                            TAG, "error while reading scratchpad data");
        // check crc
        ESP_RETURN_ON_FALSE(onewire_crc8(0, (uint8_t *)&scratchpad, 8) == scratchpad.crc_value, ESP_ERR_INVALID_CRC, TAG, "scratchpad crc error");

        const uint8_t lsb_mask[4] = {0x07, 0x03, 0x01, 0x00}; // mask bits not used in low resolution
        uint8_t lsb_masked = scratchpad.temp_lsb & (~lsb_mask[scratchpad.configuration >> 5]);
        *ret_temperature = (((int16_t)scratchpad.temp_msb << 8) | lsb_masked) / 16.0f;

        return ESP_OK;
    }
};
#undef TAG