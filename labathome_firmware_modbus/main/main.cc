#include <stdio.h>
#include <stdint.h>
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

static const char *TAG = "main";

#include "HAL.hh"
#include "HAL_labathomeV10.hh"
static HAL * hal = new HAL_Impl(MODE_MOVEMENT_OR_FAN1SENSE::MOVEMENT_SENSOR);
modbus::M<100000> *modbusSlave;


//
//COIL (binärer Ausgang)
//-Relay (auch über Holding Register - das letzte Schreiben zählt)

//Discrete Input
//-Drei Taster
//-Bewegung

    //Query Inputs:      modpoll -r 1 -c 9 -t 1 COM8
    //Set Coil Outputs:  modpoll -r 1 -c 1 -t 0 COM8 1 bzw 0
    //Set LED Color      modpoll -r 1 -c 1 -t 4 COM8 65535



//Holding Register
//-AnalogOut, Servo1, Servo2, Fan1, Fan2, DutyHeater, DutyPowerLed, RGB0, RGB1, RGB2, RGB3, Relay, Sound, UsbPowerSuppy (not implemented)

constexpr size_t COILS_CNT{16};
constexpr size_t DISCRETE_INPUTS_CNT{16};
constexpr size_t INPUT_REGISTERS_CNT{16};
constexpr size_t HOLDING_REGISTERS_CNT{16};



constexpr uart_port_t uart_num = UART_NUM_2;


static std::vector<bool> coilData(COILS_CNT);
static std::vector<bool> discreteInputsData(DISCRETE_INPUTS_CNT);
static std::vector<uint16_t> inputRegisterData(INPUT_REGISTERS_CNT);
static std::vector<uint16_t> holdingRegisterData(HOLDING_REGISTERS_CNT);

/*
Coils:
 0: Relay K3

Discrete Input:
 0: Green Button
 1: Red Button
 2: Yellow Button / Encoder Button
 3: Movement Sensor

Input Registers:
 0: Not connected
 1: Servo 1, Position in Degrees 0...180
 2: Servo 2, Position in Degrees 0...180
 3: Fan 1, Power in Percent 0...100
 4: Fan 2, Power in Percent 0...100
 5: Heater, Power in Percent 0...100
 6: White Power LED, Power in Percent 0...100
 7: RGB LED 1, Color in RGB565
 8: LED 2
 9: LED 3
10: LED 4
11: Relay State (Alternative to Coil 0), 0 means off, all other values on
12: Play Sound, 0 means silence; try other values up to 9

Holding Registers:
 0: CO2 [PPM]
 1: Air Pressure [mbar /kPa???]
 2: Ambient Brightness [?]
 3: Analog Input [mV]
 4: Button Green [0 or 1]
 5: Button Red [0 or 1]
 6: Button Yellow/Encoder [0 or 1]
 7: Fan 1 RpM
 8: Heater Temperature [°C * 100]
 9: Encoder Detents
10: Movement Sensor [0 or 1]
                
*/




