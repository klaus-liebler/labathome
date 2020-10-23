#pragma once

#include "HAL.hh"

#include <inttypes.h>


#include <driver/mcpwm.h>
#include <driver/ledc.h>
#include <driver/adc.h>
#include <driver/i2c.h>
#include <driver/rmt.h>
#include "labathomeerror.hh"
#include "WS2812.hh"
#include <bh1750.hh>
#include <bme280.hh>
#include <ads1115.hh>
#include <owb.h>
#include <owb_rmt.h>
#include <ds18b20.h>
#include <i2c.hh>

typedef gpio_num_t Pintype;

constexpr Pintype PIN_R3_1 = (Pintype)36;
constexpr Pintype PIN_MOVEMENT = (Pintype)39;
constexpr Pintype PIN_SW = (Pintype)34;
constexpr adc1_channel_t PIN_SW_CHANNEL = ADC1_CHANNEL_6;
constexpr Pintype PIN_ROTENC_A = (Pintype)35;
constexpr Pintype PIN_FAN2_DRIVE = (Pintype)32;
constexpr Pintype PIN_FAN1_SENSE = (Pintype)33;
constexpr Pintype PIN_SERVO2 = (Pintype)33;
constexpr Pintype PIN_SPEAKER = (Pintype)25;
constexpr Pintype PIN_LED_STRIP = (Pintype)26;

constexpr Pintype PIN_MULTI1 = (Pintype)27;
constexpr Pintype PIN_ONEWIRE = (Pintype)14;
constexpr Pintype PIN_FAN1_DRIVE = (Pintype)12;
constexpr Pintype PIN_LED_POWER_WHITE = (Pintype)13;

constexpr Pintype PIN_SPI_MOSI = (Pintype)23;
constexpr Pintype PIN_I2C_SDA = (Pintype)22;
constexpr Pintype PIN_I2C_SCL = (Pintype)21;
constexpr Pintype PIN_SPI_MISO = (Pintype)19;
constexpr Pintype PIN_SPI_CLK = (Pintype)18;
constexpr Pintype PIN_SPI_IO1 = (Pintype)5;
constexpr Pintype PIN_HEATER = (Pintype)17;
constexpr Pintype PIN_MULTI3 = (Pintype)16;
constexpr Pintype PIN_MULTI2 = (Pintype)4;
constexpr Pintype PIN_SPI_IO2 = (Pintype)0;
constexpr Pintype PIN_R3_ON = (Pintype)2;
constexpr Pintype PIN_ROTENC_B = (Pintype)15;

constexpr Pintype PIN_SERVO1 = PIN_MULTI1;
constexpr Pintype PIN_I2S_SCK = PIN_MULTI1;
constexpr Pintype PIN_485_DE = PIN_MULTI1;
constexpr Pintype PIN_EXT1 = PIN_MULTI1;

constexpr Pintype PN_485_RO = PIN_MULTI2;
constexpr Pintype PN_CAN_TX = PIN_MULTI2;
constexpr Pintype PN_EXT2 = PIN_MULTI2;
constexpr Pintype PN_I2S_WS = PIN_MULTI2;

constexpr Pintype PN_485_DI = PIN_MULTI3;
constexpr Pintype PN_CAN_RX = PIN_MULTI3;
constexpr Pintype PN_I2S_SD = PIN_MULTI3;
constexpr Pintype PN_EXT3 = PIN_MULTI3;

enum class MODE_IO33
{
    SERVO2,
    FAN1_SENSE,
};

enum class MODE_MULTI1_PIN
{
    I2S,
    RS485,
    SERVO1,
    EXT,
};

enum class MODE_MULTI_2_3_PINS
{
    I2S,
    RS485,
    CAN,
    EXT,
};

enum class Button : uint8_t
{
    BUT_GREEN = 1,
    BUT_ENCODER = 0,
    BUT_RED = 2,
};

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

