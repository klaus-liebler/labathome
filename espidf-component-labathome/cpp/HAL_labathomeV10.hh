#pragma once

#include "HAL.hh"

#include <inttypes.h>
#include <limits>
#include <algorithm>
#include <common.hh>
#include <common-esp32.hh>

#include <driver/mcpwm.h>
#include <driver/ledc.h>

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include <driver/i2c.h>

#include <errorcodes.hh>
#include <rgbled.hh>
#include <bh1750.hh>
#include <bme280.hh>
#include <ads1115.hh>
#include <ccs811.hh>
#include <hdc1080.hh>
#include <aht_sensor.hh>
#include <onewire_bus.hh>
#include <rotenc.hh>
#include <AudioPlayer.hh>

#include <PD_UFP.h>

#include "winfactboris_messages.hh"

FLASH_FILE(alarm_co2_mp3)
FLASH_FILE(alarm_temperature_mp3)
FLASH_FILE(nok_mp3)
FLASH_FILE(ok_mp3)
FLASH_FILE(ready_mp3)
FLASH_FILE(fanfare_mp3)
FLASH_FILE(negative_mp3)
FLASH_FILE(positive_mp3)
FLASH_FILE(siren_mp3)
const uint8_t *SOUNDS[]  = {nullptr, alarm_co2_mp3_start, alarm_temperature_mp3_start, nok_mp3_start, ok_mp3_start, ready_mp3_start, fanfare_mp3_start, negative_mp3_start, positive_mp3_start, siren_mp3_start};
const size_t SONGS_LEN[] = {0,       alarm_co2_mp3_size,  alarm_temperature_mp3_size,  nok_mp3_size,  ok_mp3_size,  ready_mp3_size,  fanfare_mp3_size,  negative_mp3_size,  positive_mp3_size,  siren_mp3_size};


typedef gpio_num_t Pintype;
constexpr Pintype PIN_SPI_IO2 = (Pintype)0;
constexpr Pintype PIN_TXD0 = (Pintype)1;
constexpr Pintype PIN_FAN2_DRIVE = (Pintype)2;
constexpr Pintype PIN_RXD0 = (Pintype)3;
constexpr Pintype PIN_SPI_IO1_OR_SERVO2 = (Pintype)4;
constexpr Pintype PIN_I2S_SCK = (Pintype)5;
constexpr Pintype PIN_HEATER_OR_LED_POWER = (Pintype)12;
constexpr Pintype PIN_I2S_WS = (Pintype)13; 
constexpr Pintype PIN_ONEWIRE = (Pintype)14;
constexpr Pintype PIN_LED_WS2812 = (Pintype)15;
constexpr Pintype PIN_SPI_MISO = (Pintype)16;
constexpr Pintype PIN_I2S_SD = (Pintype)17;
constexpr Pintype PIN_FAN1_DRIVE_OR_SERVO1 = (Pintype)18;
constexpr Pintype PIN_I2C_SDA = (Pintype)19;
constexpr Pintype PIN_SPI_MOSI = (Pintype)21;
constexpr Pintype PIN_I2C_SCL = (Pintype)22;
constexpr Pintype PIN_SPI_CLK = (Pintype)23;
constexpr Pintype PIN_SPEAKER = (Pintype)25;
constexpr Pintype PIN_MULTI2 = (Pintype)26;
constexpr Pintype PIN_MULTI1 = (Pintype)27;
constexpr Pintype PIN_K3_ON = (Pintype)32;
constexpr Pintype PIN_MULTI3 = (Pintype)33;
constexpr Pintype PIN_ANALOGIN_OR_ROTB = (Pintype)34;//
//36=VP, 39=VN
constexpr Pintype PIN_MOVEMENT_OR_FAN1SENSE = (Pintype)35;
constexpr Pintype PIN_SW = (Pintype)36;
constexpr Pintype PIN_LDR_OR_ROTA = (Pintype)39;

constexpr adc_channel_t CHANNEL_ANALOGIN_OR_ROTB{ADC_CHANNEL_6};
constexpr adc_channel_t CHANNEL_SWITCHES{ADC_CHANNEL_0};
constexpr adc_channel_t CHANNEL_LDR_OR_ROTA{ADC_CHANNEL_3};

constexpr Pintype PIN_485_DI = PIN_MULTI1;
constexpr Pintype PIN_EXT1 = PIN_MULTI1;

constexpr Pintype PN_485_DE = PIN_MULTI2;
constexpr Pintype PN_EXT2 = PIN_MULTI2;

constexpr Pintype PN_485_RO = PIN_MULTI3;
constexpr Pintype PN_EXT3 = PIN_MULTI3;

constexpr Pintype FUSB302_INT = PIN_SPI_CLK;

constexpr Pintype PIN_NS4168_WS{(Pintype)13};
constexpr Pintype PIN_NS4168_SCK{(Pintype)5};
constexpr Pintype PIN_NS4168_SDAT{(Pintype)25};
constexpr bool NS4168_ACTIVE{false};
constexpr bool MD8002_ACTIVE{!NS4168_ACTIVE};

struct Note
{
    uint16_t freq;
    uint16_t durationMs;
};

//Diese Software bietet die folgene Flexibilität NICHT:
//- Audio-Ausgang nur über internen DAC
//- Nur RS485, kein Ext-Anschluss
//- USB-IRQ blockiert SPI_CLK, Servo2 blockiert SPI_IO1

