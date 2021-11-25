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
#define TAG "NET"

esp_netif_t * wifi_netif = NULL;
esp_netif_t * eth_netif = NULL;
esp_ip4_addr_t s_ip_addr;
SemaphoreHandle_t connectSemaphore = NULL;

void on_wifi_start(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "Wi-Fi started, trying to connect...");
    esp_err_t err = esp_wifi_connect();
    if (err == ESP_ERR_WIFI_NOT_STARTED) {
        return;
    }
    ESP_ERROR_CHECK(err);
}

void on_wifi_disconnect(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "Wi-Fi disconnected, trying to reconnect...");
    esp_err_t err = esp_wifi_connect();
    if (err == ESP_ERR_WIFI_NOT_STARTED) {
        return;
    }
    ESP_ERROR_CHECK(err);
}

void on_wifi_got_ip(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "WIFI got IPv4 event: Interface \"%s\" address: " IPSTR, esp_netif_get_desc(event->esp_netif), IP2STR(&event->ip_info.ip));
    memcpy(&s_ip_addr, &event->ip_info.ip, sizeof(s_ip_addr));
    xSemaphoreGive(connectSemaphore);
}

static void on_wifi_any_ap_event(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

static void on_eth_got_ip(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    //const esp_netif_ip_info_t *ip_info = &event->ip_info;
    ESP_LOGI(TAG, "ETH Got IPv4 event: Interface \"%s\" address: " IPSTR, esp_netif_get_desc(event->esp_netif), IP2STR(&event->ip_info.ip));
}

void on_eth_any_event(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    uint8_t mac_addr[6] = {0};
    /* we can get the ethernet driver handle from event data */
    esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

    switch (event_id) {
    case ETHERNET_EVENT_CONNECTED:
        esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
        ESP_LOGI(TAG, "Ethernet Link Up");
        ESP_LOGI(TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x",
                 mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        break;
    case ETHERNET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Ethernet Link Down");
        break;
    case ETHERNET_EVENT_START:
        ESP_LOGI(TAG, "Ethernet Started");
        break;
    case ETHERNET_EVENT_STOP:
        ESP_LOGI(TAG, "Ethernet Stopped");
        break;
    default:
        break;
    }
}

void startAP(){
    ESP_ERROR_CHECK(esp_netif_init()); //s1.1
    ESP_ERROR_CHECK(esp_event_loop_create_default()); //s1.2
    
    esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();
    assert(ap_netif);

    esp_netif_set_hostname(ap_netif, "labathome");
    esp_netif_ip_info_t ip_info;

    //NOTE: This is where you set the access point (AP) IP address
    //     and gateway address. It has to be a class A internet address
    //    otherwise the captive portal sign-in prompt won't show up	on
    //    Android when you connect to the access point. 
    IP4_ADDR(&ip_info.ip, 192, 168, 210, 0);
    IP4_ADDR(&ip_info.gw, 192, 168, 210, 0);
    IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
    esp_netif_dhcps_stop(ap_netif);
    esp_netif_set_ip_info(ap_netif, &ip_info);
    esp_netif_dhcps_start(ap_netif);
    

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &on_wifi_any_ap_event, NULL, NULL));

    wifi_config_t wifi_config = {};
    strcpy((char *) wifi_config.ap.ssid, CONFIG_NETWORK_WIFI_AP_SSID);
    strcpy((char *) wifi_config.ap.password, CONFIG_NETWORK_WIFI_AP_PASSWORD);
    wifi_config.ap.ssid_len = strlen(CONFIG_NETWORK_WIFI_AP_SSID);
    wifi_config.ap.channel = CONFIG_NETWORK_WIFI_AP_CHANNEL;
    wifi_config.ap.max_connection = CONFIG_NETWORK_AP_MAX_AP_CONN;
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;  
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}


void connectSTA2AP(bool eth){
    connectSemaphore = xSemaphoreCreateBinary();
    ESP_ERROR_CHECK(esp_netif_init()); //s1.1
    ESP_ERROR_CHECK(esp_event_loop_create_default()); //s1.2
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_START, &on_wifi_start, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_wifi_got_ip, NULL));
    wifi_netif = esp_netif_create_default_wifi_sta();//s1.3
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg)); //s1.4
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
    wifi_config_t wifi_config = {};
    strcpy((char *) wifi_config.sta.ssid, CONFIG_NETWORK_WIFI_SSID);
    strcpy((char *) wifi_config.sta.password, CONFIG_NETWORK_WIFI_PASSWORD);
    wifi_config.sta.scan_method = WIFI_FAST_SCAN;
    wifi_config.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
    wifi_config.sta.threshold.rssi=-127;
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable=true;
    wifi_config.sta.pmf_cfg.required=false;

    ESP_LOGI(TAG, "Connecting to %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    char hostname[32];
    sprintf(hostname, CONFIG_NETWORK_HOSTNAME_PATTERN, mac[3], mac[4], mac[5]);
    ESP_ERROR_CHECK(tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, hostname));
    ESP_ERROR_CHECK(esp_wifi_start());
    if(eth){
        esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
        eth_netif = esp_netif_new(&cfg);
        assert(eth_netif);
        ESP_ERROR_CHECK(esp_eth_set_default_handlers(eth_netif));
        ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &on_eth_any_event, NULL));
        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &on_eth_got_ip, NULL));
        eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
        mac_config.smi_mdc_gpio_num = 18;
        mac_config.smi_mdio_gpio_num = 5;
        eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
        phy_config.phy_addr = 0;
        phy_config.reset_gpio_num = 17;
        
        esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&mac_config);
        esp_eth_phy_t *phy = esp_eth_phy_new_lan8720(&phy_config);
        esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
        esp_eth_handle_t eth_handle = NULL;
        ESP_ERROR_CHECK(esp_eth_driver_install(&config, &eth_handle));
        /* attach Ethernet driver to TCP/IP stack */
        ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));
        /* start Ethernet driver state machine */
        ESP_ERROR_CHECK(esp_eth_start(eth_handle));
    }
    
}
#undef TAG