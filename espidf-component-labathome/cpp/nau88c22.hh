#pragma once
#include <cstdio>
#include <cstdint>
#include <cmath>
#include <cstring>
#include <bit>
#include <algorithm>

#include <driver/i2s_std.h>
#include <driver/i2c_master.h>

#include <esp_log.h>
#define TAG "NAU88C22"
namespace nau88c22
{

    enum class eNauInit
    {
        ORIGINAL_DF_ROBOT,
        BTL,
        STEREO
    };

    constexpr eNauInit nauInit{eNauInit::ORIGINAL_DF_ROBOT}; // STEREO works, ORIGINAL_DF_ROBOT works, BTL does not work
    constexpr uint8_t NAU8822_I2C_ADDRESS{0X1A};
    constexpr size_t AUDIO_BUFFER_SIZE_IN_SAMPLES{2048};
    constexpr size_t TARGET_CHANNELS{2};

    class M : public CodecManager::I2sWithHardwareVolume
    {
    private:
        i2c_master_bus_handle_t bus_handle;
        i2c_master_dev_handle_t dev_handle;
        gpio_num_t power_down;
        gpio_num_t mclk;
        gpio_num_t bck;
        gpio_num_t ws;
        gpio_num_t data;
        uint8_t volumeSpeakers = 127;

        esp_err_t i2cWriteNAU8822(uint8_t addr, int16_t data)
        {
            uint8_t write_buf[2] = {(uint8_t)((addr << 1) | (data >> 8)), (uint8_t)((data & 0x00ff))};
            return i2c_master_transmit(dev_handle, write_buf, sizeof(write_buf), 1000);
        }

        esp_err_t i2cReadNAU8822(uint8_t addr, uint16_t &reg_data)
        {
            uint8_t write_buf[1] = {(uint8_t)((addr << 1))};
            uint8_t read_buf[2];
            esp_err_t espRc = i2c_master_transmit_receive(dev_handle, write_buf, sizeof(write_buf), read_buf, sizeof(read_buf), 1000);
            reg_data = read_buf[1] + (read_buf[0] << 8);
            return espRc;
        }

        esp_err_t i2cCheckNAU8822()
        {
            esp_err_t err = i2c_master_probe(bus_handle, NAU8822_I2C_ADDRESS, 1000);
            if(err!=ESP_OK){
                ESP_LOGE(TAG, "NAU88C22 not responding");
                return ESP_FAIL;
            }
            uint16_t data{0};
            i2cReadNAU8822(0x3F, data);
            if (data != 0x1A)
            {
                ESP_LOGE(TAG, "NAU8822 does respond the wrong id %d", data);
                return ESP_FAIL;
            }
            ESP_LOGI(TAG, "Found NAU8822!");
            return ESP_OK;
        }

        void setSpeakersVolume(uint8_t volume0_255)
        {
            volume0_255 >>= 2;
            i2cWriteNAU8822(54, volume0_255);
            i2cWriteNAU8822(55, volume0_255 + 256);
        }

        void muteSpeakers(void)
        {
            i2cWriteNAU8822(54, 0x040);
            i2cWriteNAU8822(55, 0x140);
        }

        void setHeadphonesVolume(uint8_t volume0_255)
        {
            volume0_255 >>= 2;
            i2cWriteNAU8822(52, volume0_255);
            i2cWriteNAU8822(53, volume0_255 + 256);
        }

        void muteHeadphones(void)
        {
            i2cWriteNAU8822(52, 0x040);
            i2cWriteNAU8822(53, 0x140);
        }

