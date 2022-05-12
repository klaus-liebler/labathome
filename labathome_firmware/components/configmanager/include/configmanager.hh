#pragma once
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/event_groups.h>
#include <freertos/timers.h>
#include <esp_system.h>
#include <esp_mac.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <esp_wifi_types.h>
#include <esp_http_server.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <lwip/err.h>
#include <lwip/sys.h>
#include <lwip/api.h>
#include <lwip/err.h>
#include <lwip/netdb.h>
#include <lwip/ip4_addr.h>
#include <driver/gpio.h>
#include <nvs.h>


#include <esp_spi_flash.h>

#define TAG "CFG"

namespace configmanager
{
    constexpr size_t KEY_SIZE_MAX{15}; // see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html
    constexpr size_t VALUE_SIZE_MAX{63}; // real MX_SIZE=4000, https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html
    
    constexpr char UNIT_SEPARATOR{0x1F};
    constexpr char RECORD_SEPARATOR{0x1E};
    constexpr char GROUP_SEPARATOR{0x1D};
    constexpr char FILE_SEPARATOR{0x1C};
    constexpr size_t POST_CONFIG_MANAGER_BUFF_SIZE{2048};
    constexpr size_t TEMP_SIZE{63};

    const char *NVS_NAMESPACE{"cfgmgr"};
    const char *NVS_PARTITION{"nvs"};
    SemaphoreHandle_t nvs_sync_mutex{nullptr};
    httpd_handle_t wifi_manager_httpd_handle{nullptr};

    extern const char configmanager_html_gz_start[] asm("_binary_configmanager_html_gz_start");
    extern const size_t configmanager_html_gz_size asm("configmanager_html_gz_length");

    esp_err_t handle_get_configmanager(httpd_req_t *req)
    {
        httpd_resp_set_type(req, "text/html");
        httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
        httpd_resp_send(req, configmanager_html_gz_start, configmanager_html_gz_size); // -1 = use strlen()
        return ESP_OK;
    }



