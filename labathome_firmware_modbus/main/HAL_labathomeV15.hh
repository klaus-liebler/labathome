#pragma once

#define LABATHOME_V15
#include "HAL.hh"

#include <inttypes.h>
#include <limits>
#include <algorithm>
#include <common.hh>
#include <common-esp32.hh>


#include <driver/i2c_master.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>

#include <sys/unistd.h>
#include <sys/stat.h>
#include <esp_vfs_fat.h>
#include "sdmmc_cmd.h"
#include <driver/sdmmc_host.h>


#include <errorcodes.hh>
#include <rgbled.hh>
#include <bme280.hh>
#include <bh1750.hh>
#include <ccs811.hh>
#include <vl53l0x.hh>
#include <aht_sensor.hh>
#include <ds18b20.hh>
#include <AudioPlayer.hh>

#include "../../labathome_firmware_stm32arduino/src/stm32_esp32_communication.hh"


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

constexpr Pintype PIN_BTN_GREEN = (Pintype)0;

constexpr Pintype PIN_CANTX = (Pintype)1;
constexpr Pintype PIN_CANRX = (Pintype)2;

constexpr Pintype PIN_RS485_DI = (Pintype)40;
constexpr Pintype PIN_RS485_DE = (Pintype)41;
constexpr Pintype PIN_RS485_RO = (Pintype)42;

constexpr Pintype PIN_EXT_CS = (Pintype)3;
constexpr Pintype PIN_EXT_MISO = (Pintype)9;
constexpr Pintype PIN_EXT_CLK = (Pintype)10;
constexpr Pintype PIN_EXT_IO1 = (Pintype)11;
constexpr Pintype PIN_EXT_IO2 = (Pintype)12;
constexpr Pintype PIN_EXT_MOSI = (Pintype)46;

constexpr Pintype PIN_I2C_SDA = (Pintype)4;
constexpr Pintype PIN_I2C_SCL = (Pintype)5;
constexpr Pintype PIN_I2C_IRQ = (Pintype)6;

constexpr Pintype PIN_uSD_CMD = (Pintype)7;
constexpr Pintype PIN_uSD_CLK = (Pintype)15;
constexpr Pintype PIN_uSD_D0 = (Pintype)16;

constexpr Pintype PIN_LCD_DC = (Pintype)8;
constexpr Pintype PIN_LCD_CLK = (Pintype)17;
constexpr Pintype PIN_LCD_DAT = (Pintype)18;
constexpr Pintype PIN_LCD_BL = (Pintype)38;

constexpr Pintype PIN_TXD0 = (Pintype)43;
constexpr Pintype PIN_RXD0 = (Pintype)44;

constexpr Pintype PIN_I2S_MCLK = (Pintype)14;
constexpr Pintype PIN_I2S_FS = (Pintype)21;
constexpr Pintype PIN_I2S_DAC = (Pintype)45;
constexpr Pintype PIN_I2S_ADC = (Pintype)47;
constexpr Pintype PIN_I2S_BCLK = (Pintype)48;

constexpr Pintype PIN_LED_WS2812 = (Pintype)13;

constexpr Pintype PIN_ONEWIRE = (Pintype)39;

constexpr size_t ANALOG_INPUTS_LEN{2};
constexpr size_t LED_NUMBER{4};

constexpr i2c_port_t I2C_PORT{I2C_NUM_0};
constexpr i2s_port_t I2S_PORT{I2S_NUM_0};//must be I2S_NUM_0, as only this hat access to internal DAC

constexpr const char* MOUNT_POINT="/sdcard";



class HAL_Impl : public HAL
{
private:
    //management objects

    i2c_master_bus_handle_t i2c_master_handle;
    AHT::M *aht21dev{nullptr};
    BH1750::M* bh1750dev{nullptr};
    CCS811::M* ccs811dev{nullptr};
    BME280::M* bme280dev{nullptr};
    VL53L0X::M* vl53l0xdev{nullptr};
    OneWire::OneWireBus<PIN_ONEWIRE>* oneWireBus{nullptr};
    i2c_master_dev_handle_t stm32_handle{nullptr};

