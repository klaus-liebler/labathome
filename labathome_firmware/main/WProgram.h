#pragma once
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "esp_timer.h"

unsigned long IRAM_ATTR millis()
{
    return (unsigned long) (esp_timer_get_time() / 1000);
}