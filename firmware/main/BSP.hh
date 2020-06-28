#pragma once

#include <stdio.h>
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "esp_system.h"

class BSP
{
    public:
        virtual esp_err_t init()=0;
        virtual esp_err_t getBinaryInput(size_t index, bool *value)=0;
        virtual esp_err_t setBinaryOutput(size_t index, bool value)=0;
        virtual esp_err_t fetchInputs()=0;
        virtual esp_err_t flushOutputs()=0;
};