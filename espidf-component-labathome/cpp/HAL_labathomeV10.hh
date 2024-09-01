#pragma once
#define LABATHOME_V10
#include "HAL.hh"

#include <inttypes.h>
#include <limits>
#include <algorithm>
#include <common.hh>
#include <common-esp32.hh>

#include <driver/mcpwm.h>
#include <driver/ledc.h>

#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>
#include <soc/adc_channel.h>
#include <driver/i2c_master.h>
#include <esp_check.h>
#include <ds18b20.hh>

#include <errorcodes.hh>
#include <rgbled.hh>
#include <bh1750.hh>
#include <bme280.hh>
#include <ads1115.hh>
#include <ccs811.hh>
#include <hdc1080.hh>
#include <aht_sensor.hh>
#include <vl53l0x.hh>
#include <rotenc.hh>
#include <AudioPlayer.hh>
#include <codec_manager_internal_dac.hh>

FLASH_FILE(alarm_co2_mp3)
FLASH_FILE(alarm_temperature_mp3)
FLASH_FILE(nok_mp3)
FLASH_FILE(ok_mp3)
FLASH_FILE(ready_mp3)
FLASH_FILE(fanfare_mp3)
FLASH_FILE(negative_mp3)
FLASH_FILE(positive_mp3)
FLASH_FILE(siren_mp3)
const uint8_t *SOUNDS[] = {nullptr, alarm_co2_mp3_start, alarm_temperature_mp3_start, nok_mp3_start, ok_mp3_start, ready_mp3_start, fanfare_mp3_start, negative_mp3_start, positive_mp3_start, siren_mp3_start};
const size_t SONGS_LEN[] = {0, alarm_co2_mp3_size, alarm_temperature_mp3_size, nok_mp3_size, ok_mp3_size, ready_mp3_size, fanfare_mp3_size, negative_mp3_size, positive_mp3_size, siren_mp3_size};

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
constexpr Pintype PIN_CONSOLE_UART_TX_ALTERNATIVE = (Pintype)21;
constexpr Pintype PIN_I2C_SCL = (Pintype)22;
constexpr Pintype PIN_SPI_CLK = (Pintype)23;
constexpr Pintype PIN_CONSOLE_UART_RX_ALTERNATIVE = (Pintype)23;
constexpr Pintype PIN_SPEAKER = (Pintype)25;
constexpr Pintype PIN_MULTI2 = (Pintype)26;
constexpr Pintype PIN_MULTI1 = (Pintype)27;
constexpr Pintype PIN_K3_ON = (Pintype)32;
constexpr Pintype PIN_MULTI3 = (Pintype)33;
constexpr Pintype PIN_ANALOGIN_OR_ROTB = (Pintype)34; //
constexpr adc_channel_t CHANNEL_ANALOGIN_OR_ROTB{(adc_channel_t)ADC1_GPIO34_CHANNEL};
// 36=VP, 39=VN
constexpr Pintype PIN_MOVEMENT_OR_FAN1SENSE = (Pintype)35;
constexpr adc_channel_t CHANNEL_MOVEMENT_OR_FAN1SENSE{(adc_channel_t)ADC1_GPIO35_CHANNEL};

constexpr Pintype PIN_SW = (Pintype)36;
constexpr adc_channel_t CHANNEL_SWITCHES{(adc_channel_t)ADC1_GPIO36_CHANNEL};

constexpr Pintype PIN_LDR_OR_ROTA = (Pintype)39;
constexpr adc_channel_t CHANNEL_LDR_OR_ROTA{(adc_channel_t)ADC1_GPIO39_CHANNEL};

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

enum class MODE_HEATER_OR_LED_POWER : uint8_t
{
    HEATER = 1,
    LED_POWER = 2,
};

enum class MODE_MOVEMENT_OR_FAN1SENSE : uint8_t
{
    MOVEMENT_SENSOR = 1,
    FAN1SENSE = 2,
};

enum class MODE_FAN1_OR_SERVO1 : uint8_t
{
    FAN1 = 1,
    SERVO1 = 2,
};

enum class Button : uint8_t
{
    BUT_ENCODER = 1,
    BUT_RED = 0,
    BUT_GREEN = 2,
};