enum class MODE_HEATER_OR_LED_POWER:uint8_t{
    HEATER=1,
    LED_POWER=2,
};

enum class MODE_MOVEMENT_OR_FAN1SENSE:uint8_t{
    MOVEMENT_SENSOR=1,
    FAN1SENSE=2,
};

enum class MODE_FAN1_OR_SERVO1:uint8_t{
    FAN1=1,
    SERVO1=2,
};

enum class Button : uint8_t
{
    BUT_ENCODER = 1,
    BUT_RED = 0,
    BUT_GREEN = 2,
};

constexpr size_t ANALOG_INPUTS_LEN{4};
constexpr size_t LED_NUMBER{4};
constexpr i2c_port_t I2C_PORT{I2C_NUM_1};
constexpr uint32_t DEFAULT_VREF{1100}; //Use adc2_vref_to_gpio() to obtain a better estimate
constexpr uint16_t sw_limits[]{160, 480, 1175, 1762, 2346, 2779, 3202};
constexpr float SERVO_MIN_PULSEWIDTH{500.0};  //Minimum pulse width in microsecond
constexpr float SERVO_MAX_PULSEWIDTH{2400.0}; //Maximum pulse width in microsecond
constexpr float SERVO_MAX_DEGREE{180.0};      //Maximum angle in degree upto which servo can rotate

constexpr ledc_timer_bit_t power_ledc_timer_duty_resolution{LEDC_TIMER_10_BIT};

constexpr mcpwm_timer_t MCPWM_TIMER_SERVO{MCPWM_TIMER_0};
constexpr mcpwm_io_signals_t MCPWM_IO_SERVO1{MCPWM0A};
constexpr mcpwm_generator_t MCPWM_GEN_SERVO1{MCPWM_GEN_A};
constexpr mcpwm_io_signals_t MCPWM_IO_SERVO2{MCPWM0B};
constexpr mcpwm_generator_t MCPWM_GEN_SERVO2{MCPWM_GEN_B};

constexpr mcpwm_timer_t MCPWM_TIMER_FAN{MCPWM_TIMER_1};
constexpr mcpwm_io_signals_t MCPWM_IO_FAN1{MCPWM1A};
constexpr mcpwm_generator_t MCPWM_GEN_FAN1{MCPWM_GEN_A};
constexpr mcpwm_io_signals_t MCPWM_IO_FAN2{MCPWM1B};
constexpr mcpwm_generator_t MCPWM_GEN_FAN2{MCPWM_GEN_B};

constexpr mcpwm_timer_t MCPWM_TIMER_HEATER_OR_LED_POWER{MCPWM_TIMER_2};
constexpr mcpwm_io_signals_t MCPWM_IO_HEATER_OR_LED_POWER{MCPWM2A};
constexpr mcpwm_generator_t MCPWM_GEN_HEATER_OR_LED_POWER{MCPWM_GEN_A};

constexpr int FREQUENCY_HEATER{1}; //see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/mcpwm.html#resolution
constexpr int FREQUENCY_SERVO{50};
constexpr int FREQUENCY_FAN{100};
constexpr int FREQUENCY_LED{300};

constexpr i2s_port_t I2S_PORT_LOUDSPEAKER{I2S_NUM_0};//must be I2S_NUM_0, as only this hat access to internal DAC
constexpr i2s_port_t I2S_PORT_MICROPHONE{I2S_NUM_1};

constexpr int average_over_N_measurements{10};
constexpr int SAMPLES {2048};
constexpr size_t SAMPLES_IN_BYTES{SAMPLES*4};
constexpr int SAMPLE_RATE_MICROPHONE{11025};
constexpr int AMPLITUDE{150};
constexpr uint16_t FREQUENCIES[]{11,22,32,43,54,65,75,97,118,140,161,183,205,226,258,291,323,355,388,431,474,517,560,614,668,721,786,851,915,991,1066,1152,1238,1335,1443,1550,1669,1798,1938,2089,2239,2401,2584,2778,2982,3198,3435,3682,3951,4231,4533,4856,5200,5566,5965,6385,6837,7321,7838,8398,8990,9625,10304,11025};
constexpr uint16_t BUCKET_INDICES[]{1,2,3,4,5,6,7,9,11,13,15,17,19,21,24,27,30,33,36,40,44,48,52,57,62,67,73,79,85,92,99,107,115,124,134,144,155,167,180,194,208,223,240,258,277,297,319,342,367,393,421,451,483,517,554,593,635,680,728,780,835,894,957,1024};

//int32_t samplesI32[SAMPLES]; //The slave serial-data port’s format is I²S, 24-bit, twos complement, There must be 64 SCK cycles in each WS stereo frame, or 32 SCK cycles per data-word.
//double real[SAMPLES];
//double imag[SAMPLES];
//arduinoFFT fft(real, imag, SAMPLES, SAMPLE_RATE_MICROPHONE);

class HAL_Impl : public HAL
{
private:
    //config
    MODE_MOVEMENT_OR_FAN1SENSE mode_MOVEMENT_OR_FAN1SENSE;
    MODE_HEATER_OR_LED_POWER mode_HEATER_OR_LED_POWER{MODE_HEATER_OR_LED_POWER::HEATER};//Heater mit 1Hz, LED mit 300Hz
    MODE_FAN1_OR_SERVO1 mode_FAN1_OR_SERVO1{MODE_FAN1_OR_SERVO1::SERVO1};
    
