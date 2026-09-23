#pragma once
#include <i2c_sensor.hh>
#include <common-esp32.hh>
#include <bme68x.h>
#include <bme68x_defs.h>
#include <bsec_interface.h>
#include <bsec_serialized_configurations_selectivity.h>
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

            bsec_init();
            uint8_t serialized_settings[BSEC_MAX_PROPERTY_BLOB_SIZE];
            uint32_t n_serialized_settings_max = BSEC_MAX_PROPERTY_BLOB_SIZE;
            uint8_t work_buffer[BSEC_MAX_PROPERTY_BLOB_SIZE];
            uint32_t n_work_buffer = BSEC_MAX_PROPERTY_BLOB_SIZE;

            // Here we will load a provided config string into serialized_settings 
    
            // Apply the configuration
            bsec_set_configuration(serialized_settings, n_serialized_settings_max, work_buffer, n_work_buffer);
            //bsec_set_state();
            bsec_sensor_configuration_t requested_virtual_sensors[3];
            uint8_t n_requested_virtual_sensors = 3;
        
            requested_virtual_sensors[0].sensor_id = BSEC_OUTPUT_IAQ;
            requested_virtual_sensors[0].sample_rate = BSEC_SAMPLE_RATE_ULP; 
            requested_virtual_sensors[1].sensor_id = BSEC_OUTPUT_RAW_TEMPERATURE;
            requested_virtual_sensors[1].sample_rate = BSEC_SAMPLE_RATE_ULP; 
            requested_virtual_sensors[2].sensor_id = BSEC_OUTPUT_RAW_PRESSURE;
            requested_virtual_sensors[2].sample_rate = BSEC_SAMPLE_RATE_DISABLED; 
            
            // Allocate a struct for the returned physical sensor settings
            bsec_sensor_configuration_t required_sensor_settings[BSEC_MAX_PHYSICAL_SENSOR];
            uint8_t  n_required_sensor_settings = BSEC_MAX_PHYSICAL_SENSOR;
        
            // Call bsec_update_subscription() to enable/disable the requested virtual sensors
            bsec_update_subscription(requested_virtual_sensors, n_requested_virtual_sensors, required_sensor_settings, &n_required_sensor_settings);
            int64_t next_ns{0};
            while(true){
                const int64_t now_ns = esp_timer_get_time()*1000;
                if(now_ns<next_ns){
                    vTaskDelay(10);
                    continue;
                }
                bsec_bme_settings_t sensor_settings;
                bsec_sensor_control(now_ns, &sensor_settings);
                //forced-mode measurement!
  

                 //A measurement should only be executed if bsec_bme_settings_t::trigger_measurement is 1.
                if(!sensor_settings.trigger_measurement){
                    continue;
                }
                //If so, the oversampling settings for temperature, humidity, and pressure should be set to the provided settings provided in 
                // temperature_oversampling, humidity_oversampling pressure_oversampling, respectively.
                conf.os_hum = sensor_settings.humidity_oversampling;
                conf.os_pres = sensor_settings.pressure_oversampling;
                conf.os_temp = sensor_settings.temperature_oversampling;
                BME680x_ON_ERROR(bme68x_set_conf(&conf, &bme));
                //In case of bsec_bme_settings_t::run_gas = 1, the gas sensor must be enabled with the provided 
                //bsec_bme_settings_t::heater_temperature and bsec_bme_settings_t::heating_duration settings.

                heatr_conf.enable = sensor_settings.run_gas;
                heatr_conf.heatr_temp = sensor_settings.heater_temperature;
                heatr_conf.heatr_dur = sensor_settings.heater_duration;
                BME680x_ON_ERROR(bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &bme));
                
                BME680x_ON_ERROR(bme68x_set_op_mode(BME68X_FORCED_MODE, &bme));
                auto waitTillReadout = 10+(bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &bme)/1000) + heatr_conf.heatr_dur;
                vTaskDelay(pdMS_TO_TICKS(waitTillReadout));
                BME680x_ON_ERROR(bme68x_get_data(BME68X_FORCED_MODE, &data, &n_fields, &bme));
                // Allocate input and output memory
                bsec_input_t input[3];
                uint8_t n_input = 3;
                bsec_output_t output[2];
                uint8_t  n_output=2;

                bsec_library_return_t status;

                // Populate the input structs, assuming the we have timestamp (ts), 
                // gas sensor resistance (R), temperature (T), and humidity (rH) available
                // as input variables
                input[0].sensor_id = BSEC_INPUT_GASRESISTOR;
                input[0].signal = data.gas_resistance;
                input[0].time_stamp= now_ns;   
                input[1].sensor_id = BSEC_INPUT_TEMPERATURE;   
                input[1].signal = data.temperature;   
                input[1].time_stamp= now_ns;   
                input[2].sensor_id = BSEC_INPUT_HUMIDITY;
                input[2].signal = data.humidity;
                input[2].time_stamp= now_ns;   

                    
                // Invoke main processing BSEC function
                status = bsec_do_steps( input, n_input, output, &n_output );

                // Iterate through the BSEC output data, if the call succeeded
                if(status == BSEC_OK)
                {
                    for(int i = 0; i < n_output; i++)
                    {   
                        switch(output[i].sensor_id)
                        {
                            case BSEC_OUTPUT_IAQ:
                                // Retrieve the IAQ results from output[i].signal
                                // and do something with the data
                                break;
                           default:
                            break;
                            
                        }
                    }
                }
    

            }



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