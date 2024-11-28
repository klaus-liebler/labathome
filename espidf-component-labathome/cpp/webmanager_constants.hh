#pragma once
#include <inttypes.h>
#include <cstring>
#include <ctime>
#include <esp_wifi.h>

namespace webmanager{
    extern const char webmanager_html_br_start[] asm("_binary_index_compressed_br_start");
    extern const size_t webmanager_html_br_length asm("index_compressed_br_length");

    constexpr size_t MAX_AP_NUM = 8;
    constexpr size_t STORAGE_LENGTH{16};
    constexpr int ATTEMPTS_TO_RECONNECT{5};
    constexpr time_t WIFI_MANAGER_RETRY_TIMER = 8000;
    constexpr time_t WIFI_MANAGER_SHUTDOWN_AP_TIMER = 60000;
    constexpr wifi_auth_mode_t AP_AUTHMODE{wifi_auth_mode_t::WIFI_AUTH_WPA2_PSK};
    constexpr char NVS_PARTITION[]{"nvs"};
    constexpr char NVS_NAMESPACE[]{"webmananger"};
    constexpr char nvs_key_wifi_ssid[]{"ssid"};
    constexpr char nvs_key_wifi_password[]{"password"};
}