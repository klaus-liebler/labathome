Webmanager ist NUR für die Basiskommunikation und die Inbetriebnahme des Wifi zuständig
- stellt HTTPS-Server zur Verfügung
- Endpoint für die SPA-HTML-Applikation
- Endpoint für OTA und Umsetzung für OTA
- Endpoint für Websockets
- SNTP
-Verfügt über ein Callback-Interface, u.a. um die Wifi-Parameter zu ändern oder sich für einen Nachrichten-Namespace zu Registrieren

 Hat eine Plugin-Möglichkeit mit den Methoden OnBegin, OnWifiConnect und OnWifiDisconnect
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

#define TAG "WMAN"
#define WLOG(mc, md) webmanager::M::GetSingleton()->Log(mc, md)
namespace webmanager
{
    class M : public MessageSender
    {
    private:
        static M *singleton;

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
        UserSettings* userSettings{nullptr};
        
        WifiStationState staState{WifiStationState::NO_CONNECTION};
        // bool accessPointIsActive{false}; // nein, diese Information kann über esp_wifi_get_mode() immer herausgefunden werden
        int remainingAttempsToConnectAsSTA{ATTEMPTS_TO_RECONNECT};
        bool initialScanIsActive{false};
        bool scanIsActive{false};
        std::vector<iMessageReceiver*> *plugins{nullptr};

        

        M(){}

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
            if ((ret == ESP_OK && strcmp((char *)tmp_password, (char *)wifi_config_sta.sta.password) != 0) || ret == ESP_ERR_NVS_NOT_FOUND))
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