void modbusAfterWriteCallback(uint8_t fc, uint16_t start, size_t len){
    ESP_LOGI(TAG, "Modbus Registers changed! fc:%d, start:%d len:%d.", fc, start, len);
    if(fc==15 || fc==5){
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
        for(int reg=start;reg<start+len;reg++){
            switch (reg)
            {
            case 0:
                break;
            case 1:
                hal->SetServo1Position(holdingRegisterData.at(reg));
                break;
            case 2:
                hal->SetServo2Position(holdingRegisterData.at(reg));
                break;
            case 3:
                hal->SetFan1Duty(holdingRegisterData.at(reg));
                break;
            case 4:
                hal->SetFan2Duty(holdingRegisterData.at(reg));
                break;
            case 5:
                hal->SetHeaterDuty(holdingRegisterData.at(reg));
                break;
            case 6://PowerWhite
                hal->SetLedPowerWhiteDuty(holdingRegisterData.at(reg));
                break;
            case 7://RGB LED 0
            case 8:
            case 9:
            case 10: //RGB LED 3
                hal->ColorizeLed(reg-7, CRGB::FromRGB565(holdingRegisterData.at(reg)));
                break;
            case 11:
                hal->SetRelayState(holdingRegisterData.at(reg));
                break;
            case 12:
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
        float tmpVal{0.0f};
        int tmpVal_I32{0};
        float tmpArr[4]={};

        for(int reg=start;reg<start+len;reg++){
            switch (reg)
            {
            case 0:
                hal->GetCO2PPM(&tmpVal);
                inputRegisterData[reg]=tmpVal*100;
                break;
            case 1:
                hal->GetAirPressure(&tmpVal);
                inputRegisterData[reg]=tmpVal*100;
                break;
            case 2:
                hal->GetAmbientBrightness(&tmpVal);
                inputRegisterData[reg]=tmpVal*100;
                break;
            case 3:
                //hal->GetAnalogInputs(tmpArr);
                inputRegisterData[reg]=tmpArr[0];
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
                hal->GetFan1Rpm(&tmpVal);
                inputRegisterData[reg]=tmpVal*100;
                break;
            case 8:
                hal->GetHeaterTemperature(&tmpVal);
                inputRegisterData[reg]=tmpVal*100;
                break;
            case 9:
                hal->GetEncoderValue(&tmpVal_I32);
                inputRegisterData[reg]=tmpVal_I32;
                break;
            case 10:
                inputRegisterData[reg]=hal->IsMovementDetected();
            default:
                break;
            }
        }
    }
}

constexpr size_t UART_BUF_SIZE{1024};
constexpr uart_port_t UART_NUM{UART_NUM_2};
constexpr gpio_num_t UART_TX{GPIO_NUM_21};
constexpr gpio_num_t UART_RX{GPIO_NUM_23};

#if CONFIG_ESP_CONSOLE_UART_CUSTOM != 1 | CONFIG_ESP_CONSOLE_UART_TX_GPIO!=21
#error "CONFIG_ESP_CONSOLE_UART_CUSTOM != y OR CONFIG_ESP_CONSOLE_UART_TX_GPIO!=21"
#endif


void mainTask(void* args){   
    ESP_LOGI(TAG, "Main Manager started");

    uart_config_t uart_config = {};

    uart_config.baud_rate = 9600;
    uart_config.data_bits = UART_DATA_8_BITS;
    uart_config.parity    = UART_PARITY_EVEN;
    uart_config.stop_bits = UART_STOP_BITS_1;
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    uart_config.source_clk = UART_SCLK_DEFAULT;
    int intr_alloc_flags = 0;



    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, UART_BUF_SIZE, 0, 0, nullptr, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM, PIN_TXD0 , PIN_RXD0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));


    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 10;
    xLastWakeTime = xTaskGetTickCount();
    hal->GreetUserOnStartup();
    size_t rx_size{0};
    size_t rx_size_max{0};
    size_t tx_size{64};
    uint8_t tx_buf[256+1];
    uint8_t* rx_buf{nullptr};

    ESP_LOGI(TAG, "Main Manager Loop starts");
    modbusSlave = new modbus::M<100000>(1, modbusAfterWriteCallback, modbusBeforeReadCallback, &coilData, &discreteInputsData, &inputRegisterData, &holdingRegisterData);
    while (true)
    {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        hal->BeforeLoop();
        size_t length{0};
        ESP_ERROR_CHECK(uart_get_buffered_data_len(uart_num, &length));
        if(length>0){
            ESP_LOGI(TAG, "Got Data Length %d", length);
            modbusSlave->ReceiveBytesPhase1(&rx_buf, &rx_size_max);
            rx_size = uart_read_bytes(uart_num, rx_buf, rx_size_max, 0);
            if(rx_size>0){
                modbusSlave->ReceiveBytesPhase2(rx_size, tx_buf, tx_size);
                if(tx_size>0){
                    vTaskDelay(pdMS_TO_TICKS(10));
                    uart_write_bytes(uart_num, tx_buf, tx_size);
                    uart_wait_tx_done(uart_num, 1000);
                }
            }
        }
        hal->AfterLoop();
    }
}


extern "C" void app_main(void)
{
    hal->InitAndRun();
    ESP_LOGI(TAG, "RED %d YEL %d GRN %d", hal->GetButtonRedIsPressed(), hal->GetButtonEncoderIsPressed(), hal->GetButtonGreenIsPressed());
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    
    xTaskCreate(mainTask, "mainTask", 4096 * 4, nullptr, 6, nullptr);
    
    while (true)
    {
        //hal->OutputOneLineStatus();
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}