    adc_oneshot_unit_handle_t adc1_handle;
    adc_cali_handle_t adc1_cali_handle{nullptr};

    //management objects
    RGBLED::M<LED_NUMBER, RGBLED::DeviceType::WS2812> *strip{nullptr};
    cRotaryEncoder *rotenc{nullptr};
    AudioPlayer::Player *mp3player;
    //pcnt_unit_handle_t speedmeter{nullptr};
    PD_UFP_core_c PD_UFP;//176byte
    hdc1080::M *hdc1080dev{nullptr};
    CCS811::M *ccs811dev{nullptr};
    AHT::M *aht21dev{nullptr};

    //SensorValues
    uint32_t buttonState{0}; //see Button-Enum for meaning of bits
    //int rotaryDetents rotary-Encoder Value in Object
    bool movementIsDetected{false};
    uint16_t voltage_USB_PD{0};
    float ambientBrightnessLux_analog{std::numeric_limits<float>::quiet_NaN()};
    float ambientBrightnessLux_digital{std::numeric_limits<float>::quiet_NaN()};
    float heaterTemperatureDegCel{std::numeric_limits<float>::quiet_NaN()};
    float airTemperatureDegCel{std::numeric_limits<float>::quiet_NaN()};
    float airPressurePa{std::numeric_limits<float>::quiet_NaN()};
    float airRelHumidityPercent{std::numeric_limits<float>::quiet_NaN()};
    float airCo2PPM{std::numeric_limits<float>::quiet_NaN()};
    float airQualityPercent{std::numeric_limits<float>::quiet_NaN()};
    float airSpeedMeterPerSecond{std::numeric_limits<float>::quiet_NaN()};
    float wifiRssiDb{std::numeric_limits<float>::quiet_NaN()};
    //float analogInputVolt{std::numeric_limits<float>::quiet_NaN()};
    uint32_t sound{0};
    float fan1RotationsRpM{std::numeric_limits<float>::quiet_NaN()};
    bool heaterEmergencyShutdown{false};

    float AnalogInputs[ANALOG_INPUTS_LEN]={0};
    
    uint16_t ds4525doPressure;
    
    void MP3Loop(){
        CodecManager::InternalDacWithPotentiometer *codec = new CodecManager::InternalDacWithPotentiometer();
        mp3player = new AudioPlayer::Player(codec);
        mp3player->Init();
        
        while(true){
            mp3player->Loop();
        }
    }

    void USBPDLoop(){
        PD_UFP.init(PD_POWER_OPTION_MAX_20V);
        while(true){
            PD_UFP.run();
            if(!PD_UFP.is_attached()){
                ESP_LOGI(TAG, "USB PD is not attached. Exiting USB PD Loop!");
                break;
            }
            u16 newVoltage = PD_UFP.get_voltage();
            if(newVoltage!=this->voltage_USB_PD){
                ESP_LOGI(TAG, "Voltage on USB-PD changed to %f", newVoltage*0.05);
                this->voltage_USB_PD=newVoltage;
            }
            if(newVoltage==400){//=20V
                ESP_LOGI(TAG, "Voltage reached Target 20V. Exiting USB PD Loop!");
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(5));
        }
        vTaskDelete(NULL);
    }

