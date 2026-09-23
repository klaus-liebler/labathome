constexpr int FIRMWARE_VERSION{5};
// Hints for the command line test software "modpoll", that I used during development
// In this tool, the register addresses ("r") is 1-based, you have to subtract 1 from the addresses in the lists below.
// Btw, in contrast to "modpoll", the CODESYS tool uses 0-based addresses.

// COMxy has to be replaced with your specific com port.

// Query Inputs:      modpoll -r 1 -c 9 -t 1 -b 9600 COM17
// Set Coil Outputs:  modpoll -r 1 -c 1 -t 0 -b 9600 COM17 0
// Set LED Color      modpoll -r 10 -c 1 -t 4 -b 9600 COM17 65535
// Set Servo          modpoll -r 2 -c 1 -t 4 -b 9600 COM7 50

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

#include <soc/soc_caps.h>
#include "iHAL.hh"

// Modbus laeuft ueber die zweite USB-CDC-Schnittstelle (TinyUSB) und gibt es daher nur auf Chips mit
// USB-OTG (ESP32-S2/S3/P4). Auf dem klassischen ESP32 (Labathome Rev. 5.x) sind ModbusSetup/ModbusLoop
// leere Platzhalter; Modbus ueber die RS485-Pins ist dort (noch) nicht umgesetzt.
#if SOC_USB_OTG_SUPPORTED
#include "tinyusb.h"
#include "tusb_cdc_acm.h"

static const char *TAG = "main";

#include "iHAL.hh"
namespace modbus
{

    modbus::M<100000> *modbusSlave;
    constexpr tinyusb_cdcacm_itf_t TINYUSB_CDC_ACM{TINYUSB_CDC_ACM_1};
    constexpr size_t COILS_CNT{16};
    constexpr size_t DISCRETE_INPUTS_CNT{16};
    constexpr size_t INPUT_REGISTERS_CNT{101};
    constexpr size_t HOLDING_REGISTERS_CNT{16};

    static std::vector<bool> coilData(COILS_CNT);
    static std::vector<bool> discreteInputsData(DISCRETE_INPUTS_CNT);
    static std::vector<uint16_t> inputRegisterData(INPUT_REGISTERS_CNT);
    static std::vector<uint16_t> holdingRegisterData(HOLDING_REGISTERS_CNT);
    static iHAL *hal{nullptr};