        void i2cSetupNAU8822Play(uint8_t initialVolume_0_255 = 255) // this is the hardware default volume
        {
            if (nauInit == eNauInit::BTL)
            {
                ESP_ERROR_CHECK(i2cWriteNAU8822(0, 0x000)); // Software Reset
                vTaskDelay(pdMS_TO_TICKS(100));
                muteSpeakers(); // according to datasheet power up procedure
                muteHeadphones();
                i2cWriteNAU8822(1, 0b1'0000'1101); // according to datasheet power up procedure
                vTaskDelay(pdMS_TO_TICKS(250));    // according to datasheet power up procedure
                i2cWriteNAU8822(2, 0b1'1000'0000); // Power 2: Enable Headphones
                i2cWriteNAU8822(3, 0b0'0110'1111); // Power 3: Enable LR-speaker and LR-mixer and LR-DAC
                i2cWriteNAU8822(4, 0b0'0001'0000); // Audio Interface: normal phase, 16bit (instead of default 24), standard I2S format
                i2cWriteNAU8822(6, 0b0'0000'1100); // MCLK, pin#11 used as master clock, divided by 1, divide by 8, slave mode
                // i2cWriteNAU8822(10, 0b0'0000'1000); //Oversampling = x128 //Achtung, kein Softmute einstellen, sonst immer! stumm!
                // i2cWriteNAU8822(14, 0x108); //ADC Oversampling = x128
                i2cWriteNAU8822(43, 0b0'0001'0000); // Right Speaker Submixer -->BTL-Configuration
                i2cWriteNAU8822(49, 0b0'0101'1110); // Output Control: Left DAC output to RMIX!. Außerdem: Thermal shutdown enable, Speaker für 5V Versorgungsspannung optimiert.
                i2cWriteNAU8822(50, 0b0'0000'0001); // Left mixer.
                i2cWriteNAU8822(51, 0x000);         // Right mixer.
            }
            else if (nauInit == eNauInit::ORIGINAL_DF_ROBOT)
            {
                i2cWriteNAU8822(0, 0x000);
                vTaskDelay(pdMS_TO_TICKS(100));
                i2cWriteNAU8822(1, 0x1FF);
                i2cWriteNAU8822(2, 0x1BF);
                i2cWriteNAU8822(3, 0x1FF);
                i2cWriteNAU8822(4, 0x010);
                i2cWriteNAU8822(5, 0x000);
                i2cWriteNAU8822(6, 0x00C);
                i2cWriteNAU8822(7, 0x000);
                i2cWriteNAU8822(13, 0x09f);
                i2cWriteNAU8822(10, 0x008);
                i2cWriteNAU8822(14, 0x108);
                i2cWriteNAU8822(15, 0x0FF);
                i2cWriteNAU8822(16, 0x1FF);
                i2cWriteNAU8822(45, 0x0bf);
                i2cWriteNAU8822(46, 0x1bf);
                i2cWriteNAU8822(47, 0x175);
                i2cWriteNAU8822(49, 0x042);
                i2cWriteNAU8822(50, 0x1DD);
                i2cWriteNAU8822(51, 0x001);
            }

            setSpeakersVolume(initialVolume_0_255);
            setHeadphonesVolume(initialVolume_0_255);
        }

    public:
        M(
            i2c_master_bus_handle_t bus_handle,
            gpio_num_t mclk,
            gpio_num_t bck,
            gpio_num_t ws,
            gpio_num_t data,
            uint8_t initialVolume = 127) : bus_handle(bus_handle), mclk(mclk), bck(bck), ws(ws), data(data), volumeSpeakers(initialVolume)
        {
        }

        ErrorCode SetPowerState(bool power) override
        {
            ESP_LOGW(TAG, "SetPowerState not yet supported");
            return ErrorCode::OK;
        }
        ErrorCode SetVolume(uint8_t volume0_255) override
        {
            if (volume0_255 == volumeSpeakers)
            {
                return ErrorCode::OK;
            }
            if (volume0_255 == 0)
            {
                muteSpeakers();
                muteHeadphones();
            }
            else
            {
                setSpeakersVolume(volume0_255);
                setHeadphonesVolume(volume0_255);
            }
            volumeSpeakers = volume0_255;
            return ErrorCode::OK;
        }

        ErrorCode Init() override
        {
            ESP_LOGI(TAG, "Setup I2S");
            RETURN_ON_ERRORCODE(this->InitI2sEsp32(mclk, bck, ws, data));
            ESP_LOGI(TAG, "Setup of the NAU88C22 begins");

            i2c_device_config_t dev_cfg = {
                .dev_addr_length = I2C_ADDR_BIT_LEN_7,
                .device_address = NAU8822_I2C_ADDRESS,
                .scl_speed_hz = 100000,
                .scl_wait_us=0,
                .flags=0,
            };
            ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &(this->dev_handle)));

            esp_err_t err = i2cCheckNAU8822();
            if(err!=ESP_OK){
                return ErrorCode::GENERIC_ERROR;
            }
            i2cSetupNAU8822Play(volumeSpeakers);
            return ErrorCode::OK;
        }
    };

    void makeStereo(int16_t *read_buf, size_t samplesAvailable)
    {
        for (size_t i = 0; i < samplesAvailable; i++)
        {
            read_buf[2 * samplesAvailable - 1 - 2 * i] = read_buf[2 * samplesAvailable - 2 - 2 * i] = read_buf[samplesAvailable - 1 - i];
        }
    }
}
#undef TAG