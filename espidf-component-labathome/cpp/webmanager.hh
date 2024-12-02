#pragma once
#include <sdkconfig.h>
#include <cstring>
#include <ctime>
#include <algorithm>
#include <vector>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/timers.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <esp_timer.h>
#include <esp_chip_info.h>
#include <esp_mac.h>
#include <esp_wifi.h>
#include <esp_https_server.h>
#include "esp_tls.h"
#include <hal/efuse_hal.h>
#include <nvs_flash.h>
#include <lwip/err.h>
#include <lwip/sys.h>
#include <lwip/api.h>
#include <lwip/netdb.h>
#include <lwip/ip4_addr.h>
#include <driver/gpio.h>
#include <driver/temperature_sensor.h>
#include <nvs.h>
#include <spi_flash_mmap.h>
#include <esp_sntp.h>
#include <time.h>
#include <mdns.h>
#include <common-esp32.hh>
#include <esp_log.h>
#include <sys/time.h>
#if (CONFIG_HTTPD_MAX_REQ_HDR_LEN<1024)
    #error CONFIG_HTTPD_MAX_REQ_HDR_LEN<1024 (Max HTTP Request Header Length)
#endif
#ifndef CONFIG_ESP_HTTPS_SERVER_ENABLE
    #error "Enable HTTPS in menuconfig"
#endif
#ifndef CONFIG_HTTPD_WS_SUPPORT
 #error "Enable Websocket support for HTTPD in menuconfig"
#endif
#include "esp_vfs.h"
#include "webmanager_constants.hh"
#include "webmanager_interfaces.hh"
#include "../generated/flatbuffers_cpp/webmanager_generated.h"

#define TAG "WMAN"



namespace webmanager
{
    constexpr size_t HTTP_BUFFER_SIZE{2*2048};
    constexpr size_t MAX_FILE_SIZE{256*1024};
    /* Max length a file path can have on storage */
    constexpr size_t FILE_PATH_MAX{ESP_VFS_PATH_MAX + CONFIG_SPIFFS_OBJ_NAME_LEN};

    constexpr const char* FILES_BASE_PATH = "/files";

    enum class WifiStationState
    {

        NO_CONNECTION,      //Zustand während der Initialisierung wenn noch unklar ist, was Sache ist
                            //und auch, wenn keine brauchbaren Daten für einen Verbindungsaufbau vorliegen
        SHOULD_CONNECT,   // Daten sind verfügbar, die passen könnten. Es soll beim nächsten Retry-Tick ein Verbindungsversuch gestartet werden. Gerade im Moment wurde aber noch kein Verbindungsversiuch gestartet. -->Scan möglich
        ABOUT_TO_CONNECT, // es wurde gerade ein Verbindungsaufbau gestartet, es ist aber noch nicht klar, ob der erfolgreich war -->Scan nicht möglich
        CONNECTED,
    };

    class M : public webmanager::iWebmanagerCallback
    {
    private:
        static M *singleton;
        uint8_t* http_buffer;

        char* hostname{nullptr};

        esp_netif_t *wifi_netif_sta{nullptr};
        esp_netif_t *wifi_netif_ap{nullptr};

        wifi_config_t wifi_config_sta = {}; // 132byte
        wifi_config_t wifi_config_ap = {};  // 132byte

        wifi_ap_record_t accessp_records[MAX_AP_NUM]; // 80byte*8=640byte
        uint16_t accessp_records_len{0};

        SemaphoreHandle_t webmanager_semaphore{nullptr};
        TimerHandle_t wifi_manager_retry_timer{nullptr};
        TimerHandle_t wifi_manager_shutdown_ap_timer{nullptr};
        temperature_sensor_handle_t temp_handle{nullptr};
        httpd_handle_t http_server{nullptr};
        int websocket_file_descriptor{-1};
        
        WifiStationState staState{WifiStationState::NO_CONNECTION};
        // bool accessPointIsActive{false}; // nein, diese Information kann über esp_wifi_get_mode() immer herausgefunden werden
        uint32_t remainingAttempsToConnectAsSTA{ATTEMPTS_TO_RECONNECT_ON_STARTUP_BEFORE_OPENING_AN_ACCESS_POINT};
        bool initialScanIsActive{false};
        bool scanIsActive{false};
        esp_ip4_addr_t ipAddr;
        const char* ssid="-";
        std::vector<iWebmanagerPlugin*> *plugins{nullptr};

        class AsyncResponse{
            public:
            uint8_t* buffer;
            size_t buffer_len;
        
            AsyncResponse(uint32_t ns, flatbuffers::FlatBufferBuilder* b){
                uint8_t* bp=b->GetBufferPointer();
                buffer_len = b->GetSize();
                buffer = new uint8_t[buffer_len+4];
                ((uint32_t*)buffer)[0]=ns;
                std::memcpy(buffer+4, bp, buffer_len);
            }

            ~AsyncResponse(){
                delete[] buffer;
            }
        };

        M(){
            http_buffer=new uint8_t[HTTP_BUFFER_SIZE];
        }

        bool hasRealtime(){
            struct timeval tv_now;
            gettimeofday(&tv_now, nullptr);
            time_t seconds_epoch = tv_now.tv_sec;
            return seconds_epoch > 1684412222; // epoch time when this code has been written
        }

