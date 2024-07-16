#pragma once
#include <common.hh>
#include "devicemanager.hh"
#include <esp_log.h>

#define TAG "BORIS"

namespace winfactboris{
    constexpr uint16_t PORT = 1600;

    static void udp_server_task(void *pvParameters)
    {
        DeviceManager* devicemanager = (DeviceManager*)pvParameters;
        char addr_str[128];
        struct sockaddr_in dest_addr;

        uint8_t rx_buffer[128];
        uint8_t tx_buffer[128];

        while (1)
        {
            dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
            dest_addr.sin_family = AF_INET;
            dest_addr.sin_port = htons(PORT);

            int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
            if (sock < 0)
            {
                ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
                break;
            }
            ESP_LOGI(TAG, "Socket created");

            int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
            if (err < 0)
            {
                ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
            }
            ESP_LOGI(TAG, "Socket bound, port %d", PORT);

            struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
            socklen_t socklen = sizeof(source_addr);

            while (1)
            {
                ESP_LOGD(TAG, "Waiting for data");

                int rx_len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

                // Error occurred during receiving
                if (rx_len < 0)
                {
                    ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                    break;
                }

                // Get the sender's ip address as string
                if (source_addr.ss_family == PF_INET)
                {
                    inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
                }
                else if (source_addr.ss_family == PF_INET6)
                {
                    inet6_ntoa_r(((struct sockaddr_in6 *)&source_addr)->sin6_addr, addr_str, sizeof(addr_str) - 1);
                }

                rx_buffer[rx_len] = 0; // Null-terminate whatever we received and treat like a string...
                ESP_LOGD(TAG, "Received %d bytes from %s: %s", rx_len, addr_str, rx_buffer);
                size_t tx_buf_len = sizeof(tx_buffer);
                devicemanager->TriggerBorisUDP(rx_buffer, rx_len, tx_buffer, tx_buf_len);
    
                if(tx_buf_len!=0){
                    int err = sendto(sock, tx_buffer, tx_buf_len, 0, (struct sockaddr *)&source_addr, sizeof(source_addr));
                    ESP_LOGD(TAG, "Sending %d bytes to %s: %s", tx_buf_len, addr_str, tx_buffer);
                    tx_buffer[0]='\0';
                    if (err < 0) {
                        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                        break;
                    }
                }
                
            }

            if (sock != -1)
            {
                ESP_LOGE(TAG, "Shutting down socket and restarting...");
                shutdown(sock, 0);
                close(sock);
            }
        }
        vTaskDelete(NULL);
    }

    void InitAndRun(DeviceManager* devicemanager)
    {
        xTaskCreate(udp_server_task, "udp_server", 4096, (void*)devicemanager, 5, nullptr);
    }

}
#undef TAG