    RGBLED::M<LED_NUMBER, RGBLED::DeviceType::WS2812> *strip{nullptr};
    AudioPlayer::Player *mp3player{nullptr};
    //spilcd16::M<SPI2_HOST, PIN_LCD_DAT, PIN_LCD_CLK, GPIO_NUM_NC, PIN_LCD_DC, PIN_EXT_IO1, GPIO_NUM_NC, LCD240x240_0, (size_t)8*240, 4096, 0> display;


    

    //SensorValues
    uint8_t stm2esp_buf[S2E::SIZE]={0};

    float analogInputsVolt[ANALOG_INPUTS_LEN]={0};
    float wifiRssiDb{std::numeric_limits<float>::quiet_NaN()};
#if 0
    float ambientBrightnessLux_analog{std::numeric_limits<float>::quiet_NaN()};
    uint16_t ambientBrightnessLux_digital{std::numeric_limits<float>::quiet_NaN()};
    float heaterTemperatureDegCel{std::numeric_limits<float>::quiet_NaN()};
    float airTemperatureDegCel{std::numeric_limits<float>::quiet_NaN()};
    float airPressurePa{std::numeric_limits<float>::quiet_NaN()};
    float airRelHumidityPercent{std::numeric_limits<float>::quiet_NaN()};
    float airCo2PPM{std::numeric_limits<float>::quiet_NaN()};
    float airQualityPercent{std::numeric_limits<float>::quiet_NaN()};
    float airSpeedMeterPerSecond{std::numeric_limits<float>::quiet_NaN()};
#endif

    //Actor Values
    uint8_t esp2stm_buf[E2S::SIZE]={0};
    uint32_t sound{0};
    
    //Safety
    bool heaterEmergencyShutdown{false};

     
    void MP3Loop(){
      
        while(mp3player){
            mp3player->Loop();
        }
    }

    void Stm32Init(){
        if(i2c_master_probe(i2c_master_handle, I2C_SETUP::STM32_I2C_ADDRESS, 1000)!=ESP_OK){
                
            ESP_LOGW(TAG, "STM32 I2C not responding");
        }
            
        i2c_device_config_t dev_cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = I2C_SETUP::STM32_I2C_ADDRESS,
            .scl_speed_hz = 100000,
            .scl_wait_us=0,
            .flags={
                .disable_ack_check=0,
            },
        };
        ESP_ERROR_CHECK(i2c_master_bus_add_device(this->i2c_master_handle, &dev_cfg, &this->stm32_handle));
    }
    
    void Stm32Loop(){
        if(i2c_master_transmit_receive(this->stm32_handle, esp2stm_buf, E2S::SIZE, stm2esp_buf, S2E::SIZE, 1000)!=ESP_OK){
            ESP_LOGW(TAG, "Error while exchanging data with STM32 slave chip on address %d", I2C_SETUP::STM32_I2C_ADDRESS);
        }
    }

    void SensorLoop()
    {
        aht21dev = new AHT::M(i2c_master_handle, AHT::ADDRESS::DEFAULT_ADDRESS);
        bme280dev = new BME280::M(i2c_master_handle, BME280::ADDRESS::PRIM);
        bh1750dev = new BH1750::M(i2c_master_handle, BH1750::ADDRESS::LOW, BH1750::OPERATIONMODE::CONTINU_H_RESOLUTION);
        ccs811dev = new CCS811::M(i2c_master_handle, CCS811::ADDRESS::ADDR0, CCS811::MODE::_1SEC, (gpio_num_t)GPIO_NUM_NC);
        aht21dev = new AHT::M(i2c_master_handle, AHT::ADDRESS::DEFAULT_ADDRESS);
        vl53l0xdev = new VL53L0X::M(i2c_master_handle);
        oneWireBus = new OneWire::OneWireBus<PIN_ONEWIRE>();
        Stm32Init(); //see below loop
 
        while (true)
        {
            int64_t now = GetMillis64();
            oneWireBus->Loop(now);
            bh1750dev->Loop(GetMillis64());
            ccs811dev->Loop(GetMillis64());
            aht21dev->Loop(GetMillis64());
            vl53l0xdev->Loop(GetMillis64());
            Stm32Loop(); //see above Init;
            delayMs(50);
        }
    }


