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
#include <protocol_examples_common.h>
#include <esp_http_server.h>

static const char *TAG = "main";
#include "HAL.hh"
#include "functionblocks.hh"
#include "WS2812.hh"
#include "esp_log.h"


#include "HAL_labathomeV5.hh"
HAL *hal = new HAL_labathomeV5(MODE_IO33::SERVO2, MODE_MULTI1_PIN::EXT, MODE_MULTI_2_3_PINS::EXT);
//#include "HAL_wroverkit.hh"
//HAL *hal = new HAL_wroverkit();

PLCManager *manager = new PLCManager(hal);


extern "C"
{
    void app_main();
    void experimentTask(void *);
    void plcTask(void *);
}


// Perform an action every 10 ticks.
void experimentTask(void *pvParameters)
{

    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 10;

    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();

    while (true)
    {
        // Wait for the next cycle.
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

    }
    
}

// Main PLC Task
void plcTask(void *pvParameters)
{
    ESP_LOGI(TAG, "plcTask started");
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 10;
    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();
    while (true)
    {
        // Wait for the next cycle.
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        ESP_LOGD(TAG, "CheckForNewExecutable");
        manager->CheckForNewExecutable();
        ESP_LOGD(TAG, "BeforeLoop");
        hal->BeforeLoop();
        ESP_LOGD(TAG, "Loop");
        manager->Loop();
        ESP_LOGD(TAG, "AfterLoop");
        hal->AfterLoop();
    }
}

/* This handler allows the custom error handling functionality to be
 * tested from client side. For that, when a PUT request 0 is sent to
 * URI /ctrl, the /hello and /echo URIs are unregistered and following
 * custom error handler http_404_error_handler() is registered.
 * Afterwards, when /hello or /echo is requested, this custom error
 * handler is invoked which, after sending an error message to client,
 * either closes the underlying socket (when requested URI is /echo)
 * or keeps it open (when requested URI is /hello). This allows the
 * client to infer if the custom error handler is functioning as expected
 * by observing the socket state.
 */
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    if (strcmp("/hello", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/hello URI is not available");
        /* Return ESP_OK to keep underlying socket open */
        return ESP_OK;
    } else if (strcmp("/echo", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/echo URI is not available");
        /* Return ESP_FAIL to close underlying socket */
        return ESP_FAIL;
    }
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
}



extern const char index_html_gz_start[] asm("_binary_index_html_gz_start");
extern const char index_html_gz_end[] asm("_binary_index_html_gz_end");
extern const size_t index_html_gz_size asm("index_html_gz_length");

static esp_err_t root_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    httpd_resp_send(req, index_html_gz_start, index_html_gz_size); // -1 = use strlen()

    return ESP_OK;
}

const size_t MAX_FDB_BUFFER = 8192;
static esp_err_t fbd_put_handler(httpd_req_t *req)
{    
    int ret=0;
    int remaining = req->content_len;
    if(remaining>=MAX_FDB_BUFFER)
        return ESP_FAIL;
    ESP_LOGI(TAG, "fbd_put_handler, expecting %d bytes of data", remaining);
    uint8_t buf[remaining];
    while (remaining > 0) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, (char*)buf,  MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }
        remaining -= ret;
    }
    // End response
    uint32_t *buf32 = (uint32_t*)buf;
    printf("Received Buffer:\n");
    for(size_t i= 0; i<(req->content_len/4); i++)
    {
        printf("0x%08x ",buf32[i]);
    }
    printf("\n");
    manager->ParseNewExecutableAndEnqueue((uint8_t*)buf, req->content_len);
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}
/*
Structure Client-->Server

*/

static esp_err_t experiment_handler(httpd_req_t *req)
{    
    int ret=0;
    int remaining = req->content_len;
    if(remaining!=24){
        ESP_LOGE(TAG, "Unexpected Data length %d in experiment_put_handler", remaining);
        return ESP_FAIL;
    }
    uint8_t buf[remaining];
    while (remaining > 0) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, (char*)buf,  MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }
        remaining -= ret;
    }
    // End response
    float *bufF32 = (float*)buf;
    uint32_t *bufU32 = (uint32_t*)buf;
    uint32_t modeU32 = bufU32[0];
    float setpointTempOrHeater = bufF32[1];
    float setpointFan = bufF32[2];
    float KP = bufF32[3];
    float KI = bufF32[4];
    float KD = bufF32[5];
    
    ESP_LOGI(TAG, "Set mode %d and setpointTorH %F and Setpoint Fan %F", modeU32, setpointTempOrHeater, setpointFan);
    ExperimentData returnData;
    switch (modeU32)
    {
    case 0: manager->TriggerHeaterExperimentFunctionblock(&returnData); break;
    case 1: manager->TriggerHeaterExperimentOpenLoop(setpointTempOrHeater, setpointFan, &returnData); break;
    case 2: manager->TriggerHeaterExperimentClosedLoop(setpointTempOrHeater, setpointFan, KP, KI, KD, &returnData); break;
    default:break;
    }
    
    httpd_resp_set_type(req, "application/octet-stream");
    float retbuf[4];
    retbuf[0]=returnData.SetpointTemperature;
    retbuf[1]=returnData.Heater;
    retbuf[2]=returnData.Fan;
    retbuf[3]=returnData.ActualTemperature;
    httpd_resp_send(req, (const char*)retbuf, 16);
    return ESP_OK;
}

static const httpd_uri_t root = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = root_get_handler,
    .user_ctx = 0
};

static const httpd_uri_t fbd = {
    .uri       = "/fbd",
    .method    = HTTP_PUT,
    .handler   = fbd_put_handler,
    .user_ctx = 0
};

static const httpd_uri_t experiment_put = {
    .uri       = "/experiment",
    .method    = HTTP_PUT,
    .handler   = experiment_handler,
    .user_ctx = 0
};

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &root);
        httpd_register_uri_handler(server, &fbd);
        httpd_register_uri_handler(server, &experiment_put);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}

static void disconnect_handler(void* arg, esp_event_base_t event_base, 
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base, 
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

void _lab_error_check_failed(LabAtHomeErrorCode rc, const char *file, int line, const char *function, const char *expression)
{
    printf("Error %d occured in File %s in line %d in expression %s", (int)rc, file, line, expression);
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
        LabAtHomeErrorCode __err_rc = (x);                                       \
        if (__err_rc != LabAtHomeErrorCode::OK) {                                       \
            _lab_error_check_failed(__err_rc, __FILE__, __LINE__,       \
                                    __ASSERT_FUNC, #x);                 \
        }                                                               \
    } while(0)
#endif

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

    LAB_ERROR_CHECK(hal->Init());


    static httpd_handle_t server;
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));



    server = start_webserver(); 

    //xTaskCreate(experimentTask, "experimentTask", 1024 * 2, NULL, 5, NULL);
    xTaskCreate(plcTask, "plcTask", 4096 * 4, NULL, 6, NULL);
    int i = 0;

    while (true)
    {
        printf("Start was %d seconds ago. Free heap: %d\n", i, esp_get_free_heap_size());
        i += 5;
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
