#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <esp_system.h>
#include "esp_spi_flash.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include <esp_wifi.h>
#include <esp_event.h>
#include <sys/param.h>
#include <nvs_flash.h>
#include <esp_netif.h>
#include "connect.hh"
#include <esp_http_server.h>
#include "http_handlers.hh"
static const char *TAG = "main";
#include "HAL.hh"
#include "functionblocks.hh"
#include "WS2812.hh"
#include "esp_log.h"
#include "spiffs.hh"

//#include "HAL_labathomeV5.hh"
//HAL *hal = new HAL_labathome(MODE_IO33::SERVO2, MODE_MULTI1_PIN::I2S, MODE_MULTI_2_3_PINS::I2S);
#include "HAL_labathomeV10.hh"
HAL *hal = new HAL_labathome(MODE_SPI_IO1_OR_SERVO2::SERVO2, MODE_HEATER_OR_LED_POWER::HEATER, MODE_K3A1_OR_ROTB::ROTB, MODE_MOVEMENT_OR_FAN1SENSE::MOVEMENT_SENSOR, MODE_FAN1_DRIVE_OR_SERVO1::FAN1_DRIVE, MODE_RS485_OR_EXT::RS485);
//#include "HAL_wroverkit.hh"
//HAL *hal = new HAL_wroverkit();

PLCManager *plcmanager = new PLCManager(hal);

extern "C"
{
    void app_main();
    void experimentTask(void *);
    void plcTask(void *);
}

void plcTask(void *pvParameters)
{
    ESP_LOGI(TAG, "plcTask started");
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 10;
    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();
    plcmanager->Init();
    /*
    for(int i=0;i<3;i++)
    {
        hal->ColorizeLed(LED::LED_RED, CRGB::DarkRed);
        hal->ColorizeLed(LED::LED_GREEN, CRGB::DarkGreen);
        hal->ColorizeLed(LED::LED_YELLOW, CRGB::Yellow);
        hal->ColorizeLed(LED::LED_3, CRGB::DarkBlue);
        hal->AfterLoop();
        vTaskDelay(pdMS_TO_TICKS(150));
        hal->ColorizeLed(LED::LED_RED, CRGB::DarkBlue);
        hal->ColorizeLed(LED::LED_GREEN, CRGB::Yellow);
        hal->ColorizeLed(LED::LED_YELLOW, CRGB::DarkGreen);
        hal->ColorizeLed(LED::LED_3, CRGB::DarkRed);
        hal->AfterLoop();
        vTaskDelay(pdMS_TO_TICKS(150));
    }
    */
    // hal->SetFan1State(30);
    // hal->SetFan2State(30);
    // while (true)
    // {
    //     vTaskDelay(pdMS_TO_TICKS(2000)); 
    // }
    
    // for(int i=0;i<5;i++)
    // {
    //     hal->SetFan1State(30);
    //     hal->SetFan2State(30);
    //     ESP_LOGI(TAG, "Fans 30");
    //     vTaskDelay(pdMS_TO_TICKS(2000));
    //     hal->SetFan1State(60);
    //     hal->SetFan2State(60);
    //     ESP_LOGI(TAG, "Fans 60");
    //     vTaskDelay(pdMS_TO_TICKS(2000));
    //     hal->SetFan1State(100);
    //     hal->SetFan2State(100);
    //     ESP_LOGI(TAG, "Fans 100");
    //     vTaskDelay(pdMS_TO_TICKS(2000));
    //     hal->SetFan1State(60);
    //     hal->SetFan2State(60);
    //     ESP_LOGI(TAG, "Fans 60");
    //     vTaskDelay(pdMS_TO_TICKS(2000));
    // }

    // hal->UnColorizeAllLed();
    // #define PI 3.14159265
    // for(int i=0;i<3;i++)
    // {
    //     for(int deg=0;deg<360;deg+=10){
    //         uint8_t result = 50*(sin(deg*PI/180)+1);
    //         hal->SetLedPowerWhiteState(result);
    //         ESP_LOGI(TAG, "LED POWER WHITE %d", result);
    //         vTaskDelay(pdMS_TO_TICKS(30));
    //     }
    // }
    
    
    hal->PlaySong(0);
    ESP_LOGI(TAG, "plcmanager main loop starts");
    while (true)
    {
        // Wait for the next cycle.
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        ESP_LOGD(TAG, "CheckForNewExecutable");
        plcmanager->CheckForNewExecutable();
        ESP_LOGD(TAG, "BeforeLoop");
        hal->BeforeLoop();
        ESP_LOGD(TAG, "Loop");
        plcmanager->Loop();
        ESP_LOGD(TAG, "AfterLoop");
        hal->AfterLoop();
    }
}


static const httpd_uri_t getroot = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = handle_get_root,
    .user_ctx = plcmanager,
};

static const httpd_uri_t putfbd = {
    .uri       = "/fbd",
    .method    = HTTP_PUT,
    .handler   = handle_put_fbd,
    .user_ctx = plcmanager,
};

static const httpd_uri_t getfbd = {
    .uri       = "/fbd",
    .method    = HTTP_GET,
    .handler   = handle_get_fbd,
    .user_ctx = plcmanager,
};

