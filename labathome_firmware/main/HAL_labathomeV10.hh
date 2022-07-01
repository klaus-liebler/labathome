#pragma once

#include "HAL.hh"

#include <inttypes.h>
#include <limits>
#include <algorithm>
#include <common.hh>
#include <common-esp32.hh>

#include <driver/mcpwm.h>
#include <driver/ledc.h>
#include <driver/adc.h>
#include <driver/i2c.h>
#include <driver/rmt.h>
#include <driver/i2s.h>
#include <arduinoFFT.h>

#include <errorcodes.hh>
#include <ws2812.hh>
#include <bh1750.hh>
#include <ms4525.hh>
#include <bme280.hh>
#include <ads1115.hh>
#include <ccs811.hh>
#include <hdc1080.hh>
#include <aht_sensor.hh>
#include <owb.h>
#include <owb_rmt.h>
#include <ds18b20.h>
#include <rotenc.hh>
#include <i2c.hh>
#include <MP3Player.hh>
#include <PD_UFP.h>

#include "winfactboris_messages.hh"

FLASH_FILE(fanfare_mp3)
FLASH_FILE(ready_mp3)
FLASH_FILE(alarm14heulen_mp3)


const uint8_t *SOUNDS[] = {fanfare_mp3_start, ready_mp3_start, alarm14heulen_mp3_start};
const size_t SONGS_LEN[] = {fanfare_mp3_size, ready_mp3_size, alarm14heulen_mp3_size};


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
constexpr adc1_channel_t PIN_ANALOGIN_OR_ROTB_CHANNEL{ADC1_CHANNEL_6};
//36=VP, 39=VN
constexpr Pintype PIN_MOVEMENT_OR_FAN1SENSE = (Pintype)35;
constexpr Pintype PIN_SW = (Pintype)36;
constexpr adc1_channel_t PIN_SW_CHANNEL{ADC1_CHANNEL_0};
constexpr Pintype PIN_LDR_OR_ROTA = (Pintype)39;
constexpr adc1_channel_t PIN_LDR_OR_ROTA_CHANNEL{ADC1_CHANNEL_3};


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

enum class MODE_ROT_LDR_ANALOGIN:uint8_t{
    ROT=1,
    LDR=2,
    ANALOGIN=3,
    LDR_AND_ANALOGIN=4,
};

enum class MODE_MOVEMENT_OR_FAN1SENSE:uint8_t{
    MOVEMENT_SENSOR=1,
    FAN1SENSE=2,
};

enum class MODE_FAN1_DRIVE_OR_SERVO1:uint8_t{
    FAN1_DRIVE=1,
    SERVO1=2,
};



enum class Button : uint8_t
{
    BUT_ENCODER = 1,
    BUT_RED = 0,
    BUT_GREEN = 2,
};
constexpr size_t ANALOG_INPUTS_LEN{1};
constexpr size_t LED_NUMBER{4};
constexpr rmt_channel_t CHANNEL_ONEWIRE_TX{RMT_CHANNEL_1};
constexpr rmt_channel_t CHANNEL_ONEWIRE_RX{RMT_CHANNEL_2};
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

constexpr i2s_port_t I2S_PORT_MICROPHONE{I2S_NUM_1};
constexpr i2s_port_t I2S_PORT_LOUDSPEAKER{I2S_NUM_0};//must be I2S_NUM_0, as only this hat access to internal DAC

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

MP3::Player mp3player;


class HAL_labathome : public HAL
{
private:
    //config
    MODE_ROT_LDR_ANALOGIN mode_ROT_LDR_ANALOGIN;
    MODE_MOVEMENT_OR_FAN1SENSE mode_MOVEMENT_OR_FAN1SENSE;
    MODE_HEATER_OR_LED_POWER mode_HEATER_OR_LED_POWER;//Heater mit 1Hz, LED mit 300Hz
    MODE_FAN1_DRIVE_OR_SERVO1 mode_FAN1_DRIVE_OR_SERVO1; //FAN1 drive mit 100Hz, servo mit 50Hz

    //various
    esp_adc_cal_characteristics_t *adc_chars{nullptr};
    