constexpr size_t LED_NUMBER = 8;
constexpr rmt_channel_t CHANNEL_WS2812 = RMT_CHANNEL_0;
constexpr rmt_channel_t CHANNEL_ONEWIRE_TX = RMT_CHANNEL_1;
constexpr rmt_channel_t CHANNEL_ONEWIRE_RX = RMT_CHANNEL_2;
constexpr i2c_port_t I2C_PORT = I2C_NUM_1;
constexpr uint32_t  DEFAULT_VREF        = 1100; //Use adc2_vref_to_gpio() to obtain a better estimate
constexpr uint16_t sw_limits[7] = {160, 480, 1175, 1762, 2346, 2779, 3202};
constexpr int SERVO_MIN_PULSEWIDTH = 500;  //Minimum pulse width in microsecond
constexpr int SERVO_MAX_PULSEWIDTH = 2400; //Maximum pulse width in microsecond
constexpr int SERVO_MAX_DEGREE = 180;      //Maximum angle in degree upto which servo can rotate

extern "C" void sensorTask(void *pvParameters);

class HAL_labathomeV5:public HAL
{
private:
    bool movementIsDetected = false;
    esp_adc_cal_characteristics_t *adc_chars;
    MODE_IO33 mode_io33;
    MODE_MULTI1_PIN mode_multi1;
    MODE_MULTI_2_3_PINS mode_multi23;
    bool needLedStripUpdate = false;
    

     WS2812_Strip<LED_NUMBER> *strip=NULL;

    uint32_t buttonState = 0;

    //SensorValues
    float heaterTemperatureDegCel=0.0;
    float airTemperatureDegCel=0.0;
    float airPressurePa=0.0;
    float airRelHumidityPercent=0.0;
    float ambientBrightnessLux = 0.0;
    float ADS1115Values[4];
    uint16_t ds4525doTemperature;
    uint16_t ds4525doPressure;
    //Actor Values
    uint16_t pca9685Values[16];
public:


    HAL_labathomeV5(MODE_IO33 mode_io33, MODE_MULTI1_PIN mode_multi1, MODE_MULTI_2_3_PINS mode_multi23) : mode_io33(mode_io33), mode_multi1(mode_multi1), mode_multi23(mode_multi23)
    {
        
    }

    int64_t IRAM_ATTR GetMicros()
    {
        return esp_timer_get_time();
    }

    uint32_t GetMillis()
    {
        return (uint32_t) (esp_timer_get_time() / 1000ULL);
    }

    int64_t GetMillis64(){
        return esp_timer_get_time() / 1000ULL;
    }

    LabAtHomeErrorCode GetADCValues(float **voltages){
        *voltages = this->ADS1115Values;
        return LabAtHomeErrorCode::OK;
    }
  
