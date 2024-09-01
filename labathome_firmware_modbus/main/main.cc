constexpr int FIRMWARE_VERSION{5};
// Hints for the command line test software "modpoll", that I used during development
//In this tool, the register addresses ("r") is 1-based
// COMxy has to be replaced with your specific com port.

//Query Inputs:      modpoll -r 1 -c 9 -t 1 -b 9600 COM17
//Set Coil Outputs:  modpoll -r 1 -c 1 -t 0 -b 9600 COM17 0
//Set LED Color      modpoll -r 10 -c 1 -t 4 -b 9600 COM17 65535
//Set Servo          modpoll -r 2 -c 1 -t 4 -b 9600 COM7 50

/*
These are the addresses and the formats at LabAtHome
Adresses are 0-Based
Coils:
 0: Relay K3

Discrete Input:
 0: Green Button
 1: Red Button
 2: Yellow Button / Encoder Button
 3: Movement Sensor

Holding Registers:
 0: Not connected
 1: Servo 0, Position in Degrees 0...180
 2: Servo 1, Position in Degrees 0...180
 3: Servo 2, Position in Degrees 0...180
 4: Servo 3, Position in Degrees 0...180
 5: Fan 0, Power in Percent 0...100
 6: Fan 1, Power in Percent 0...100
 7: Heater, Power in Percent 0...100
 8: White Power LED, Power in Percent 0...100
 9: RGB LED 0, Color in RGB565
 10: LED 1
 11: LED 2
 12: LED 3
 13: Relay State (Alternative to Coil 0), 0 means off, all other values on
 14: Play Sound, 0 means silence; try other values up to 9

Input Registers:
 0: CO2 [PPM]
 1: Air Pressure [hPa]
 2: Ambient Brightness [?]
 3: Analog Input [mV] //CHANNEL_ANALOGIN_OR_ROTB I34
 4: Button Green [0 or 1]
 5: Button Red [0 or 1]
 6: Button Yellow/Encoder [0 or 1]
 7: Fan 1 RpM
 8: Heater Temperature [°C * 100] (Temperatur des "Dreibeiners")
 9: Encoder Detents
10: Movement Sensor [0 or 1]
11: Distance Sensor [millimeters]
12: Analog Voltage on Pin 35 //CHANNEL_MOVEMENT_OR_FAN1SENSE I35
13: placeholder
14: placeholder
15: Relative Humidity AHT21 [%]
16: Temperature AHT21 [°C * 100]
17: Relative Humidity BME280 [%]
18: Temperature BME280 [°C * 100]
100: Firmware Version
                
*/

#include <cstdio>
#include <cstdint>
#include <cstring>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "sdkconfig.h"
#include "esp_timer.h"
#include "modbusslave.hh"
#include <vector>
#include <driver/uart.h>
#include <common.hh>
#include <array>
#include <nvs_flash.h>
#include <spiffs.hh>
#include <wifi_sta.hh>


static const char *TAG = "main";

#include "HAL.hh"
#if (CONFIG_IDF_TARGET_ESP32)
    #include "HAL_labathomeV10.hh"
    static HAL * hal = new HAL_Impl(MODE_MOVEMENT_OR_FAN1SENSE::MOVEMENT_SENSOR);
#elif(CONFIG_IDF_TARGET_ESP32S3)
#include "HAL_labathomeV15.hh"
static HAL * hal = new HAL_Impl();
#endif

constexpr TickType_t xFrequency {pdMS_TO_TICKS(50)};

modbus::M<100000> *modbusSlave;

constexpr size_t COILS_CNT{16};
constexpr size_t DISCRETE_INPUTS_CNT{16};
constexpr size_t INPUT_REGISTERS_CNT{101};
constexpr size_t HOLDING_REGISTERS_CNT{16};

static std::vector<bool> coilData(COILS_CNT);
static std::vector<bool> discreteInputsData(DISCRETE_INPUTS_CNT);
static std::vector<uint16_t> inputRegisterData(INPUT_REGISTERS_CNT);
static std::vector<uint16_t> holdingRegisterData(HOLDING_REGISTERS_CNT);
#define NVS_PARTITION_NAME NVS_DEFAULT_PART_NAME