    //management objects
    WS2812_Strip<LED_NUMBER> *strip{nullptr};
    cRotaryEncoder *rotenc{nullptr};
    pcnt_unit_handle_t speedmeter{nullptr};
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
        //sizeof(MP3::Player);64byte
        if(MD8002_ACTIVE){
            mp3player.InitInternalDACMonoRightPin25();
        }else if(NS4168_ACTIVE){
            mp3player.InitExternalI2SDAC(I2S_PORT_LOUDSPEAKER, PIN_NS4168_SCK, PIN_NS4168_WS, PIN_NS4168_SDAT);
        }
        
        while(true){
            mp3player.Loop();
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
        
        int adc_reading = adc1_get_raw(PIN_SW_CHANNEL);
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

        wifi_ap_record_t ap_info;
        esp_wifi_sta_get_ap_info(&ap_info);
        this->wifiRssiDb=ap_info.rssi;

        if(mode_ROT_LDR_ANALOGIN == MODE_ROT_LDR_ANALOGIN::ANALOGIN || mode_ROT_LDR_ANALOGIN==MODE_ROT_LDR_ANALOGIN::LDR_AND_ANALOGIN){
            //Es wird nicht unterschieden, ob der eingang "nur" zum Messen von analogen Spannung verwendet wird oder ob es sich um den Trigger-Eingang des Zeitrelais handelt. Im zweiten Fall muss einfach eine Zeitrelais-Schaltung basierend auf der Grenzüberschreitung des Analogen Messwertes in der Funktionsblock-Spache realisiert werden
            int adc_reading = adc1_get_raw(PIN_ANALOGIN_OR_ROTB_CHANNEL);
            this->AnalogInputs[0] = 11*esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);//die Multiplikation mit 11 wegen dem Spannungsteiler
        }else{
            this->AnalogInputs[0]=std::numeric_limits<float>::quiet_NaN();
        }

        if(mode_ROT_LDR_ANALOGIN == MODE_ROT_LDR_ANALOGIN::LDR || mode_ROT_LDR_ANALOGIN==MODE_ROT_LDR_ANALOGIN::LDR_AND_ANALOGIN){
            int adc_reading = adc1_get_raw(PIN_LDR_OR_ROTA_CHANNEL);
            this->ambientBrightnessLux_analog = 11*esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);//die Multiplikation mit 11 wegen dem Spannungsteiler
        }else{
            this->ambientBrightnessLux_analog=std::numeric_limits<float>::quiet_NaN();
        }

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
        TickType_t nextADS1115Readout{UINT32_MAX};
        uint16_t nextADS1115Mux{0b100}; //100...111

        uint32_t oneWireReadoutIntervalMs{800}; //10bit -->187ms Conversion time, 12bit--> 750ms
        uint32_t bme280ReadoutIntervalMs{UINT32_MAX};
        uint32_t bh1750ReadoutIntervalMs{200};
        TickType_t ads1115ReadoutInterval{portMAX_DELAY};

        
        //OneWire
        DS18B20_Info *ds18b20_info = NULL;
        owb_rmt_driver_info rmt_driver_info;
        OneWireBus *owb = owb_rmt_initialize(&rmt_driver_info, PIN_ONEWIRE, CHANNEL_ONEWIRE_TX, CHANNEL_ONEWIRE_RX);
        owb_use_crc(owb, true); // enable CRC check for ROM code
        // Find all connected devices
        OneWireBus_ROMCode rom_code;
        owb_status status = owb_read_rom(owb, &rom_code);
        if (status == OWB_STATUS_OK)
        {
            ds18b20_info = ds18b20_malloc();                                 // heap allocation
            ds18b20_init_solo(ds18b20_info, owb);                            // only one device on bus
            ds18b20_use_crc(ds18b20_info, true);                             // enable CRC check on all reads
            ds18b20_set_resolution(ds18b20_info, DS18B20_RESOLUTION_12_BIT); 
            ds18b20_convert_all(owb);
            nextOneWireReadout = GetMillis() + oneWireReadoutIntervalMs;
            ESP_LOGI(TAG, "OneWire: DS18B20 successfully initialized.");
        }
        else
        {
            ESP_LOGE(TAG, "OneWire: An error occurred reading ROM code: %d", status);
        }

        //BME280
        BME280 *bme280 = new BME280(I2C_PORT, BME280_ADRESS::PRIM);
        if (bme280->Init(&bme280ReadoutIntervalMs) == ESP_OK)
        {
            bme280->TriggerNextMeasurement();
            bme280ReadoutIntervalMs += 20;
            bme280->TriggerNextMeasurement();
            nextBME280Readout = GetMillis() + bme280ReadoutIntervalMs;
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
            nextBH1750Readout = GetMillis() + bh1750ReadoutIntervalMs;
            ESP_LOGI(TAG, "I2C: BH1750 successfully initialized.");
        }
        else
        {
            ESP_LOGW(TAG, "I2C: BH1750 not found");
        }

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
                if(!mp3player.IsEmittingSamples()){
                    this->sound=0;
                }
            }

            if (GetMillis64() > nextOneWireReadout)
            {
                ds18b20_read_temp(ds18b20_info, &(this->heaterTemperatureDegCel));
                ds18b20_convert_all(owb);
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

            if (xTaskGetTickCount() >= nextADS1115Readout)
            {
                ads1115->GetVoltage(&AnalogInputs[nextADS1115Mux & 0b11]);
                nextADS1115Mux++;
                if (nextADS1115Mux > 0b111)
                    nextADS1115Mux = 0b100;
                ads1115->TriggerMeasurement((ads1115_mux_t)nextADS1115Mux);
                nextADS1115Readout = xTaskGetTickCount() + ads1115ReadoutInterval;
            }
            vTaskDelay(1);
        }
    }


