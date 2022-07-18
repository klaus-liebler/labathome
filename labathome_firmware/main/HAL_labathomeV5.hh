#pragma once

#include "HAL.hh"

#include <inttypes.h>
#include <algorithm>

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
#include <owb.h>
#include <owb_rmt.h>
#include <ds18b20.h>
#include <rotenc.hh>
#include <i2c.hh>

typedef gpio_num_t Pintype;

constexpr Pintype PIN_R3_1 = (Pintype)36;
constexpr Pintype PIN_MOVEMENT = (Pintype)39;
constexpr Pintype PIN_SW = (Pintype)34;
constexpr adc1_channel_t CHANNEL_SWITCHES = ADC1_CHANNEL_6;
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
//constexpr Pintype PIN_485_DE = PIN_MULTI1;
//constexpr Pintype PIN_EXT1 = PIN_MULTI1;

//constexpr Pintype PIN_485_RO = PIN_MULTI2;
//constexpr Pintype PIN_CAN_TX = PIN_MULTI2;
//constexpr Pintype PIN_EXT2 = PIN_MULTI2;
constexpr Pintype PIN_I2S_WS = PIN_MULTI2;

//constexpr Pintype PIN_485_DI = PIN_MULTI3;
//constexpr Pintype PIN_CAN_RX = PIN_MULTI3;
constexpr Pintype PIN_I2S_SD = PIN_MULTI3;
//constexpr Pintype PIN_EXT3 = PIN_MULTI3;

struct Note
{
    uint16_t freq;
    uint16_t durationMs;
};

#include "songs.hh"

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

#define CHECK_BIT(var, pos) ((var) & (1 << (pos)))

constexpr size_t LED_NUMBER = 8;
constexpr rmt_channel_t CHANNEL_WS2812 = RMT_CHANNEL_0;
constexpr rmt_channel_t CHANNEL_ONEWIRE_TX = RMT_CHANNEL_1;
constexpr rmt_channel_t CHANNEL_ONEWIRE_RX = RMT_CHANNEL_2;
constexpr i2c_port_t I2C_PORT = I2C_NUM_1;
constexpr uint32_t DEFAULT_VREF = 1100; //Use adc2_vref_to_gpio() to obtain a better estimate
constexpr uint16_t sw_limits[7] = {160, 480, 1175, 1762, 2346, 2779, 3202};
constexpr int SERVO_MIN_PULSEWIDTH = 500;  //Minimum pulse width in microsecond
constexpr int SERVO_MAX_PULSEWIDTH = 2400; //Maximum pulse width in microsecond
constexpr int SERVO_MAX_DEGREE = 180;      //Maximum angle in degree upto which servo can rotate
constexpr ledc_timer_bit_t power_ledc_timer_duty_resolution = LEDC_TIMER_10_BIT;

constexpr i2s_port_t I2S_PORT_MICROPHONE{I2S_NUM_1};
constexpr i2s_port_t I2S_PORT_LOUDSPEAKER{I2S_NUM_0};

constexpr int average_over_N_measurements{10};
constexpr int SAMPLES {2048};
constexpr size_t SAMPLES_IN_BYTES{SAMPLES*4};
constexpr int SAMPLE_RATE_MICROPHONE{22050};
constexpr int AMPLITUDE{150};
constexpr uint16_t FREQUENCIES[]{11,22,32,43,54,65,75,97,118,140,161,183,205,226,258,291,323,355,388,431,474,517,560,614,668,721,786,851,915,991,1066,1152,1238,1335,1443,1550,1669,1798,1938,2089,2239,2401,2584,2778,2982,3198,3435,3682,3951,4231,4533,4856,5200,5566,5965,6385,6837,7321,7838,8398,8990,9625,10304,11025};
constexpr uint16_t BUCKET_INDICES[]{1,2,3,4,5,6,7,9,11,13,15,17,19,21,24,27,30,33,36,40,44,48,52,57,62,67,73,79,85,92,99,107,115,124,134,144,155,167,180,194,208,223,240,258,277,297,319,342,367,393,421,451,483,517,554,593,635,680,728,780,835,894,957,1024};