        void connectAsSTA()
        {
            ESP_LOGI(TAG, "Trying to connect as station. ssid='%s', password='%s'.", wifi_config_sta.sta.ssid, wifi_config_sta.sta.password);
            ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
            ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config_sta));
            ESP_ERROR_CHECK(esp_wifi_connect());
            staState = WifiStationState::ABOUT_TO_CONNECT;
        }

        esp_err_t delete_sta_config()
        {
            nvs_handle handle{0};
            esp_err_t ret = ESP_OK;
            GOTO_ERROR_ON_ERROR(nvs_open_from_partition(NVS_PARTITION, NVS_NAMESPACE, NVS_READWRITE, &handle), "Unable to open nvs partition");
            GOTO_ERROR_ON_ERROR(nvs_erase_key(handle, nvs_key_wifi_ssid), "Unable to delete wifi ssid");
            GOTO_ERROR_ON_ERROR(nvs_erase_key(handle, nvs_key_wifi_password), "Unable to delete wifi password");
            ret = nvs_commit(handle);
            ESP_LOGI(TAG, "Successfully erased Wifi Sta configuration in flash");
        error:
            nvs_close(handle);
            return ret;
        }

        esp_err_t update_sta_config_lazy()
        {
            nvs_handle handle;
            esp_err_t ret = ESP_OK;
            char tmp_ssid[33];     /**< SSID of target AP. */
            char tmp_password[64]; /**< Password of target AP. */
            bool change{false};
            size_t sz{0};

            ESP_LOGI(TAG, "About to save config to flash!!");
            GOTO_ERROR_ON_ERROR(nvs_open_from_partition(NVS_PARTITION, NVS_NAMESPACE, NVS_READWRITE, &handle), "Unable to open nvs partition");
            sz = sizeof(tmp_ssid);
            ret = nvs_get_str(handle, nvs_key_wifi_ssid, tmp_ssid, &sz);
            if ((ret == ESP_OK && strcmp((char *)tmp_ssid, (char *)wifi_config_sta.sta.ssid) != 0) || ret == ESP_ERR_NVS_NOT_FOUND)
            {
                /* different ssid or ssid does not exist in flash: save new ssid */
                GOTO_ERROR_ON_ERROR(nvs_set_str(handle, nvs_key_wifi_ssid, (const char *)wifi_config_sta.sta.ssid), "Unable to nvs_set_str(handle, \"ssid\", ssid_sta)");
                ESP_LOGI(TAG, "wifi_manager_wrote wifi_sta_config: ssid: %s", wifi_config_sta.sta.ssid);
                change = true;
            }

            sz = sizeof(tmp_password);
            ret = nvs_get_str(handle, nvs_key_wifi_password, tmp_password, &sz);
            if ((ret == ESP_OK && strcmp((char *)tmp_password, (char *)wifi_config_sta.sta.password) != 0) || ret == ESP_ERR_NVS_NOT_FOUND)
            {
                /* different password or password does not exist in flash: save new password */
                GOTO_ERROR_ON_ERROR(nvs_set_str(handle, nvs_key_wifi_password, (const char *)wifi_config_sta.sta.password), "Unable to nvs_set_str(handle, \"password\", password_sta)");
                ESP_LOGI(TAG, "wifi_manager_wrote wifi_sta_config: password: %s", wifi_config_sta.sta.password);
                change = true;
            }
            if (change)
            {
                ret = nvs_commit(handle);
            }
            else
            {
                ESP_LOGI(TAG, "Wifi config was not saved to flash because no change has been detected.");
            }
        error:
            nvs_close(handle);
            return ret;
        }

        esp_err_t read_sta_config()
        {
            nvs_handle handle;
            esp_err_t ret = ESP_OK;
            size_t sz;
            GOTO_ERROR_ON_ERROR(nvs_open_from_partition(NVS_PARTITION, NVS_NAMESPACE, NVS_READWRITE, &handle), "Unable to open nvs partition");
            sz = sizeof(wifi_config_sta.sta.ssid);
            ret = nvs_get_str(handle, nvs_key_wifi_ssid, (char *)wifi_config_sta.sta.ssid, &sz);
            if (ret != ESP_OK)
            {
                ESP_LOGI(TAG, "Unable to read Wifi SSID from NVS");
                goto error;
            }

            sz = sizeof(wifi_config_sta.sta.password);
            ret = nvs_get_str(handle, nvs_key_wifi_password, (char *)wifi_config_sta.sta.password, &sz);
            if (ret != ESP_OK)
            {
                ESP_LOGI(TAG, "Unable to read Wifi password from NVS");
                goto error;
            }
            ESP_LOGI(TAG, "Successfully read Wifi credentials: ssid: %s password: %s", wifi_config_sta.sta.ssid, wifi_config_sta.sta.password);
            ret = (wifi_config_sta.sta.ssid[0] == '\0') ? ESP_FAIL : ESP_OK;
        error:
            nvs_close(handle);
            return ret;
        }

        void webmanager_timer_retry_cb(TimerHandle_t xTimer)
        {
            ESP_LOGI(TAG, "Retry Timer Tick!");
            xTimerStop(xTimer, (TickType_t)0);
            xSemaphoreTake(webmanager_semaphore, portMAX_DELAY);
            if (staState == WifiStationState::SHOULD_CONNECT)
            {
                connectAsSTA();
            }
            xSemaphoreGive(webmanager_semaphore);
        }

        void webmanager_timer_shutdown_ap_cb(TimerHandle_t xTimer)
        {
            ESP_LOGI(TAG, "ShutdownAP Timer Tick!");
            xTimerStop(xTimer, (TickType_t)0);
            xSemaphoreTake(webmanager_semaphore, portMAX_DELAY);
            if (staState == WifiStationState::CONNECTED)
            {
                esp_wifi_set_mode(WIFI_MODE_STA);
            }
            else{
                ESP_LOGE(TAG, "AP should be shut down, but staState != WifiStationState::CONNECTED");
            }
            xSemaphoreGive(webmanager_semaphore);
        }

        void wifi_event_handler(esp_event_base_t event_base, int32_t event_id, void *event_data)
        {
            switch (event_id)
            {
            case WIFI_EVENT_SCAN_DONE:
            {
                ESP_LOGD(TAG, "WIFI_EVENT_SCAN_DONE");
                initialScanIsActive=false;
                scanIsActive=false;
                wifi_event_sta_scan_done_t *event_sta_scan_done = (wifi_event_sta_scan_done_t *)event_data;
                if (event_sta_scan_done->status == 0){//0="Success"
                    xSemaphoreTake(webmanager_semaphore, portMAX_DELAY);
                    // As input param, it stores max AP number ap_records can hold. As output param, it receives the actual AP number this API returns.
                    // As a consequence, ap_num MUST be reset to MAX_AP_NUM at every scan */
                    accessp_records_len = MAX_AP_NUM;
                    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&accessp_records_len, accessp_records));
                    xSemaphoreGive(webmanager_semaphore);
                    ESP_LOGI(TAG, "Wifi Scan successfully completed. Found %d access points.", accessp_records_len);
                }
                if(staState==WifiStationState::NO_CONNECTION){
                    //nur wenn wir uns weiterhin im NO_CONNECTION befinden, soll beständig weiter gesucht werden
                    //in den "Übergangs"-Zuständen (about to connect, should_connect) darf nichts passierte
                    //im Zustand CONNECTED soll höchstens einmalig auf expliziten Wunsch gesucht werden
                    //TODO esp_wifi_scan_start(&scan_config, false);
                }
                break;
            }
            case WIFI_EVENT_STA_DISCONNECTED:
            {
                wifi_event_sta_disconnected_t *wifi_event_sta_disconnected = (wifi_event_sta_disconnected_t *)event_data;
                xSemaphoreTake(webmanager_semaphore, portMAX_DELAY);
                //scanIsActive = false; // if a DISCONNECT message is posted while a scan is in progress this scan will NEVER end, causing scan to never work again. For this reason SCAN_BIT is cleared too
                // if there was a timer on to stop the AP, well now it's time to cancel that since connection was lost! */
                xTimerStop(wifi_manager_shutdown_ap_timer, portMAX_DELAY);
                switch (staState)
                {
                case WifiStationState::NO_CONNECTION:
                    ESP_LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED, when STA_STATE::NO_CONNECTION --> disconnection was requested by user. Start Access Point");
                    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
                    break;
                case WifiStationState::CONNECTED:
                    ESP_LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED, when STA_STATE::CONNECTED --> unexpected disconnection. Try to reconnect %lu times.", remainingAttempsToConnectAsSTA);
                    // Die Verbindung bestand zuvor und wurde jetzt getrennt. Versuche, erneut zu verbinden
                    staState = WifiStationState::SHOULD_CONNECT;
                    xTimerStart(wifi_manager_retry_timer, portMAX_DELAY);
                    break;
                case WifiStationState::ABOUT_TO_CONNECT:
                    remainingAttempsToConnectAsSTA--;
                    // Die Verbindung war bereits getrennt und es wurde über den Retry Timer versucht, diese neu aufzubauen. Das schlug fehl
                    if (remainingAttempsToConnectAsSTA <= 0)
                    {
                        ESP_LOGD(TAG, "After (several?) attemps it was not possible to establish connection as STA with ssid %s and password %s (Reason %d). Start AccessPoint mode with ssid %s and password %s.", wifi_config_sta.sta.ssid, wifi_config_sta.sta.password, wifi_event_sta_disconnected->reason, wifi_config_ap.ap.ssid, wifi_config_ap.ap.password);
                        ESP_LOGI(TAG, "Re-establishing connection failed finally (Reason %d). Start AccessPoint mode with ssid %s and password %s..", wifi_event_sta_disconnected->reason, wifi_config_ap.ap.ssid, wifi_config_ap.ap.password);
                        
                        staState = WifiStationState::NO_CONNECTION;
                        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
                        esp_wifi_scan_start(nullptr/*for default config*/, false);
                        scanIsActive=true;
                        flatbuffers::FlatBufferBuilder b(256);
                        b.Finish(webmanager::CreateResponseWifiConnectDirect(b, false, (char*)wifi_config_sta.sta.ssid, 0,0,0));
                        WrapAndSendAsync(webmanager::Namespace::Namespace_Value, b);
                    }
                    else
                    {
                        ESP_LOGD(TAG, "WIFI_EVENT_STA_DISCONNECTED, when STA_STATE::ABOUT_TO_CONNECT --> disconnection occured earlier and we tried to establish it again...which was not successful (Reason %d). Still %lu attempt(s) to go.", wifi_event_sta_disconnected->reason, remainingAttempsToConnectAsSTA);
                        ESP_LOGI(TAG, "Re-establishing connection failed (Reason %d). Still %lu attempt(s) to go.", wifi_event_sta_disconnected->reason, remainingAttempsToConnectAsSTA);
                        staState = WifiStationState::SHOULD_CONNECT;
                        xTimerStart(wifi_manager_retry_timer, portMAX_DELAY);
                    }
                    break;
                default:
                    ESP_LOGE(TAG, "In WIFI_EVENT_STA_DISCONNECTED Event loop: Unexpected state %d", (int)staState);
                    break;
                }
                xSemaphoreGive(webmanager_semaphore);
                break;
            }
            case WIFI_EVENT_STA_CONNECTED:{
                wifi_event_sta_connected_t *wifi_event_sta_connected = (wifi_event_sta_connected_t *)event_data;
                ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTED --> Generally, a connection to the given AP is possible. Set remainingAttempsToConnectAsSTA to a high value");
                remainingAttempsToConnectAsSTA = ATTEMPTS_TO_RECONNECT_DURING_OPERATION_BEFORE_OPENING_AN_ACCESS_POINT;
                this->ssid= (const char*)wifi_event_sta_connected->ssid;
                break;
            }
            case WIFI_EVENT_AP_START:
            {
                ESP_LOGI(TAG, "Successfully started Access Point with ssid %s and password '%s'. Webmanager is here: https://%s", wifi_config_ap.ap.ssid, wifi_config_ap.ap.password, hostname);
                break;
            }
            case WIFI_EVENT_AP_STOP:
            {
                ESP_LOGI(TAG, "Successfully closed Access Point.");
                break;
            }
            case WIFI_EVENT_AP_STACONNECTED:
            {
                wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
                ESP_LOGI(TAG, "Station " MACSTR " joined this AccessPoint, AID=%d", MAC2STR(event->mac), event->aid);
                break;
            }
            case WIFI_EVENT_AP_STADISCONNECTED:
            {
                wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
                ESP_LOGI(TAG, "Station " MACSTR " leaved this AccessPoint, AID=%d", MAC2STR(event->mac), event->aid);
                break;
            }
            }
        }
        
        void ip_event_handler(esp_event_base_t event_base, int32_t event_id, void *event_data)
        {
            switch (event_id)
            {

            /* This event arises when the DHCP client successfully gets the IPV4 address from the DHCP server,
             * or when the IPV4 address is changed. The event means that everything is ready and the application can begin
             * its tasks (e.g., creating sockets).
             * The IPV4 may be changed because of the following reasons:
             *    The DHCP client fails to renew/rebind the IPV4 address, and the station’s IPV4 is reset to 0.
             *    The DHCP client rebinds to a different address.
             *    The static-configured IPV4 address is changed.
             * Whether the IPV4 address is changed or NOT is indicated by field ip_change of ip_event_got_ip_t.
             * The socket is based on the IPV4 address, which means that, if the IPV4 changes, all sockets relating to this
             * IPV4 will become abnormal. Upon receiving this event, the application needs to close all sockets and recreate
             * the application when the IPV4 changes to a valid one. */
            case IP_EVENT_STA_GOT_IP:
            {
                ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
                ESP_LOGI(TAG, "Got IP from DHCP: ip=" IPSTR " netmask=" IPSTR " gw=" IPSTR " hostname=%s", IP2STR(&event->ip_info.ip), IP2STR(&event->ip_info.netmask), IP2STR(&event->ip_info.gw), hostname);
                remainingAttempsToConnectAsSTA=ATTEMPTS_TO_RECONNECT_DURING_OPERATION_BEFORE_OPENING_AN_ACCESS_POINT;
                staState = WifiStationState::CONNECTED;
                update_sta_config_lazy();
                xSemaphoreTake(webmanager_semaphore, portMAX_DELAY);
                wifi_mode_t mode;
                esp_wifi_get_mode(&mode);
                if (mode == WIFI_MODE_APSTA)
                {
                    xTimerStart(wifi_manager_shutdown_ap_timer, portMAX_DELAY);
                }
                xSemaphoreGive(webmanager_semaphore);
                esp_sntp_init();
                if(http_server){
                    wifi_ap_record_t ap = {};
                    esp_wifi_sta_get_ap_info(&ap);
                    flatbuffers::FlatBufferBuilder b(256);
                    b.Finish(webmanager::CreateResponseWifiConnectDirect(b, true, (char*)wifi_config_sta.sta.ssid, event->ip_info.ip.addr, event->ip_info.netmask.addr, event->ip_info.gw.addr));
                    this->ipAddr=event->ip_info.ip;
                    WrapAndSendAsync(webmanager::Namespace::Namespace_Value, b);
                }
                break;
            }
            /* This event arises when the IPV4 address become invalid.
             * IP_STA_LOST_IP doesn’t arise immediately after the WiFi disconnects, instead it starts an IPV4 address lost timer,
             * if the IPV4 address is got before ip lost timer expires, IP_EVENT_STA_LOST_IP doesn’t happen. Otherwise, the event
             * arises when IPV4 address lost timer expires.
             * Generally the application don’t need to care about this event, it is just a debug event to let the application
             * know that the IPV4 address is lost. */
            case IP_EVENT_STA_LOST_IP:
            {
                ESP_LOGD(TAG, "IP_EVENT_STA_LOST_IP");
                break;
            }
            }
        }

        void sntp_handler(){
            time_t now;
            char strftime_buf[64];
            struct tm timeinfo;
            time(&now);
            localtime_r(&now, &timeinfo);
            strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
            ESP_LOGI(TAG, "Notification of a time synchronization. The current date/time in Berlin is: %s", strftime_buf);
            for(const auto& p : *this->plugins){p->OnTimeUpdate(this);}
            //LogJournal(messagecodes::C::SNTP, esp_timer_get_time() / 1000);
        }

        static void ws_async_send(void *arg)
        {
            M* myself = M::GetSingleton();
            AsyncResponse *a = static_cast<AsyncResponse*>(arg);
            assert(a);
            assert(a->buffer);
            assert(a->buffer_len);
            assert(myself);
            if(myself->http_server && myself->websocket_file_descriptor!=-1){
                httpd_ws_frame_t ws_pkt={false, false, HTTPD_WS_TYPE_BINARY, a->buffer, a->buffer_len};
                httpd_ws_send_frame_async(myself->http_server, myself->websocket_file_descriptor, &ws_pkt);
                //printf("httpd_ws_send_frame_async: http:%lu fd:%i data_len:%u\n", (uint32_t)myself->http_server, myself->websocket_file_descriptor, a->buffer_len);
                //should be syncronous. So the buffer can be deleted, when the function returns
            }
            delete a;
        }

        esp_err_t handle_webmanager_ws(httpd_req_t *req)
        {
            if (req->method == HTTP_GET)
            {
                ESP_LOGI(TAG, "Handshake done, the new websocket connection was opened");
                return ESP_OK;
            }
            httpd_ws_frame_t ws_pkt={false, false, HTTPD_WS_TYPE_BINARY, nullptr, 0};

            //always store the last websocket file descriptor
            this->websocket_file_descriptor = httpd_req_to_sockfd(req);

            /* Set max_len = 0 to get the frame len */
            ESP_ERROR_CHECK(httpd_ws_recv_frame(req, &ws_pkt, 0));
            if (ws_pkt.len == 0 || ws_pkt.type != HTTPD_WS_TYPE_BINARY)
            {
                ESP_LOGE(TAG, "Received an empty or an non binary websocket frame");
                return ESP_OK;
            }
            uint8_t *buf = new uint8_t[ws_pkt.len];
            assert(buf);
            ws_pkt.payload = buf;
            esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
            
            if (ret != ESP_OK)
            {
                ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
                delete[] buf;
                return ret;
            }
            uint32_t ns = *((uint32_t*)buf);
            uint8_t* fb_buf=buf+4;
            if(ns==webmanager::Namespace::Namespace_Value){
                auto rw = flatbuffers::GetRoot<webmanager::RequestWrapper>(fb_buf);
                webmanager::Requests reqType=rw->request_type();
                ESP_LOGI(TAG, "Received websocket frame: len=%d, requestType=%d ", (int)ws_pkt.len, reqType);
                switch (reqType){
                case webmanager::Requests::Requests_RequestNetworkInformation:
                    sendResponseNetworkInformation(req, &ws_pkt, rw->request_as_RequestNetworkInformation());
                    break;
                
                case webmanager::Requests::Requests_RequestWifiConnect:
                    handleRequestWifiConnect(req, &ws_pkt, rw->request_as_RequestWifiConnect());
                    break;
                case webmanager::Requests::Requests_RequestWifiDisconnect:
                    handleRequestWifiDisconnect(req, &ws_pkt, rw->request_as_RequestWifiDisconnect());
                    break;
                default:                
                    break;
                }
            }else{
                eMessageReceiverResult success{eMessageReceiverResult::NOT_FOR_ME};
                if(plugins){
                    for(auto p:*plugins){
                        success=p->ProvideWebsocketMessage(this, req, &ws_pkt, ns, fb_buf);
                        if(success!=eMessageReceiverResult::NOT_FOR_ME){
                            break;
                        }
                    }
                }
                if(success==eMessageReceiverResult::NOT_FOR_ME){
                    ESP_LOGW(TAG, "Not yet implemented request for namespace %lu, neither internal nor in a plugin", ns);
                }else if(success==eMessageReceiverResult::FOR_ME_BUT_FAILED){
                    ESP_LOGW(TAG, "Request for namespace %lu has been implemented by plugin, but processing failed", ns);
                }
            }
            delete[] buf;
            return ESP_OK;
        }
        
        esp_err_t handleRequestWifiConnect(httpd_req_t *req, httpd_ws_frame_t *ws_pkt, const webmanager::RequestWifiConnect *wifiConnect){
             
            esp_err_t ret{ESP_OK};//necessary for ESP_GOTO_ON_FALSE
            const char* ssid = wifiConnect->ssid()->c_str();
            const char* password = wifiConnect->password()->c_str();
            size_t len{0};
            len = strlen(ssid);
            ESP_GOTO_ON_FALSE(len <= MAX_SSID_LEN - 1, ESP_FAIL, negativeresponse, TAG, "SSID too long");
            len = strlen(password);
            ESP_GOTO_ON_FALSE(len>0, ESP_FAIL, negativeresponse, TAG, "no password given");
            snprintf((char *)wifi_config_sta.sta.ssid, MAX_SSID_LEN - 1, ssid); //TODO: Maximum number of bytes to be used in the buffer. The generated string has a length of at most n-1, leaving space for the additional terminating null character.
            snprintf((char *)wifi_config_sta.sta.password, MAX_PASSPHRASE_LEN - 1, password);
            ESP_LOGI(TAG, "Got a new SSID %s and PASSWORD %s from browser.", wifi_config_sta.sta.ssid, wifi_config_sta.sta.password);
            remainingAttempsToConnectAsSTA = 1;
            connectAsSTA();
            return ret;
        negativeresponse:
            flatbuffers::FlatBufferBuilder b(256);
            b.Finish(CreateResponseWifiConnectDirect(b, false, (char*)wifi_config_sta.sta.ssid, 0,0,0,0));
            return WrapAndSendAsync(webmanager::Namespace::Namespace_Value,  b);
        }

        esp_err_t handleRequestWifiDisconnect(httpd_req_t *req, httpd_ws_frame_t *ws_pkt, const webmanager::RequestWifiDisconnect *wifiDisconnect){
            flatbuffers::FlatBufferBuilder b(256);
            auto res =  CreateResponseWifiDisconnect(b);
            auto mwresp= CreateResponseWrapper(b, Responses::Responses_ResponseWifiDisconnect, res.Union());
            b.Finish(mwresp);
            AsyncResponse *a = new AsyncResponse(webmanager::Namespace::Namespace_Value, &b);
            if(httpd_queue_work(http_server, M::ws_async_send, a)!=ESP_OK){delete(a);}
            vTaskDelay(pdMS_TO_TICKS(2000)); // warte 200ms, um die Beantwortung des Requests noch zu ermöglichen
            xSemaphoreTake(webmanager_semaphore, portMAX_DELAY);
            staState = WifiStationState::NO_CONNECTION;
            ESP_ERROR_CHECK(esp_wifi_disconnect());
            delete_sta_config();
            ESP_LOGI(TAG, "Disconnected as STA from ssid %s.", wifi_config_sta.sta.ssid);
            xSemaphoreGive(webmanager_semaphore);
            return ESP_OK;
        
        }

        esp_err_t sendResponseNetworkInformation(httpd_req_t *req, httpd_ws_frame_t *ws_pkt, const webmanager::RequestNetworkInformation  *netInfo){
            bool forceUpdate = netInfo->force_new_search();
            ESP_LOGI(TAG, "Prepare to send ResponseWifiAccesspoints");
            esp_err_t forcedScanResult{ESP_OK};
            if(!initialScanIsActive && !scanIsActive && forceUpdate){
                forcedScanResult=esp_wifi_scan_start(nullptr/*for default config*/, true);
                this->scanIsActive=true;
                ESP_LOGI(TAG, "Forced scan finished with %s", esp_err_to_name(forcedScanResult));
            }
            xSemaphoreTake(webmanager_semaphore, portMAX_DELAY);
            flatbuffers::FlatBufferBuilder b(1024);
            std::vector<flatbuffers::Offset<AccessPoint>> ap_vector;
            if(forceUpdate && forcedScanResult!=ESP_OK){
                ap_vector.push_back(CreateAccessPoint(b, b.CreateString("Error while forced scanning"), forcedScanResult, 0, 0));
            }
            else if(initialScanIsActive){
                ap_vector.push_back(CreateAccessPoint(b, b.CreateString("Initial Scan was still active. Refresh page in a few seconds!"), 0, 0, 0));
            
            }else{
                for (size_t i = 0; i < accessp_records_len; i++)
                {
                    wifi_ap_record_t* ap = accessp_records+i;
                    ap_vector.push_back(CreateAccessPoint(b, b.CreateString((char*)ap->ssid), ap->primary, ap->rssi, ap->authmode));
                }
            }
            esp_netif_ip_info_t ap_ip_info = {};
            ESP_ERROR_CHECK(esp_netif_get_ip_info(wifi_netif_ap, &ap_ip_info));
            esp_netif_ip_info_t sta_ip_info = {};
            ESP_ERROR_CHECK(esp_netif_get_ip_info(wifi_netif_sta, &sta_ip_info));
            wifi_ap_record_t ap={};
            esp_wifi_sta_get_ap_info(&ap);
            xSemaphoreGive(webmanager_semaphore);
            b.Finish(CreateResponseNetworkInformationDirect(b, 
                hostname, 
                (char*)wifi_config_ap.ap.ssid,
                (char*)wifi_config_ap.ap.password,
                ap_ip_info.ip.addr,
                this->staState==WifiStationState::CONNECTED,
                (char*)wifi_config_sta.sta.ssid,
                sta_ip_info.ip.addr,
                sta_ip_info.netmask.addr,
                sta_ip_info.gw.addr,
                ap.rssi,
                &ap_vector));
            return WrapAndSendAsync(webmanager::Namespace::Namespace_Value, b);
        }
        
        esp_err_t handle_ota_post(httpd_req_t *req)
        {
            ESP_LOGI(TAG, "in handle_ota_post");
            char buf[1024];
            esp_ota_handle_t ota_handle;
            size_t remaining = req->content_len;

            const esp_partition_t *ota_partition = esp_ota_get_next_update_partition(NULL);
            ESP_ERROR_CHECK(esp_ota_begin(ota_partition, OTA_SIZE_UNKNOWN, &ota_handle));

            while (remaining > 0)
            {
                int recv_len = httpd_req_recv(req, buf, std::min(remaining, (size_t)sizeof(buf)));
                if (recv_len <= 0)
                {
                    // Serious Error: Abort OTA
                    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Protocol Error");
                    return ESP_FAIL;
                }
                if (recv_len == HTTPD_SOCK_ERR_TIMEOUT)
                {
                    // Timeout Error: Just retry
                    continue;
                }
                if (esp_ota_write(ota_handle, (const void *)buf, recv_len) != ESP_OK)
                {
                    // Successful Upload: Flash firmware chunk
                    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Flash Error");
                    return ESP_FAIL;
                }

                remaining -= recv_len;
            }

            // Validate and switch to new OTA image and reboot
            if (esp_ota_end(ota_handle) != ESP_OK || esp_ota_set_boot_partition(ota_partition) != ESP_OK)
            {
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Validation / Activation Error");
                return ESP_FAIL;
            }

            httpd_resp_sendstr(req, "Firmware update complete, rebooting now!\n");

            vTaskDelay(500 / portTICK_PERIOD_MS);
            esp_restart();

            return ESP_OK;
        }

        /* Send HTTP response with a run-time generated html consisting of
        * a list of all files and folders under the requested path.
        * In case of SPIFFS this returns empty list when path is any
        * string other than '/', since SPIFFS doesn't support directories */
        esp_err_t http_resp_dir_html(httpd_req_t *req, const char *dirpath)
        {
            struct dirent *entry;
            DIR *dir = opendir(dirpath);
            if (!dir) {
                ESP_LOGE(TAG, "Failed to stat dir : %s", dirpath);
                httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Directory does not exist");
                return ESP_FAIL;
            }
            httpd_resp_set_type(req, "application/json");
            httpd_resp_sendstr_chunk(req, "{'files':[");
            while ((entry = readdir(dir)) != nullptr) {
                if(entry->d_type == DT_DIR) continue;
                httpd_resp_sendstr_chunk(req, "'");
                httpd_resp_sendstr_chunk(req, entry->d_name);
                httpd_resp_sendstr_chunk(req, "',");
            }
            closedir(dir);
            dir = opendir(dirpath);
            
            httpd_resp_sendstr_chunk(req, "], 'dirs':[");
            while ((entry = readdir(dir)) != nullptr) {
                if(entry->d_type != DT_DIR) continue;
                httpd_resp_sendstr_chunk(req, "'");
                httpd_resp_sendstr_chunk(req, entry->d_name);
                httpd_resp_sendstr_chunk(req, "',");
            }
            closedir(dir);
            httpd_resp_sendstr_chunk(req, "]}");
            httpd_resp_sendstr_chunk(req, NULL);
            return ESP_OK;
        }

        const char* get_path_from_uri(char *dest, const char *base_path, const char *uri, size_t destsize)
        {
            const size_t base_pathlen = strlen(base_path);
            size_t pathlen = strlen(uri);

            const char *quest = strchr(uri, '?');
            if (quest) {
                pathlen = std::min((int)pathlen, (int)(quest - uri));
            }
            const char *hash = strchr(uri, '#');
            if (hash) {
                pathlen = std::min((int)pathlen, (int)(hash - uri));
            }

            if (base_pathlen + pathlen + 1 > destsize) {
                /* Full path string won't fit into destination buffer */
                return nullptr;
            }

            /* Construct full path (base + path) */
            strcpy(dest, base_path);
            strlcpy(dest + base_pathlen, uri, pathlen + 1);

            /* Return pointer to path, skipping the base */
            return dest + base_pathlen;
        }
        

        esp_err_t handle_files_get(httpd_req_t *req)
        {
            char filepath[FILE_PATH_MAX];
            FILE *fd = nullptr;
            struct stat file_stat;

            const char *filename = get_path_from_uri(filepath, FILES_BASE_PATH, req->uri, sizeof(filepath));
            if (!filename) {
                ESP_LOGE(TAG, "Filename is too long");
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
                return ESP_FAIL;
            }

            /* If name has trailing '/', respond with directory contents */
            if (filename[strlen(filename) - 1] == '/') {
                return http_resp_dir_html(req, filepath);
            }

            if (stat(filepath, &file_stat) == -1) {
                httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File does not exist");
                return ESP_FAIL;
            }

            fd = fopen(filepath, "r");
            if (!fd) {
                ESP_LOGE(TAG, "Failed to read existing file : %s", filepath);
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
                return ESP_FAIL;
            }

            ESP_LOGI(TAG, "Sending file : %s (%ld bytes)...", filename, file_stat.st_size);

            size_t chunksize;
            do {
                /* Read file in chunks into the scratch buffer */
                chunksize = fread(http_buffer, 1, HTTP_BUFFER_SIZE, fd);

                if (chunksize > 0) {
                    if (httpd_resp_send_chunk(req, (const char*)http_buffer, chunksize) != ESP_OK) {
                        fclose(fd);
                        ESP_LOGE(TAG, "File sending failed!");
                        httpd_resp_sendstr_chunk(req, nullptr);
                        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                        return ESP_FAIL;
                    }
                }
            } while (chunksize != 0);

            fclose(fd);
            httpd_resp_send_chunk(req, nullptr, 0);
            return ESP_OK;
        }

        esp_err_t handle_files_post(httpd_req_t *req)
        {
            char filepath[FILE_PATH_MAX];
            FILE *fd = NULL;
            struct stat file_stat;

   
            const char *filename = get_path_from_uri(filepath, FILES_BASE_PATH, req->uri, sizeof(filepath));
            if (!filename) {
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
                return ESP_FAIL;
            }


            if (filename[strlen(filename) - 1] == '/') {
                ESP_LOGE(TAG, "Invalid filename : %s", filename);
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Invalid filename");
                return ESP_FAIL;
            }

            if (stat(filepath, &file_stat) == 0) {
                ESP_LOGE(TAG, "File already exists : %s", filepath);
                httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "File already exists");
                return ESP_FAIL;
            }


            if (req->content_len > MAX_FILE_SIZE) {
                ESP_LOGE(TAG, "File too large : %d bytes", req->content_len);
                httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "File too large");
                return ESP_FAIL;
            }

            fd = fopen(filepath, "w");
            if (!fd) {
                ESP_LOGE(TAG, "Failed to create file : %s", filepath);
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create file");
                return ESP_FAIL;
            }

            ESP_LOGI(TAG, "Receiving file : %s...", filename);
            size_t received;
            size_t remaining = req->content_len;

            while (remaining > 0) {

                ESP_LOGI(TAG, "Remaining size : %d", remaining);
                /* Receive the file part by part into a buffer */
                if ((received = httpd_req_recv(req, (char*)http_buffer, std::min(remaining, HTTP_BUFFER_SIZE))) <= 0) {
                    if (received == HTTPD_SOCK_ERR_TIMEOUT) continue;
                    /* In case of unrecoverable error,
                    * close and delete the unfinished file*/
                    fclose(fd);
                    unlink(filepath);
                    ESP_LOGE(TAG, "File reception failed!");
                    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive file");
                    return ESP_FAIL;
                }

                /* Write buffer content to file on storage */
                if (received && (received != fwrite(http_buffer, 1, received, fd))) {
                    /* Couldn't write everything to file!
                    * Storage may be full? */
                    fclose(fd);
                    unlink(filepath);

                    ESP_LOGE(TAG, "File write failed!");
                    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to write file to storage");
                    return ESP_FAIL;
                }
                remaining -= received;
            }


            fclose(fd);
            ESP_LOGI(TAG, "File reception complete");
            httpd_resp_sendstr(req, "File uploaded successfully");
            return ESP_OK;
        }

        esp_err_t handle_files_delete(httpd_req_t *req)
        {
            char filepath[FILE_PATH_MAX];
            struct stat file_stat;
            const char *filename = get_path_from_uri(filepath, FILES_BASE_PATH, req->uri, sizeof(filepath));
            if (!filename) {
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
                return ESP_FAIL;
            }

            /* Filename cannot have a trailing '/' */
            if (filename[strlen(filename) - 1] == '/') {
                ESP_LOGE(TAG, "Invalid filename : %s", filename);
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Invalid filename");
                return ESP_FAIL;
            }

            if (stat(filepath, &file_stat) == -1) {
                ESP_LOGE(TAG, "File does not exist : %s", filename);
                httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "File does not exist");
                return ESP_FAIL;
            }

            ESP_LOGI(TAG, "Deleting file : %s", filename);
            unlink(filepath);
            httpd_resp_sendstr(req, "File deleted successfully");
            return ESP_OK;
        }

    public:
        static M *GetSingleton()
        {
            if (!singleton)
            {
                singleton = new M();
            }
            return singleton;
        }

        WifiStationState GetStaState(){
            return this->staState;
        }

        const char* GetHostname(){
            return this->hostname;
        }
 
        esp_ip4_addr_t GetIpAddress(){
            return this->ipAddr;
        }

        const char* GetSsid(){
            return this->ssid;
        }

        esp_err_t WrapAndSendAsync(uint32_t ns, ::flatbuffers::FlatBufferBuilder &b)override
        {
            auto *a = new AsyncResponse(ns, &b);
            if(httpd_queue_work(http_server, M::ws_async_send, a)!=ESP_OK){delete(a);}
            return ESP_OK;
        }

        void RegisterHTTPDHandlers(httpd_handle_t httpd_handle)
        {
            httpd_uri_t files_get = {
                FILES_BASE_PATH, 
                HTTP_GET, 
                [](httpd_req_t *req){return static_cast<M *>(req->user_ctx)->handle_files_get(req);}, 
                this, false, false, nullptr
            };
            ESP_ERROR_CHECK(httpd_register_uri_handler(httpd_handle, &files_get));

            httpd_uri_t files_post = {
                FILES_BASE_PATH, 
                HTTP_POST, 
                [](httpd_req_t *req){return static_cast<M *>(req->user_ctx)->handle_files_post(req);}, 
                this, false, false, nullptr
            };
            ESP_ERROR_CHECK(httpd_register_uri_handler(httpd_handle, &files_post));

            httpd_uri_t files_delete = {
                FILES_BASE_PATH, 
                HTTP_DELETE, 
                [](httpd_req_t *req){return static_cast<M *>(req->user_ctx)->handle_files_delete(req);}, 
                this, false, false, nullptr
            };
            ESP_ERROR_CHECK(httpd_register_uri_handler(httpd_handle, &files_delete));
            
            httpd_uri_t ota_post = {
                "/ota", 
                HTTP_POST, 
                [](httpd_req_t *req){return static_cast<M *>(req->user_ctx)->handle_ota_post(req);}, 
                this, false, false, nullptr
            };
            ESP_ERROR_CHECK(httpd_register_uri_handler(httpd_handle, &ota_post));
            httpd_uri_t webmanager_ws = {
                "/webmanager_ws",
                HTTP_GET, 
                [](httpd_req_t *req){return static_cast<webmanager::M *>(req->user_ctx)->handle_webmanager_ws(req);}, this, true, false, nullptr
            };
            ESP_ERROR_CHECK(httpd_register_uri_handler(httpd_handle, &webmanager_ws));
            httpd_uri_t webmanager_get = {
                "/*", HTTP_GET, 
                [](httpd_req_t *req)
                {
                    httpd_resp_set_type(req, "text/html");
                    httpd_resp_set_hdr(req, "Content-Encoding", "br");
                    httpd_resp_send(req, webmanager_html_br_start, webmanager_html_br_length);
                    return ESP_OK;
                }, 
                this, false, false, nullptr
            };
            ESP_ERROR_CHECK(httpd_register_uri_handler(httpd_handle, &webmanager_get));
            this->http_server=httpd_handle;
        }

        esp_err_t Begin(const char *accessPointSsidPattern, const char *accessPointPassword, const char *hostnamePattern, bool resetStoredWifiConnection, std::vector<iWebmanagerPlugin*>* plugins, bool init_netif_and_create_event_loop = true)
        {
            ESP_LOGI(TAG, "Stating Webmanager");
            if (strlen(accessPointPassword) < 8 && AP_AUTHMODE != WIFI_AUTH_OPEN)
            {
                ESP_LOGE(TAG, "Password too short for authentication. Minimal length is 8. Exiting Webmanager");
                return ESP_FAIL;
            }

            if (webmanager_semaphore != nullptr)
            {
                ESP_LOGE(TAG, "webmanager already started. Exiting webmanager");
                return ESP_FAIL;
            }
            webmanager_semaphore = xSemaphoreCreateBinary();
            xSemaphoreGive(webmanager_semaphore);

            if (init_netif_and_create_event_loop)
            {
                ESP_ERROR_CHECK(esp_netif_init());
                ESP_ERROR_CHECK(esp_event_loop_create_default());
            }

            this->plugins=plugins;

            wifi_manager_retry_timer = xTimerCreate("retry timer", pdMS_TO_TICKS(WIFI_MANAGER_RETRY_TIMER), pdFALSE, (void *)0, [](TimerHandle_t t){webmanager::M::GetSingleton()->webmanager_timer_retry_cb(t);});
            wifi_manager_shutdown_ap_timer = xTimerCreate("shutdown_ap_timer", pdMS_TO_TICKS(WIFI_MANAGER_SHUTDOWN_AP_TIMER), pdFALSE, (void *)0, [](TimerHandle_t t){webmanager::M::GetSingleton()->webmanager_timer_shutdown_ap_cb(t);});

            // Create and check netifs
            wifi_netif_sta = esp_netif_create_default_wifi_sta();
            wifi_netif_ap = esp_netif_create_default_wifi_ap();
            assert(wifi_netif_sta);
            assert(wifi_netif_ap);

            // attach event handler for wifi & ip
            ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data){static_cast<webmanager::M *>(arg)->wifi_event_handler(event_base, event_id, event_data);}, this, nullptr));
            ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, [](void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data){static_cast<webmanager::M *>(arg)->ip_event_handler(event_base, event_id, event_data);}, this, nullptr));

            // init WIFI base
            wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
            ESP_ERROR_CHECK(esp_wifi_init(&cfg));
            ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

            // Prepare WIFI_CONFIG for sta mode
            wifi_config_sta.sta.scan_method = WIFI_FAST_SCAN;
            wifi_config_sta.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
            wifi_config_sta.sta.threshold.rssi = -127;
            wifi_config_sta.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
            wifi_config_sta.sta.pmf_cfg.capable = true;
            wifi_config_sta.sta.pmf_cfg.required = false;

            // Prepare WIFI_CONFIG for ap mode
            // make ssid unique --> append Mac Adress of wifi station to ssid
            uint8_t mac[6];
            esp_read_mac(mac, ESP_MAC_WIFI_STA);
            wifi_config_ap.ap.ssid_len = snprintf((char *)wifi_config_ap.ap.ssid, sizeof(wifi_config_ap.ap.ssid), accessPointSsidPattern, mac[3], mac[4], mac[5]);
            strcpy((char *)wifi_config_ap.ap.password, accessPointPassword);

            wifi_config_ap.ap.channel = 0;
            wifi_config_ap.ap.max_connection = 1;
            wifi_config_ap.ap.authmode = AP_AUTHMODE;
            
            asprintf(&hostname, hostnamePattern, mac[3], mac[4], mac[5]);
            ESP_ERROR_CHECK(esp_netif_set_hostname(wifi_netif_sta, hostname));
            ESP_ERROR_CHECK(esp_netif_set_hostname(wifi_netif_ap, hostname));

            ESP_ERROR_CHECK(mdns_init());
            ESP_ERROR_CHECK(mdns_hostname_set(hostname));
            const char* MDNS_INSTANCE="ESP32_MDNS_INSTANCE";
            ESP_ERROR_CHECK(mdns_instance_name_set(MDNS_INSTANCE));

            //turn wifi logging nearly off
            esp_log_level_set("wifi", ESP_LOG_WARN);

            //Turn Power Saving off
            ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

            // SNTP (simple network time protocol) client and start it, when we got an IP address (see event handler)
            esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
            esp_sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
            esp_sntp_setservername(0, "pool.ntp.org");
            esp_sntp_set_time_sync_notification_cb([](struct timeval *tv){webmanager::M::GetSingleton()->sntp_handler();});
            setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);// Germany
            tzset();


            if (resetStoredWifiConnection || read_sta_config()!=ESP_OK)
            {
                ESP_LOGI(TAG, "Forced to delete saved wifi configuration or no config found. Starting access point and do an initial scan.");
                delete_sta_config();
                ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
                ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config_ap));
                ESP_ERROR_CHECK(esp_wifi_start());
                ESP_ERROR_CHECK(esp_wifi_scan_start(nullptr/*for default config*/, false));
                staState = WifiStationState::NO_CONNECTION;
                scanIsActive=true;
                initialScanIsActive=true;
            }
            else
            {
                remainingAttempsToConnectAsSTA=ATTEMPTS_TO_RECONNECT_ON_STARTUP_BEFORE_OPENING_AN_ACCESS_POINT;
                ESP_LOGI(TAG, "Saved wifi found on startup. Will attempt %lu times to connect to ssid %s with password %s.",remainingAttempsToConnectAsSTA, wifi_config_sta.sta.ssid, wifi_config_sta.sta.password);
                //auf keinen Fall einen AccessPoint aufmachen
                ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
                ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config_sta));
                ESP_ERROR_CHECK(esp_wifi_start());
                ESP_ERROR_CHECK(esp_wifi_connect());
                staState = WifiStationState::ABOUT_TO_CONNECT;
                scanIsActive=false;
                initialScanIsActive=false;
            }
            ESP_LOGI(TAG, "Webmanager has been succcessfully initialized");
            return ESP_OK;
        }

        esp_err_t CallMeAfterInitializationToMarkCurrentPartitionAsValid()
        {
            /* Mark current app as valid */
             ESP_LOGI(TAG, "Webmanager marks current Partition as valid");
            const esp_partition_t *partition = esp_ota_get_running_partition();
            esp_ota_img_states_t ota_state;
            if (esp_ota_get_state_partition(partition, &ota_state) == ESP_OK)
            {
                if (ota_state == ESP_OTA_IMG_PENDING_VERIFY)
                {
                    esp_ota_mark_app_valid_cancel_rollback();
                }
            }
            return ESP_OK;
        }
    };

}
#undef TAG
