#include "bme280.h"
#include "bme280.hh"
#include "bme280_defs.h"
#include <inttypes.h>
#include <driver/i2c.h>

int8_t user_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    BME280_I2C *bme280 = (BME280_I2C *)intf_ptr;
    esp_err_t espRc;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (bme280->addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (bme280->addr << 1) | I2C_MASTER_READ, true);
    if (len > 1)
    {
        i2c_master_read(cmd, reg_data, len - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, reg_data + len - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    espRc = i2c_master_cmd_begin(bme280->i2c_port, cmd, 10 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return espRc;
}

int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{

    BME280_I2C *bme280 = (BME280_I2C *)intf_ptr;
    esp_err_t espRc;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (bme280->addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_write(cmd, (uint8_t *)reg_data, len, true);
    i2c_master_stop(cmd);

    espRc = i2c_master_cmd_begin(bme280->i2c_port, cmd, 10 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return espRc;
}

void user_delay_us(uint32_t period, void *intf_ptr)
{
    period = (period / 1000) + 1;
    vTaskDelay(period / portTICK_PERIOD_MS);
}

BME280::BME280(i2c_port_t i2c_num, BME280_ADRESS adress)
{
    this->intf_ptr.i2c_port = i2c_num;
    this->intf_ptr.addr = (uint8_t)adress;
}

BME280::~BME280() {}

esp_err_t BME280::Init(uint32_t *calculatedDelay)
{
    int8_t com_rslt = BME280_OK;
    dev.intf_ptr = &intf_ptr;
    dev.intf = BME280_I2C_INTF;
    dev.read = user_i2c_read;
    dev.write = user_i2c_write;
    dev.delay_us = user_delay_us;
    com_rslt = bme280_init(&dev);
    if (com_rslt != 0)
        return ESP_FAIL;

    /* Recommended mode of operation: Indoor navigation */
    dev.settings.osr_h = BME280_OVERSAMPLING_1X;
    dev.settings.osr_p = BME280_OVERSAMPLING_16X;
    dev.settings.osr_t = BME280_OVERSAMPLING_2X;
    dev.settings.filter = BME280_FILTER_COEFF_16;

    uint8_t settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;

    com_rslt = bme280_set_sensor_settings(settings_sel, &dev);
    if (com_rslt != 0)
        return ESP_FAIL;
    /*Calculate the minimum delay required between consecutive measurement based upon the sensor enabled
        *  and the oversampling configuration. */
    *calculatedDelay= bme280_cal_meas_delay(&(dev.settings));
    if (com_rslt != 0)
        return ESP_FAIL;
    return com_rslt == BME280_OK ? ESP_OK : ESP_FAIL;
}
esp_err_t BME280::GetDataAndTriggerNextMeasurement(float *tempDegCel, float *pressurePa, float *relHumidityPercent)
{
    struct bme280_data comp_data;
    int8_t com_rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, &dev);
    *tempDegCel = comp_data.temperature;
    *pressurePa = comp_data.pressure;
    *relHumidityPercent = comp_data.humidity;
    if (com_rslt != 0)
        return ESP_FAIL;
    com_rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, &dev);
    return com_rslt == BME280_OK ? ESP_OK : ESP_FAIL;
}

esp_err_t BME280::TriggerNextMeasurement()
{
    return bme280_set_sensor_mode(BME280_FORCED_MODE, &dev) == BME280_OK ? ESP_OK : ESP_FAIL;
}
