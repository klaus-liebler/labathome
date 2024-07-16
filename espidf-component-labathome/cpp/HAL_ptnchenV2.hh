#pragma once

#include "HAL.hh"

#include <inttypes.h>
#include <limits>
#include <algorithm>
#include <common.hh>
#include <common-esp32.hh>
#include <driver/dac.h>
#include <driver/i2c.h>


#include <errorcodes.hh>
#include <rgbled.hh>
#include <ads1115.hh>
#include <i2c.hh>
#include "winfactboris_messages.hh"
#include <wifimanager.hh>




typedef gpio_num_t Pintype;

constexpr Pintype PIN_TXD0 = (Pintype)1;
constexpr Pintype PIN_RXD0 = (Pintype)3;
constexpr Pintype PIN_LED_WS2812 = (Pintype)15;
constexpr Pintype PIN_FAN1_DRIVE_OR_SERVO1 = (Pintype)18;
constexpr Pintype PIN_I2C_SDA = (Pintype)19;
constexpr Pintype PIN_I2C_SCL = (Pintype)22;
constexpr Pintype PIN_SW = (Pintype)36;
constexpr adc1_channel_t CHANNEL_SWITCHES{ADC1_CHANNEL_0};







enum class Button : uint8_t
{
    BUT_ENCODER = 1,
    BUT_RED = 0,
    BUT_GREEN = 2,
};
constexpr size_t ANALOG_INPUTS_LEN{4};
constexpr size_t LED_NUMBER{5};

constexpr i2c_port_t I2C_PORT{I2C_NUM_1};
constexpr uint32_t DEFAULT_VREF{1100}; //Use adc2_vref_to_gpio() to obtain a better estimate
constexpr uint16_t sw_limits[]{160, 480, 1175, 1762, 2346, 2779, 3202};

BlinkPattern blinkFastRedBlack(CRGB::Red, 300, CRGB::Black, 300);
BlinkPattern blinkFastGreenBlack(CRGB::Green, 300, CRGB::Black, 300);
BlinkPattern blinkFastBlueGreen(CRGB::Blue, 300, CRGB::Green, 300);

class HAL_Impl : public HAL
{
private:

    
    //management objects
    RGBLED<LED_NUMBER, DeviceType::PL9823> *strip{nullptr};

    //SensorValues
    uint32_t buttonState{0}; //see Button-Enum for meaning of bits
    float AnalogInputs[ANALOG_INPUTS_LEN]={0};

    esp_adc_cal_characteristics_t *adc_chars{nullptr};

