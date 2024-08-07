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

#include <mdns.h>
#ifndef CONFIG_ESP_HTTPS_SERVER_ENABLE
#error "Enable HTTPS_SERVER in menuconfig!"
#endif
#include <esp_https_server.h>
#include <esp_tls.h>
#include <webmanager.hh> //include esp_https_server before!!!

#include "http_handlers.hh"
static const char *TAG = "main";
#include "HAL.hh"

//#include "HAL_labathomeV5.hh"
//HAL *hal = new HAL_labathome(MODE_IO33::SERVO2, MODE_MULTI1_PIN::I2S, MODE_MULTI_2_3_PINS::I2S);

//#include "HAL_labathomeV10.hh"
//static HAL * hal = new HAL_Impl(MODE_MOVEMENT_OR_FAN1SENSE::MOVEMENT_SENSOR);
#include "HAL_labathomeV15.hh"
static HAL * hal = new HAL_Impl();

#include "functionblocks.hh"
#include "rgbled.hh"
#include "esp_log.h"
#include "spiffs.hh"
#include "winfactboris.hh"

DeviceManager *devicemanager{nullptr};
httpd_handle_t http_server{nullptr};


#ifndef CONFIG_SMOPLA_HTTP_SCRATCHPAD_SIZE
#define CONFIG_SMOPLA_HTTP_SCRATCHPAD_SIZE 2048
#endif

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



void app_main(void)
{
    //Configure Logging
    esp_log_level_set(TAG, ESP_LOG_INFO);
    esp_log_level_set("esp_https_server", ESP_LOG_WARN);

    //Configure NVS and SPIFFS
    esp_err_t ret = nvs_flash_init_partition(NVS_PARTITION_NAME);
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init_partition(NVS_PARTITION_NAME));
    }
    ESP_ERROR_CHECK(SpiffsManager::Init());

    //Prepare WebManager (configures Network as well)
    webmanager::M *wman = webmanager::M::GetSingleton();
    ESP_ERROR_CHECK(wman->Begin("ESP32AP_", "password", "labathome", gpio_get_level(GPIO_NUM_0) == 1 ? false : true, true));

    //Start HTTPS Server
    httpd_ssl_config_t httpd_conf = HTTPD_SSL_CONFIG_DEFAULT();
    httpd_conf.servercert = cert_start;
    httpd_conf.servercert_len = cert_end - cert_start;
    httpd_conf.prvtkey_pem = privkey_start;
    httpd_conf.prvtkey_len = privkey_end - privkey_start;
    httpd_conf.httpd.uri_match_fn = httpd_uri_match_wildcard;
    httpd_conf.max_uri_handlers = 15;
    httpd_conf.global_user_ctx = http_scatchpad;
    ESP_ERROR_CHECK(httpd_ssl_start(&http_server, &httpd_conf));
    ESP_LOGI(TAG, "HTTPS Server listening on https://%s:%d", wman->GetHostname(), httpd_conf.port_secure);

    //Start all managers
    devicemanager = new DeviceManager(hal);
    hal->InitAndRun();
    devicemanager->InitAndRun();
    //winfactboris::InitAndRun(devicemanager);
    ESP_LOGI(TAG, "RED %d YEL %d GRN %d", hal->GetButtonRedIsPressed(), hal->GetButtonEncoderIsPressed(), hal->GetButtonGreenIsPressed());
    
    //Allow Browser Access
    wman->RegisterHTTPDHandlers(http_server, "/webmanager/*");
    //TODO: Das alles in den Device-Manager packen (siehe Muster im WebManager)
    ESP_ERROR_CHECK(httpd_register_uri_handler(http_server, &getroot));
    ESP_ERROR_CHECK(httpd_register_uri_handler(http_server, &putfbd));
    ESP_ERROR_CHECK(httpd_register_uri_handler(http_server, &getfbd));
    ESP_ERROR_CHECK(httpd_register_uri_handler(http_server, &putheaterexperiment));
    ESP_ERROR_CHECK(httpd_register_uri_handler(http_server, &putairspeedexperiment));
    ESP_ERROR_CHECK(httpd_register_uri_handler(http_server, &putfftexperiment));
    ESP_ERROR_CHECK(httpd_register_uri_handler(http_server, &getfbdstorejson));
    ESP_ERROR_CHECK(httpd_register_uri_handler(http_server, &deletefbdstorejson));
    ESP_ERROR_CHECK(httpd_register_uri_handler(http_server, &postfbdstorejson));
    ESP_ERROR_CHECK(httpd_register_uri_handler(http_server, &postfbddefaultbin));
    ESP_ERROR_CHECK(httpd_register_uri_handler(http_server, &postfbddefaultjson));
    ESP_ERROR_CHECK(httpd_register_uri_handler(http_server, &putptnexperiment));

    //Start eternal supervisor loop
    wman->CallMeAfterInitializationToMarkCurrentPartitionAsValid();
    while (true)
    {
        hal->OutputOneLineStatus();
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