    void readBinaryAndAnalogIOs(){

        int adc_reading;
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, CHANNEL_SWITCHES, &adc_reading));
        
        //uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        //printf("Raw: %d\tVoltage: %dmV\n", adc_reading, voltage);
        int i = 0;
        for (i = 0; i < sizeof(sw_limits) / sizeof(uint16_t); i++)
        {
            if (adc_reading < sw_limits[i])
                break;
        }
        i = ~i;
        this->buttonState = i;

        //wifi_ap_record_t ap_info;
        //esp_wifi_sta_get_ap_info(&ap_info);//TODO may only be called, when ESP32 is connected...
        //this->wifiRssiDb=ap_info.rssi;

        //Es wird nicht unterschieden, ob der eingang "nur" zum Messen von analogen Spannung verwendet wird oder ob es sich um den Trigger-Eingang des Zeitrelais handelt. Im zweiten Fall muss einfach eine Zeitrelais-Schaltung basierend auf der Grenzüberschreitung des Analogen Messwertes in der Funktionsblock-Spache realisiert werden
        int analogInRaw;
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, CHANNEL_ANALOGIN_OR_ROTB, &analogInRaw));
        int analogInVoltage;
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, analogInRaw, &analogInVoltage));
        this->AnalogInputs[0]=11*analogInVoltage;//die Multiplikation mit 11 wegen dem Spannungsteiler
        int ldrInRaw;
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, CHANNEL_LDR_OR_ROTA, &ldrInRaw));
        int ldrInVoltage;
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, ldrInRaw, &ldrInVoltage));

        this->ambientBrightnessLux_analog = 1.2*ldrInVoltage;//TODO Messkurce erstellen   

        if(mode_MOVEMENT_OR_FAN1SENSE == MODE_MOVEMENT_OR_FAN1SENSE::MOVEMENT_SENSOR){
            this->movementIsDetected = gpio_get_level(PIN_MOVEMENT_OR_FAN1SENSE);
            this->fan1RotationsRpM=std::numeric_limits<float>::quiet_NaN();
        }else{
            this->movementIsDetected=false;
            this->fan1RotationsRpM=std::numeric_limits<float>::quiet_NaN(); //TODO not implemented
        }
    }

    void SensorLoop()
    {
        int64_t nextOneWireReadout{INT64_MAX};
        int64_t nextBME280Readout{INT64_MAX};
        int64_t nextBH1750Readout{INT64_MAX};
        int64_t nextBinaryAndAnalogReadout{0};
       

        uint32_t oneWireReadoutIntervalMs{800}; //10bit -->187ms Conversion time, 12bit--> 750ms
        uint32_t bme280ReadoutIntervalMs{UINT32_MAX};
        uint32_t bh1750ReadoutIntervalMs{200};

        
        //OneWire
        onewire::M* ds18b20 = new onewire::M();
        ds18b20->Init(PIN_ONEWIRE);
        ds18b20->ResetSearch();
        if(ds18b20->SearchRom()!=ESP_OK){
            ESP_LOGE(TAG, "OneWire: An error occurred searching ROM");
        }else{
            ds18b20->TriggerTemperatureConversion(nullptr);
            nextOneWireReadout = GetMillis64() + oneWireReadoutIntervalMs;
            ESP_LOGI(TAG, "OneWire: DS18B20 successfully initialized.");
        }
        

        //BME280
        BME280 *bme280 = new BME280(I2C_PORT, BME280_ADRESS::PRIM);
        if (bme280->Init(&bme280ReadoutIntervalMs) == ESP_OK)
        {
            bme280->TriggerNextMeasurement();
            bme280ReadoutIntervalMs += 20;
            bme280->TriggerNextMeasurement();
            nextBME280Readout = GetMillis64() + bme280ReadoutIntervalMs;
            ESP_LOGI(TAG, "I2C: BME280 successfully initialized.");
        }
        else
        {
            ESP_LOGW(TAG, "I2C: BME280 not found");
        }

        //BH1750
        BH1750 *bh1750 = new BH1750(I2C_PORT, BH1750_ADRESS::LOW);
        if (bh1750->Init(BH1750_OPERATIONMODE::CONTINU_H_RESOLUTION) == ESP_OK)
        {
            bh1750ReadoutIntervalMs = 200;
            nextBH1750Readout = GetMillis64() + bh1750ReadoutIntervalMs;
            ESP_LOGI(TAG, "I2C: BH1750 successfully initialized.");
        }
        else
        {
            ESP_LOGW(TAG, "I2C: BH1750 not found");
        }

        //CCS811
        ccs811dev = new CCS811::M(I2C_PORT, CCS811::ADDRESS::ADDR0, CCS811::MODE::_1SEC, (gpio_num_t)GPIO_NUM_NC);

        //HDC1080
        hdc1080dev = new hdc1080::M(I2C_PORT);

        //AHT21
        aht21dev = new AHT::M(I2C_PORT, AHT::ADDRESS::default_address);

        while (true)
        {
            if(GetMillis64() > nextBinaryAndAnalogReadout){
                readBinaryAndAnalogIOs();
                nextBinaryAndAnalogReadout = GetMillis64()+100;
                if(!mp3player->IsEmittingSamples()){
                    this->sound=0;
                }
            }

            if (GetMillis64() > nextOneWireReadout)
            {
                ds18b20->GetTemperature(nullptr, &(this->heaterTemperatureDegCel));
                ds18b20->TriggerTemperatureConversion(nullptr);
                nextOneWireReadout = GetMillis64() + oneWireReadoutIntervalMs;
            }
            if (GetMillis64() > nextBME280Readout)
            {
                bme280->GetDataAndTriggerNextMeasurement(&this->airTemperatureDegCel, &this->airPressurePa, &this->airRelHumidityPercent);
                nextBME280Readout = GetMillis64() + bme280ReadoutIntervalMs;
            }
            if (GetMillis64() > nextBH1750Readout)
            {
                bh1750->Read(&(this->ambientBrightnessLux_digital));
                nextBH1750Readout = GetMillis64() + bh1750ReadoutIntervalMs;
            }

            hdc1080dev->Loop(GetMillis64());
            ccs811dev->Loop(GetMillis64());
            aht21dev->Loop(GetMillis64());

            if(ccs811dev->HasValidData()){
                this->airCo2PPM=ccs811dev->Get_eCO2();
            }
            if(this->aht21dev->HasValidData()){
                this->aht21dev->Read(this->airRelHumidityPercent, this->airTemperatureDegCel);
            }else if(this->hdc1080dev->HasValidData()){
                this->hdc1080dev->ReadOut(this->airRelHumidityPercent, this->airTemperatureDegCel);
            }
            vTaskDelay(1);
        }
    }