int32_t samplesI32[SAMPLES]; //The slave serial-data port’s format is I²S, 24-bit, twos complement, There must be 64 SCK cycles in each WS stereo frame, or 32 SCK cycles per data-word.
double real[SAMPLES];
double imag[SAMPLES];
arduinoFFT fft(real, imag, SAMPLES, SAMPLE_RATE_MICROPHONE);

extern "C" void sensorTask(void *pvParameters);

class HAL_labathome : public HAL
{
private:
    bool movementIsDetected = false;
    esp_adc_cal_characteristics_t *adc_chars;
    MODE_IO33 mode_io33;
    MODE_MULTI1_PIN mode_multi1;
    MODE_MULTI_2_3_PINS mode_multi23;
    bool needLedStripUpdate = false;
    uint32_t songNumber = 0;
    int32_t song_nextNoteIndex = 0;
    uint32_t song_nextNoteTimeMs = UINT32_MAX;

    WS2812_Strip<LED_NUMBER> *strip = NULL;
    cRotaryEncoder *rotenc = NULL;

    uint32_t buttonState = 0;

    //SensorValues
    float heaterTemperatureDegCel = 0.0;
    bool heaterEmergencyShutdown=false;
    float airTemperatureDegCel = 0.0;
    float airPressurePa = 0.0;
    float airRelHumidityPercent = 0.0;
    float ambientBrightnessLux = 0.0;
    float AnalogInputs[4];
    float airSpeedMeterPerSecond;
    uint16_t ds4525doPressure;

public:
    HAL_labathome(MODE_IO33 mode_io33, MODE_MULTI1_PIN mode_multi1, MODE_MULTI_2_3_PINS mode_multi23) : mode_io33(mode_io33), mode_multi1(mode_multi1), mode_multi23(mode_multi23)
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

    void SensorLoop_ForInternalUseOnly()
    {
        int64_t nextOneWireReadout = INT64_MAX;
        int64_t nextBME280Readout = INT64_MAX;
        int64_t nextBH1750Readout = INT64_MAX;
        int64_t nextMS4525Readout = INT64_MAX;
        TickType_t nextADS1115Readout = UINT32_MAX;
        uint16_t nextADS1115Mux = 0b100; //100...111

        uint32_t oneWireReadoutIntervalMs = 800; //10bit -->187ms Conversion time, 12bit--> 750ms
        uint32_t bme280ReadoutIntervalMs = UINT32_MAX;
        uint32_t bh1750ReadoutIntervalMs = 200;
        TickType_t ads1115ReadoutInterval = portMAX_DELAY;
        uint32_t ms4525ReadoutInterval = 200;

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
        }
        else
        {
            ESP_LOGE(TAG, "OneWire: An error occurred reading ROM code: %d", status);
        }

        //BME280
        BME280 *bme280 = new BME280(I2C_PORT, BME280_ADRESS::PRIM);
        if (bme280->InitAndRun(&bme280ReadoutIntervalMs) == ESP_OK)
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
        if (bh1750->InitAndRun(BH1750_OPERATIONMODE::CONTINU_H_RESOLUTION) == ESP_OK)
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
        if (ads1115->InitAndRun(ads1115_sps_t::ADS1115_SPS_16, &ads1115ReadoutInterval) == ESP_OK)
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

        MS4525DO *ms4525 = new MS4525DO(I2C_PORT, MS4523_Adress::I);
        if(ms4525->InitAndRun()==ESP_OK){
            nextMS4525Readout = GetMillis() + ms4525ReadoutInterval;
        }