    esp_err_t handle_post_configmanager(httpd_req_t *req)
    {
        //Variables definition
        char delimiter_unit[] = {UNIT_SEPARATOR, '\0'};
        char delimiter_record[] = {RECORD_SEPARATOR, '\0'};
        char value_stored[VALUE_SIZE_MAX+1];
        int32_t value_stored_i{0};
        int32_t value_parsed_i{0};
        nvs_handle_t nvsHandle{0};
        esp_err_t esp_err{ESP_OK};
        char *key_ptr{nullptr};
        char *type_ptr{nullptr};
        char *value_ptr{nullptr};
        size_t len{0};
        size_t pos{0};
        int writtenValues{0};

        //receive the request from browser

        if (req->content_len > POST_CONFIG_MANAGER_BUFF_SIZE)
        { // too large
            ESP_LOGW(TAG, "req->content_len > POST_CONFIG_MANAGER_BUFF_SIZE");
            return ESP_FAIL;
        }
        char buf[POST_CONFIG_MANAGER_BUFF_SIZE + 1];
        int received = httpd_req_recv(req, buf, req->content_len);
        if (received != req->content_len){
            ESP_LOGW(TAG, "received != req->content_len");
            return ESP_FAIL;
        }
        buf[req->content_len] = '\0';
        
        
        if (req->content_len == 0)
        {
            ESP_LOGD(TAG, "In handle_post_configmanager(): req->content_len==0");
            return ESP_FAIL;
        }

        //Prepare saving data to flash
        ESP_LOGI(TAG, "About to save config to flash!!");

        if (!xSemaphoreTake(nvs_sync_mutex, portMAX_DELAY))
        {
            ESP_LOGE(TAG, "Unable to aquire lock");
            return ESP_FAIL;
        }

        esp_err = nvs_open_from_partition(NVS_PARTITION, NVS_NAMESPACE, NVS_READWRITE, &nvsHandle);
        if (esp_err != ESP_OK)
            ESP_LOGE(TAG, "Unable to open NVS");
            return ESP_FAIL;
        
        key_ptr = strtok(buf, delimiter_unit);
        while (key_ptr) //one loop iteration -> one key-type-value
        {
            len = strlen(key_ptr);
            if (len > KEY_SIZE_MAX)
            {
                ESP_LOGE(TAG, "Key too long");
                goto response;
            }
           
            
            type_ptr = strtok(NULL, delimiter_unit);
            if (!type_ptr)
            {
                ESP_LOGE(TAG, "No type for key %s found", key_ptr);
                goto response;
            }
            value_ptr = strtok(NULL, delimiter_record);
            if (!value_ptr)
            {
                ESP_LOGE(TAG, "No value for key %s found in request", key_ptr);
                goto response;
            }
            char type = type_ptr[0];
            switch (type)
            {
            case 's'://String
                len=VALUE_SIZE_MAX;
                esp_err = nvs_get_str(nvsHandle, key_ptr, value_stored, &len);//len incudes zero teminator
                if (esp_err == ESP_ERR_NVS_NOT_FOUND || (esp_err == ESP_OK && strcmp(value_ptr, value_stored) != 0))
                { 
                    esp_err = nvs_set_str(nvsHandle, key_ptr, value_ptr);
                    if (esp_err != ESP_OK){
                        ESP_LOGE(TAG, "Unable write key %s and value %s", key_ptr, value_ptr);
                        goto response;
                    }
                    writtenValues++;
                    ESP_LOGI(TAG, "Successfully written key %s and value %s", key_ptr, value_ptr);
                }
                break;
            case 'i'://Integer and enumeration
                value_parsed_i = strtol(value_ptr, nullptr, 10);
                if(errno!=0){
                    ESP_LOGE(TAG, "value %s for key %s cound not be parsed to integer", value_ptr, key_ptr);
                    goto response;
                }
                esp_err = nvs_get_i32(nvsHandle, key_ptr, &value_stored_i);
                if (esp_err == ESP_ERR_NVS_NOT_FOUND || value_stored_i!=value_parsed_i)
                {
                    esp_err = nvs_set_i32(nvsHandle, key_ptr, value_parsed_i);
                    if (esp_err != ESP_OK){
                        ESP_LOGE(TAG, "Unable write key %s and value %d", key_ptr, value_parsed_i);
                        goto response;
                    }
                    writtenValues++;
                    ESP_LOGI(TAG, "Successfully written key %s and value %s", key_ptr, value_ptr);
                }
                break;
            default:
                ESP_LOGE(TAG, "Unknown data type %c for key %s and value %s", type, key_ptr, value_ptr);
                goto response;
                break;
            }


            key_ptr = strtok(buf, delimiter_unit);
        }
        if (writtenValues)
        {
            esp_err = nvs_commit(nvsHandle);
            ESP_LOGI(TAG, "new config with %d written values was commited to flash", writtenValues);
        }
        else
        {
            ESP_LOGI(TAG, "new config was not committed to flash because no change has been detected.");
        }
    response:
        pos=0;
        nvs_iterator_t it = nvs_entry_find(NVS_PARTITION, NVS_NAMESPACE, NVS_TYPE_STR);
        while (it != NULL) {
                nvs_entry_info_t info;
                nvs_entry_info(it, &info);
                len=VALUE_SIZE_MAX;
                esp_err = nvs_get_str(nvsHandle, info.key, value_stored, &len);//len incudes zero teminator
                if(esp_err!=ESP_OK){
                    goto release_resources;
                }
                pos += snprintf(buf + pos, POST_CONFIG_MANAGER_BUFF_SIZE - pos, "%s%c%c%c%s%c", info.key, UNIT_SEPARATOR, 's', UNIT_SEPARATOR, value_stored, RECORD_SEPARATOR);
                if (pos >= POST_CONFIG_MANAGER_BUFF_SIZE)
                {
                    ESP_LOGE(TAG, "Problems in handle_put_wifimanager: Buffer too small");
                    
                    goto release_resources;
                }
                it = nvs_entry_next(it);
        }
        it = nvs_entry_find(NVS_PARTITION, NVS_NAMESPACE, NVS_TYPE_I32);
        while (it != NULL) {
                nvs_entry_info_t info;
                nvs_entry_info(it, &info);
                int32_t value{0};
                esp_err = nvs_get_i32(nvsHandle, info.key, &value);
                if(esp_err!=ESP_OK){
                    goto release_resources;
                }
                pos += snprintf(buf + pos, POST_CONFIG_MANAGER_BUFF_SIZE - pos, "%s%c%c%c%s%c", info.key, UNIT_SEPARATOR, 'i', UNIT_SEPARATOR, value_stored, RECORD_SEPARATOR);
                if (pos >= POST_CONFIG_MANAGER_BUFF_SIZE)
                {
                    ESP_LOGE(TAG, "Problems in handle_put_wifimanager: Buffer too small");
                    
                    goto release_resources;
                }
                it = nvs_entry_next(it);
        }   
release_resources:     
        nvs_close(nvsHandle);
        xSemaphoreGive(nvs_sync_mutex);
        if(esp_err != ESP_OK){
            return esp_err;
        }
        httpd_resp_set_type(req, "text/plain");
        httpd_resp_set_hdr(req, "Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
        httpd_resp_set_hdr(req, "Pragma", "no-cache");
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
        httpd_resp_send(req, (char *)buf, pos);
        return ESP_OK;
    }

    static const httpd_uri_t get_configmanager = {
        .uri = "/configmanager",
        .method = HTTP_GET,
        .handler = handle_get_configmanager,
        .user_ctx = nullptr,
    };

    static const httpd_uri_t post_configmanager = {
        .uri = "/wifimanager",
        .method = HTTP_POST,
        .handler = handle_post_configmanager,
        .user_ctx = nullptr,
    };



    
    esp_err_t configmanager_start()
    {
        if (nvs_sync_mutex != nullptr)
        {
            ESP_LOGE(TAG, "configmanager already started");
            return ESP_FAIL;
        }

        ESP_LOGI(TAG, "configmanager successfully started");
        return ESP_OK;
    }

    void RegisterHTTPDHandlers(httpd_handle_t httpd_handle)
    {
        wifi_manager_httpd_handle = httpd_handle;
        ESP_ERROR_CHECK(httpd_register_uri_handler(wifi_manager_httpd_handle, &get_configmanager));
        ESP_ERROR_CHECK(httpd_register_uri_handler(wifi_manager_httpd_handle, &post_configmanager));
    }

}
#undef TAG