    void SensorLoop_ForInternalUseOnly()
    {
        int64_t nextOneWireReadout = INT64_MAX;
        int64_t nextBME280Readout = INT64_MAX;
        int64_t nextBH1750Readout = INT64_MAX;
        TickType_t  nextADS1115Readout = UINT32_MAX;
        uint16_t nextADS1115Mux = 0b100; //100...111
        
        uint32_t oneWireReadoutIntervalMs = 200;
        uint32_t bme280ReadoutIntervalMs = UINT32_MAX;
        uint32_t bh1750ReadoutIntervalMs = 200;
        TickType_t ads1115ReadoutInterval= portMAX_DELAY;

        //OneWire
        DS18B20_Info *ds18b20_info=NULL;
        owb_rmt_driver_info rmt_driver_info;
        OneWireBus *owb = owb_rmt_initialize(&rmt_driver_info, PIN_ONEWIRE, CHANNEL_ONEWIRE_TX, CHANNEL_ONEWIRE_RX);
        owb_use_crc(owb, true);  // enable CRC check for ROM code
        // Find all connected devices
        OneWireBus_ROMCode rom_code;
        owb_status status = owb_read_rom(owb, &rom_code);
        if (status == OWB_STATUS_OK)
        {
            
            ds18b20_info=ds18b20_malloc();  // heap allocation
            ds18b20_init_solo(ds18b20_info, owb);// only one device on bus
            ds18b20_use_crc(ds18b20_info, true); // enable CRC check on all reads
            ds18b20_set_resolution(ds18b20_info, DS18B20_RESOLUTION_10_BIT); //10bit -->187ms Conversion time
            ds18b20_convert_all(owb);
            nextOneWireReadout=GetMillis()+200;
        }
        else
        {
            ESP_LOGE(TAG, "OneWire: An error occurred reading ROM code: %d", status);
        }

        
        //BME280
        BME280* bme280 = new BME280(I2C_PORT, BME280_ADRESS::PRIM);
        if(bme280->Init(&bme280ReadoutIntervalMs)==ESP_OK){
            bme280->TriggerNextMeasurement();
            bme280ReadoutIntervalMs+=20;
            bme280->TriggerNextMeasurement();
            nextBME280Readout=GetMillis()+bme280ReadoutIntervalMs;
            ESP_LOGI(TAG, "I2C: BME280 successfully initialized.");  
        }
        else
        {
            ESP_LOGW(TAG, "I2C: BME280 not found");
        }
        
        //BH1750
        BH1750* bh1750 = new BH1750(I2C_PORT, BH1750_ADRESS::LOW);
        if(bh1750->Init(BH1750_OPERATIONMODE::CONTINU_H_RESOLUTION)==ESP_OK){
            bh1750ReadoutIntervalMs = 200;
            nextBH1750Readout=GetMillis()+bh1750ReadoutIntervalMs;
            ESP_LOGI(TAG, "I2C: BH1750 successfully initialized.");
        }
        else{
            ESP_LOGW(TAG, "I2C: BH1750 not found");
        }

        //ADS1115
        ADS1115 *ads1115 = new ADS1115(I2C_PORT, (uint8_t)0x48);
        if(ads1115->Init(ads1115_sps_t::ADS1115_SPS_16, &ads1115ReadoutInterval)==ESP_OK){
            ads1115->TriggerMeasurement((ads1115_mux_t)nextADS1115Mux);
            nextADS1115Mux++;
            nextADS1115Readout=xTaskGetTickCount()+ads1115ReadoutInterval;
            ESP_LOGI(TAG, "I2C: ADS1115 successfully initialized.");
        }
        else{
            ESP_LOGW(TAG, "I2C: ADS1115 not found");
        }

        while(true){
           
            this->movementIsDetected= gpio_get_level(PIN_MOVEMENT);
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

            if (GetMillis64() > nextOneWireReadout)
            {
                ds18b20_read_temp(ds18b20_info, &(this->heaterTemperatureDegCel));
                ds18b20_convert_all(owb);
                nextOneWireReadout = GetMillis64()+oneWireReadoutIntervalMs;
            }
            if(GetMillis64()>nextBME280Readout)
            {
                bme280->GetDataAndTriggerNextMeasurement(&this->airTemperatureDegCel, &this->airPressurePa, &this->airRelHumidityPercent);
                nextBME280Readout=GetMillis64()+bme280ReadoutIntervalMs;
            }
            if(GetMillis64()>nextBH1750Readout)
            {
                bh1750->Read(&(this->ambientBrightnessLux));
                nextBH1750Readout=GetMillis64()+bh1750ReadoutIntervalMs;
            }
            if(GetMillis64()>nextBH1750Readout)
            {
                bh1750->Read(&(this->ambientBrightnessLux));
                nextBH1750Readout=GetMillis64()+bh1750ReadoutIntervalMs;
            }

            if(xTaskGetTickCount()>=nextADS1115Readout){
                ads1115->GetVoltage(&ADS1115Values[nextADS1115Mux&0b11]);
                nextADS1115Mux++;
                if(nextADS1115Mux>0b111) nextADS1115Mux=0b100;
                ads1115->TriggerMeasurement((ads1115_mux_t)nextADS1115Mux);
                nextADS1115Readout=xTaskGetTickCount()+ads1115ReadoutInterval;
            }
        }
    }