    void modbusAfterWriteCallback(uint8_t fc, uint16_t start, size_t len)
    {

        if (fc == 15 || fc == 5)
        {
            bool b = coilData.at(start);
            ESP_LOGI(TAG, "Modbus Coil Registers changed! fc:%d, start:%d len:%d, firstChangedBit=%d", fc, start, len, b);
            for (int i = start; i < start + len; i++)
            {
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
        else if (fc == 16 || fc == 6)
        {
            ESP_LOGI(TAG, "Modbus Output Registers changed! fc:%d, start:%d len:%d, firstChangesValues=%d", fc, start, len, holdingRegisterData.at(start));

            for (int reg = start; reg < start + len; reg++)
            {
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
                case 8: // PowerWhite
                    hal->SetLedPowerWhiteDuty(holdingRegisterData.at(reg));
                    break;
                case 9: // RGB LED 0, this number has to be inserted in call in "case 12"
                case 10:
                case 11:
                case 12: // RGB LED 3
                    hal->ColorizeLed(reg - 9, CRGB::FromRGB565(holdingRegisterData.at(reg)));
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

    void modbusBeforeReadCallback(uint8_t fc, uint16_t start, size_t len)
    {
        ESP_LOGI(TAG, "Modbus Register Read! fc:%d, start:%d len:%d.", fc, start, len);
        if (fc == 2)
        { // Discrete Inputs --> Green, Red, Yellow, Movement
            for (int reg = start; reg < start + len; reg++)
            {
                switch (reg)
                {
                case 0:
                    discreteInputsData[reg] = hal->GetButtonGreenIsPressed();
                    break;
                case 1:
                    discreteInputsData[reg] = hal->GetButtonRedIsPressed();
                    break;
                case 2:
                    discreteInputsData[reg] = hal->GetButtonEncoderIsPressed();
                    break;
                case 3:
                    discreteInputsData[reg] = hal->IsMovementDetected();
                    break;
                default:
                    break;
                }
            }
        }
        else if (fc == 4)
        { // Input Registers
            float tmpVal_F{0.0f};
            float *tmpPtr_F{nullptr};
            int tmpVal_I32{0};
            uint16_t tmpVal_U16{0};

            for (int reg = start; reg < start + len; reg++)
            {
                switch (reg)
                {
                case 0:
                    hal->GetCO2PPM(&tmpVal_F);
                    inputRegisterData[reg] = tmpVal_F;
                    break;
                case 1:
                    hal->GetAirPressure(&tmpVal_F);
                    inputRegisterData[reg] = tmpVal_F;
                    break;
                case 2:
                    hal->GetAmbientBrightness(&tmpVal_F);
                    inputRegisterData[reg] = tmpVal_F;
                    break;
                case 3:
                    hal->GetAnalogInputs(&tmpPtr_F);
                    inputRegisterData[reg] = tmpPtr_F[0]; // CHANNEL_ANALOGIN_OR_ROTB, Pin 34
                    break;
                case 4:
                    inputRegisterData[reg] = hal->GetButtonGreenIsPressed();
                    break;
                case 5:
                    inputRegisterData[reg] = hal->GetButtonRedIsPressed();
                    break;
                case 6:
                    inputRegisterData[reg] = hal->GetButtonEncoderIsPressed();
                    break;
                case 7:
                    // hal->GetFan1Rpm(&tmpVal_F);
                    inputRegisterData[reg] = 0;
                    break;
                case 8:
                    hal->GetHeaterTemperature(&tmpVal_F);
                    inputRegisterData[reg] = tmpVal_F * 100;
                    break;
                case 9:
                    hal->GetEncoderValue(&tmpVal_I32);
                    inputRegisterData[reg] = tmpVal_I32;
                    break;
                case 10:
                    inputRegisterData[reg] = hal->IsMovementDetected();
                    break;
                case 11:
                    hal->GetDistanceMillimeters(&tmpVal_U16);
                    inputRegisterData[reg] = tmpVal_U16;
                    break;
                case 12:
                    hal->GetAnalogInputs(&tmpPtr_F);
                    inputRegisterData[reg] = tmpPtr_F[1]; // CHANNEL_MOVEMENT_OR_FAN1SENSE I35
                    break;
                case 13:
                    hal->GetAnalogInputs(&tmpPtr_F);
                    inputRegisterData[reg] = tmpPtr_F[2];
                    break;
                case 14:
                    hal->GetAnalogInputs(&tmpPtr_F);
                    inputRegisterData[reg] = tmpPtr_F[3];
                    break;
                case 15:
                    hal->GetAirRelHumidityAHT21(&tmpVal_F);
                    inputRegisterData[reg] = tmpVal_F;
                    break;
                case 16:
                    hal->GetAirTemperatureAHT21(&tmpVal_F);
                    inputRegisterData[reg] = tmpVal_F * 100;
                    break;
                case 17:
                    hal->GetAirRelHumidityBME280(&tmpVal_F);
                    inputRegisterData[reg] = tmpVal_F;
                    break;
                case 18:
                    hal->GetAirTemperatureBME280(&tmpVal_F);
                    inputRegisterData[reg] = tmpVal_F * 100;
                    break;
                case 100:
                    inputRegisterData[reg] = FIRMWARE_VERSION;
                    break;
                default:
                    break;
                }
            }
        }
    }

    void ModbusSetup(iHAL *_hal)
    {
        ESP_LOGI(TAG, "InitModbus");
        hal = _hal;
        ESP_LOGI(TAG, "Main Manager Loop starts");
        modbusSlave = new modbus::M<100000>(1, modbusAfterWriteCallback, modbusBeforeReadCallback, &coilData, &discreteInputsData, &inputRegisterData, &holdingRegisterData);
    }

    bool ModbusLoop()
    {
        size_t rx_size{0};
        size_t rx_size_max{0};
        size_t tx_size{64};
        
        static uint8_t rx_buf_tmp[CFG_TUD_CDC_RX_BUFSIZE + 1];
        static uint8_t tx_buf[256 + 1];

        uint8_t *rx_buf{nullptr};
 

        ESP_ERROR_CHECK(tinyusb_cdcacm_read(TINYUSB_CDC_ACM, rx_buf_tmp, CONFIG_TINYUSB_CDC_RX_BUFSIZE, &rx_size));

        if (rx_size == 0) return false; // no data received
        
        ESP_LOGD(TAG, "Received Data Length %d", rx_size);
        //Holt sich einen ReadBuffer vom ModbusSlave, der dann mit den empfangenen Daten gefüllt wird.
        //in diesen können maximal rx_size_max Bytes geschrieben werden.
        modbusSlave->ReceiveBytesPhase1(&rx_buf, &rx_size_max);

        if (rx_size > rx_size_max)
        {
            //Prüfe, ob der Buffer lang genug für die Nachricht ist. Wenn nicht lang genug, wird die Verarbeitung abgebrochen.
            ESP_LOGE(TAG, "length %d >rx_size_max %d", rx_size, rx_size_max);
            return true; // data processed, but not valid
        }
        std::memcpy(rx_buf, rx_buf_tmp, rx_size);

        modbusSlave->ReceiveBytesPhase2(rx_size, tx_buf, tx_size);
        if (tx_size > 0)
        {
            vTaskDelay(pdMS_TO_TICKS(10));

            tinyusb_cdcacm_write_queue(TINYUSB_CDC_ACM, tx_buf, tx_size);
            tinyusb_cdcacm_write_flush(TINYUSB_CDC_ACM, 0);
        }
        return true; // data processed
    }
}
#else
static const char *TAG = "modbus";

namespace modbus
{
    void ModbusSetup(iHAL *)
    {
        ESP_LOGI(TAG, "Modbus over USB not available on this chip");
    }

    bool ModbusLoop()
    {
        return false;
    }
}
#endif
