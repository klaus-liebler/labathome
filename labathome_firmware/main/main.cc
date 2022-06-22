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
#include "functionblocks.hh"
#include "WS2812.hh"
#include "esp_log.h"
#include "spiffs.hh"
#include "winfactboris.hh"


uint8_t http_scatchpad[labathome::config::HTTP_SCRATCHPAD_SIZE] ALL4;

//#include "HAL_labathomeV5.hh"
//HAL *hal = new HAL_labathome(MODE_IO33::SERVO2, MODE_MULTI1_PIN::I2S, MODE_MULTI_2_3_PINS::I2S);
//#include "HAL_wroverkit.hh"
//static HAL *hal = new HAL_wroverkit();
#include "HAL_labathomeV10.hh"
static HAL * hal = new HAL_labathome(MODE_SPI_IO1_OR_SERVO2::SERVO2, MODE_HEATER_OR_LED_POWER::HEATER, MODE_K3A1_OR_ROTB::ROTB, MODE_MOVEMENT_OR_FAN1SENSE::MOVEMENT_SENSOR, MODE_FAN1_DRIVE_OR_SERVO1::FAN1_DRIVE, MODE_RS485_OR_EXT::RS485);
static PLCManager *plcmanager = new PLCManager(hal);

extern "C" void app_main();

constexpr httpd_uri_t getroot = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = handle_get_root,
    .user_ctx = &plcmanager,
};

constexpr httpd_uri_t putfbd = {
    .uri       = "/fbd",
    .method    = HTTP_PUT,
    .handler   = handle_put_fbd,
    .user_ctx = &plcmanager,
};

constexpr httpd_uri_t getfbd = {
    .uri       = "/fbd",
    .method    = HTTP_GET,
    .handler   = handle_get_fbd,
    .user_ctx = &plcmanager,
};

constexpr httpd_uri_t getfbdstorejson = {
    .uri       = "/fbdstorejson/*",
    .method    = HTTP_GET,
    .handler   = handle_get_fbdstorejson,
    .user_ctx = &plcmanager,
};

constexpr httpd_uri_t postfbdstorejson = {
    .uri       = "/fbdstorejson/*",
    .method    = HTTP_POST,
    .handler   = handle_post_fbdstorejson,
    .user_ctx = &plcmanager,
};

constexpr httpd_uri_t deletefbdstorejson = {
    .uri       = "/fbdstorejson/*",
    .method    = HTTP_DELETE,
    .handler   = handle_delete_fbdstorejson,
    .user_ctx = &plcmanager,
};

constexpr httpd_uri_t postfbddefaultbin = {
    .uri       = "/fbddefaultbin",
    .method    = HTTP_POST,
    .handler   = handle_post_fbddefaultbin,
    .user_ctx = &plcmanager,
};

constexpr httpd_uri_t postfbddefaultjson = {
    .uri       = "/fbddefaultjson",
    .method    = HTTP_POST,
    .handler   = handle_post_fbddefaultjson,
    .user_ctx = &plcmanager,
};

constexpr httpd_uri_t putheaterexperiment = {
    .uri       = "/heaterexperiment",
    .method    = HTTP_PUT,
    .handler   = handle_put_heaterexperiment,
    .user_ctx = &plcmanager,
};

constexpr httpd_uri_t putairspeedexperiment = {
    .uri       = "/airspeedexperiment",
    .method    = HTTP_PUT,
    .handler   = handle_put_airspeedexperiment,
    .user_ctx = &plcmanager,
};

constexpr httpd_uri_t putfftexperiment = {
    .uri       = "/fftexperiment",
    .method    = HTTP_PUT,
    .handler   = handle_put_fftexperiment,
    .user_ctx = &plcmanager,
};


constexpr httpd_uri_t getadcexperiment = {
    .uri       = "/adcexperiment",
    .method    = HTTP_GET,
    .handler   = handle_get_adcexperiment,
    .user_ctx = &plcmanager,
};

static httpd_handle_t InitAndRunWebserver(void)
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
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &getadcexperiment));

    const char *hostnameptr;
    ESP_ERROR_CHECK(esp_netif_get_hostname(WIFIMGR::wifi_netif_ap, &hostnameptr));
    ESP_LOGI(TAG, "HTTPd successfully started for website http://%s", hostnameptr);
    return server;
}


void _lab_error_check_failed(ErrorCode rc, const char *file, int line, const char *function, const char *expression)
{
    ESP_LOGE(TAG, "Error %d occured in File %s in line %d in expression %s", (int)rc, file, line, expression);
}

#ifdef NDEBUG
#define LAB_ERROR_CHECK(x) do {                                         \
        esp_err_t __err_rc = (x);                                       \
        (void) sizeof(__err_rc);                                        \
    } while(0)
#elif defined(CONFIG_COMPILER_OPTIMIZATION_ASSERTIONS_SILENT)
#define LAB_ERROR_CHECK(x) do {                                         \
        esp_err_t __err_rc = (x);                                       \
        if (__err_rc != ESP_OK) {                                       \
            abort();                                                    \
        }                                                               \
    } while(0)
#else
#define LAB_ERROR_CHECK(x) do {                                         \
        ErrorCode __err_rc = (x);                                       \
        if (__err_rc != ErrorCode::OK) {                                       \
            _lab_error_check_failed(__err_rc, __FILE__, __LINE__,       \
                                    __ASSERT_FUNC, #x);                 \
        }                                                               \
    } while(0)
#endif


void app_main(void)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);
    ESP_ERROR_CHECK(SpiffsManager::Init());
    LAB_ERROR_CHECK(hal->InitAndRun());
    ESP_LOGI(TAG, "RED %d YEL %d GRN %d", hal->GetButtonRedIsPressed(), hal->GetButtonEncoderIsPressed(), hal->GetButtonGreenIsPressed());
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(WIFIMGR::InitAndRun(hal->GetButtonGreenIsPressed() && hal->GetButtonRedIsPressed(), http_scatchpad, sizeof(http_scatchpad)));
    otamanager::M otamanager;
    otamanager.InitAndRun();
    winfactboris::InitAndRun(hal);


    httpd_handle_t httpd_handle = InitAndRunWebserver();
    WIFIMGR::RegisterHTTPDHandlers(httpd_handle);

    int secs = 0;
    
    plcmanager->InitAndRun();

    while (true)
    {
        float heaterTemp{0.f};
        hal->GetHeaterTemperature(&heaterTemp);
        float airTemp{0.f};
        hal->GetAirTemperature(&airTemp);
        float airPres{0.f};
        hal->GetAirPressure(&airPres);
        float airHumid{0.f};
        hal->GetAirRelHumidity(&airHumid);
        int encoderValue{0};
        hal->GetEncoderValue(&encoderValue);
        uint16_t co2{0};
        hal->GetCO2PPM(&co2);     
        ESP_LOGI(TAG, "Run %4d, Heap %6d, RED %d YEL %d ENC %d GRN %d MOV %d HEAT %4.1f AIRT %4.1f PRS %5.0f HUM %3.0f  CO2 %d", secs, esp_get_free_heap_size(),
            hal->GetButtonRedIsPressed(), hal->GetButtonEncoderIsPressed(), encoderValue, hal->GetButtonGreenIsPressed(),  hal->IsMovementDetected(), heaterTemp, airTemp, airPres, airHumid, co2);
        secs += 5;
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