// measurement and calibration constants
constexpr uint32_t DEFAULT_VREF{1100}; // Use adc2_vref_to_gpio() to obtain a better estimate
constexpr uint16_t sw_limits[]{160, 480, 1175, 1762, 2346, 2779, 3202};
constexpr float SERVO_MIN_PULSEWIDTH{500.0};  // Minimum pulse width in microsecond
constexpr float SERVO_MAX_PULSEWIDTH{2400.0}; // Maximum pulse width in microsecond
constexpr float SERVO_MAX_DEGREE{180.0};      // Maximum angle in degree upto which servo can rotate

constexpr int FREQUENCY_HEATER{1}; // see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/mcpwm.html#resolution
constexpr int FREQUENCY_SERVO{50};
constexpr int FREQUENCY_FAN{100};
constexpr int FREQUENCY_LED{300};

// used resources
constexpr i2c_port_t I2C_PORT{I2C_NUM_1};

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

constexpr i2s_port_t I2S_PORT_LOUDSPEAKER{I2S_NUM_0}; // must be I2S_NUM_0, as only this hat access to internal DAC

// ressource sizes
constexpr size_t ANALOG_INPUTS_LEN{4};
constexpr size_t LED_NUMBER{4};
constexpr size_t ONEWIRE_MAX_DS18B20{2};

class HAL_Impl : public HAL
{
private:
    // config
    MODE_MOVEMENT_OR_FAN1SENSE mode_MOVEMENT_OR_FAN1SENSE;
    MODE_HEATER_OR_LED_POWER mode_HEATER_OR_LED_POWER{MODE_HEATER_OR_LED_POWER::HEATER}; // Heater mit 1Hz, LED mit 300Hz
    MODE_FAN1_OR_SERVO1 mode_FAN1_OR_SERVO1{MODE_FAN1_OR_SERVO1::SERVO1};

    // management objects
    adc_oneshot_unit_handle_t adc1_handle;
    adc_cali_handle_t adc1_cali_handle{nullptr};
    RGBLED::M<LED_NUMBER, RGBLED::DeviceType::WS2812> *strip{nullptr};
    cRotaryEncoder *rotenc{nullptr};
    AudioPlayer::Player *mp3player;

    // Busses and bus devices
    i2c_master_bus_handle_t bus_handle;
    CCS811::M *ccs811dev{nullptr};
    AHT::M *aht21dev{nullptr};
    VL53L0X::M *vl53l0xdev{nullptr};
    BME280::M *bme280dev{nullptr};
    BH1750::M *bh1750dev{nullptr};

    OneWire::OneWireBus<PIN_ONEWIRE> *oneWireBus{nullptr};

    // SensorValues
    float AnalogInputVoltage_volts[ANALOG_INPUTS_LEN] = {0};
    uint32_t buttonState{0}; // see Button-Enum for meaning of bits
    // int rotaryDetents rotary-Encoder Value in Object
    bool movementIsDetected{false};
    uint16_t voltageUSBPD_50Millivolts{100};
    uint16_t ambientBrightnessLDR_Lux{UINT16_MAX};                               // BH1750 oder Analog
    uint16_t ambientBrightnessBH1750_Lux{UINT16_MAX};                            // BH1750 oder Analog
    float heaterTemperature_DegCel{std::numeric_limits<float>::quiet_NaN()};     // höherer DS18B20
    float airTemperatureDS18B20_DegCel{std::numeric_limits<float>::quiet_NaN()}; // wenn es zwei DS18B20 gibt, dann der niedrigere Wert
    float airTemperatureAHT21_DegCel{std::numeric_limits<float>::quiet_NaN()};
    float airRelHumidityAHT21_Percent{std::numeric_limits<float>::quiet_NaN()};
    float airTemperatureBME280_DegCel{std::numeric_limits<float>::quiet_NaN()};
    float airRelHumidityBME280_Percent{std::numeric_limits<float>::quiet_NaN()};
    float airPressureBME280_Pa{std::numeric_limits<float>::quiet_NaN()};

    uint16_t airCo2CCS811_PPM{0};

    uint16_t distanceMillimeters{UINT16_MAX};

    bool heaterEmergencyShutdown{false};

    int sound{0};

    void MP3Loop()
    {
        CodecManager::InternalDacWithPotentiometer *codec = new CodecManager::InternalDacWithPotentiometer();
        mp3player = new AudioPlayer::Player(codec);
        mp3player->Init();

        while (true)
        {
            mp3player->Loop();
        }
    }

