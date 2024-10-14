#pragma once
#include <i2c_sensor.hh>
#include <common-esp32.hh>
#include <bme68x.h>
#include <bme68x_defs.h>
#define TAG "BME680"
#include <esp_log.h>

#define BME680x_ON_ERROR(x)                                                                                    \
    do                                                                                                        \
    {                                                                                                         \
        int8_t err_rc_ = (x);                                                                              \
        if (err_rc_ != 0)                                                                      \
        {                                                                                                     \
            ESP_LOGE(TAG, "%s(%d): BME68x-function returned %d!", __FUNCTION__, __LINE__, err_rc_); \
            return ErrorCode::GENERIC_ERROR;                                                                                   \
        }                                                                                                     \
    } while (0)

namespace BME68x
{
    
    enum class ADDRESS{

        LOW=0x76,
        HIGH=0x77,
    };

    struct Status_t{
            union
            {
                struct
                {
                    bool RSVD : 4;
                    bool HEATER_STABLITY : 1;//0x10
                    bool GAS_MEASUREMENT_VALID : 1;//0x20
                    bool RSVD2 : 1;
                    bool NEW_DATA : 1;//0x80
                } bits;

                uint8_t byte;
            };
        };

    
    class M : public I2CSensor
    {
    private:
        struct bme68x_dev bme;
        struct bme68x_conf conf;
        struct bme68x_heatr_conf heatr_conf;
        struct bme68x_data data={};
        
        uint32_t time_ms = 0;
        uint8_t n_fields;
        uint16_t sample_count = 1;

        

    public:
        M(i2c_master_bus_handle_t bus_handle, ADDRESS addr) : I2CSensor(bus_handle, (uint8_t)addr)
        {
        }

        ErrorCode Initialize(int64_t &waitTillFirstTrigger) override
        {
            bme.read = M::bme68x_i2c_read;
            bme.write = M::bme68x_i2c_write;
            bme.intf = BME68X_I2C_INTF;
            bme.delay_us=M::user_delay_us;
            bme.intf_ptr = this;
            bme.amb_temp = 25;
            BME680x_ON_ERROR(bme68x_init(&bme));

            conf.filter = BME68X_FILTER_OFF;
            conf.odr = BME68X_ODR_NONE;
            conf.os_hum = BME68X_OS_16X;
            conf.os_pres = BME68X_OS_1X;
            conf.os_temp = BME68X_OS_2X;
            BME680x_ON_ERROR(bme68x_set_conf(&conf, &bme));
            
            heatr_conf.enable = BME68X_ENABLE;
            heatr_conf.heatr_temp = 300;
            heatr_conf.heatr_dur = 100;
            BME680x_ON_ERROR(bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &bme));
            return ErrorCode::OK;
        }

        ErrorCode Trigger(int64_t &waitTillReadout) override
        {
            BME680x_ON_ERROR(bme68x_set_op_mode(BME68X_FORCED_MODE, &bme));
            waitTillReadout = 10+(bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &bme)/1000) + heatr_conf.heatr_dur;
            ESP_LOGI(TAG, "Trigger BME68x, waitTillReadout=%lli", waitTillReadout);
            return ErrorCode::OK;
        }

        

        ErrorCode Readout(int64_t &waitTillNextTrigger) override
        {
            BME680x_ON_ERROR(bme68x_get_data(BME68X_FORCED_MODE, &data, &n_fields, &bme));
            Status_t s;
            s.byte=data.status;
            ESP_LOGI(TAG, "Readout BME68x! nFields=%d {\"temperature\":%f,\"pressure\":%f, \"humidity\":%f, \"gas_resistance\":%f, \"NEW_DATA\":%d, \"GAS_MEASUREMENT_VALID\":%d, \"HEATER_STABLITY\":%d, }", n_fields, data.temperature, data.pressure, data.humidity, data.gas_resistance, s.bits.NEW_DATA, s.bits.GAS_MEASUREMENT_VALID, s.bits.HEATER_STABLITY);
            waitTillNextTrigger = 2000;
            return ErrorCode::OK;
        }
        
        size_t FormatJSON(char* buffer, size_t maxLen){
            size_t used=0;
            used += snprintf(buffer, maxLen-used, "{\"temperature\":%f,\"pressure\":%f, \"humidity\":%f, \"gas_resistance\":%f, \"status\":%d}",
                data.temperature,
                data.pressure,
                data.humidity,
                data.gas_resistance,
                data.status
            );
            return used;
        }

        static BME68X_INTF_RET_TYPE bme68x_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
        {
            M* myself = static_cast<M*>(intf_ptr);
            ERRORCODE_CHECK(myself->ReadRegs8(reg_addr, reg_data, len));
            return 0;
        }


        static BME68X_INTF_RET_TYPE bme68x_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
        {
            M* myself = static_cast<M*>(intf_ptr);
            ERRORCODE_CHECK(myself->WriteRegs8(reg_addr, reg_data, len));
            return 0;
        }

        static void user_delay_us(uint32_t period, void *intf_ptr)
        {
            delayMs((period / 1000) + 1);
        }
        
    };
}
#undef TAG