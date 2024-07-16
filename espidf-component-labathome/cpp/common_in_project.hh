#pragma once
#include <stdint.h>
#include <errorcodes.hh>
#include <esp_log.h>
#include <common.hh>
#include "common_projectconfig.hh"

#define LOGI ESP_LOGI
#define LOGD ESP_LOGD
#define LOGE ESP_LOGE


namespace labathome::strings
{
    constexpr char SUCCESSFUL_STRING[] = "Initialization of %s was successful";
    constexpr char NOT_SUCCESSFUL_STRING[] = "Initialization of %s was NOT successful";
}

namespace labathome
{
    enum class ePushState : uint8_t
    {
        RELEASED = 0,
        PRESSED = 1
    };
}

namespace labathome::magic
{    
    constexpr uint16_t ACTIVE = UINT16_MAX;
    constexpr uint16_t INACTIVE = 0;
}