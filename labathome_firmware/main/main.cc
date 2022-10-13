#include <stdio.h>
#include "common_in_project.hh"
#include <sdkconfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>
#include <driver/gpio.h>
#include <esp_system.h>
#include <esp_spi_flash.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>
#include <esp_wifi.h>
#include <driver/gpio.h>
#include <esp_event.h>
#include <sys/param.h>
#include <nvs_flash.h>
#include <esp_netif.h>
#include <wifimanager.hh>
#include <otamanager.hh>
#include <esp_http_server.h>
#include "http_handlers.hh"
static const char *TAG = "main";
#include "HAL.hh"

//#include "HAL_labathomeV5.hh"
//HAL *hal = new HAL_labathome(MODE_IO33::SERVO2, MODE_MULTI1_PIN::I2S, MODE_MULTI_2_3_PINS::I2S);

#include "HAL_labathomeV10.hh"
static HAL * hal = new HAL_Impl(MODE_MOVEMENT_OR_FAN1SENSE::MOVEMENT_SENSOR);


#include "functionblocks.hh"
#include "rgbled.hh"
#include "esp_log.h"
#include "spiffs.hh"
#include "winfactboris.hh"

static DeviceManager *devicemanager = new DeviceManager(hal);

uint8_t http_scatchpad[CONFIG_SMOPLA_HTTP_SCRATCHPAD_SIZE] ALL4;



extern "C" void app_main();

constexpr httpd_uri_t getroot = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = handle_get_root,
    .user_ctx = &devicemanager,
};

constexpr httpd_uri_t putfbd = {
    .uri       = "/fbd",
    .method    = HTTP_PUT,
    .handler   = handle_put_fbd,
    .user_ctx = &devicemanager,
};

constexpr httpd_uri_t getfbd = {
    .uri       = "/fbd",
    .method    = HTTP_GET,
    .handler   = handle_get_fbd,
    .user_ctx = &devicemanager,
};

constexpr httpd_uri_t getfbdstorejson = {
    .uri       = "/fbdstorejson/*",
    .method    = HTTP_GET,
    .handler   = handle_get_fbdstorejson,
    .user_ctx = &devicemanager,
};

constexpr httpd_uri_t postfbdstorejson = {
    .uri       = "/fbdstorejson/*",
    .method    = HTTP_POST,
    .handler   = handle_post_fbdstorejson,
    .user_ctx = &devicemanager,
};

constexpr httpd_uri_t deletefbdstorejson = {
    .uri       = "/fbdstorejson/*",
    .method    = HTTP_DELETE,
    .handler   = handle_delete_fbdstorejson,
    .user_ctx = &devicemanager,
};

constexpr httpd_uri_t postfbddefaultbin = {
    .uri       = "/fbddefaultbin",
    .method    = HTTP_POST,
    .handler   = handle_post_fbddefaultbin,
    .user_ctx = &devicemanager,
};

constexpr httpd_uri_t postfbddefaultjson = {
    .uri       = "/fbddefaultjson",
    .method    = HTTP_POST,
    .handler   = handle_post_fbddefaultjson,
    .user_ctx = &devicemanager,
};

constexpr httpd_uri_t putheaterexperiment = {
    .uri       = "/heaterexperiment",
    .method    = HTTP_PUT,
    .handler   = handle_put_heaterexperiment,
    .user_ctx = &devicemanager,
};

constexpr httpd_uri_t putairspeedexperiment = {
    .uri       = "/airspeedexperiment",
    .method    = HTTP_PUT,
    .handler   = handle_put_airspeedexperiment,
    .user_ctx = &devicemanager,
};

constexpr httpd_uri_t putfftexperiment = {
    .uri       = "/fftexperiment",
    .method    = HTTP_PUT,
    .handler   = handle_put_fftexperiment,
    .user_ctx = &devicemanager,
};

constexpr httpd_uri_t putptnexperiment = {
    .uri       = "/ptnexperiment",
    .method    = HTTP_PUT,
    .handler   = handle_put_ptnexperiment,
    .user_ctx = &devicemanager,
};

static httpd_handle_t SetupAndRunWebserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;
    config.max_uri_handlers = 15;
    config.global_user_ctx = http_scatchpad;

    if (httpd_start(&server, &config) != ESP_OK)
    {
        ESP_LOGE(TAG, "Error starting HTTPd!");
        esp_restart();
    }
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &getroot));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &putfbd));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &getfbd));
    
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &putheaterexperiment));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &putairspeedexperiment));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &putfftexperiment));

    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &getfbdstorejson));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &deletefbdstorejson));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &postfbdstorejson));

    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &postfbddefaultbin));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &postfbddefaultjson));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &putptnexperiment));

    const char *hostnameptr;
    ESP_ERROR_CHECK(esp_netif_get_hostname(WIFIMGR::wifi_netif_ap, &hostnameptr));
    ESP_LOGI(TAG, "HTTPd successfully started for website http://%s", hostnameptr);
    return server;
}


void app_main(void)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);
    ESP_ERROR_CHECK(SpiffsManager::Init());
    hal->InitAndRun();
    ESP_LOGI(TAG, "RED %d YEL %d GRN %d", hal->GetButtonRedIsPressed(), hal->GetButtonEncoderIsPressed(), hal->GetButtonGreenIsPressed());
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(WIFIMGR::InitAndRun(hal->GetButtonGreenIsPressed() && hal->GetButtonRedIsPressed(), http_scatchpad, sizeof(http_scatchpad), WIFIMGR::NETWORK_MODE::WIFI_ONLY, GPIO_NUM_1, GPIO_NUM_1, GPIO_NUM_1));
    otamanager::M otamanager;
    otamanager.InitAndRun(CONFIG_SMOPLA_OTA_URL);
    winfactboris::InitAndRun(devicemanager);


    httpd_handle_t httpd_handle = SetupAndRunWebserver();
    WIFIMGR::RegisterHTTPDHandlers(httpd_handle);

    devicemanager->InitAndRun();
    while (true)
    {
        hal->OutputOneLineStatus();
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