public:
    HAL_Impl(
        MODE_MOVEMENT_OR_FAN1SENSE mode_MOVEMENT_OR_FAN1SENSE):
        mode_MOVEMENT_OR_FAN1SENSE(mode_MOVEMENT_OR_FAN1SENSE)
    {
    }

    ErrorCode OutputOneLineStatus() override{
        uint32_t heap = esp_get_free_heap_size();
        bool red=GetButtonRedIsPressed();
        bool yel=GetButtonEncoderIsPressed();
        bool grn = GetButtonGreenIsPressed();
        bool mov = IsMovementDetected();
        float htrTemp{0.f};
        int enc{0};
        GetEncoderValue(&enc);
        int32_t sound{0};
        GetSound(&sound);
        float spply = GetUSBCVoltage();

        float bright{0.0};
        GetAmbientBrightness(&bright);

        float co2{0};
        GetHeaterTemperature(&htrTemp);
        float airTemp{0.f};
        GetAirTemperature(&airTemp);
        float airPres{0.f};
        GetAirPressure(&airPres);
        float airHumid{0.f};
        GetAirRelHumidity(&airHumid);
        GetCO2PPM(&co2); 
        float* analogVolt{nullptr};
        GetAnalogInputs(&analogVolt);  
        ESP_LOGI(TAG, "Heap %6lu  RED %d YEL %d GRN %d MOV %d ENC %i SOUND %ld SUPPLY %4.1f BRGHT %4.1f HEAT %4.1f AIRT %4.1f AIRPRS %5.0f AIRHUM %3.0f CO2 %5.0f, ANALOGIN %4.1f",
                         heap,   red,   yel,   grn,   mov,   enc,   sound,    spply,      bright,     htrTemp,   airTemp,   airPres,     airHumid,     co2,       analogVolt[0]);
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
         bool isPressed;
        int16_t val;
        ErrorCode err = this->rotenc->GetValue(val, isPressed)==ESP_OK?ErrorCode::OK:ErrorCode::GENERIC_ERROR;
        *value=val;
        return err;
    }

    ErrorCode GetFan1Rpm(float* rpm){
        *rpm=std::numeric_limits<float>::quiet_NaN();
        return ErrorCode::OK;
    }
    
    ErrorCode SetSound(int32_t soundNumber)
    {
        if (soundNumber<0  || soundNumber >= sizeof(SOUNDS) / sizeof(uint8_t*)){
            soundNumber = 0;
        }
        this->sound=soundNumber;
        mp3player->PlayMP3(SOUNDS[soundNumber], SONGS_LEN[soundNumber], 255, true);
        ESP_LOGI(TAG, "Set Sound to %ld", soundNumber);
        return ErrorCode::OK;
    }

    ErrorCode GetSound(int32_t* soundNumber){
        *soundNumber=this->sound;
        return ErrorCode::OK;
    }

    
    ErrorCode GetCO2PPM(float *co2PPM){
        *co2PPM=this->airCo2PPM;
        return ErrorCode::OK;
    }

    ErrorCode GetHeaterTemperature(float *degreesCelcius)
    {
        *degreesCelcius = this->heaterTemperatureDegCel;
        return ErrorCode::OK;
    }

    ErrorCode GetAirTemperature(float *degreesCelcius)
    {
        *degreesCelcius = this->airTemperatureDegCel;
        return ErrorCode::OK;
    }

    ErrorCode GetAirPressure(float *pa)
    {
        *pa = this->airPressurePa;
        return ErrorCode::OK;
    }

    ErrorCode GetAirQuality(float *qualityPercent)
    {
        *qualityPercent = std::numeric_limits<float>::quiet_NaN();
        return ErrorCode::OK;
    }

    ErrorCode GetAirRelHumidity(float *percent)
    {
        *percent = this->airRelHumidityPercent;
        return ErrorCode::OK;
    }

    ErrorCode GetAirSpeed(float *meterPerSecond)
    {
        *meterPerSecond = this->airSpeedMeterPerSecond;
        return ErrorCode::OK;
    }

    ErrorCode GetAmbientBrightness(float *lux)
    {
        //digital value has priority
        *lux = std::isnan(this->ambientBrightnessLux_digital)?this->ambientBrightnessLux_analog:this->ambientBrightnessLux_digital;
        return ErrorCode::OK;
    }

    ErrorCode GetWifiRssiDb(float *db){
        *db=this->wifiRssiDb;
        return ErrorCode::OK;
    }

    ErrorCode SetAnalogOutput(float volts){

        //mp3player->OutputConstantVoltage(volts);

        return ErrorCode::OK;
    }
    
    //see https://github.com/squix78/esp32-mic-fft/blob/master/esp32-mic-fft.ino
    ErrorCode GetFFT64(float *magnitudes64_param){
        // for (int i = 1; i < 64; i++){
        //     magnitudes64_param[i]=0;
        // }    
        // for(int cnt=0;cnt<average_over_N_measurements;cnt++){
        //     float magnitudes64[64];
        //     size_t num_bytes_read{0};
        //     if (ESP_OK != i2s_read(I2S_PORT_MICROPHONE, samplesI32, SAMPLES_IN_BYTES, &num_bytes_read, portMAX_DELAY)){
        //         ESP_LOGE(TAG, "ESP_OK!=i2s_read");
        //         return ErrorCode::GENERIC_ERROR;
        //     }
        //     if(num_bytes_read!=SAMPLES_IN_BYTES){
        //         ESP_LOGE(TAG, "num_bytes_read!=SAMPLES_IN_BYTES");
        //         return ErrorCode::GENERIC_ERROR;
        //     }
        //     for (int i = 0; i < SAMPLES; i++){
        //         real[i] = samplesI32[i];
        //         imag[i]=0.0;
        //     }
        //     fft.Windowing(FFT_WIN_TYP_HANN, FFT_FORWARD);
        //     fft.Compute(FFT_FORWARD);
        //     fft.ComplexToMagnitude();
        //     size_t resultPointer = 1;
        //     magnitudes64[0]=0;
        //     for (int i = 1; i < 64; i++){
        //         while(resultPointer<BUCKET_INDICES[i]){
        //             //if(magnitudes64[i] > 13*AMPLITUDE){
        //                 magnitudes64[i]=std::max(magnitudes64[i], (float)(real[resultPointer]/AMPLITUDE));
        //             //}
        //             resultPointer++;
        //         }
        //         //magnitudes64_param[i]+=magnitudes64[i];
        //         magnitudes64_param[i] = std::max(magnitudes64_param[i], magnitudes64[i]);
        //     }
        // }
        //uncomment the following lines, if necessary
        //for (int i = 1; i < 64; i++){
            //magnitudes64_param[i]/=average_over_N_measurements;
        //}
        return ErrorCode::OK;
    }

    ErrorCode UpdatePinConfiguration(uint8_t* configMessage, size_t configMessagelen){
        return ErrorCode::OK;
    }