void modbusAfterWriteCallback(uint8_t fc, uint16_t start, size_t len){
    
    if(fc==15 || fc==5){
        bool b=coilData.at(start);
        ESP_LOGI(TAG, "Modbus Coil Registers changed! fc:%d, start:%d len:%d, firstChangedBit=%d", fc, start, len, b);
        for(int i=start;i<start+len;i++){
            switch (i)
            {
            case 0:
                hal->SetRelayState(coilData.at(0));
                break;
            default:
                break;
            }
        }
    }
    else if(fc==16 || fc==6){
        ESP_LOGI(TAG, "Modbus Output Registers changed! fc:%d, start:%d len:%d, firstChangesValues=%d", fc, start, len, holdingRegisterData.at(start));
        
        for(int reg=start;reg<start+len;reg++){
            switch (reg)
            {
            case 0:
                break;
            case 1:
                hal->SetServoPosition(0, holdingRegisterData.at(reg));
                break;
            case 2:
                hal->SetServoPosition(1, holdingRegisterData.at(reg));
                break;
            case 3:
                hal->SetServoPosition(2, holdingRegisterData.at(reg));
                break;
            case 4:
                hal->SetServoPosition(3, holdingRegisterData.at(reg));
                break;
            case 5:
                hal->SetFanDuty(0, holdingRegisterData.at(reg));
                break;
            case 6:
                hal->SetFanDuty(1, holdingRegisterData.at(reg));
                break;
            case 7:
                hal->SetHeaterDuty(holdingRegisterData.at(reg));
                break;
            case 8://PowerWhite
                hal->SetLedPowerWhiteDuty(holdingRegisterData.at(reg));
                break;
            case 9://RGB LED 0, this number has to be inserted in call in "case 12"
            case 10:
            case 11:
            case 12: //RGB LED 3
                hal->ColorizeLed(reg-9, CRGB::FromRGB565(holdingRegisterData.at(reg)));
                break;
            case 13:
                hal->SetRelayState(holdingRegisterData.at(reg));
                break;
            case 14:
                hal->SetSound(holdingRegisterData.at(reg));
                break;
            default:
                break;
            }
        }
    }
}

