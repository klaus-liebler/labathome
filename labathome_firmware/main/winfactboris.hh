#include <common.hh>
#include <HAL.hh>
#include <esp_log.h>

#define TAG "BORIS"
/**
 * @brief 
 OnBoard-Datenpunkte!
 Typen sind bool (1byte), float (4byte), integer (4byte), enum (4byte), color (4byte, RGBA) 
 Bei JEDER Nachricht werden ALLE Datenpunkte in der o.g. Reihenfolge übertragen, als Prefix wird eine 4byte MessageType mitgesendet. Durch diesen wird klar, welches Format die nachfolgende Nachricht hat. Wir beginnen mit dem MessageType 42.
 Soll ein Wert nicht aktiv gesetzt werden, dann wird als Wert 0xFF für bool, QuietNan für float, INT_MAX_VALUE für integer, INT_MAX_VALUE für enum und INT_MAX_VALUE für color gesendet
 
 Configuration
 Heater vs LED
 RelayInput vs RotaryEncoder
 Fan1_Sense vs MovementSensor
 * 
 */

namespace winfactboris
{
    constexpr uint16_t PORT = 1600;

    HAL* hal;
    
    // Global Variables


    char rx_buffer[128];
    char tx_buffer[128];
    char currentPinMode[32];
    uint16_t currentPinState[32];
    int pwmPin{-1};

    uint8_t parseTwoDigits(char* ptr){
        return 10*(ptr[0] - 48) + (ptr[1]-48);
    }

    // Hauptprogramm
    void parseCommand()
    {
        uint16_t pin{0};
        uint16_t value{0};
        //float f = std::numeric_limits<float>::quiet_NaN();
        switch (rx_buffer[0])
        {
        case 'w':
            if (pwmPin<0) break;
            value = atoi(rx_buffer+1); //0...1023
            currentPinState[pwmPin] = value;
            ESP_LOGI(TAG, "Set PWM Output %i to %i", pwmPin, value);
            sprintf(tx_buffer, "Set PWM Output");
            pwmPin = -1;
            break;
        case 'p':
            if(rx_buffer[1]!= 'w' || rx_buffer[2]!='m') break;
            pwmPin = parseTwoDigits(rx_buffer+3);
            ESP_LOGI(TAG, "Switched to PWM Mode for pin %i", pwmPin);
            sprintf(tx_buffer, "Switched to PWM Mode");
            break;
        case '!': 
            pin = parseTwoDigits(rx_buffer+1);
            if(rx_buffer[3]=='s'){
                if (rx_buffer[4] == '0')
                {
                    currentPinState[pin] = 0;
                    ESP_LOGI(TAG, "Set Pin %i to LOW", pin);
                    hal->ColorizeLed(LED::LED_YELLOW, CRGB::Black);
                    sprintf(tx_buffer, "Set Pin to LOW");
                }
                else
                {
                    currentPinState[pin] = 1;
                    ESP_LOGI(TAG, "Set Pin %i to HIGH", pin);
                    hal->ColorizeLed(LED::LED_YELLOW, CRGB::Yellow);
                    sprintf(tx_buffer, "Set Pin to HIGH");
                }
            }
            // Modus setzen
            else if (rx_buffer[3] == 'm')
            {
                if (rx_buffer[4] == 'o')
                {
                    currentPinMode[pin] = 'O';
                    ESP_LOGI(TAG, "Set Pin %i to OUTPUT", pin);
                    sprintf(tx_buffer, "Set Pin to OUTPUT");
                }
                else
                {
                    currentPinMode[pin] = 'I';
                    ESP_LOGI(TAG, "Set Pin %i to INPUT", pin);
                    sprintf(tx_buffer, "Set Pin to INPUT");
                }
            }
            break;
        case '?':
            pin = parseTwoDigits(rx_buffer+1);
            if (rx_buffer[3] == 'm')
            {
                if (currentPinMode[pin] == 'O'){
                    ESP_LOGI(TAG, "Get Pin Mode %i as OUTPUT", pin);
                    sprintf(tx_buffer, "O");
                }
                else if (currentPinMode[pin] == 'I'){
                    ESP_LOGI(TAG, "Get Pin Mode %i as INPUT", pin);
                    sprintf(tx_buffer, "I");
                }
            }
            else if (rx_buffer[3] == 's' && currentPinMode[pin] == 'O')
            {
                ESP_LOGI(TAG, "Get PWM/Binary Output Pin State %d as %d", pin, currentPinState[pin]);
                sprintf(tx_buffer, "%d", currentPinState[pin]);
            }
            else if (rx_buffer[3] == 's' && currentPinMode[pin] == 'I')
            {
                //(answer is "LOW" or "HIGH" für pin 1-13, 0 ... 1023 für analog inputs 14-19)
                if (pin < 14)
                {
                    bool state = hal->GetButtonGreenIsPressed();
                    ESP_LOGI(TAG, "Get Binary Input Pin State %d as %d", pin, state);
                    sprintf(tx_buffer, "%s", state?"HIGH":"LOW");
                }
                if (pin >= 14)
                {
                    float degreesCelcius;
                    hal->GetAirTemperature(&degreesCelcius);
                    ESP_LOGI(TAG, "Get Analog Input Pin State %d as %d", pin, (int)degreesCelcius);
                    itoa((int)degreesCelcius, tx_buffer, 10);
                }
            }
            break;
        default:
            break;
        }
    }

    static void udp_server_task(void *pvParameters)
    {

        char addr_str[128];
        struct sockaddr_in dest_addr;

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
                ESP_LOGI(TAG, "Waiting for data");

                int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

                // Error occurred during receiving
                if (len < 0)
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

                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string...
                ESP_LOGI(TAG, "Received %d bytes from %s: %s", len, addr_str, rx_buffer);

                parseCommand();
                size_t tx_buf_len = strlen(tx_buffer);
                if(tx_buf_len!=0){
                    int err = sendto(sock, tx_buffer, tx_buf_len, 0, (struct sockaddr *)&source_addr, sizeof(source_addr));
                    ESP_LOGI(TAG, "Sending %d bytes to %s: %s", tx_buf_len, addr_str, tx_buffer);
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

    void InitAndRun(HAL* halref)
    {
        hal=halref;
        xTaskCreate(udp_server_task, "udp_server", 4096, nullptr, 5, nullptr);
    }

}
#undef TAG