static const httpd_uri_t getfbdstorejson = {
    .uri       = "/fbdstorejson/*",
    .method    = HTTP_GET,
    .handler   = handle_get_fbdstorejson,
    .user_ctx = plcmanager,
};

static const httpd_uri_t postfbdstorejson = {
    .uri       = "/fbdstorejson/*",
    .method    = HTTP_POST,
    .handler   = handle_post_fbdstorejson,
    .user_ctx = plcmanager,
};

static const httpd_uri_t deletefbdstorejson = {
    .uri       = "/fbdstorejson/*",
    .method    = HTTP_DELETE,
    .handler   = handle_delete_fbdstorejson,
    .user_ctx = plcmanager,
};

static const httpd_uri_t postfbddefaultbin = {
    .uri       = "/fbddefaultbin",
    .method    = HTTP_POST,
    .handler   = handle_post_fbddefaultbin,
    .user_ctx = plcmanager,
};

static const httpd_uri_t postfbddefaultjson = {
    .uri       = "/fbddefaultjson",
    .method    = HTTP_POST,
    .handler   = handle_post_fbddefaultjson,
    .user_ctx = plcmanager,
};

static const httpd_uri_t putheaterexperiment = {
    .uri       = "/heaterexperiment",
    .method    = HTTP_PUT,
    .handler   = handle_put_heaterexperiment,
    .user_ctx = plcmanager,
};

static const httpd_uri_t putairspeedexperiment = {
    .uri       = "/airspeedexperiment",
    .method    = HTTP_PUT,
    .handler   = handle_put_airspeedexperiment,
    .user_ctx = plcmanager,
};

static const httpd_uri_t putfftexperiment = {
    .uri       = "/fftexperiment",
    .method    = HTTP_PUT,
    .handler   = handle_put_fftexperiment,
    .user_ctx = plcmanager,
};


static const httpd_uri_t getadcexperiment = {
    .uri       = "/adcexperiment",
    .method    = HTTP_GET,
    .handler   = handle_get_adcexperiment,
    .user_ctx = plcmanager,
};

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;
    config.max_uri_handlers = 12;

    if (httpd_start(&server, &config) != ESP_OK)
    {
        ESP_LOGE(TAG, "Error starting HTTPd!");
        esp_restart();
    }

    const char *hostnameptr;
    if(tcpip_adapter_get_hostname(TCPIP_ADAPTER_IF_STA, &hostnameptr)!=ESP_OK || hostnameptr==NULL){
        ESP_ERROR_CHECK(tcpip_adapter_get_hostname(TCPIP_ADAPTER_IF_AP, &hostnameptr));
    }

    ESP_LOGI(TAG, "HTTPd successfully started for website http://%s:%d", hostnameptr, config.server_port);
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
    LAB_ERROR_CHECK(hal->Init());

    ESP_ERROR_CHECK(nvs_flash_init());
    //connectSTA2AP(false);
    //xSemaphoreTake( connectSemaphore, portMAX_DELAY);
    startAP();
    start_webserver(); 

    //xTaskCreate(experimentTask, "experimentTask", 1024 * 2, NULL, 5, NULL);
    xTaskCreate(plcTask, "plcTask", 4096 * 4, NULL, 6, NULL);
    int secs = 0;
    bool hasAlreadyPlayedTheWarnSound = false;
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
        int16_t encoderValue{0};
        hal->GetEncoderValue(&encoderValue);
        uint16_t co2{0};
        hal->GetCO2PPM(&co2);
        if(co2<800){
            hal->ColorizeLed(LED::LED_YELLOW, CRGB::Green);            
            hal->ColorizeLed(LED::LED_GREEN, CRGB::Green);
            hal->ColorizeLed(LED::LED_3, CRGB::Green); 
            hasAlreadyPlayedTheWarnSound=false;
        }else if(co2<1000){
            hal->ColorizeLed(LED::LED_YELLOW, CRGB::Yellow);
            hal->ColorizeLed(LED::LED_GREEN, CRGB::Yellow);
            hal->ColorizeLed(LED::LED_3, CRGB::Yellow);
            hasAlreadyPlayedTheWarnSound=false;
        }else{
            hal->ColorizeLed(LED::LED_YELLOW, CRGB::Red);
            hal->ColorizeLed(LED::LED_GREEN, CRGB::Red);
            hal->ColorizeLed(LED::LED_3, CRGB::Red);
            if(!hasAlreadyPlayedTheWarnSound){
                hal->PlaySong(2);
            }   
            hasAlreadyPlayedTheWarnSound=true;       
        }
        
        ESP_LOGI(TAG, "Run %4d, Heap %6d, RED %d YEL %d ENC %d GRN %d MOV %d HEAT %4.1f AIRT %4.1f PRS %5.0f HUM %3.0f  CO2 %d", secs, esp_get_free_heap_size(),
            hal->GetButtonRedIsPressed(), hal->GetButtonEncoderIsPressed(), encoderValue, hal->GetButtonGreenIsPressed(),  hal->IsMovementDetected(), heaterTemp, airTemp, airPres, airHumid, co2);
        secs += 5;
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
