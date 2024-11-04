#include <stdio.h>
#include "common_in_project.hh"
#include <sdkconfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>
#include <driver/gpio.h>
#include <esp_system.h>
#include <spi_flash_mmap.h>
#include <esp_wifi.h>
#include <driver/gpio.h>
#include <esp_event.h>
#include <sys/param.h>
#include <nvs_flash.h>
#include <esp_netif.h>

#include <__build_config.hh>

constexpr TickType_t xFrequency {pdMS_TO_TICKS(50)};

#ifndef CONFIG_ESP_HTTPS_SERVER_ENABLE
#error "Enable HTTPS_SERVER in menuconfig!"
#endif
#include <esp_https_server.h>
#include <esp_tls.h>
// #include <webmanager.hh> //include esp_https_server before!!!
#include <wifi_sta.hh>

#include "http_handlers.hh"
static const char *TAG = "main";
#include "HAL.hh"



#if(__BOARD_VERSION__>=100000 && __BOARD_VERSION__<110000)
#include "HAL_labathomeV10.hh"
static HAL * hal = new HAL_Impl(MODE_MOVEMENT_OR_FAN1SENSE::MOVEMENT_SENSOR);
#elif(__BOARD_VERSION__>=150000 && __BOARD_VERSION__<160000)
#include "HAL_labathomeV15.hh"
static HAL * hal = new HAL_Impl();
#else
#error Unknown __BOARD_VERSION__ ##__BOARD_VERSION
#endif

#include "functionblocks.hh"
#include "rgbled.hh"
#include "esp_log.h"
#include "spiffs.hh"

DeviceManager *devicemanager{nullptr};
httpd_handle_t http_server{nullptr};

#ifndef CONFIG_SMOPLA_HTTP_SCRATCHPAD_SIZE
#define CONFIG_SMOPLA_HTTP_SCRATCHPAD_SIZE 2048
#endif
#define NVS_PARTITION_NAME NVS_DEFAULT_PART_NAME

FLASH_FILE(esp32_pem_crt);
FLASH_FILE(esp32_pem_key);


uint8_t http_scatchpad[CONFIG_SMOPLA_HTTP_SCRATCHPAD_SIZE] ALL4;

extern "C" void app_main()
{
    // Configure Logging
    //esp_log_level_set(TAG, ESP_LOG_INFO);
    //esp_log_level_set("esp_https_server", ESP_LOG_WARN);

    // Configure NVS and SPIFFS
    esp_err_t ret = nvs_flash_init_partition(NVS_PARTITION_NAME);
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init_partition(NVS_PARTITION_NAME));
    }
    ESP_ERROR_CHECK(SpiffsManager::Init());

    //Configure Network
    #include "secrets.hh"
    WIFISTA::InitAndRun(WIFI_SSID, WIFI_PASS, "labathome_%02x%02x%02x");
    const char *hostname = WIFISTA::GetHostname();
    httpd_ssl_config_t httpd_conf = HTTPD_SSL_CONFIG_DEFAULT();
    httpd_conf.servercert = esp32_pem_crt_start;
    httpd_conf.servercert_len = esp32_pem_crt_end-esp32_pem_crt_start;
    httpd_conf.prvtkey_pem = esp32_pem_key_start;
    httpd_conf.prvtkey_len = esp32_pem_key_end-esp32_pem_key_start;
    httpd_conf.httpd.uri_match_fn = httpd_uri_match_wildcard;
    httpd_conf.httpd.max_uri_handlers = 15;
    httpd_conf.httpd.global_user_ctx = http_scatchpad;
    ESP_ERROR_CHECK(httpd_ssl_start(&http_server, &httpd_conf));
    ESP_LOGI(TAG, "HTTPS Server listening on https://%s:%d", hostname, httpd_conf.port_secure);

    // Start all managers
    devicemanager = new DeviceManager(hal);
    hal->InitAndRun();
    devicemanager->InitAndRun();
    ESP_LOGI(TAG, "RED %d YEL %d GRN %d", hal->GetButtonRedIsPressed(), hal->GetButtonEncoderIsPressed(), hal->GetButtonGreenIsPressed());

    // Allow Browser Access
    RegisterHandlers();
 

    // Start eternal supervisor loop
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (true)
    {
        xTaskDelayUntil(&xLastWakeTime, xFrequency);
        hal->DoMonitoring();
        
    }
}
