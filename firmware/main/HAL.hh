#pragma once

#include <stdio.h>
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "labathomeerror.hh"

class HAL
{
    public:
        virtual LabAtHomeErrorCode Init()=0;
        virtual LabAtHomeErrorCode SetLedRed(bool value)=0;
        virtual LabAtHomeErrorCode SetLedYellow(bool value)=0;
        virtual LabAtHomeErrorCode SetLedGreen(bool value)=0;
        virtual LabAtHomeErrorCode GetButtonRed(bool *value)=0;
        virtual LabAtHomeErrorCode GetButtonEncoder(bool *value)=0;
        virtual LabAtHomeErrorCode GetButtonGreen(bool *value)=0;
        virtual LabAtHomeErrorCode FetchInputs()=0;
        virtual LabAtHomeErrorCode FlushOutputs()=0;
        virtual int64_t GetMicroseconds()=0; 
};