public:
    HAL_Impl(){}

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
        this->analogInputsVolt[0] = ParseU16(this->stm2esp_buf, S2E::ADC0_POS);
        this->analogInputsVolt[1] = ParseU16(this->stm2esp_buf, S2E::ADC1_POS);
        *voltages = this->analogInputsVolt;
        return ErrorCode::OK;
    }

    ErrorCode GetEncoderValue(int *value){
        ParseU16(this->stm2esp_buf, S2E::ROTENC_POS);
        return ErrorCode::OK;
    }

    ErrorCode SetSound(int32_t soundNumber)
    {
        if(!mp3player){
            ESP_LOGW(TAG, "Audio Player not initialized!");
            return ErrorCode::OK;
        }
        if (soundNumber<0  || soundNumber >= sizeof(SOUNDS) / sizeof(uint8_t*)){
            soundNumber = 0;
        }
        this->sound=soundNumber;
        ESP_LOGI(TAG, "Set Sound to %ld", soundNumber);
        
        mp3player->PlayMP3(SOUNDS[soundNumber], SONGS_LEN[soundNumber], 255, true);
        return ErrorCode::OK;
    }

    ErrorCode GetSound(int32_t* soundNumber){
        *soundNumber=this->sound;
        return ErrorCode::OK;
    }

    ErrorCode GetCO2PPM(float *co2PPM){
        *co2PPM=std::numeric_limits<float>::quiet_NaN();
        return ErrorCode::OK;
    }

    ErrorCode GetHeaterTemperature(float *degreesCelcius)
    {
        if(!this->oneWireBus){
            return ErrorCode::GENERIC_ERROR;
        }
        *degreesCelcius = this->oneWireBus->GetMaxTemp();
        return ErrorCode::OK;
    }

    ErrorCode GetAirTemperature(float *degreesCelcius)
    {
        if(!this->aht21dev){
            return ErrorCode::GENERIC_ERROR;
        }
        float dummyHumid;
        return this->aht21dev->Read(dummyHumid, *degreesCelcius);
    }

    ErrorCode GetAirPressure(float *pa)
    {
        *pa = std::numeric_limits<float>::quiet_NaN();
        return ErrorCode::OK;
    }

    ErrorCode GetAirQuality(float *qualityPercent)
    {
        *qualityPercent = std::numeric_limits<float>::quiet_NaN();
        return ErrorCode::OK;
    }

    ErrorCode GetAirRelHumidity(float *percent)
    {
        if(!this->aht21dev){
            return ErrorCode::GENERIC_ERROR;
        }
        float dummyTemp;
        return this->aht21dev->Read(*percent, dummyTemp);
    }

    ErrorCode GetAirSpeed(float *meterPerSecond)
    {
        *meterPerSecond = std::numeric_limits<float>::quiet_NaN();
        return ErrorCode::OK;
    }

    ErrorCode GetAmbientBrightness(float *lux) override
    {
        uint16_t temp;
        //digital value has priority
        if(bh1750dev->HasValidData()){
            bh1750dev->Read(temp);
        }else{
            temp=ParseU16(stm2esp_buf, S2E::BRIGHTNESS_POS);
        }
        *lux=temp;
        return ErrorCode::OK;
    }

    ErrorCode GetWifiRssiDb(float *db) override{
        *db=this->wifiRssiDb;
        return ErrorCode::OK;
    }

    ErrorCode SetAnalogOutput(uint8_t outputIndex, float volts) override{

        return ErrorCode::OK;
    }

    ErrorCode InitAndRun() override
    {
        //I2C Master Bus
        i2c_master_bus_config_t i2c_mst_config = {
            .i2c_port = I2C_PORT,
            .sda_io_num = PIN_I2C_SDA,
            .scl_io_num = PIN_I2C_SCL,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority=0,
            .trans_queue_depth=0,
            .flags={
                .enable_internal_pullup=1,
            }
        };

        ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &i2c_master_handle));
        
        
        //ESP_ERROR_CHECK(i2c_master_probe(i2c_master_handle, I2C_SETUP::STM32_I2C_ADDRESS, 1000));
        ESP_ERROR_CHECK(i2c_master_probe(i2c_master_handle, (uint8_t)AHT::ADDRESS::DEFAULT_ADDRESS, 1000));
        ESP_ERROR_CHECK(i2c_master_probe(i2c_master_handle, 0x6A, 1000)); //LSM6DS3
        ESP_LOGI(TAG, "I2C bus successfully initialized and probed");



        //LED Strip
        strip = new RGBLED::M<LED_NUMBER, RGBLED::DeviceType::WS2812>();
        ERRORCODE_CHECK(strip->Begin(SPI3_HOST, PIN_LED_WS2812));
        ERRORCODE_CHECK(strip->Clear(100));

        //MP3
        //nau88c22::M *codec = new nau88c22::M(i2c_master_handle,  PIN_I2S_MCLK, PIN_I2S_BCLK, PIN_I2S_FS, PIN_I2S_DAC);
        //mp3player = new AudioPlayer::Player(codec);
        //mp3player->Init();


        GreetUserOnStartup();
        //Tasks
        xTaskCreate(sensorTask, "sensorTask", 4096 * 4, this, 6, nullptr);
        //xTaskCreate(mp3Task, "mp3task", 6144 * 4, this, 16, nullptr); //Stack Size = 4096 --> Stack overflow!!
        return ErrorCode::OK;
    }

    ErrorCode BeforeLoop() override
    {
        float ht;
        GetHeaterTemperature(&ht);
        if(ht>85){
            ESP_LOGE(TAG, "Emergency Shutdown. Heater Temperature too high!!!");
            this->SetHeaterDuty(0);
            this->heaterEmergencyShutdown=true;
        }
        return ErrorCode::OK;
    }

    ErrorCode AfterLoop() override
    {
        strip->Refresh(100);  //checks internally, whether data is dirty and has to be pushed out
        return ErrorCode::OK;
    }

    ErrorCode StartBuzzer(float freqHz) override
    {
        return ErrorCode::OK;
    }

    ErrorCode EndBuzzer() override
    {
        return ErrorCode::OK;
    }

    ErrorCode ColorizeLed(uint8_t ledIndex, CRGB colorCRGB) override
    {
        if(ledIndex>=LED_NUMBER) return ErrorCode::INDEX_OUT_OF_BOUNDS;
        return strip->SetPixel(LED_NUMBER-ledIndex-1, colorCRGB, true);

    }

    ErrorCode UnColorizeAllLed() override
    {
        strip->Clear(1000);
        return ErrorCode::OK;
    }

    ErrorCode SetRelayState(bool state) override
    {
        if(state){
            SetBitIdx(this->esp2stm_buf[E2S::RELAY_BLRESET_POS], 0);
        }else{
            ClearBitIdx(this->esp2stm_buf[E2S::RELAY_BLRESET_POS], 0);
        }
        ESP_LOGI(TAG, "this->esp2stm_buf[E2S::RELAY_BLRESET_POS] is %d", this->esp2stm_buf[E2S::RELAY_BLRESET_POS]);
        return ErrorCode::OK;
    }

    ErrorCode SetHeaterDuty(float power_0_100) override {
        this->esp2stm_buf[E2S::HEATER_POS]=power_0_100;
        return ErrorCode::OK;
    }

    float GetHeaterState() override {
        return this->esp2stm_buf[E2S::HEATER_POS];
    }

    ErrorCode SetServoPosition(uint8_t servoIndex, float angle_0_to_180) override {
        if(servoIndex>=4) return ErrorCode::NONE_AVAILABLE;
        this->esp2stm_buf[E2S::SERVO0_POS+servoIndex]=angle_0_to_180;
        return ErrorCode::OK;
    }


    ErrorCode SetFanDuty(uint8_t fanIndex, float power_0_100) override {               
        if(fanIndex>=1) return ErrorCode::NONE_AVAILABLE;
        this->esp2stm_buf[E2S::FAN0_POS]=power_0_100;
        return ErrorCode::OK;
    }

    ErrorCode GetFanDuty(uint8_t fanIndex, float* power_0_100) override {               
        *power_0_100 = this->esp2stm_buf[E2S::FAN0_POS];
        return ErrorCode::OK;
    }

    ErrorCode SetLedPowerWhiteDuty(float power_0_100) override {
        this->esp2stm_buf[E2S::LED_POWER_POS]=power_0_100;
        return ErrorCode::OK;
    }

    bool GetButtonRedIsPressed() override {
        return GetBitIdx(this->stm2esp_buf[S2E::BTN_MOVEMENT_BLFAULT_POS], 0);
    }

    bool GetButtonEncoderIsPressed() override {
        return GetBitIdx(this->stm2esp_buf[S2E::BTN_MOVEMENT_BLFAULT_POS], 1);
    }

    bool GetButtonGreenIsPressed() override {
        return gpio_get_level(PIN_BTN_GREEN)==0;
    }

    bool IsMovementDetected() override {
        return GetBitIdx(this->stm2esp_buf[S2E::BTN_MOVEMENT_BLFAULT_POS], 2);
    }

    float GetUSBCVoltage() override{
        return ParseU16(this->stm2esp_buf, S2E::USBPD_VOLTAGE_IS_POS);
    }

    ErrorCode GreetUserOnStartup() override{  
        for(int i=0;i<3;i++)
        {
            ColorizeLed(0, CRGB::DarkRed);
            ColorizeLed(1, CRGB::Yellow);
            ColorizeLed(2, CRGB::DarkGreen);
            ColorizeLed(3, CRGB::DarkBlue);
            strip->Refresh();
            vTaskDelay(pdMS_TO_TICKS(150));
            ColorizeLed(0, CRGB::DarkBlue);
            ColorizeLed(1, CRGB::DarkGreen);
            ColorizeLed(2, CRGB::Yellow);
            ColorizeLed(3, CRGB::DarkRed);
            strip->Refresh();
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

    ErrorCode GetAmbientBrightnessAnalog(float *lux){
        uint16_t temp;
        temp=ParseU16(stm2esp_buf, S2E::BRIGHTNESS_POS);
        *lux=temp;
        return ErrorCode::OK;
    }

    ErrorCode GetAmbientBrightnessDigital(float *lux){
        uint16_t temp;
        bh1750dev->Read(temp);
        *lux=temp;
        return ErrorCode::OK;
    }
    virtual ErrorCode GetAirTemperatureDS18B20(float *degreesCelcius){
        *degreesCelcius = this->oneWireBus->GetMinTemp();
        return ErrorCode::OK;
    }
    ErrorCode GetAirTemperatureAHT21(float *degreesCelcius){
        if(!this->aht21dev){
            return ErrorCode::GENERIC_ERROR;
        }
        float dummyHumid;
        this->aht21dev->Read(dummyHumid, *degreesCelcius);
        return ErrorCode::OK;
    }
    
    ErrorCode GetAirTemperatureBME280(float *degreesCelcius){
        if(!this->bme280dev){
            return ErrorCode::GENERIC_ERROR;
        }
        float dummy1;
        float dummy2;
        this->bme280dev->GetData(degreesCelcius, &dummy1, &dummy2);
        return ErrorCode::OK;
    }
    ErrorCode GetAirRelHumidityAHT21(float *percent){
        if(!this->aht21dev){
            return ErrorCode::GENERIC_ERROR;
        }
        float dummyTemp;
        this->aht21dev->Read(*percent, dummyTemp);
        return ErrorCode::OK;
    }
    ErrorCode GetAirRelHumidityBME280(float *percent){
        if(!this->bme280dev){
            return ErrorCode::GENERIC_ERROR;
        }
        float dummy1;
        float dummy2;
        this->bme280dev->GetData(&dummy1, &dummy2, percent);
        return ErrorCode::OK;
    }
    ErrorCode GetDistanceMillimeters(uint16_t *value){
        *value=this->vl53l0xdev->ReadMillimeters();
        return ErrorCode::OK;
    }

};