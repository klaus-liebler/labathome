#pragma once
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <esp_spi_flash.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include <string.h>

#include <lwip/err.h>
#include <lwip/sys.h>
#include "esp_http_client.h"
#include "esp_ota_ops.h"
#include "esp_https_ota.h"

#define TAG "OTA"
namespace otamanager
{
    constexpr int32_t AUTO_CHECK_INTERVAL_MINUTES{1440};
    constexpr int64_t AUTO_CHECK_INTERVAL_MICROSECONDS{AUTO_CHECK_INTERVAL_MINUTES*60*1000*1000LL};

    extern const uint8_t server_cert_pem_start[] asm("_binary_sciebo_cer_start");
    extern const uint8_t server_cert_pem_end[] asm("_binary_sciebo_cer_end");

    const char *OTA_URL{"https://w-hs.sciebo.de/s/M43yoLOweqCjl5v/download"};

    const char *TIMER = "timer";
    const char *MANUAL_REQUEST = "manual request";

    class M
    {
    private:
        bool manualUpdateRequested{true}; // on boot or first start -->do a manual Update
        int64_t lastUpdateCheckUs{INT64_MAX};
        char *http_response_buffer;
        int http_response_buffer_len;

        static void updaterTask(void *pvParameters)
        {
            M *myself = static_cast<M *>(pvParameters);
                 
            ESP_LOGI(TAG, "OTA task started!");
            const esp_partition_t *configured_partition = esp_ota_get_boot_partition();
            const esp_partition_t *running_partition = esp_ota_get_running_partition();
            const esp_partition_t *last_invalid_partition = esp_ota_get_last_invalid_partition();
            const esp_partition_t *update_partition = esp_ota_get_next_update_partition(running_partition);
            esp_app_desc_t app_desc_ota;
            esp_app_desc_t app_desc_run;
            esp_app_desc_t app_desc_invalid;

            if (configured_partition != running_partition)
            {
                ESP_LOGW(TAG, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x. (This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)", configured_partition->address, running_partition->address);
            }

            esp_ota_get_partition_description(running_partition, &app_desc_run);
            ESP_LOGI(TAG, "Running partition type %d subtype %d (offset 0x%08x) having firmware version: %s", running_partition->type, running_partition->subtype, running_partition->address, app_desc_run.version);
            
            
            if (esp_ota_get_partition_description(last_invalid_partition, &app_desc_invalid) == ESP_OK)
            {
                ESP_LOGI(TAG, "Last invalid firmware version: %s", app_desc_invalid.version);
            }
            
            esp_http_client_config_t config = {};
            config.url = OTA_URL;
            config.cert_pem = (char *)server_cert_pem_start;
            config.timeout_ms = 5000;
            config.keep_alive_enable = true;
            config.user_data = (void *)myself;
            config.skip_cert_common_name_check = true;


            esp_https_ota_config_t ota_config = {};
            ota_config.http_config = &config;
            ota_config.http_client_init_cb = nullptr; // Register a callback to be invoked after esp_http_client is initialized


            esp_https_ota_handle_t https_ota_handle{nullptr};
            
            
            while (true) //the task loop
            {
                vTaskDelay(pdMS_TO_TICKS(5000));
                int64_t now = esp_timer_get_time();
                if (!myself->manualUpdateRequested && now - myself->lastUpdateCheckUs < AUTO_CHECK_INTERVAL_MICROSECONDS)
                    continue;
                
                ESP_LOGI(TAG, "Update Process starts due to %s", myself->manualUpdateRequested ? MANUAL_REQUEST : TIMER);
                esp_https_ota_abort(https_ota_handle); //Nur nochmal zur sicherheit. nullptr-Checks sind integriert
                if (esp_https_ota_begin(&ota_config, &https_ota_handle) != ESP_OK) {
                    ESP_LOGE(TAG, "ESP HTTPS OTA Begin failed");
                    continue;
                }

                if (esp_https_ota_get_img_desc(https_ota_handle, &app_desc_ota) != ESP_OK) {
                    ESP_LOGE(TAG, "esp_https_ota_read_img_desc failed");
                    continue;
                }

                // check current ota version with last invalid partition
                if (last_invalid_partition != NULL && memcmp(app_desc_invalid.version, app_desc_ota.version, sizeof(app_desc_ota.version)) == 0)
                {
                    ESP_LOGI(TAG, "New version is the same as invalid version. We will not continue the update and check again in %d min.", AUTO_CHECK_INTERVAL_MINUTES);
                    esp_https_ota_abort(https_ota_handle);
                    myself->manualUpdateRequested=false;
                    myself->lastUpdateCheckUs=esp_timer_get_time();
                    continue;
                }

                if (memcmp(app_desc_ota.version, app_desc_run.version, sizeof(app_desc_ota.version)) == 0) {
                    ESP_LOGI(TAG, "New version ist the same as currently running version. We will not continue the update and check again in %d min.", AUTO_CHECK_INTERVAL_MINUTES);
                    esp_https_ota_abort(https_ota_handle);
                    myself->manualUpdateRequested=false;
                    myself->lastUpdateCheckUs=esp_timer_get_time();
                    continue;
                }
                int image_size = esp_https_ota_get_image_size(https_ota_handle);
                ESP_LOGI(TAG, "There is a new version %s available for firmware \"%s\" compiled on %s. We try to write this to partition subtype %d at offset 0x%x", app_desc_ota.version, app_desc_ota.project_name, app_desc_ota.date, update_partition->subtype, update_partition->address);
    
                esp_err_t err{ESP_ERR_HTTPS_OTA_IN_PROGRESS};
                while (err == ESP_ERR_HTTPS_OTA_IN_PROGRESS) 
                {
                    err = esp_https_ota_perform(https_ota_handle);
                    int image_already_read = esp_https_ota_get_image_len_read(https_ota_handle);
                    int percentage = (image_already_read*100)/image_size;
                    ESP_LOGI(TAG, "Flash complete: %d", percentage);
                }

                if (esp_https_ota_is_complete_data_received(https_ota_handle) != true) {
                    // the OTA image was not completely received and user can customise the response to this situation.
                    ESP_LOGE(TAG, "Complete data was not received.");
                    esp_https_ota_abort(https_ota_handle);
                    continue;
                } 
                esp_err_t ota_finish_err = esp_https_ota_finish(https_ota_handle);

                if ((err != ESP_OK) || (ota_finish_err != ESP_OK)) {
                    if (ota_finish_err == ESP_ERR_OTA_VALIDATE_FAILED) {
                        ESP_LOGE(TAG, "Image validation failed, image is corrupted");
                    }
                    ESP_LOGE(TAG, "ESP_HTTPS_OTA upgrade failed 0x%x", ota_finish_err);
                    continue;
                }
                ESP_LOGI(TAG, "ESP_HTTPS_OTA upgrade successful. Rebooting ...");
                vTaskDelay(pdMS_TO_TICKS(1000));
                esp_restart();
            }
        }

    public:
        void InitAndRun()
        {
            xTaskCreate(M::updaterTask, "updaterTask", 4096 * 4, this, 0, NULL);
        }

        void TriggerManualUpdate()
        {
            this->manualUpdateRequested = true;
        }
    };
}
#undef TAG