void modbusBeforeReadCallback(uint8_t fc, uint16_t start, size_t len){
    ESP_LOGI(TAG, "Modbus Register Read! fc:%d, start:%d len:%d.", fc, start, len);
    if(fc==2){ // Discrete Inputs --> Green, Red, Yellow, Movement
        for(int reg=start;reg<start+len;reg++){
            switch (reg)
            {
            case 0:
                discreteInputsData[reg]= hal->GetButtonGreenIsPressed();
                break;
            case 1:
                discreteInputsData[reg]= hal->GetButtonRedIsPressed();
                break;
            case 2:
                discreteInputsData[reg]= hal->GetButtonEncoderIsPressed();
                break;
            case 3:
                discreteInputsData[reg]= hal->IsMovementDetected();
                break;
            default:
                break;
            }
        }
    }
    else if(fc==4){//Input Registers
        float tmpVal_F{0.0f};
        float* tmpPtr_F{nullptr};
        int tmpVal_I32{0};
        uint16_t tmpVal_U16{0};

        for(int reg=start;reg<start+len;reg++){
            switch (reg)
            {
            case 0:
                hal->GetCO2PPM(&tmpVal_F);
                inputRegisterData[reg]=tmpVal_F;
                break;
            case 1:
                hal->GetAirPressure(&tmpVal_F);
                inputRegisterData[reg]=tmpVal_F;
                break;
            case 2:
                hal->GetAmbientBrightness(&tmpVal_F);
                inputRegisterData[reg]=tmpVal_F;
                break;
            case 3:
                hal->GetAnalogInputs(&tmpPtr_F);
                inputRegisterData[reg]=tmpPtr_F[0]; //CHANNEL_ANALOGIN_OR_ROTB, Pin 34
                break;
            case 4:
                inputRegisterData[reg]=hal->GetButtonGreenIsPressed();
                break;
            case 5:
                inputRegisterData[reg]=hal->GetButtonRedIsPressed();
                break;
            case 6:
                inputRegisterData[reg]=hal->GetButtonEncoderIsPressed();
                break;
            case 7:
                //hal->GetFan1Rpm(&tmpVal_F);
                inputRegisterData[reg]=0;
                break;
            case 8:
                hal->GetHeaterTemperature(&tmpVal_F);
                inputRegisterData[reg]=tmpVal_F*100;
                break;
            case 9:
                hal->GetEncoderValue(&tmpVal_I32);
                inputRegisterData[reg]=tmpVal_I32;
                break;
            case 10:
                inputRegisterData[reg]=hal->IsMovementDetected();
                break;
            case 11:
                hal->GetDistanceMillimeters(&tmpVal_U16);
                inputRegisterData[reg]=tmpVal_U16;
                break;
            case 12:
                hal->GetAnalogInputs(&tmpPtr_F);
                inputRegisterData[reg]=tmpPtr_F[1]; //CHANNEL_MOVEMENT_OR_FAN1SENSE I35
                break;
            case 13:
                hal->GetAnalogInputs(&tmpPtr_F);
                inputRegisterData[reg]=tmpPtr_F[2];
                break;
            case 14:
                hal->GetAnalogInputs(&tmpPtr_F);
                inputRegisterData[reg]=tmpPtr_F[3];
                break;
            case 15:
                hal->GetAirRelHumidityAHT21(&tmpVal_F);
                inputRegisterData[reg]=tmpVal_F;
                break;
            case 16:
                hal->GetAirTemperatureAHT21(&tmpVal_F);
                inputRegisterData[reg]=tmpVal_F*100;
                break;
            case 17:
                hal->GetAirRelHumidityBME280(&tmpVal_F);
                inputRegisterData[reg]=tmpVal_F;
                break;
            case 18:
                hal->GetAirTemperatureBME280(&tmpVal_F);
                inputRegisterData[reg]=tmpVal_F*100;
                break;
            case 100:
                inputRegisterData[reg]=FIRMWARE_VERSION;
                break;
            default:
                break;
            }
        }
    }
}

constexpr size_t UART_BUF_SIZE{1024};


#if defined(LABATHOME_V10)
    constexpr uart_port_t MODBUS_UART_NUM{UART_NUM_2};
    constexpr gpio_num_t UART_TX{GPIO_NUM_21};//Pin 4 am Diplay Connector ("SPI_MOSI")
    constexpr gpio_num_t UART_RX{GPIO_NUM_23};

    #if CONFIG_ESP_CONSOLE_UART_CUSTOM != 1 | CONFIG_ESP_CONSOLE_UART_TX_GPIO!=21 |  CONFIG_ESP_CONSOLE_UART_RX_GPIO!=23
        #error "CONFIG_ESP_CONSOLE_UART_CUSTOM != 1 | CONFIG_ESP_CONSOLE_UART_TX_GPIO!=21 |  CONFIG_ESP_CONSOLE_UART_RX_GPIO!=23"
    #endif
#elif defined(LABATHOME_V15)
#include "tinyusb.h"
#include "tusb_cdc_acm.h"
#else
#error "No Labathome version defined, see main.cc"
#endif

