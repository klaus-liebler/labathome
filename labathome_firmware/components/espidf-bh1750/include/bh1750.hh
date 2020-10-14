#pragma once

#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "sdkconfig.h"


enum class BH1750_ADRESS:uint8_t
{
    LOW=0x23,
    HIGH=0x5C,
};

enum class BH1750_OPERATIONMODE:uint8_t
{
    
    CONTINU_H_RESOLUTION=0x10,
    CONTINU_H_RESOLUTION2=0x11,
    CONTINU_L_RESOLUTION=0x13,
    ONETIME_H_RESOLUTION=0x20,
    ONETIME_H_RESOLUTION2=0x21,
    ONETIME_L_RESOLUTION=0x23,

};


class BH1750{
public:
    static esp_err_t Command(i2c_port_t i2c_num, BH1750_ADRESS adress, BH1750_OPERATIONMODE operation);
    static esp_err_t Read(i2c_port_t i2c_num, BH1750_ADRESS adress, float *lux);
};