    void SensorLoop()
    {
        
        int64_t nextADS1115Readout{UINT32_MAX};
        uint16_t nextADS1115Mux{0b100}; //100...111
        int64_t ads1115ReadoutInterval{portMAX_DELAY};

        int64_t nextBinaryAndAnalogReadout{0};

        //ADS1115
        ADS1115 *ads1115 = new ADS1115(I2C_PORT, (uint8_t)0x48);
        if (ads1115->Init(ads1115_sps_t::ADS1115_SPS_16, &ads1115ReadoutInterval) == ESP_OK)
        {
            ads1115->TriggerMeasurement((ads1115_mux_t)nextADS1115Mux);
            nextADS1115Mux++;
            nextADS1115Readout = xTaskGetTickCount() + ads1115ReadoutInterval;
            ESP_LOGI(TAG, "I2C: ADS1115 successfully initialized.");
        }
        else
        {
            ESP_LOGW(TAG, "I2C: ADS1115 not found");
        }
        
        WIFIMGR::SimpleState lastWifiState{WIFIMGR::SimpleState::OFFLINE};
        while (true)
        {
            if(GetMillis64() > nextBinaryAndAnalogReadout){
                readBinaryAndAnalogIOs();
                nextBinaryAndAnalogReadout = GetMillis64()+100;
            }
            
            if (xTaskGetTickCount() >= nextADS1115Readout)
            {
                ads1115->GetVoltage(&AnalogInputs[nextADS1115Mux & 0b11]);
                nextADS1115Mux++;
                if (nextADS1115Mux > 0b111)
                    nextADS1115Mux = 0b100;
                ads1115->TriggerMeasurement((ads1115_mux_t)nextADS1115Mux);
                nextADS1115Readout = xTaskGetTickCount() + ads1115ReadoutInterval;
                WIFIMGR::SimpleState newWifiState = WIFIMGR::GetState();
                if(newWifiState!=lastWifiState){
                    switch (newWifiState)
                    {
                    case WIFIMGR::SimpleState::AP_AVAILABLE:
                        strip->AnimatePixel(0, &blinkFastBlueGreen);
                        break;
                    case WIFIMGR::SimpleState::OFFLINE:
                        strip->AnimatePixel(0, &blinkFastRedBlack);
                        break;
                    case WIFIMGR::SimpleState::STA_CONNECTED:
                        strip->SetPixel(0, CRGB::DarkGreen);
                        break;
                    default:
                        break;
                    }
                    lastWifiState=newWifiState;
                }
                strip->Refresh(100);  //checks internally, whether data is dirty and has to be pushed out
            }
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }


public:
    HAL_Impl()
    {
    }

    ErrorCode OutputOneLineStatus(){
        uint32_t heap = esp_get_free_heap_size();
        bool but=GetButtonRedIsPressed();
        ESP_LOGI(TAG, "Heap %6d BUTT %d ANAIN %4.1f %4.1f %4.1f %4.1f",
                       heap,    but,    this->AnalogInputs[0], this->AnalogInputs[1], this->AnalogInputs[2], this->AnalogInputs[3]);
        return ErrorCode::OK;
    }

    ErrorCode HardwareTest() override{
        return ErrorCode::OK;
    }

    int64_t IRAM_ATTR GetMicros()
    {
        return esp_timer_get_time();
    }

    uint32_t GetMillis()
    {
        return (uint32_t)(esp_timer_get_time() / 1000ULL);
    }

    int64_t GetMillis64()
    {
        return esp_timer_get_time() / 1000ULL;
    }

    ErrorCode GetAnalogInputs(float **voltages)
    {
        *voltages = this->AnalogInputs;
        return ErrorCode::OK;
    }

    ErrorCode GetEncoderValue(int *value){
        return ErrorCode::FUNCTION_NOT_AVAILABLE;
    }

    ErrorCode GetFan1Rpm(float* rpm){
        return ErrorCode::FUNCTION_NOT_AVAILABLE;
    }
    
    ErrorCode SetSound(int32_t soundNumber)
    {
        return ErrorCode::FUNCTION_NOT_AVAILABLE;
    }

    ErrorCode GetSound(int32_t* soundNumber){
        return ErrorCode::FUNCTION_NOT_AVAILABLE;
    }

    
    ErrorCode GetCO2PPM(float *co2PPM){
        return ErrorCode::FUNCTION_NOT_AVAILABLE;
    }

    ErrorCode GetHeaterTemperature(float *degreesCelcius)
    {
        return ErrorCode::FUNCTION_NOT_AVAILABLE;
    }

    ErrorCode GetAirTemperature(float *degreesCelcius)
    {
        return ErrorCode::FUNCTION_NOT_AVAILABLE;
    }

    ErrorCode GetAirPressure(float *pa)
    {
        return ErrorCode::FUNCTION_NOT_AVAILABLE;
    }

    ErrorCode GetAirQuality(float *qualityPercent)
    {
        return ErrorCode::FUNCTION_NOT_AVAILABLE;
    }

    ErrorCode GetAirRelHumidity(float *percent)
    {
        return ErrorCode::FUNCTION_NOT_AVAILABLE;
    }

    ErrorCode GetAirSpeed(float *meterPerSecond)
    {
        return ErrorCode::FUNCTION_NOT_AVAILABLE;
    }

    ErrorCode GetAmbientBrightness(float *lux)
    {
        return ErrorCode::FUNCTION_NOT_AVAILABLE;
    }

    ErrorCode GetWifiRssiDb(float *db){
        return ErrorCode::FUNCTION_NOT_AVAILABLE;
    }
    
    uint8_t dacValue{142};
    ErrorCode SetAnalogOutput(float volts){
        uint8_t newDacValue = volts*255.0/3.3;
        if(newDacValue!=dacValue){
            ESP_LOGI(TAG, "Set DAC to %d", newDacValue);
            dacValue=newDacValue;
        }
        dac_output_voltage(DAC_CHANNEL_1, newDacValue);
        return ErrorCode::OK;
    }
    
    ErrorCode GetFFT64(float *magnitudes64_param){
        return ErrorCode::FUNCTION_NOT_AVAILABLE;
    }

    ErrorCode UpdatePinConfiguration(uint8_t* configMessage, size_t configMessagelen){
        return ErrorCode::FUNCTION_NOT_AVAILABLE;
    }


    void readBinaryAndAnalogIOs(){
        
        int adc_reading = adc1_get_raw(CHANNEL_SWITCHES);
        int i = 0;
        for (i = 0; i < sizeof(sw_limits) / sizeof(uint16_t); i++)
        {
            if (adc_reading < sw_limits[i])
                break;
        }
        i = ~i;
        this->buttonState = i;
    }

    ErrorCode InitAndRun()
    {
        //LED Strip
        strip = new RGBLED<LED_NUMBER, DeviceType::PL9823>();
        ESP_ERROR_CHECK(strip->Init(VSPI_HOST, PIN_LED_WS2812, 2 ));
        ESP_ERROR_CHECK(strip->Clear(100));
        ESP_ERROR_CHECK(strip->AnimatePixel(0, &blinkFastRedBlack));      
        //Configure Analog
        adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));
        esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_0db, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(CHANNEL_SWITCHES, ADC_ATTEN_DB_0);