    void readBinaryAndAnalogIOs()
    {

        int adc_reading;
        int adc_calibrated;

        // Es wird nicht unterschieden, ob der eingang "nur" zum Messen von analogen Spannung verwendet wird oder ob es sich um den Trigger-Eingang des Zeitrelais handelt. Im zweiten Fall muss einfach eine Zeitrelais-Schaltung basierend auf der Grenzüberschreitung des Analogen Messwertes in der Funktionsblock-Spache realisiert werden
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, CHANNEL_ANALOGIN_OR_ROTB, &adc_reading));
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_reading, &adc_calibrated));
        this->AnalogInputVoltage_volts[0] = adc_calibrated / 1000.; // die Multiplikation mit 11 wegen dem Spannungsteiler

        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, CHANNEL_MOVEMENT_OR_FAN1SENSE, &adc_reading));
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_reading, &adc_calibrated));
        this->AnalogInputVoltage_volts[1] = adc_calibrated / 1000.; // die Multiplikation mit 11 wegen dem Spannungsteiler

        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, CHANNEL_LDR_OR_ROTA, &adc_reading));
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_reading, &adc_calibrated));
        this->AnalogInputVoltage_volts[2] = adc_calibrated / 1000.;

        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, CHANNEL_SWITCHES, &adc_reading));
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_reading, &adc_calibrated));
        this->AnalogInputVoltage_volts[3] = adc_calibrated / 1000.;
        int i = 0;
        for (i = 0; i < sizeof(sw_limits) / sizeof(uint16_t); i++)
        {
            if (adc_reading < sw_limits[i])
                break;
        }
        i = ~i;
        this->buttonState = i;
        this->movementIsDetected = gpio_get_level(PIN_MOVEMENT_OR_FAN1SENSE);
    }

    void SensorLoop()
    {

        int64_t nextBinaryAndAnalogReadout{0};
        bh1750dev = new BH1750::M(bus_handle, BH1750::ADDRESS::LOW, BH1750::OPERATIONMODE::CONTINU_H_RESOLUTION);
        bme280dev = new BME280::M(bus_handle, BME280::ADDRESS::PRIM);
        ccs811dev = new CCS811::M(bus_handle, CCS811::ADDRESS::ADDR0, CCS811::MODE::_1SEC, (gpio_num_t)GPIO_NUM_NC);
        aht21dev = new AHT::M(bus_handle, AHT::ADDRESS::DEFAULT_ADDRESS);
        vl53l0xdev = new VL53L0X::M(bus_handle);
        oneWireBus = new OneWire::OneWireBus<PIN_ONEWIRE>();
        oneWireBus->Init();

        while (true)
        {
            if (GetMillis64() > nextBinaryAndAnalogReadout)
            {
                readBinaryAndAnalogIOs();
                nextBinaryAndAnalogReadout = GetMillis64() + 100;

                if (!mp3player->IsEmittingSamples())
                {
                    this->sound = 0;
                }
            }

            oneWireBus->Loop(GetMillis64());
            this->heaterTemperature_DegCel = oneWireBus->GetMaxTemp();
            this->airTemperatureDS18B20_DegCel = oneWireBus->GetMinTemp();

            bh1750dev->Loop(GetMillis64());
            ccs811dev->Loop(GetMillis64());
            aht21dev->Loop(GetMillis64());
            vl53l0xdev->Loop(GetMillis64());

            if (ccs811dev->HasValidData())
            {
                this->airCo2CCS811_PPM = ccs811dev->Get_eCO2();
            }
            if (vl53l0xdev->HasValidData())
            {
                this->distanceMillimeters = vl53l0xdev->ReadMillimeters();
            }
            if (this->aht21dev->HasValidData())
            {
                this->aht21dev->Read(this->airRelHumidityAHT21_Percent, this->airTemperatureAHT21_DegCel);
            }

            if (this->bh1750dev->HasValidData())
            {
                this->bh1750dev->Read(this->ambientBrightnessBH1750_Lux);
            }
            vTaskDelay(1);
        }
    }

