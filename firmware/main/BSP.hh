#pragma once

#include <stdio.h>
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "labathomeerror.hh"

class BSP
{
    public:
        virtual LabAtHomeErrorCode init()=0;
        virtual LabAtHomeErrorCode getBinaryInput(size_t index, bool *value)=0;
        virtual LabAtHomeErrorCode setBinaryOutput(size_t index, bool value)=0;
        virtual LabAtHomeErrorCode fetchInputs()=0;
        virtual LabAtHomeErrorCode flushOutputs()=0;
};