        dac_output_enable(DAC_CHANNEL_1);


        //I2C Master
        ESP_ERROR_CHECK(I2C::Init(I2C_PORT, PIN_I2C_SCL, PIN_I2C_SDA));


        readBinaryAndAnalogIOs();//do this while init to avoid race condition (wifimanager is resettet when red and green buttons are pressed during startup)
        xTaskCreate(sensorTask, "sensorTask", 4096 * 4, this, 6, nullptr);
      
        return ErrorCode::OK;
    }

    ErrorCode BeforeLoop()
    {
        return ErrorCode::OK;
    }

    ErrorCode AfterLoop()
    {
        
        return ErrorCode::OK;
    }

    ErrorCode StartBuzzer(float freqHz)
    {
        return ErrorCode::FUNCTION_NOT_AVAILABLE;
    }

    ErrorCode EndBuzzer()
    {
        return ErrorCode::FUNCTION_NOT_AVAILABLE;
    }
    
    ErrorCode GreetUserOnStartup() override{
        return ErrorCode::OK;
    }

    ErrorCode ColorizeLed(uint8_t ledIndex, CRGB colorCRGB)
    {
        if(ledIndex>=LED_NUMBER-1) return ErrorCode::INDEX_OUT_OF_BOUNDS;
        ledIndex = (ledIndex+1)%LED_NUMBER;
        strip->SetPixel(ledIndex, colorCRGB);
        return ErrorCode::OK;
    }

    ErrorCode UnColorizeAllLed()
    {
        //all except the status led 0, which is controlled from hal directly
        strip->SetPixel(1, CRGB::Black);
        strip->SetPixel(2, CRGB::Black);
        strip->SetPixel(3, CRGB::Black);
        strip->SetPixel(4, CRGB::Black);
        return ErrorCode::OK;
    }

    ErrorCode SetRelayState(bool state)
    {
        return ErrorCode::FUNCTION_NOT_AVAILABLE;
    }

    ErrorCode SetHeaterDuty(float dutyInPercent)
    {
        return ErrorCode::FUNCTION_NOT_AVAILABLE;
    }

    float GetHeaterState()
    {
        return 0;
    }

    ErrorCode SetFan1Duty(float dutyInPercent)
    {
        return ErrorCode::FUNCTION_NOT_AVAILABLE;
    }

    float GetFan1State()
    {
        return 0;
    }

    ErrorCode SetFan2Duty(float dutyInPercent)
    {
        return ErrorCode::FUNCTION_NOT_AVAILABLE;
    }

    float GetFan2State()
    {
        return 0;
    }

    ErrorCode SetLedPowerWhiteDuty(float dutyInPercent)
    {
        return ErrorCode::FUNCTION_NOT_AVAILABLE;
    }

    bool GetButtonRedIsPressed()
    {
        return GetBitIdx(this->buttonState, (uint8_t)Button::BUT_RED);
    }

    bool GetButtonEncoderIsPressed()
    {
        return GetBitIdx(this->buttonState, (uint8_t)Button::BUT_ENCODER);
    }

    bool GetButtonGreenIsPressed()
    {
        return GetBitIdx(this->buttonState, (uint8_t)Button::BUT_GREEN);
    }

    bool IsMovementDetected()
    {
        return false;
    }

    ErrorCode SetServo1Position(float angle_0_to_180)
    {
        return ErrorCode::FUNCTION_NOT_AVAILABLE;
    }

    ErrorCode SetServo2Position(float angle_0_to_180)
    {
        return ErrorCode::FUNCTION_NOT_AVAILABLE;
    }

    float GetUSBCVoltage(){
        return 5.0;
    }

    static void sensorTask(void *pvParameters)
    {
        HAL_Impl *hal = (HAL_Impl *)pvParameters;
        hal->SensorLoop();
    }
};