public:
    HAL_Impl(
        MODE_MOVEMENT_OR_FAN1SENSE mode_MOVEMENT_OR_FAN1SENSE) : mode_MOVEMENT_OR_FAN1SENSE(mode_MOVEMENT_OR_FAN1SENSE)
    {
    }

    void DoMonitoring() override
    {
        static int64_t nextOneLineStatus{0};
        int64_t now = millis();
        if (now > nextOneLineStatus)
        {
            uint32_t heap = esp_get_free_heap_size();
            bool red = GetButtonRedIsPressed();
            bool yel = GetButtonEncoderIsPressed();
            bool grn = GetButtonGreenIsPressed();
            bool mov = IsMovementDetected();
            float htrTemp{0.f};
            int enc{0};
            GetEncoderValue(&enc);
            int32_t sound{0};
            GetSound(&sound);
            float spply = GetUSBCVoltage();

            float bright{0};
            GetAmbientBrightness(&bright);

            GetHeaterTemperature(&htrTemp);
            float airTemp{0.f};
            GetAirTemperature(&airTemp);
            float airPres{0};
            GetAirPressure(&airPres);
            float airHumid{0.f};
            GetAirRelHumidity(&airHumid);
            float co2{0};
            GetCO2PPM(&co2);
            float *analogVolt{nullptr};
            GetAnalogInputs(&analogVolt);
            ESP_LOGI(TAG, "Heap %6lu  RED %d YEL %d GRN %d MOV %d ENC %i SOUND %ld SUPPLY %4.1f BRGHT %4f HEAT %4.1f AIRT %4.1f AIRPRS %5f AIRHUM %3.0f CO2 %5f, ANALOGIN [%4.1f %4.1f]",
                     heap, red, yel, grn, mov, enc, sound, spply, bright, htrTemp, airTemp, airPres, airHumid, co2, analogVolt[0], analogVolt[1]);
            nextOneLineStatus += 5000;
        }
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

    ErrorCode GetAnalogInputs(float **volts)
    {
        *volts = this->AnalogInputVoltage_volts;
        return ErrorCode::OK;
    }

    ErrorCode GetEncoderValue(int *value)
    {
        bool isPressed;
        int16_t val;
        ErrorCode err = this->rotenc->GetValue(val, isPressed) == ESP_OK ? ErrorCode::OK : ErrorCode::GENERIC_ERROR;
        *value = val;
        return err;
    }

    ErrorCode GetDistanceMillimeters(uint16_t *value)
    {
        *value = this->distanceMillimeters;
        return ErrorCode::OK;
    }

    ErrorCode GetFan1Rpm(float *rpm)
    {
        *rpm = std::numeric_limits<float>::quiet_NaN();
        return ErrorCode::OK;
    }

    ErrorCode SetSound(int32_t soundNumber)
    {
        if (soundNumber < 0 || soundNumber >= sizeof(SOUNDS) / sizeof(uint8_t *))
        {
            soundNumber = 0;
        }
        this->sound = soundNumber;
        mp3player->PlayMP3(SOUNDS[soundNumber], SONGS_LEN[soundNumber], 255, true);
        ESP_LOGI(TAG, "Set Sound to %ld", soundNumber);
        return ErrorCode::OK;
    }

    ErrorCode GetSound(int32_t *soundNumber)
    {
        *soundNumber = this->sound;
        return ErrorCode::OK;
    }

    ErrorCode GetCO2PPM(float *co2PPM) override
    {
        *co2PPM = this->airCo2CCS811_PPM;
        return ErrorCode::OK;
    }

    ErrorCode GetHeaterTemperature(float *degreesCelcius)
    {
        *degreesCelcius = this->heaterTemperature_DegCel;
        return ErrorCode::OK;
    }

    ErrorCode GetAirTemperature(float *degreesCelcius) override
    {
        if (!std::isnan(this->airTemperatureDS18B20_DegCel))
        {
            *degreesCelcius = this->airTemperatureDS18B20_DegCel;
        }
        else
        {
            *degreesCelcius = this->airTemperatureAHT21_DegCel;
        }
        return ErrorCode::OK;
    }

    ErrorCode GetAirTemperatureDS18B20(float *degreesCelcius) override
    {
        *degreesCelcius = this->airTemperatureDS18B20_DegCel;
        return ErrorCode::OK;
    }

    ErrorCode GetAirTemperatureAHT21(float *degreesCelcius) override
    {
        *degreesCelcius = this->airTemperatureAHT21_DegCel;
        return ErrorCode::OK;
    }

    ErrorCode GetAirTemperatureBME280(float *degreesCelcius) override
    {
        *degreesCelcius = this->airTemperatureBME280_DegCel;
        return ErrorCode::OK;
    }

    ErrorCode GetAirPressure(float *pa) override
    {
        *pa = this->airPressureBME280_Pa / 100.0;
        return ErrorCode::OK;
    }

    ErrorCode GetAirQuality(float *qualityPercent) override
    {
        *qualityPercent = std::numeric_limits<float>::quiet_NaN();
        return ErrorCode::OK;
    }

    ErrorCode GetAirRelHumidity(float *percent) override
    {
        *percent = this->airRelHumidityAHT21_Percent;
        return ErrorCode::OK;
    }

    ErrorCode GetAirRelHumidityAHT21(float *percent) override
    {
        *percent = this->airRelHumidityAHT21_Percent;
        return ErrorCode::OK;
    }

    ErrorCode GetAirRelHumidityBME280(float *percent) override
    {
        *percent = this->airRelHumidityBME280_Percent;
        return ErrorCode::OK;
    }

    ErrorCode GetAirSpeed(float *meterPerSecond)
    {
        *meterPerSecond = 0;
        return ErrorCode::OK;
    }

    ErrorCode GetAmbientBrightness(float *lux) override
    {
        // digital value has priority
        *lux = this->ambientBrightnessBH1750_Lux != UINT16_MAX ? this->ambientBrightnessBH1750_Lux : this->ambientBrightnessLDR_Lux;
        return ErrorCode::OK;
    }

    ErrorCode GetAmbientBrightnessAnalog(float *lux) override
    {
        *lux = this->ambientBrightnessLDR_Lux;
        return ErrorCode::OK;
    }

    ErrorCode GetAmbientBrightnessDigital(float *lux) override
    {
        *lux = this->ambientBrightnessBH1750_Lux;
        return ErrorCode::OK;
    }

    ErrorCode GetWifiRssiDb(float *db)
    {
        *db = 0;
        return ErrorCode::OK;
    }

    ErrorCode SetAnalogOutput(uint8_t outputIndex, float volts)
    {

        // mp3player.OutputConstantVoltage(volts);
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
        config.atten = ADC_ATTEN_DB_12;
        ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, CHANNEL_ANALOGIN_OR_ROTB, &config));
        ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, CHANNEL_MOVEMENT_OR_FAN1SENSE, &config));
        ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, CHANNEL_LDR_OR_ROTA, &config));
        //-------------ADC1 Calibration Init---------------//
        ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {};
        cali_config.unit_id = init_config1.unit_id;
        cali_config.atten = ADC_ATTEN_DB_12;
        cali_config.bitwidth = ADC_BITWIDTH_DEFAULT;
        ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&cali_config, &this->adc1_cali_handle));

        // Rotary Encoder Input
        rotenc = new cRotaryEncoder(PIN_LDR_OR_ROTA, PIN_ANALOGIN_OR_ROTB, GPIO_NUM_NC);
        ESP_ERROR_CHECK(rotenc->Init());
        ESP_ERROR_CHECK(rotenc->Start());

        // CHANNEL_MOVEMENT_OR_FAN1SENSE Input (Auch wenn es nur ein Input ist, muss das passieren, weil dieser Input sonst nur einen Analogwert liefern würde)
        ConfigGpioInput(PIN_MOVEMENT_OR_FAN1SENSE, GPIO_FLOATING);

        // Relay K3 output
        gpio_set_level(PIN_K3_ON, 0);
        ConfigGpioOutputPP(PIN_K3_ON);

        // Hinweis (vermutlich schwer zu verstehen...:-()):
        //- beim PIN_FAN1_DRIVE_OR_SERVO1 wird je nach Modus der GPIO vom einem zum anderen Timer umgeschaltet, weil es noch einen anderen FAN/Servo gibt und dessen freuqenz ja nicht beeinflusst werden darf
        //- beim PIN_HEATER_OR_LED_POWER wird je nach Modus die Frequenz des Timers umgeschaltet, weil es nur einen Timer gibt

        ESP_ERROR_CHECK(mcpwm_group_set_resolution(MCPWM_UNIT_0, 1000000));
        // MCPWM GPIO
        if (mode_FAN1_OR_SERVO1 == MODE_FAN1_OR_SERVO1::FAN1)
        {
            ESP_ERROR_CHECK(mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_IO_FAN1, PIN_FAN1_DRIVE_OR_SERVO1));
        }
        else if (mode_FAN1_OR_SERVO1 == MODE_FAN1_OR_SERVO1::SERVO1)
        {
            ESP_ERROR_CHECK(mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_IO_SERVO1, PIN_FAN1_DRIVE_OR_SERVO1));
        }
        ESP_ERROR_CHECK(mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_IO_SERVO2, PIN_SPI_IO1_OR_SERVO2));
        ESP_ERROR_CHECK(mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_IO_FAN2, PIN_FAN2_DRIVE));
        ESP_ERROR_CHECK(mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_IO_HEATER_OR_LED_POWER, PIN_HEATER_OR_LED_POWER));
        ESP_ERROR_CHECK(mcpwm_timer_set_resolution(MCPWM_UNIT_0, MCPWM_TIMER_HEATER_OR_LED_POWER, 10000));

        // MCPWM base config
        mcpwm_config_t pwm_config;
        pwm_config.cmpr_a = 0; // duty cycle of PWMxA = 0
        pwm_config.cmpr_b = 0; // duty cycle of PWMxb = 0
        pwm_config.counter_mode = MCPWM_UP_COUNTER;
        pwm_config.duty_mode = MCPWM_DUTY_MODE_0;

        pwm_config.frequency = FREQUENCY_SERVO;
        ESP_ERROR_CHECK(mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_SERVO, &pwm_config));
        pwm_config.frequency = FREQUENCY_FAN;
        ESP_ERROR_CHECK(mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_FAN, &pwm_config));
        pwm_config.frequency = FREQUENCY_HEATER;
        ESP_ERROR_CHECK(mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_HEATER_OR_LED_POWER, &pwm_config));

        // I2C Master
        // I2C Master
        i2c_master_bus_config_t i2c_mst_config = {
            .i2c_port = I2C_PORT,
            .sda_io_num = PIN_I2C_SDA,
            .scl_io_num = PIN_I2C_SCL,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0,
            //.enable_internal_pullup=0,
            .flags = 0,
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));

        for (uint8_t i = 0; i < 128; i++)
        {
            if (i2c_master_probe(bus_handle, i, 50) != ESP_ERR_NOT_FOUND)
            {
                ESP_LOGI(TAG, "Found I2C-Device @ 0x%02X", i);
            }
        }

        // LED Strip
        strip = new RGBLED::M<LED_NUMBER, RGBLED::DeviceType::WS2812>();
        ERRORCODE_CHECK(strip->Begin(SPI3_HOST, PIN_LED_WS2812));
        ERRORCODE_CHECK(strip->Clear(100));

        readBinaryAndAnalogIOs(); // do this while init to avoid race condition (wifimanager is resettet when red and green buttons are pressed during startup)
        xTaskCreate(sensorTask, "sensorTask", 4096 * 4, this, 6, nullptr);
        xTaskCreate(mp3Task, "mp3task", 6144 * 4, this, 16, nullptr); // Stack Size = 4096 --> Stack overflow!!

        return ErrorCode::OK;
    }

    ErrorCode BeforeLoop()
    {
        if (this->heaterTemperature_DegCel > 85)
        {
            ESP_LOGE(TAG, "Emergency Shutdown. Heater Temperature too high!!!");
            this->SetHeaterDuty(0);
            this->heaterEmergencyShutdown = true;
        }
        return ErrorCode::OK;
    }

    ErrorCode AfterLoop()
    {
        strip->Refresh(100); // checks internally, whether data is dirty and has to be pushed out
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
        if (ledIndex >= LED_NUMBER)
            return ErrorCode::INDEX_OUT_OF_BOUNDS;
        strip->SetPixel(LED_NUMBER - ledIndex - 1, colorCRGB);
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
        if (this->heaterEmergencyShutdown)
        {
            ESP_ERROR_CHECK(mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_HEATER_OR_LED_POWER, MCPWM_GEN_HEATER_OR_LED_POWER, 0));
            return ErrorCode::EMERGENCY_SHUTDOWN;
        }
        if (mode_HEATER_OR_LED_POWER != MODE_HEATER_OR_LED_POWER::HEATER)
        {
            ESP_ERROR_CHECK(mcpwm_set_frequency(MCPWM_UNIT_0, MCPWM_TIMER_HEATER_OR_LED_POWER, FREQUENCY_HEATER));
            mode_HEATER_OR_LED_POWER = MODE_HEATER_OR_LED_POWER::LED_POWER;
        }
        dutyInPercent = std::max(0.0f, dutyInPercent);
        dutyInPercent = std::min(100.0f, dutyInPercent);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_HEATER_OR_LED_POWER, MCPWM_GEN_HEATER_OR_LED_POWER, dutyInPercent);
        return ErrorCode::OK;
    }

    float GetHeaterState()
    {
        return mcpwm_get_duty(MCPWM_UNIT_0, MCPWM_TIMER_HEATER_OR_LED_POWER, MCPWM_GEN_HEATER_OR_LED_POWER);
    }

    ErrorCode SetServoPosition(uint8_t servoIndex, float angle_0_to_180)
    {
        if (servoIndex == 0)
        {
            if (mode_FAN1_OR_SERVO1 != MODE_FAN1_OR_SERVO1::SERVO1)
            {
                ESP_ERROR_CHECK(mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_IO_SERVO1, PIN_FAN1_DRIVE_OR_SERVO1));
                mode_FAN1_OR_SERVO1 = MODE_FAN1_OR_SERVO1::SERVO1;
            }
            uint32_t cal_pulsewidth = (SERVO_MIN_PULSEWIDTH + (((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * (angle_0_to_180)) / (SERVO_MAX_DEGREE)));
            esp_err_t err = mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_SERVO, MCPWM_GEN_SERVO1, cal_pulsewidth);
            return err == ESP_OK ? ErrorCode::OK : ErrorCode::GENERIC_ERROR;
        }
        else
        {
            uint32_t cal_pulsewidth = (SERVO_MIN_PULSEWIDTH + (((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * (angle_0_to_180)) / (SERVO_MAX_DEGREE)));
            esp_err_t err = mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_SERVO, MCPWM_GEN_SERVO2, cal_pulsewidth);
            return err == ESP_OK ? ErrorCode::OK : ErrorCode::GENERIC_ERROR;
        }
    }

    ErrorCode SetFanDuty(uint8_t fanIndex, float dutyInPercent)
    {
        if (fanIndex == 0)
        {
            if (mode_FAN1_OR_SERVO1 != MODE_FAN1_OR_SERVO1::FAN1)
            {
                ESP_ERROR_CHECK(mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_IO_FAN1, PIN_FAN1_DRIVE_OR_SERVO1));
                mode_FAN1_OR_SERVO1 = MODE_FAN1_OR_SERVO1::FAN1;
            }
            dutyInPercent = std::max(0.0f, dutyInPercent);
            dutyInPercent = std::min(100.0f, dutyInPercent);
            mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_FAN, MCPWM_GEN_FAN1, dutyInPercent);
            return ErrorCode::OK;
        }
        else
        {
            mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_FAN, MCPWM_GEN_FAN2, dutyInPercent);
            return ErrorCode::OK;
        }
    }

    ErrorCode GetFanDuty(uint8_t fanIndex, float *dutyInPercent)
    {
        if (fanIndex == 0)
        {
            *dutyInPercent = mcpwm_get_duty(MCPWM_UNIT_0, MCPWM_TIMER_FAN, MCPWM_GEN_FAN1);
            return ErrorCode::OK;
        }
        else
        {
            *dutyInPercent = mcpwm_get_duty(MCPWM_UNIT_0, MCPWM_TIMER_FAN, MCPWM_GEN_FAN2);
            return ErrorCode::OK;
        }
    }

    ErrorCode SetLedPowerWhiteDuty(float dutyInPercent)
    {
        if (mode_HEATER_OR_LED_POWER != MODE_HEATER_OR_LED_POWER::LED_POWER)
        {
            mcpwm_set_frequency(MCPWM_UNIT_0, MCPWM_TIMER_HEATER_OR_LED_POWER, FREQUENCY_LED);
            mode_HEATER_OR_LED_POWER = MODE_HEATER_OR_LED_POWER::LED_POWER;
        }
        dutyInPercent = std::max(0.0f, dutyInPercent);
        dutyInPercent = std::min(100.0f, dutyInPercent);
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

    float GetUSBCVoltage()
    {
        return voltageUSBPD_50Millivolts * 0.05f;
    }

    ErrorCode GreetUserOnStartup() override
    {
        for (int i = 0; i < 3; i++)
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
};