        //Logik: Selbständig niemals die WifiCreentials löschen. Immer versuchen, weiter auf die vom Nutzer eingestellte SSID zuzugreifen
        //Wenn nach dem Start (RETRIES_ON_STARTUP) Verbindungsversuche fehlschlagen, dann den AP einschalten. Zahl von 0 deaktiviert dieses Verhalten. Aber stets weiter versuchen
        //Wenn im laufenden Betrieb die Verbindungsversuche fehlschlagen, einfach weiter machen (aka sobald einmal erfolgreich verbunden wurde, wird die Zahl der verbleibenden Versuche bis zum Einschalten des AP auf UINT32_MAX gestellt)

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
                    ESP_LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED, when STA_STATE::CONNECTED --> unexpected disconnection. Try to reconnect %d times.", remainingAttempsToConnectAsSTA);
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
                        auto res = CreateResponseWifiConnectFailedDirect(b, (char*)wifi_config_sta.sta.ssid);
                        WrapAndFinishAndSendAsync(b, Responses::Responses_ResponseWifiConnectFailed, res.Union());
                    }
                    else
                    {
                        ESP_LOGD(TAG, "WIFI_EVENT_STA_DISCONNECTED, when STA_STATE::ABOUT_TO_CONNECT --> disconnection occured earlier and we tried to establish it again...which was not successful (Reason %d). Still %d attempt(s) to go.", wifi_event_sta_disconnected->reason, remainingAttempsToConnectAsSTA);
                        ESP_LOGI(TAG, "Re-establishing connection failed (Reason %d). Still %d attempt(s) to go.", wifi_event_sta_disconnected->reason, remainingAttempsToConnectAsSTA);
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
                remainingAttempsToConnectAsSTA = INT_MAX;
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
                    auto res =  CreateResponseWifiConnectSuccessfulDirect(b, (char*)wifi_config_sta.sta.ssid, event->ip_info.ip.addr, event->ip_info.netmask.addr, event->ip_info.gw.addr);
                    WrapAndFinishAndSendAsync(b, Responses::Responses_ResponseWifiConnectSuccessful, res.Union());
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
                ESP_LOGI(TAG, "IP_EVENT_STA_LOST_IP");
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
            LogJournal(messagecodes::C::SNTP, esp_timer_get_time() / 1000);
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
            auto mw = flatbuffers::GetRoot<RequestWrapper>(buf);
            webmanager::Requests reqType=mw->request_type();
            ESP_LOGI(TAG, "Received websocket frame: len=%d, requestType=%d ", (int)ws_pkt.len, reqType);
            switch (reqType)
            {
            case webmanager::Requests::Requests_RequestSystemData:
                sendResponseSystemData(req, &ws_pkt);
                break;
            case webmanager::Requests::Requests_RequestJournal:
                sendResponseJournal(req, &ws_pkt);
                break;
            case webmanager::Requests::Requests_RequestNetworkInformation:
                sendResponseNetworkInformation(req, &ws_pkt, mw->request_as_RequestNetworkInformation());
                break;
            case webmanager::Requests::Requests_RequestRestart:
                esp_restart();
                break;
            case webmanager::Requests::Requests_RequestWifiConnect:
                handleRequestWifiConnect(req, &ws_pkt, mw->request_as_RequestWifiConnect());
                break;
            case webmanager::Requests::Requests_RequestWifiDisconnect:
                handleRequestWifiDisconnect(req, &ws_pkt, mw->request_as_RequestWifiDisconnect());
                break;
            case webmanager::Requests::Requests_RequestGetUserSettings:
                this->userSettings->handleRequestGetUserSettings(req, &ws_pkt, mw->request_as_RequestGetUserSettings());
                break;
            case webmanager::Requests::Requests_RequestSetUserSettings:
                this->userSettings->handleRequestSetUserSettings(req, &ws_pkt, mw->request_as_RequestSetUserSettings());
                break;
            default:{
                eMessageReceiverResult success{eMessageReceiverResult::NOT_FOR_ME};
                if(plugins){
                    for(auto mr:*plugins){
                        success=mr->provideWebsocketMessage(this, req, &ws_pkt, mw);
                        if(success!=eMessageReceiverResult::NOT_FOR_ME){
                            break;
                        }
                    }
                }
                if(success==eMessageReceiverResult::NOT_FOR_ME){
                    ESP_LOGW(TAG, "Not yet implemented request %d, neither internal nor in a plugin", (int)mw->request_type());
                }else if(success==eMessageReceiverResult::FOR_ME_BUT_FAILED){
                    ESP_LOGW(TAG, "Request %d has been implemented by plugin, but processing failed", (int)mw->request_type());
                }
            }
                
                break;
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
            auto res =  CreateResponseWifiConnectFailedDirect(b, (char*)wifi_config_sta.sta.ssid);
            return WrapAndFinishAndSendAsync(b, Responses::Responses_ResponseWifiConnectFailed, res.Union());
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
                int recv_len = httpd_req_recv(req, buf, std::min(remaining, sizeof(buf)));
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


        
        esp_err_t WrapAndFinishAndSendAsync(::flatbuffers::FlatBufferBuilder &b, webmanager::Responses message_type = webmanager::Responses_NONE, ::flatbuffers::Offset<void> message = 0){
            b.Finish(CreateResponseWrapper(b, message_type, message));
            auto *a = new AsyncResponse(&b);
            if(httpd_queue_work(http_server, M::ws_async_send, a)!=ESP_OK){delete(a);}
             return ESP_OK;
        }



        void SetPlugins(std::vector<iMessageReceiver*> *plugins){
            this->plugins=plugins;
        }
        
        void RegisterHTTPDHandlers(httpd_handle_t httpd_handle, const char* webmanager_uri_prefix="/*")
        {
            httpd_uri_t ota_post = {"/ota", HTTP_POST, [](httpd_req_t *req){return static_cast<M *>(req->user_ctx)->handle_ota_post(req);}, this, false, false, nullptr};
            ESP_ERROR_CHECK(httpd_register_uri_handler(httpd_handle, &ota_post));
            httpd_uri_t webmanager_ws = {"/webmanager_ws", HTTP_GET, [](httpd_req_t *req){return static_cast<webmanager::M *>(req->user_ctx)->handle_webmanager_ws(req);}, this, true, false, nullptr};
            ESP_ERROR_CHECK(httpd_register_uri_handler(httpd_handle, &webmanager_ws));
            httpd_uri_t webmanager_get = {
                webmanager_uri_prefix, HTTP_GET, 
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

        esp_err_t Begin(const char *accessPointSsidPattern, const char *accessPointPassword, const char *hostnamePattern, bool resetStoredWifiConnection, bool init_netif_and_create_event_loop = true)
        {
            if (strlen(accessPointPassword) < 8 && AP_AUTHMODE != WIFI_AUTH_OPEN)
            {
                ESP_LOGE(TAG, "Password too short for authentication. Minimal length is 8.");
                return ESP_FAIL;
            }

            if (webmanager_semaphore != nullptr)
            {
                ESP_LOGE(TAG, "webmanager already started");
                return ESP_FAIL;
            }
            webmanager_semaphore = xSemaphoreCreateBinary();
            xSemaphoreGive(webmanager_semaphore);

            if (init_netif_and_create_event_loop)
            {
                ESP_ERROR_CHECK(esp_netif_init());
                ESP_ERROR_CHECK(esp_event_loop_create_default());
            }

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
            const char* MDNS_INSTANCE="SENSACT_MDNS_INSTANCE";
            ESP_ERROR_CHECK(mdns_instance_name_set(MDNS_INSTANCE));

            //turn wifi logging nearly off
            esp_log_level_set("wifi", ESP_LOG_WARN);

            // prepare simple network time protocol client and start it, when we got an IP-Adress (see event handler)
            esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
            esp_sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
            esp_sntp_setservername(0, "pool.ntp.org");
            esp_sntp_set_time_sync_notification_cb([](struct timeval *tv){webmanager::M::GetSingleton()->sntp_handler();});
            // Set timezone to Berlin
            setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
            tzset();


            ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

            if (resetStoredWifiConnection)
            {
                ESP_LOGI(TAG, "Forced to delete saved wifi configuration. Starting access point and do an initial scan.");
                delete_sta_config();
                ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
                ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config_ap));
                ESP_ERROR_CHECK(esp_wifi_start());
                ESP_ERROR_CHECK(esp_wifi_scan_start(nullptr/*for default config*/, false));
                staState = WifiStationState::NO_CONNECTION;
                scanIsActive=true;
                initialScanIsActive=true;
            }
            else if (read_sta_config() != ESP_OK)
            {
                ESP_LOGI(TAG, "No saved wifi configuration found on startup. Starting access point and do an initial scan.");
                
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
                ESP_LOGI(TAG, "Saved wifi found on startup. Will attempt to connect to ssid %s with password %s.", wifi_config_sta.sta.ssid, wifi_config_sta.sta.password);
   
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
