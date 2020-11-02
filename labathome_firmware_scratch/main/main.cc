#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <esp_system.h>
#include "esp_spi_flash.h"
#include "driver/adc.h"


static const char *TAG = "main";

#include "esp_log.h"


extern "C"
{
    void app_main();
}


void app_main(void)
{
    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU cores, WiFi%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    
    printf("=======================================================\n");

    esp_log_level_set(TAG, ESP_LOG_INFO);
    uint32_t i=0;
    while (true)
    {
        float airTemp=0.0;
        
        float heaterTemp=0.0;
        printf("Start was %d seconds ago. Free heap: %d, Ambient Temp: %F, Heater Temp: %F\n", i, esp_get_free_heap_size(), airTemp, heaterTemp);
        i += 5;
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