        while (true)
        {
            this->movementIsDetected = gpio_get_level(PIN_MOVEMENT);
            int adc_reading = adc1_get_raw(CHANNEL_SWITCHES);
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
                nextOneWireReadout = GetMillis64() + oneWireReadoutIntervalMs;
            }
            if (GetMillis64() > nextBME280Readout)
            {
                bme280->GetDataAndTriggerNextMeasurement(&this->airTemperatureDegCel, &this->airPressurePa, &this->airRelHumidityPercent);
                nextBME280Readout = GetMillis64() + bme280ReadoutIntervalMs;
            }
            if (GetMillis64() > nextBH1750Readout)
            {
                bh1750->Read(&(this->ambientBrightnessLux));
                nextBH1750Readout = GetMillis64() + bh1750ReadoutIntervalMs;
            }
            if (GetMillis64() > nextMS4525Readout)
            {
                ms4525->Read();
                this->airSpeedMeterPerSecond=ms4525->GetAirSpeedMetersPerSecond();
                nextMS4525Readout = GetMillis64() + ms4525ReadoutInterval;
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
            vTaskDelay(100/portTICK_PERIOD_MS);
        }
        
    }

    ErrorCode SetSound(uint32_t songNumber)
    {
        if (songNumber >= sizeof(SOUNDS) / sizeof(Note))
            songNumber = 0;
        this->song_nextNoteIndex = 0;
        this->song_nextNoteTimeMs = GetMillis();
        this->songNumber = songNumber;
        ESP_LOGI(TAG, "Set Song to %d", songNumber);
        return ErrorCode::OK;
    }

    ErrorCode GetCO2PPM(uint16_t *co2PPM){
        
        *co2PPM=0;
        return ErrorCode::NOT_YET_IMPLEMENTED;
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
        *lux = this->ambientBrightnessLux;
        return ErrorCode::OK;
    }


    //see https://github.com/squix78/esp32-mic-fft/blob/master/esp32-mic-fft.ino
    ErrorCode GetFFT64(float *magnitudes64_param){
        for (int i = 1; i < 64; i++){
            magnitudes64_param[i]=0;
        }
        
        for(int cnt=0;cnt<average_over_N_measurements;cnt++){
            float magnitudes64[64];
            size_t num_bytes_read{0};
            if (ESP_OK != i2s_read(I2S_PORT_MICROPHONE, samplesI32, sizeof(uint32_t) * SAMPLES, &num_bytes_read, portMAX_DELAY)){
                ESP_LOGE(TAG, "ESP_OK!=i2s_read");
                return ErrorCode::GENERIC_ERROR;
            }
            int samples_read = num_bytes_read / sizeof(int32_t);
            //int16_t minimum{INT16_MAX};
            //int16_t maximum{INT16_MIN};
            for (int i = 0; i < samples_read; i++){
                int16_t sample = samplesI32[i] >> 14; //>>16 to be on the veeeeery safe side, but then "normal" voice is even tooo quiet in replay
                //minimum = std::min(minimum, sample);
                //maximum = std::max(maximum, sample);
                real[i] = sample;
                imag[i]=0.0;
            }
            fft.Windowing(FFT_WIN_TYP_HANN, FFT_FORWARD);
            fft.Compute(FFT_FORWARD);
            fft.ComplexToMagnitude();
            size_t resultPointer = 1;
            magnitudes64[0]= (float)std::max({real[2],real[3], real[4], real[5], real[6], real[7]});
            for (int i = 1; i < 64; i++){
                while(resultPointer<BUCKET_INDICES[i]){
                    magnitudes64[i]=std::max(magnitudes64[i], (float)real[resultPointer]);
                    resultPointer++;
                }
                magnitudes64_param[i]+=magnitudes64[i];
            }
        }
        for (int i = 1; i < 64; i++){
            magnitudes64_param[i]/=average_over_N_measurements;
        }
        return ErrorCode::OK;
    }



    ErrorCode InitAndRun()
    {
        if (mode_io33 == MODE_IO33::FAN1_SENSE)
            return ErrorCode::NOT_YET_IMPLEMENTED;

        if(mode_multi1!=MODE_MULTI1_PIN::I2S || mode_multi23!=MODE_MULTI_2_3_PINS::I2S)
            return ErrorCode::NOT_YET_IMPLEMENTED;

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

        gpio_reset_pin(PIN_R3_1);
        gpio_set_direction(PIN_R3_1, GPIO_MODE_INPUT);
        gpio_set_pull_mode(PIN_R3_1, GPIO_FLOATING);

        gpio_reset_pin(PIN_MOVEMENT);
        gpio_set_direction(PIN_MOVEMENT, GPIO_MODE_INPUT);
        gpio_set_pull_mode(PIN_MOVEMENT, GPIO_FLOATING);

        adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));
        esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_0db, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(CHANNEL_SWITCHES, ADC_ATTEN_0db);

        gpio_reset_pin(PIN_ROTENC_A);
        gpio_set_direction(PIN_ROTENC_A, GPIO_MODE_INPUT);
        gpio_set_pull_mode(PIN_ROTENC_A, GPIO_FLOATING);

        gpio_reset_pin(PIN_ROTENC_B);
        gpio_set_direction(PIN_ROTENC_B, GPIO_MODE_INPUT);
        gpio_set_pull_mode(PIN_ROTENC_B, GPIO_PULLUP_ONLY);

        gpio_set_level(PIN_R3_ON, 0);
        gpio_reset_pin(PIN_R3_ON);
        gpio_set_direction(PIN_R3_ON, GPIO_MODE_OUTPUT);
        gpio_set_pull_mode(PIN_R3_ON, GPIO_FLOATING);

        //Servos
        if(mode_multi1==MODE_MULTI1_PIN::SERVO1){
            mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, PIN_SERVO1);
        }
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
        pwm_config.cmpr_a = 0; //duty cycle of PWMxA = 0
        pwm_config.cmpr_b = 0; //duty cycle of PWMxb = 0
        pwm_config.counter_mode = MCPWM_UP_COUNTER;
        pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
        ESP_ERROR_CHECK(mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &pwm_config));

        //Heater
        mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM2A, PIN_HEATER);
        pwm_config.frequency = 20;
        pwm_config.cmpr_a = 0; //duty cycle of PWMxA = 0
        pwm_config.cmpr_b = 0; //duty cycle of PWMxb = 0
        pwm_config.counter_mode = MCPWM_UP_COUNTER;
        pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
        ESP_ERROR_CHECK(mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_2, &pwm_config));

        //White Power LED
        ledc_timer_config_t power_ledc_timer;
        power_ledc_timer.duty_resolution = power_ledc_timer_duty_resolution; // resolution of PWM duty
        power_ledc_timer.freq_hz = 500;                                      // frequency of PWM signal
        power_ledc_timer.speed_mode = LEDC_HIGH_SPEED_MODE;                  // timer mode
        power_ledc_timer.timer_num = LEDC_TIMER_0;                           // timer index
        power_ledc_timer.clk_cfg = LEDC_AUTO_CLK;
        ESP_ERROR_CHECK(ledc_timer_config(&power_ledc_timer));

        ledc_channel_config_t ledc_channel;
        ledc_channel.channel = LEDC_CHANNEL_0;
        ledc_channel.duty = 0;
        ledc_channel.gpio_num = PIN_LED_POWER_WHITE;
        ledc_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
        ledc_channel.hpoint = 0;
        ledc_channel.timer_sel = LEDC_TIMER_0;
        ledc_channel.intr_type=LEDC_INTR_DISABLE;
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
        
        //Buzzer
        ledc_timer_config_t buzzer_timer;
        buzzer_timer.duty_resolution = LEDC_TIMER_10_BIT; // resolution of PWM duty
        buzzer_timer.freq_hz = 440;                       // frequency of PWM signal
        buzzer_timer.speed_mode = LEDC_HIGH_SPEED_MODE;   // timer mode
        buzzer_timer.timer_num = LEDC_TIMER_2;            // timer index
        buzzer_timer.clk_cfg = LEDC_AUTO_CLK;
        ESP_ERROR_CHECK(ledc_timer_config(&buzzer_timer));

        ledc_channel_config_t buzzer_channel;
        buzzer_channel.channel = LEDC_CHANNEL_2;
        buzzer_channel.duty = 0;
        buzzer_channel.gpio_num = PIN_SPEAKER;
        buzzer_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
        buzzer_channel.hpoint = 0;
        buzzer_channel.timer_sel = LEDC_TIMER_2;
        buzzer_channel.intr_type=LEDC_INTR_DISABLE;
        ESP_ERROR_CHECK(ledc_channel_config(&buzzer_channel));

        //I2C Master
        i2c_config_t conf;
        conf.mode = I2C_MODE_MASTER;
        conf.sda_io_num = PIN_I2C_SDA;
        conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
        conf.scl_io_num = PIN_I2C_SCL;
        conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
        conf.master.clk_speed = 100000;
        conf.clk_flags=0;
        i2c_param_config(I2C_PORT, &conf);
        ESP_ERROR_CHECK(i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0));

        ESP_ERROR_CHECK(I2C::InitAndRun());

        //LED Strip
        strip = new WS2812_Strip<LED_NUMBER>();
        ESP_ERROR_CHECK(strip->InitAndRun(VSPI_HOST, PIN_LED_STRIP, 2 ));
        ESP_ERROR_CHECK(strip->Clear(100));

        rotenc=new cRotaryEncoder(PIN_ROTENC_A, PIN_ROTENC_B, -100, 100);
        rotenc->InitAndRun();
        rotenc->Start();

        xTaskCreate(sensorTask, "sensorTask", 4096 * 4, this, 6, NULL);

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
        if (needLedStripUpdate)
        {
            strip->Refresh(100);
            needLedStripUpdate = false;
        }
        if (songNumber != 0)
        {
            if (GetMillis() > song_nextNoteTimeMs)
            {
                const Note note = SOUNDS[songNumber][song_nextNoteIndex];
                if (note.freq == 0)
                {
                    ESP_LOGI(TAG, "Mute Note and wait %d", note.durationMs);
                    EndBuzzer();
                    if (note.durationMs == 0)
                    {
                        songNumber = 0;
                        song_nextNoteTimeMs = UINT32_MAX;
                        song_nextNoteIndex = 0;
                    }
                    else
                    {
                        song_nextNoteTimeMs += note.durationMs;
                        song_nextNoteIndex++;
                    }
                }
                else
                {
                    ESP_LOGI(TAG, "Set Note to Frequency %d and wait %d", note.freq, note.durationMs);
                    StartBuzzer(note.freq);
                    song_nextNoteTimeMs += note.durationMs;
                    song_nextNoteIndex++;
                }
            }
        }
        return ErrorCode::OK;
    }

    ErrorCode StartBuzzer(double freqHz)
    {
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
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2, 0);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2);
        return ErrorCode::OK;
    }

    ErrorCode ColorizeLed(LED led, uint32_t color)
    {
        CRGB colorCRGB(color);
        if(colorCRGB.a==0x00) return ErrorCode::OK;
        strip->SetPixel((uint8_t)led, colorCRGB);
        //TODO: Hier Prüfung, ob sich tatsächlich etwas verändert hat und ein Update tatsächlich erforderlich ist
        this->needLedStripUpdate = true;
        return ErrorCode::OK;
    }

    ErrorCode UnColorizeAllLed()
    {
        strip->Clear(1000);
        this->needLedStripUpdate = true;
        return ErrorCode::OK;
    }

    ErrorCode SetRelayState(bool state)
    {
        gpio_set_level(PIN_R3_ON, state);
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
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, dutyInPercent);
        return ErrorCode::OK;
    }

    float GetFan1State()
    {
        return mcpwm_get_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A);
    }

    ErrorCode SetFan2Duty(float dutyInPercent)
    {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B, dutyInPercent);
        return ErrorCode::OK;
    }

    float GetFan2State()
    {
        return mcpwm_get_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B);
    }

    ErrorCode SetLedPowerWhiteDuty(uint8_t dutyInpercent)
    {
        if (dutyInpercent > 100)
            dutyInpercent = 100;
        uint32_t duty = (((2 ^ power_ledc_timer_duty_resolution) - 1) * dutyInpercent) / 100;
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
        return ErrorCode::OK;
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

    ErrorCode SetServo1Position(uint32_t angle_0_to_180)
    {
        uint32_t cal_pulsewidth = (SERVO_MIN_PULSEWIDTH + (((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * (angle_0_to_180)) / (SERVO_MAX_DEGREE)));
        esp_err_t err =  mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, cal_pulsewidth);
        return err==ESP_OK?ErrorCode::OK:ErrorCode::GENERIC_ERROR;
    }

    ErrorCode SetServo2Position(uint32_t angle_0_to_180)
    {
        uint32_t cal_pulsewidth = (SERVO_MIN_PULSEWIDTH + (((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * (angle_0_to_180)) / (SERVO_MAX_DEGREE)));
        esp_err_t err =  mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, cal_pulsewidth);
        return err==ESP_OK?ErrorCode::OK:ErrorCode::GENERIC_ERROR;
    }
};

void sensorTask(void *pvParameters)
{
    HAL_labathome *hal = (HAL_labathome *)pvParameters;
    hal->SensorLoop_ForInternalUseOnly();
}