    LabAtHomeErrorCode GetHeaterTemperature(float * degreesCelcius)
    {
        *degreesCelcius= this->heaterTemperatureDegCel;
        return LabAtHomeErrorCode::OK;
    }

    LabAtHomeErrorCode GetAirTemperature(float *degreesCelcius){
        *degreesCelcius= this->airTemperatureDegCel;
        return LabAtHomeErrorCode::OK;
    }

    LabAtHomeErrorCode GetAirPressure(float *pa){
        *pa=this->airPressurePa;
        return LabAtHomeErrorCode::OK;
    }
    LabAtHomeErrorCode GetAirRelHumidity(float *percent){
        *percent=this->airRelHumidityPercent;
        return LabAtHomeErrorCode::OK;
    }

    LabAtHomeErrorCode GetAmbientBrightness(float *lux)
    {
        *lux=this->ambientBrightnessLux;
        return LabAtHomeErrorCode::OK;
    }


    LabAtHomeErrorCode Init()
    {
        if (mode_io33 == MODE_IO33::FAN1_SENSE) return LabAtHomeErrorCode::NOT_YET_IMPLEMENTED;
        
        gpio_pad_select_gpio((uint8_t)PIN_R3_1);
        gpio_set_direction(PIN_R3_1, GPIO_MODE_INPUT);
        gpio_set_pull_mode(PIN_R3_1, GPIO_FLOATING);

        gpio_pad_select_gpio((uint8_t)PIN_MOVEMENT);
        gpio_set_direction(PIN_MOVEMENT, GPIO_MODE_INPUT);
        gpio_set_pull_mode(PIN_MOVEMENT, GPIO_FLOATING);

        adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));
        esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_0db, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(PIN_SW_CHANNEL, ADC_ATTEN_0db);
        
        gpio_pad_select_gpio((uint8_t)PIN_ROTENC_A);
        gpio_set_direction(PIN_ROTENC_A, GPIO_MODE_INPUT);
        gpio_set_pull_mode(PIN_ROTENC_A, GPIO_FLOATING);

        gpio_pad_select_gpio((uint8_t)PIN_ROTENC_B);
        gpio_set_direction(PIN_ROTENC_B, GPIO_MODE_INPUT);
        gpio_set_pull_mode(PIN_ROTENC_B, GPIO_PULLUP_ONLY);

        gpio_set_level(PIN_R3_ON, 0);
        gpio_pad_select_gpio((uint8_t)PIN_R3_ON);
        gpio_set_direction(PIN_R3_ON, GPIO_MODE_OUTPUT);
        gpio_set_pull_mode(PIN_R3_ON, GPIO_FLOATING);

        //Servos
        mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, PIN_SERVO1);
        if (mode_io33 == MODE_IO33::SERVO2)
        {
            mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, PIN_SERVO2);
        }
        mcpwm_config_t pwm_config;
        pwm_config.frequency = 50; //frequency = 50Hz, i.e. for every servo motor time period should be 20ms
        pwm_config.cmpr_a = 0;     //duty cycle of PWMxA = 0
        pwm_config.cmpr_b = 0;     //duty cycle of PWMxb = 0
        pwm_config.counter_mode = MCPWM_UP_COUNTER;
        pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
        ESP_ERROR_CHECK(mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config)); //Configure PWM0A & PWM0B with above settings

        //Fans
        mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, PIN_FAN1_DRIVE);
        mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1B, PIN_FAN2_DRIVE);
        pwm_config.frequency = 50;
        pwm_config.cmpr_a = 0;     //duty cycle of PWMxA = 0
        pwm_config.cmpr_b = 0;     //duty cycle of PWMxb = 0
        pwm_config.counter_mode = MCPWM_UP_COUNTER;
        pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
        ESP_ERROR_CHECK(mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &pwm_config));
        
        //Heater
        mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM2A, PIN_HEATER);
        pwm_config.frequency = 20;
        pwm_config.cmpr_a = 0;     //duty cycle of PWMxA = 0
        pwm_config.cmpr_b = 0;     //duty cycle of PWMxb = 0
        pwm_config.counter_mode = MCPWM_UP_COUNTER;
        pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
        ESP_ERROR_CHECK(mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_2, &pwm_config));

        //LED
        ledc_timer_config_t power_ledc_timer;
        power_ledc_timer.duty_resolution = LEDC_TIMER_13_BIT; // resolution of PWM duty
        power_ledc_timer.freq_hz = 5000;                      // frequency of PWM signal
        power_ledc_timer.speed_mode = LEDC_HIGH_SPEED_MODE;   // timer mode
        power_ledc_timer.timer_num = LEDC_TIMER_0;            // timer index
        ESP_ERROR_CHECK(ledc_timer_config(&power_ledc_timer));

        ledc_channel_config_t ledc_channel;
        ledc_channel.channel = LEDC_CHANNEL_0;
        ledc_channel.duty = 0;
        ledc_channel.gpio_num = PIN_LED_POWER_WHITE;
        ledc_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
        ledc_channel.hpoint = 0;
        ledc_channel.timer_sel = LEDC_TIMER_0;
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

        ledc_timer_config_t buzzer_timer;
        buzzer_timer.duty_resolution = LEDC_TIMER_10_BIT; // resolution of PWM duty
        buzzer_timer.freq_hz = 440;                       // frequency of PWM signal
        buzzer_timer.speed_mode = LEDC_HIGH_SPEED_MODE;   // timer mode
        buzzer_timer.timer_num = LEDC_TIMER_2;            // timer index
        ESP_ERROR_CHECK(ledc_timer_config(&buzzer_timer));

        ledc_channel_config_t buzzer_channel;
        buzzer_channel.channel = LEDC_CHANNEL_2;
        buzzer_channel.duty = 0;
        buzzer_channel.gpio_num = PIN_SPEAKER;
        buzzer_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
        buzzer_channel.hpoint = 0;
        buzzer_channel.timer_sel = LEDC_TIMER_2;
        ESP_ERROR_CHECK(ledc_channel_config(&buzzer_channel));

        //I2C Master
        i2c_config_t conf;
        conf.mode = I2C_MODE_MASTER;
        conf.sda_io_num = PIN_I2C_SDA;
        conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
        conf.scl_io_num = PIN_I2C_SCL;
        conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
        conf.master.clk_speed = 100000;
        i2c_param_config(I2C_PORT, &conf);
        ESP_ERROR_CHECK(i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0));

        ESP_ERROR_CHECK(I2C::Init());

        //LED Strip
        strip = new WS2812_Strip<LED_NUMBER>(CHANNEL_WS2812);
        ESP_ERROR_CHECK(strip->Init(PIN_LED_STRIP));
        ESP_ERROR_CHECK(strip->Clear(100));


        xTaskCreate(sensorTask, "sensorTask", 4096 * 4, this, 6, NULL);

        return LabAtHomeErrorCode::OK;
    }

    LabAtHomeErrorCode BeforeLoop()
    {
        
        return LabAtHomeErrorCode::OK;
    }

    LabAtHomeErrorCode AfterLoop()
    {
        if (needLedStripUpdate)
        {
            strip->Refresh(100);
            needLedStripUpdate = false;
        }
        return LabAtHomeErrorCode::OK;
    }

    LabAtHomeErrorCode StartBuzzer(double freqHz)
    {
        ledc_timer_config_t buzzer_timer;
        buzzer_timer.duty_resolution = LEDC_TIMER_10_BIT; // resolution of PWM duty
        buzzer_timer.freq_hz = freqHz;                      // frequency of PWM signal
        buzzer_timer.speed_mode = LEDC_HIGH_SPEED_MODE;   // timer mode
        buzzer_timer.timer_num = LEDC_TIMER_2;            // timer index
        ledc_timer_config(&buzzer_timer);
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2, 512);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2);
        return LabAtHomeErrorCode::OK;
    }

    LabAtHomeErrorCode EndBuzzer()
    {
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2, 0);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2);
        return LabAtHomeErrorCode::OK;
    }

    LabAtHomeErrorCode ColorizeLed(LED led, uint32_t color)
    {
        CRGB colorCRGB(color);
        strip->SetPixel((uint8_t)led, colorCRGB);
        //TODO: Hier Prüfung, ob sich tatsächlich etwas verändert hat und ein Update tatsächlich erforderlich ist
        this->needLedStripUpdate = true;
        return LabAtHomeErrorCode::OK;
    }

    LabAtHomeErrorCode UnColorizeAllLed()
    {
        strip->Clear(1000);
        this->needLedStripUpdate = true;
        return LabAtHomeErrorCode::OK;
    }

    LabAtHomeErrorCode SetRelayState(bool state)
    {
        gpio_set_level(PIN_R3_ON, state);
        return LabAtHomeErrorCode::OK;
    }


    LabAtHomeErrorCode SetHeaterState(float dutyInPercent)
    {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_OPR_A, dutyInPercent);
        return LabAtHomeErrorCode::OK;
    }

    float GetHeaterState(){
        return mcpwm_get_duty(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_OPR_A);
    }


    LabAtHomeErrorCode SetFan1State(float dutyInPercent)
    {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, dutyInPercent);
        return LabAtHomeErrorCode::OK;
    }

    float GetFan1State(){
        return mcpwm_get_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A);
    }


    LabAtHomeErrorCode SetFan2State(float dutyInPercent)
    {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B, dutyInPercent);
        return LabAtHomeErrorCode::OK;
    }

    float GetFan2State(){
        return mcpwm_get_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B);
    }

    LabAtHomeErrorCode SetLedPowerWhiteState(uint8_t dutyInpercent)
    {
        if (dutyInpercent > 100)
            dutyInpercent = 100;
        uint32_t duty = ((1 << 13) * dutyInpercent) / 100;
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
        return LabAtHomeErrorCode::OK;
    }

    bool GetButtonRedIsPressed()
    {
        return CHECK_BIT(this->buttonState, (uint8_t)Button::BUT_RED);
    }

    bool GetButtonEncoderIsPressed()
    {
        return CHECK_BIT(this->buttonState, (uint8_t)Button::BUT_ENCODER);
    }

    bool GetButtonGreenIsPressed()
    {
        return CHECK_BIT(this->buttonState, (uint8_t)Button::BUT_GREEN);
    }


    bool IsMovementDetected()
    {
        return this->movementIsDetected;
    }

    LabAtHomeErrorCode SetServoPosition(Servo servo, uint32_t angle_0_to_180)
    {
        uint32_t cal_pulsewidth = (SERVO_MIN_PULSEWIDTH + (((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * (angle_0_to_180)) / (SERVO_MAX_DEGREE)));
        switch (servo)
        {
        case Servo::Servo1:
            mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, cal_pulsewidth);
            break;
        case Servo::Servo2:
            mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, cal_pulsewidth);
            break;
        default:
            break;
        }
        return LabAtHomeErrorCode::OK;
    }
};

void sensorTask(void *pvParameters){
    HAL_labathomeV5 *hal = (HAL_labathomeV5 *)pvParameters;
    hal->SensorLoop_ForInternalUseOnly();
}