public:
    HAL_labathome(
        MODE_ROT_LDR_ANALOGIN mode_ROT_LDR_ANALOGIN, 
        MODE_MOVEMENT_OR_FAN1SENSE mode_MOVEMENT_OR_FAN1SENSE,
        MODE_HEATER_OR_LED_POWER mode_HEATER_OR_LED_POWER,//Heater mit 1Hz, LED mit 300Hz
        MODE_FAN1_DRIVE_OR_SERVO1 mode_FAN1_DRIVE_OR_SERVO1) //FAN1 drive mit 100Hz, servo mit 50Hz):ioConfig(defaultConfig)
        :
        mode_ROT_LDR_ANALOGIN(mode_ROT_LDR_ANALOGIN),
        mode_MOVEMENT_OR_FAN1SENSE(mode_MOVEMENT_OR_FAN1SENSE),
        mode_HEATER_OR_LED_POWER(mode_HEATER_OR_LED_POWER),
        mode_FAN1_DRIVE_OR_SERVO1(mode_FAN1_DRIVE_OR_SERVO1)
    {
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
        return this->rotenc->GetValue(value)==ESP_OK?ErrorCode::OK:ErrorCode::GENERIC_ERROR;
    }

    ErrorCode GetFan1Rpm(float* rpm){
        *rpm=std::numeric_limits<float>::quiet_NaN();
        return ErrorCode::OK;
    }
    
    ErrorCode SetSound(int32_t soundNumber)
    {
        if (soundNumber >= sizeof(SOUNDS) / sizeof(uint8_t*))
            soundNumber = 0;
        this->sound=soundNumber;
        mp3player.Play(SOUNDS[soundNumber], SONGS_LEN[soundNumber]);
        ESP_LOGI(TAG, "Set Sound to %d", soundNumber);
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
        mp3player.OutputConstantVoltage(volts);
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
        if(configMessagelen!=sizeof(MessageConfig)){
            ESP_LOGE(TAG, "configMessagelen %d!=sizeof(MessageConfig)", configMessagelen);
            return ErrorCode::INDEX_OUT_OF_BOUNDS;
        }
        MessageConfig* cfg = (MessageConfig*)configMessage;
        UpdatePinConfiguration(
            false,
            (MODE_ROT_LDR_ANALOGIN)cfg->mode_ROT_LDR_ANALOGIN,
            (MODE_MOVEMENT_OR_FAN1SENSE)cfg->mode_MOVEMENT_OR_FAN1SENSE,
            (MODE_HEATER_OR_LED_POWER)cfg->mode_HEATER_OR_LED_POWER,
            (MODE_FAN1_DRIVE_OR_SERVO1)cfg->mode_FAN1_DRIVE_OR_SERVO1
        );
        return ErrorCode::OK;
    }