void mainTask(void* args){   
    ESP_LOGI(TAG, "Main Manager started");
#if defined(LABATHOME_V10)
    uart_config_t uart_config = {
        .baud_rate=9600,
        .data_bits=UART_DATA_8_BITS,
        .parity=UART_PARITY_EVEN,
        .stop_bits=UART_STOP_BITS_1,
        .flow_ctrl=UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh=0,
        .source_clk=UART_SCLK_DEFAULT,
    };
    int intr_alloc_flags = 0;



    ESP_ERROR_CHECK(uart_driver_install(MODBUS_UART_NUM, UART_BUF_SIZE, 0, 0, nullptr, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(MODBUS_UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(MODBUS_UART_NUM, PIN_TXD0 , PIN_RXD0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
#elif defined(LABATHOME_V15)
 const tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,
        .string_descriptor = NULL,
        .external_phy = false,
        .configuration_descriptor = NULL,
    };

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

    tinyusb_config_cdcacm_t acm_cfg = {
        .usb_dev = TINYUSB_USBDEV_0,
        .cdc_port = TINYUSB_CDC_ACM_0,
        .rx_unread_buf_sz = 64,
        .callback_rx = nullptr, // the first way to register a callback
        .callback_rx_wanted_char = nullptr,
        .callback_line_state_changed = nullptr,
        .callback_line_coding_changed = nullptr
    };

    ESP_ERROR_CHECK(tusb_cdc_acm_init(&acm_cfg));
    ESP_LOGI(TAG, "USB initialization DONE");
#endif

    TickType_t xLastWakeTime;
    
    xLastWakeTime = xTaskGetTickCount();
    hal->GreetUserOnStartup();
    size_t rx_size{0};
    size_t rx_size_max{0};
    size_t tx_size{64};
    uint8_t tx_buf[256+1];

    uint8_t* rx_buf{nullptr};
#if defined(LABATHOME_V15)
    static uint8_t rx_buf_tmp[CFG_TUD_CDC_RX_BUFSIZE + 1];
#endif
    ESP_LOGI(TAG, "Main Manager Loop starts");
    modbusSlave = new modbus::M<100000>(1, modbusAfterWriteCallback, modbusBeforeReadCallback, &coilData, &discreteInputsData, &inputRegisterData, &holdingRegisterData);
    while (true)
    {
        xTaskDelayUntil(&xLastWakeTime, xFrequency);
        hal->BeforeLoop();
        size_t length{0};
#if defined(LABATHOME_V15)
        ESP_ERROR_CHECK(tinyusb_cdcacm_read(TINYUSB_CDC_ACM_0, rx_buf_tmp, CONFIG_TINYUSB_CDC_RX_BUFSIZE, &length));
#elif defined(LABATHOME_V10)
        ESP_ERROR_CHECK(uart_get_buffered_data_len(MODBUS_UART_NUM, &length));
#endif
        if(length>0){
            ESP_LOGD(TAG, "Got Data Length %d", length);
            modbusSlave->ReceiveBytesPhase1(&rx_buf, &rx_size_max);
            //In den rx_buf kann jetzt reingeschrieben werden, allerdings maximal rx_size_max bytes!
#if defined(LABATHOME_V15)
            if(length>rx_size_max){
                ESP_LOGE(TAG, "length %d >rx_size_max %d", length, rx_size_max);
                continue;
            }
            std::memcpy(rx_buf, rx_buf_tmp, length);
            rx_size=length;
#elif defined(LABATHOME_V10)
            rx_size = uart_read_bytes(MODBUS_UART_NUM, rx_buf, rx_size_max, 0);
#endif
            
            if(rx_size>0){
                modbusSlave->ReceiveBytesPhase2(rx_size, tx_buf, tx_size);
                if(tx_size>0){
                    vTaskDelay(pdMS_TO_TICKS(10));
#if defined(LABATHOME_V15)
                    tinyusb_cdcacm_write_queue(TINYUSB_CDC_ACM_0, tx_buf, tx_size);
                    tinyusb_cdcacm_write_flush(TINYUSB_CDC_ACM_0, 0);
#elif defined(LABATHOME_V10)
                    uart_write_bytes(MODBUS_UART_NUM, tx_buf, tx_size);
                    uart_wait_tx_done(MODBUS_UART_NUM, 1000);
#endif
                }
            }
        }
        hal->AfterLoop();
    }
}


extern "C" void app_main(void)
{
    
    esp_err_t ret = nvs_flash_init_partition(NVS_PARTITION_NAME);
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init_partition(NVS_PARTITION_NAME));
    }

    //Configure Network
    #include "secrets.hh"
    WIFISTA::InitAndRun(WIFI_SSID, WIFI_PASS, "labathome_%02x%02x%02x");
    
    hal->InitAndRun();
    xTaskCreate(mainTask, "mainTask", 4096 * 4, nullptr, 6, nullptr);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (true)
    {
         xTaskDelayUntil(&xLastWakeTime, xFrequency);
        hal->DoMonitoring();
    
    }
}