//Annahme: Pins können sowohl digital konfiguriert sein als auch für eine analoge Spannungsmessung zuganglich sein
    ErrorCode UpdatePinConfiguration(
        bool forceReconfiguration,
        MODE_MOVEMENT_OR_FAN1SENSE mode_MOVEMENT_OR_FAN1SENSE){
        
        if(forceReconfiguration || this->mode_MOVEMENT_OR_FAN1SENSE!=mode_MOVEMENT_OR_FAN1SENSE){
            if(mode_MOVEMENT_OR_FAN1SENSE==MODE_MOVEMENT_OR_FAN1SENSE::MOVEMENT_SENSOR){
                //pcnt_unit_stop(speedmeter);
                gpio_reset_pin(PIN_MOVEMENT_OR_FAN1SENSE);
                ConfigGpioInput(PIN_MOVEMENT_OR_FAN1SENSE, GPIO_FLOATING);
            }
            else if(mode_MOVEMENT_OR_FAN1SENSE==MODE_MOVEMENT_OR_FAN1SENSE::FAN1SENSE){
                //pcnt_unit_start(speedmeter);
            }
            this->mode_MOVEMENT_OR_FAN1SENSE=mode_MOVEMENT_OR_FAN1SENSE;
        }
        return ErrorCode::OK;
    }


    ErrorCode InitAndRun() override
    {
        //-------------ADC1 Init---------------//
        
        adc_oneshot_unit_init_cfg_t init_config1 = {};
        init_config1.unit_id = ADC_UNIT_1;
        ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

        //-------------ADC1 Config---------------//
        adc_oneshot_chan_cfg_t config = {};
        config.bitwidth = ADC_BITWIDTH_12;
        config.atten = ADC_ATTEN_DB_0;
        ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, CHANNEL_SWITCHES, &config));
        config.atten = ADC_ATTEN_DB_11;
        ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, CHANNEL_ANALOGIN_OR_ROTB, &config));
        ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, CHANNEL_LDR_OR_ROTA, &config));    
        //-------------ADC1 Calibration Init---------------//
        ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {};
        cali_config.unit_id = init_config1.unit_id;
        cali_config.atten = ADC_ATTEN_DB_11;
        cali_config.bitwidth = ADC_BITWIDTH_DEFAULT;
        ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&cali_config, &this->adc1_cali_handle));

        //Rotary Encoder Input
        rotenc=new cRotaryEncoder(PIN_LDR_OR_ROTA, PIN_ANALOGIN_OR_ROTB, GPIO_NUM_NC);
        ESP_ERROR_CHECK(rotenc->Init());
        ESP_ERROR_CHECK(rotenc->Start());
        
        //Relay K3 output
        gpio_set_level(PIN_K3_ON, 0);
        ConfigGpioOutputPP(PIN_K3_ON);

        //Pulse Counter for RPM of Fan1
        //pcnt_unit_config_t unit_config = {};
        //unit_config.high_limit = 100;
        //unit_config.low_limit = -100;
        
        //ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit));
        //ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit));
        //TODO: Not yet implemented. Do it like the rotary Encoder

    
        //Hinweis (vermutlich schwer zu verstehen...:-()):
        //- beim PIN_FAN1_DRIVE_OR_SERVO1 wird je nach Modus der GPIO vom einem zum anderen Timer umgeschaltet, weil es noch einen anderen FAN/Servo gibt und dessen freuqenz ja nicht beeinflusst werden darf
        //- beim PIN_HEATER_OR_LED_POWER wird je nach Modus die Frequenz des Timers umgeschaltet, weil es nur einen Timer gibt
        

        ESP_ERROR_CHECK(mcpwm_group_set_resolution(MCPWM_UNIT_0, 1000000));
        //MCPWM GPIO
        if(mode_FAN1_OR_SERVO1==MODE_FAN1_OR_SERVO1::FAN1){
            ESP_ERROR_CHECK(mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_IO_FAN1, PIN_FAN1_DRIVE_OR_SERVO1));
        }else if(mode_FAN1_OR_SERVO1==MODE_FAN1_OR_SERVO1::SERVO1){
            ESP_ERROR_CHECK(mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_IO_SERVO1, PIN_FAN1_DRIVE_OR_SERVO1));
        }
        ESP_ERROR_CHECK(mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_IO_SERVO2, PIN_SPI_IO1_OR_SERVO2));       
        ESP_ERROR_CHECK(mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_IO_FAN2,   PIN_FAN2_DRIVE));
        ESP_ERROR_CHECK(mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_IO_HEATER_OR_LED_POWER, PIN_HEATER_OR_LED_POWER));
        ESP_ERROR_CHECK(mcpwm_timer_set_resolution(MCPWM_UNIT_0, MCPWM_TIMER_HEATER_OR_LED_POWER, 10000));

        //MCPWM base config
        mcpwm_config_t pwm_config;
        pwm_config.cmpr_a = 0;     //duty cycle of PWMxA = 0
        pwm_config.cmpr_b = 0;     //duty cycle of PWMxb = 0
        pwm_config.counter_mode = MCPWM_UP_COUNTER;
        pwm_config.duty_mode = MCPWM_DUTY_MODE_0;

        pwm_config.frequency = FREQUENCY_SERVO;
        ESP_ERROR_CHECK(mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_SERVO, &pwm_config));
        pwm_config.frequency = FREQUENCY_FAN;
        ESP_ERROR_CHECK(mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_FAN, &pwm_config));
        pwm_config.frequency = FREQUENCY_HEATER;
        ESP_ERROR_CHECK(mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_HEATER_OR_LED_POWER, &pwm_config));

        //I2C Master
        ESP_ERROR_CHECK(I2C::Init(I2C_PORT, PIN_I2C_SCL, PIN_I2C_SDA));

        //LED Strip
        strip = new RGBLED::M<LED_NUMBER, RGBLED::DeviceType::WS2812>();
        ESP_ERROR_CHECK(strip->Init(VSPI_HOST, PIN_LED_WS2812, 2 ));
        ESP_ERROR_CHECK(strip->Clear(100));

        UpdatePinConfiguration(true, this->mode_MOVEMENT_OR_FAN1SENSE);

        readBinaryAndAnalogIOs();//do this while init to avoid race condition (wifimanager is resettet when red and green buttons are pressed during startup)
        xTaskCreate(sensorTask, "sensorTask", 4096 * 4, this, 6, nullptr);
        xTaskCreate(usbpdTask, "usbpdTask", 2048 * 4, this, 16, nullptr);
        xTaskCreate(mp3Task, "mp3task", 6144 * 4, this, 16, nullptr); //Stack Size = 4096 --> Stack overflow!!
        return ErrorCode::OK;
    }

    ErrorCode BeforeLoop()
    {
        if(this->heaterTemperatureDegCel>85){
            ESP_LOGE(TAG, "Emergency Shutdown. Heater Temperature too high!!!");
            this->SetHeaterDuty(0);
            this->heaterEmergencyShutdown=true;
        }
        return ErrorCode::OK;
    }

    ErrorCode AfterLoop()
    {
        strip->Refresh(100);  //checks internally, whether data is dirty and has to be pushed out
        return ErrorCode::OK;
    }

    ErrorCode StartBuzzer(float freqHz)
    {
        return ErrorCode::PIN_DOES_NOT_SUPPORT_MODE;
        ledc_timer_config_t buzzer_timer;
        buzzer_timer.duty_resolution = LEDC_TIMER_10_BIT; // resolution of PWM duty
        buzzer_timer.freq_hz = freqHz;                    // frequency of PWM signal
        buzzer_timer.speed_mode = LEDC_HIGH_SPEED_MODE;   // timer mode
        buzzer_timer.timer_num = LEDC_TIMER_2;            // timer index
        buzzer_timer.clk_cfg = LEDC_AUTO_CLK;
        ledc_timer_config(&buzzer_timer);
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2, 512);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2);
        return ErrorCode::OK;
    }

    ErrorCode EndBuzzer()
    {
        return ErrorCode::PIN_DOES_NOT_SUPPORT_MODE;
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2, 0);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2);
        return ErrorCode::OK;
    }

    ErrorCode ColorizeLed(uint8_t ledIndex, CRGB colorCRGB)
    {
        if(ledIndex>=LED_NUMBER) return ErrorCode::INDEX_OUT_OF_BOUNDS;
        strip->SetPixel(LED_NUMBER-ledIndex-1, colorCRGB);
        return ErrorCode::OK;
    }

    ErrorCode UnColorizeAllLed()
    {
        strip->Clear(1000);
        return ErrorCode::OK;
    }

    ErrorCode SetRelayState(bool state)
    {
        gpio_set_level(PIN_K3_ON, state);
        return ErrorCode::OK;
    }

    ErrorCode SetHeaterDuty(float dutyInPercent)
    {
        if(this->heaterEmergencyShutdown){
            ESP_ERROR_CHECK(mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_HEATER_OR_LED_POWER, MCPWM_GEN_HEATER_OR_LED_POWER, 0));
            return ErrorCode::EMERGENCY_SHUTDOWN;
        }
        if(mode_HEATER_OR_LED_POWER!=MODE_HEATER_OR_LED_POWER::HEATER){
            ESP_ERROR_CHECK(mcpwm_set_frequency(MCPWM_UNIT_0, MCPWM_TIMER_HEATER_OR_LED_POWER, FREQUENCY_HEATER));
            mode_HEATER_OR_LED_POWER=MODE_HEATER_OR_LED_POWER::LED_POWER;
        }
        dutyInPercent=std::max(0.0f, dutyInPercent);
        dutyInPercent=std::min(100.0f, dutyInPercent);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_HEATER_OR_LED_POWER, MCPWM_GEN_HEATER_OR_LED_POWER, dutyInPercent);
        return ErrorCode::OK;
    }

    float GetHeaterState()
    {
        return mcpwm_get_duty(MCPWM_UNIT_0, MCPWM_TIMER_HEATER_OR_LED_POWER, MCPWM_GEN_HEATER_OR_LED_POWER);
    }

    ErrorCode SetServo1Position(float angle_0_to_180)
    {
        if(mode_FAN1_OR_SERVO1!=MODE_FAN1_OR_SERVO1::SERVO1){
            ESP_ERROR_CHECK(mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_IO_SERVO1, PIN_FAN1_DRIVE_OR_SERVO1));
            mode_FAN1_OR_SERVO1=MODE_FAN1_OR_SERVO1::SERVO1;
        }
        uint32_t cal_pulsewidth = (SERVO_MIN_PULSEWIDTH + (((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * (angle_0_to_180)) / (SERVO_MAX_DEGREE)));
        esp_err_t err =  mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_SERVO, MCPWM_GEN_SERVO1, cal_pulsewidth);
        return err==ESP_OK?ErrorCode::OK:ErrorCode::GENERIC_ERROR;
    }

    ErrorCode SetServo2Position(float angle_0_to_180)
    {
        uint32_t cal_pulsewidth = (SERVO_MIN_PULSEWIDTH + (((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * (angle_0_to_180)) / (SERVO_MAX_DEGREE)));
        esp_err_t err =  mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_SERVO, MCPWM_GEN_SERVO2, cal_pulsewidth);
        return err==ESP_OK?ErrorCode::OK:ErrorCode::GENERIC_ERROR;
    }

    ErrorCode SetFan1Duty(float dutyInPercent)
    {               
        if(mode_FAN1_OR_SERVO1!=MODE_FAN1_OR_SERVO1::FAN1){
            ESP_ERROR_CHECK(mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_IO_FAN1, PIN_FAN1_DRIVE_OR_SERVO1));
            mode_FAN1_OR_SERVO1=MODE_FAN1_OR_SERVO1::FAN1;
        }
        dutyInPercent=std::max(0.0f, dutyInPercent);
        dutyInPercent=std::min(100.0f, dutyInPercent);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_FAN, MCPWM_GEN_FAN1, dutyInPercent);
        return ErrorCode::OK;
    }

    float GetFan1State()
    {
        return mcpwm_get_duty(MCPWM_UNIT_0, MCPWM_TIMER_FAN, MCPWM_GEN_FAN1);
    }

    ErrorCode SetFan2Duty(float dutyInPercent)
    {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_FAN, MCPWM_GEN_FAN2, dutyInPercent);
        return ErrorCode::OK;
    }

    float GetFan2State()
    {
        return mcpwm_get_duty(MCPWM_UNIT_0, MCPWM_TIMER_FAN, MCPWM_GEN_FAN2);
    }

    ErrorCode SetLedPowerWhiteDuty(float dutyInPercent)
    {
        if(mode_HEATER_OR_LED_POWER!=MODE_HEATER_OR_LED_POWER::LED_POWER){
            mcpwm_set_frequency(MCPWM_UNIT_0, MCPWM_TIMER_HEATER_OR_LED_POWER, FREQUENCY_LED);
            mode_HEATER_OR_LED_POWER=MODE_HEATER_OR_LED_POWER::LED_POWER;
        }
        dutyInPercent=std::max(0.0f, dutyInPercent);
        dutyInPercent=std::min(100.0f, dutyInPercent);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_HEATER_OR_LED_POWER, MCPWM_GEN_HEATER_OR_LED_POWER, dutyInPercent);
        return ErrorCode::OK;
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
        return this->movementIsDetected;
    }

    float GetUSBCVoltage(){
        return voltage_USB_PD*0.05f;
    }

    ErrorCode GreetUserOnStartup() override{  
        for(int i=0;i<3;i++)
        {
            ColorizeLed(0, CRGB::DarkRed);
            ColorizeLed(1, CRGB::Yellow);
            ColorizeLed(2, CRGB::DarkGreen);
            ColorizeLed(3, CRGB::DarkBlue);
            AfterLoop();
            vTaskDelay(pdMS_TO_TICKS(150));
            ColorizeLed(0, CRGB::DarkBlue);
            ColorizeLed(1, CRGB::DarkGreen);
            ColorizeLed(2, CRGB::Yellow);
            ColorizeLed(3, CRGB::DarkRed);
            AfterLoop();
            vTaskDelay(pdMS_TO_TICKS(150));
        }    
        SetSound(5);
        UnColorizeAllLed();
        return ErrorCode::OK;
    }

    static void sensorTask(void *pvParameters)
    {
        HAL_Impl *hal = (HAL_Impl *)pvParameters;
        hal->SensorLoop();
    }

    static void mp3Task(void *pvParameters)
    {
        HAL_Impl *hal = (HAL_Impl *)pvParameters;
        hal->MP3Loop();
    }


    static void usbpdTask(void *pvParameters)
    {
    #if CONFIG_FREERTOS_HZ!=1000
        #error "Set CONFIG_FREERTOS_HZ to 1000 as we need a well defined 1ms delay for proper USB-PD protocol handling"
    #endif
        HAL_Impl *hal = (HAL_Impl *)pvParameters;
        hal->USBPDLoop();
    }
};