//Annahme: Pins können sowohl digital konfiguriert sein als auch für eine analoge Spannungsmessung zuganglich sein
    ErrorCode UpdatePinConfiguration(
        bool forceReconfiguration,
        MODE_ROT_LDR_ANALOGIN mode_ROT_LDR_ANALOGIN, 
        MODE_MOVEMENT_OR_FAN1SENSE mode_MOVEMENT_OR_FAN1SENSE,
        MODE_HEATER_OR_LED_POWER mode_HEATER_OR_LED_POWER,//Heater mit 1Hz, LED mit 300Hz
        MODE_FAN1_DRIVE_OR_SERVO1 mode_FAN1_DRIVE_OR_SERVO1){
        
        if(forceReconfiguration || this->mode_HEATER_OR_LED_POWER!=mode_HEATER_OR_LED_POWER){
            if(mode_HEATER_OR_LED_POWER==MODE_HEATER_OR_LED_POWER::HEATER){
                mcpwm_set_frequency(MCPWM_UNIT_0, MCPWM_TIMER_HEATER_OR_LED_POWER, 1);
            }
            else{
                mcpwm_set_frequency(MCPWM_UNIT_0, MCPWM_TIMER_HEATER_OR_LED_POWER, 300);
            }
            this->mode_HEATER_OR_LED_POWER=mode_HEATER_OR_LED_POWER;
        }
        
        /*
        if(forceReconfiguration || this->mode_ROT_LDR_ANALOGIN ANALOGIN_OR_ROTB!=mode_K3A1_OR_ANALOGIN_OR_ROTB){
            if(mode_K3A1_OR_ANALOGIN_OR_ROTB==MODE_K3A1_OR_ANALOGIN_OR_ROTB::K3A1){
                rotenc->Stop();
                adc1_config_channel_atten(PIN_ANALOGIN_OR_ROTB_CHANNEL, ADC_ATTEN_DB_0);
            }
            else if(mode_K3A1_OR_ANALOGIN_OR_ROTB==MODE_K3A1_OR_ANALOGIN_OR_ROTB::ANALOGIN){
                rotenc->Stop();
                adc1_config_channel_atten(PIN_ANALOGIN_OR_ROTB_CHANNEL, ADC_ATTEN_DB_11);
            }
            else if (mode_K3A1_OR_ANALOGIN_OR_ROTB==MODE_K3A1_OR_ANALOGIN_OR_ROTB::ROTB){
                rotaryChanged=true;
                //see below
            }
            this->mode_K3A1_OR_ANALOGIN_OR_ROTB=mode_K3A1_OR_ANALOGIN_OR_ROTB;
        }

        if(forceReconfiguration || this->mode_LDR_OR_ROTA != mode_LDR_OR_ROTA){
            if(mode_LDR_OR_ROTA==MODE_LDR_OR_ROTA::LDR){
                rotenc->Stop();
                adc1_config_channel_atten(PIN_LDR_OR_ROTA_CHANNEL, ADC_ATTEN_DB_11);//TODO check the measuring interval
            }
            else if(mode_LDR_OR_ROTA==MODE_LDR_OR_ROTA::ROTA){
                rotaryChanged=true;
                //see below
            }
            this->mode_LDR_OR_ROTA=mode_LDR_OR_ROTA;
        }

        if(forceReconfiguration || (rotaryChanged && mode_K3A1_OR_ANALOGIN_OR_ROTB==MODE_K3A1_OR_ANALOGIN_OR_ROTB::ROTB && mode_LDR_OR_ROTA==MODE_LDR_OR_ROTA::ROTA)){
            rotenc->Start();
        }
        */

        if(forceReconfiguration || this->mode_MOVEMENT_OR_FAN1SENSE!=mode_MOVEMENT_OR_FAN1SENSE){
            if(mode_MOVEMENT_OR_FAN1SENSE==MODE_MOVEMENT_OR_FAN1SENSE::MOVEMENT_SENSOR){
                pcnt_unit_stop(speedmeter);
                gpio_reset_pin(PIN_MOVEMENT_OR_FAN1SENSE);
                esp32::ConfigGpioInput(PIN_MOVEMENT_OR_FAN1SENSE, GPIO_FLOATING);
            }
            else if(mode_MOVEMENT_OR_FAN1SENSE==MODE_MOVEMENT_OR_FAN1SENSE::FAN1SENSE){
                pcnt_unit_start(speedmeter);
            }
            this->mode_MOVEMENT_OR_FAN1SENSE=mode_MOVEMENT_OR_FAN1SENSE;
        }

        if(forceReconfiguration || this->mode_FAN1_DRIVE_OR_SERVO1 != mode_FAN1_DRIVE_OR_SERVO1){
            if(mode_FAN1_DRIVE_OR_SERVO1==MODE_FAN1_DRIVE_OR_SERVO1::FAN1_DRIVE){
                ESP_ERROR_CHECK(mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_IO_FAN1, PIN_FAN1_DRIVE_OR_SERVO1));
            }
            else if(mode_FAN1_DRIVE_OR_SERVO1==MODE_FAN1_DRIVE_OR_SERVO1::SERVO1){
                ESP_ERROR_CHECK(mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_IO_SERVO1, PIN_FAN1_DRIVE_OR_SERVO1));
            }
            this->mode_FAN1_DRIVE_OR_SERVO1 = mode_FAN1_DRIVE_OR_SERVO1;
        }


           
        return ErrorCode::OK;
    }


    ErrorCode InitAndRun()
    {
        // i2s config for reading from left channel of I2S - this is standard for microphones
        i2s_config_t i2sMemsConfigLeftChannel = {};
        i2sMemsConfigLeftChannel.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX);
        i2sMemsConfigLeftChannel.sample_rate = SAMPLE_RATE_MICROPHONE;
        i2sMemsConfigLeftChannel.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT;
        i2sMemsConfigLeftChannel.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
        i2sMemsConfigLeftChannel.communication_format = I2S_COMM_FORMAT_STAND_I2S;
        i2sMemsConfigLeftChannel.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
        i2sMemsConfigLeftChannel.dma_desc_num = 8;
        i2sMemsConfigLeftChannel.dma_frame_num = 64;
        i2sMemsConfigLeftChannel.use_apll = false;
        i2sMemsConfigLeftChannel.tx_desc_auto_clear = false;
        i2sMemsConfigLeftChannel.fixed_mclk = 0;
        i2s_pin_config_t i2sPins = {};
        i2sPins.bck_io_num = PIN_I2S_SCK;
        i2sPins.ws_io_num = PIN_I2S_WS;
        i2sPins.data_out_num = I2S_PIN_NO_CHANGE;
        i2sPins.data_in_num = PIN_I2S_SD;
        i2s_driver_install(I2S_PORT_MICROPHONE, &i2sMemsConfigLeftChannel, 0, NULL);
        i2s_set_pin(I2S_PORT_MICROPHONE, &i2sPins);


        //Rotary Encoder Input
        rotenc=new cRotaryEncoder(PIN_LDR_OR_ROTA, PIN_ANALOGIN_OR_ROTB, -100, 100);
        rotenc->Init();
        rotenc->Start();



        //Relay K3 output
        gpio_set_level(PIN_K3_ON, 0);
        esp32::ConfigGpioOutputPP(PIN_K3_ON);

        //Configure Analog
        adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));
        esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_0db, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(PIN_SW_CHANNEL, ADC_ATTEN_DB_0);
        adc1_config_channel_atten(PIN_ANALOGIN_OR_ROTB_CHANNEL, ADC_ATTEN_DB_11);
        adc1_config_channel_atten(PIN_LDR_OR_ROTA_CHANNEL, ADC_ATTEN_DB_11);//TODO check the measuring interval

        //Pulse Counter for RPM of Fan1
        //pcnt_unit_config_t unit_config = {};
        //unit_config.high_limit = 100;
        //unit_config.low_limit = -100;
        
        //ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit));
        //ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit));
        //TODO: Not yet implemented. Do it like the rotary Encoder

            
        //MCPWM for Servos
        mcpwm_config_t pwm_config;
        pwm_config.frequency = 50; //frequency = 50Hz, i.e. for every servo motor time period should be 20ms
        pwm_config.cmpr_a = 0;     //duty cycle of PWMxA = 0
        pwm_config.cmpr_b = 0;     //duty cycle of PWMxb = 0
        pwm_config.counter_mode = MCPWM_UP_COUNTER;
        pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
        ESP_ERROR_CHECK(mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_SERVO, &pwm_config));
        mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_IO_SERVO2, PIN_SPI_IO1_OR_SERVO2);

        //MCPWM for Fans (same settings as servo)
        ESP_ERROR_CHECK(mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_FAN, &pwm_config));
        
        //MCPWM for Heater and PowerLED
        pwm_config.frequency = 1;
        ESP_ERROR_CHECK(mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_HEATER_OR_LED_POWER, &pwm_config));

        //FAN2Drive
        ESP_ERROR_CHECK(mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_IO_FAN2, PIN_FAN2_DRIVE));

        //I2C Master
        ESP_ERROR_CHECK(I2C::Init(I2C_PORT, PIN_I2C_SCL, PIN_I2C_SDA));

        //LED Strip
        strip = new WS2812_Strip<LED_NUMBER>();
        ESP_ERROR_CHECK(strip->Init(VSPI_HOST, PIN_LED_WS2812, 2 ));
        ESP_ERROR_CHECK(strip->Clear(100));

        UpdatePinConfiguration(
            true, 
            this->mode_ROT_LDR_ANALOGIN,
            this->mode_MOVEMENT_OR_FAN1SENSE,
            this->mode_HEATER_OR_LED_POWER,
            this->mode_FAN1_DRIVE_OR_SERVO1);

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

    ErrorCode ColorizeLed(LED led, uint32_t color)
    {
        CRGB colorCRGB(color);
        strip->SetPixel((uint8_t)led, colorCRGB);
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
        if(this->heaterEmergencyShutdown) return ErrorCode::EMERGENCY_SHUTDOWN;
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_OPR_A, dutyInPercent);
        return ErrorCode::OK;
    }

    float GetHeaterState()
    {
        return mcpwm_get_duty(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_OPR_A);
    }

    ErrorCode SetFan1Duty(float dutyInPercent)
    {
        if(mode_FAN1_DRIVE_OR_SERVO1!=MODE_FAN1_DRIVE_OR_SERVO1::FAN1_DRIVE){
            return ErrorCode::FUNCTION_NOT_AVAILABLE;
        }
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
            return ErrorCode::FUNCTION_NOT_AVAILABLE;
        }
        if (dutyInPercent > 100)
            dutyInPercent = 100;

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

    ErrorCode SetServo1Position(float angle_0_to_180)
    {
        if(this->mode_FAN1_DRIVE_OR_SERVO1 != MODE_FAN1_DRIVE_OR_SERVO1::SERVO1){
            return ErrorCode::FUNCTION_NOT_AVAILABLE;
        }
        uint32_t cal_pulsewidth = (SERVO_MIN_PULSEWIDTH + (((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * (angle_0_to_180)) / (SERVO_MAX_DEGREE)));
        esp_err_t err =  mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, cal_pulsewidth);
        return err==ESP_OK?ErrorCode::OK:ErrorCode::GENERIC_ERROR;
    }

    ErrorCode SetServo2Position(float angle_0_to_180)
    {
        uint32_t cal_pulsewidth = (SERVO_MIN_PULSEWIDTH + (((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * (angle_0_to_180)) / (SERVO_MAX_DEGREE)));
        esp_err_t err =  mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, cal_pulsewidth);
        return err==ESP_OK?ErrorCode::OK:ErrorCode::GENERIC_ERROR;
    }

    float GetUSBCVoltage(){
        return voltage_USB_PD*0.05f;
    }

    static void sensorTask(void *pvParameters)
    {
        HAL_labathome *hal = (HAL_labathome *)pvParameters;
        hal->SensorLoop();
    }

    static void mp3Task(void *pvParameters)
    {
        HAL_labathome *hal = (HAL_labathome *)pvParameters;
        hal->MP3Loop();
    }

    static void usbpdTask(void *pvParameters)
    {
    #if CONFIG_FREERTOS_HZ!=1000
        #error "Set CONFIG_FREERTOS_HZ to 1000 as we need a well defined 1ms delay for proper USB-PD protocol handling"
    #endif
        HAL_labathome *hal = (HAL_labathome *)pvParameters;
        hal->USBPDLoop();
    }
};



