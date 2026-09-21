#pragma once
// HAL fuer Labathome Rev. 5.x (klassischer ESP32, WROOM/WROVER, 4 MB Flash, kein USB-OTG).
// Alle Ein-/Ausgaenge haengen direkt am ESP32 (im Gegensatz zu Rev. 15, wo ein STM32-Koprozessor
// die IOs bedient). Umgeschrieben auf iHAL und die "neuen" ESP-IDF-Treiber (ab IDF 6.0 sind die
// Legacy-Treiber driver/mcpwm.h, driver/i2c.h, driver/adc.h entfernt).
//
// Nicht umgesetzt (wie bisher): die per Jumper umschaltbaren Multi-Pin-Funktionen. Es gilt die
// Standard-Bestueckung: GPIO33 = Servo 2, MULTI1..3 = I2S (Mikrofon, ungenutzt), kein RS485/CAN.
#include "iHAL.hh"

#include <inttypes.h>
#include <limits>
#include <algorithm>
#include <cmath>
#include <common.hh>
#include <common-esp32.hh>

#include <driver/gpio.h>
#include <driver/mcpwm_prelude.h>
#include <driver/i2c_master.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_check.h>

#include "../i2c_discover.hh"
#include <errorcodes.hh>
#include <rgb_strip.hh>
#include <bh1750.hh>
#include <bme280.hh>
#include <ccs811.hh>
#include <ds18b20.hh>
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

// Pins (GPIO 34..39 sind reine Eingaenge)
constexpr gpio_num_t PIN_K3_1 = GPIO_NUM_36;         // Relais K3, Rueckmeldung
constexpr gpio_num_t PIN_MOVEMENT = GPIO_NUM_39;     // Bewegungsmelder
constexpr adc_channel_t CHANNEL_SWITCHES = ADC_CHANNEL_6; // GPIO34: Widerstandsleiter der drei Taster
constexpr gpio_num_t PIN_ROTENC_A = GPIO_NUM_35;
constexpr gpio_num_t PIN_ROTENC_B = GPIO_NUM_15;
constexpr gpio_num_t PIN_FAN2_DRIVE = GPIO_NUM_32;
constexpr gpio_num_t PIN_SERVO2 = GPIO_NUM_33;       // Standard-Bestueckung (Alternative: Fan1-Tacho)
constexpr gpio_num_t PIN_LED_WS2812 = GPIO_NUM_26;
constexpr gpio_num_t PIN_ONEWIRE = GPIO_NUM_14;
constexpr gpio_num_t PIN_FAN1_DRIVE = GPIO_NUM_12;
constexpr gpio_num_t PIN_LED_POWER_WHITE = GPIO_NUM_13;
constexpr gpio_num_t PIN_I2C_SDA = GPIO_NUM_22;
constexpr gpio_num_t PIN_I2C_SCL = GPIO_NUM_21;
constexpr gpio_num_t PIN_HEATER = GPIO_NUM_17;
constexpr gpio_num_t PIN_K3_ON = GPIO_NUM_2;         // Relais K3, Ansteuerung
// Lautsprecher: interner DAC, Kanal 0 = GPIO25 (s. CodecManager::InternalDacWithPotentiometer)

enum class Button : uint8_t
{
    BUT_GREEN = 1,
    BUT_ENCODER = 0,
    BUT_RED = 2,
};

constexpr size_t ANALOG_INPUTS_LEN{1};
constexpr size_t LED_NUMBER{8};
constexpr i2c_port_t I2C_PORT{I2C_NUM_0};
constexpr uint16_t sw_limits[]{160, 480, 1175, 1762, 2346, 2779, 3202};
constexpr float SERVO_MIN_PULSEWIDTH{500.0};  // Minimum pulse width in microsecond
constexpr float SERVO_MAX_PULSEWIDTH{2400.0}; // Maximum pulse width in microsecond
constexpr float SERVO_MAX_DEGREE{180.0};      // Maximum angle in degree upto which servo can rotate

// PWM-Ausgaenge: Heizung 1 Hz (thermisch traege), Luefter 100 Hz, Servo 50 Hz
constexpr uint32_t PWM_HEATER_RESOLUTION_HZ{10000};
constexpr uint32_t PWM_HEATER_PERIOD_TICKS{10000};
// Der MCPWM-Gruppentakt des ESP32 (160 MHz) laesst sich nicht auf exakt 1 MHz teilen; die Treiber runden auf
// 1,25 MHz (s. Warnung "adjust timer resolution"). Deshalb direkt mit 1,25 MHz rechnen.
constexpr uint32_t PWM_FAN_RESOLUTION_HZ{1250000};
constexpr uint32_t PWM_FAN_PERIOD_TICKS{12500};    // 100 Hz
constexpr uint32_t PWM_SERVO_RESOLUTION_HZ{1250000};
constexpr uint32_t PWM_SERVO_PERIOD_TICKS{25000};  // 50 Hz

#define TAG "HAL"

// Ein MCPWM-Ausgang (Timer+Operator+Vergleicher+Generator). Duty 0 % bzw. 100 % wird ueber
// "force level" erzeugt, damit keine Glitches durch gleichzeitige Timer-/Vergleicher-Ereignisse entstehen.
class PwmOutput
{
private:
    mcpwm_cmpr_handle_t cmpr{nullptr};
    mcpwm_gen_handle_t gen{nullptr};
    uint32_t periodTicks{0};
    uint32_t resolutionHz{0};
    float dutyPercent{0.0f};

public:
    // Erzeugt einen zweiten Ausgang auf einem bereits vorhandenen Operator (jeder Operator hat 2 Generatoren)
    esp_err_t Init(mcpwm_oper_handle_t oper, gpio_num_t pin, uint32_t resolutionHz_, uint32_t periodTicks_)
    {
        resolutionHz = resolutionHz_;
        periodTicks = periodTicks_;
        mcpwm_comparator_config_t cmprCfg = {};
        cmprCfg.flags.update_cmp_on_tez = true;
        ESP_RETURN_ON_ERROR(mcpwm_new_comparator(oper, &cmprCfg, &cmpr), TAG, "comparator");
        mcpwm_generator_config_t genCfg = {};
        genCfg.gen_gpio_num = pin;
        ESP_RETURN_ON_ERROR(mcpwm_new_generator(oper, &genCfg, &gen), TAG, "generator");
        ESP_RETURN_ON_ERROR(mcpwm_generator_set_action_on_timer_event(gen, MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)), TAG, "timer action");
        ESP_RETURN_ON_ERROR(mcpwm_generator_set_action_on_compare_event(gen, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, cmpr, MCPWM_GEN_ACTION_LOW)), TAG, "compare action");
        return SetDutyPercent(0.0f);
    }

    esp_err_t SetDutyPercent(float percent)
    {
        percent = std::min(100.0f, std::max(0.0f, percent));
        dutyPercent = percent;
        if (percent <= 0.0f)
            return mcpwm_generator_set_force_level(gen, 0, true);
        if (percent >= 100.0f)
            return mcpwm_generator_set_force_level(gen, 1, true);
        ESP_RETURN_ON_ERROR(mcpwm_comparator_set_compare_value(cmpr, (uint32_t)(periodTicks * percent / 100.0f)), TAG, "compare value");
        return mcpwm_generator_set_force_level(gen, -1, true); // -1 = Force-Level aufheben
    }

    // Impulsbreite in Mikrosekunden (fuer Servos); 0 = kein Impuls
    esp_err_t SetPulseWidthUs(uint32_t us)
    {
        if (us == 0)
            return mcpwm_generator_set_force_level(gen, 0, true);
        ESP_RETURN_ON_ERROR(mcpwm_comparator_set_compare_value(cmpr, (uint32_t)((uint64_t)us * resolutionHz / 1000000ULL)), TAG, "compare value");
        return mcpwm_generator_set_force_level(gen, -1, true);
    }

    float GetDutyPercent() const { return dutyPercent; }
};

class HAL_Impl : public iHAL
{
private:
    // management objects
    adc_oneshot_unit_handle_t adc1_handle{nullptr};
    i2c_master_bus_handle_t i2c_master_handle{nullptr};
    BH1750::M *bh1750dev{nullptr};
    CCS811::M *ccs811dev{nullptr};
    BME280::M *bme280dev{nullptr};
    OneWire::OneWireBus<PIN_ONEWIRE> *oneWireBus{nullptr};
    led::RgbStrip<LED_NUMBER, led::DeviceType::WS2812> *strip{nullptr};
    cRotaryEncoder *rotenc{nullptr};
    AudioPlayer::Player *mp3player{nullptr};
    CodecManager::InternalDacWithPotentiometer *dacCodec{nullptr};

    PwmOutput heater;
    PwmOutput fan1;
    PwmOutput fan2;
    PwmOutput servo2;

    // SensorValues
    uint32_t buttonState{0}; // see Button-Enum for meaning of bits
    bool movementIsDetected{false};
    float wifiRssiDb{std::numeric_limits<float>::quiet_NaN()};
    float analogInputsVolt[ANALOG_INPUTS_LEN] = {};
    uint32_t sound{0};

    // Safety
    bool heaterEmergencyShutdown{false};

    void readBinaryAndAnalogIOs()
    {
        int adc_reading;
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, CHANNEL_SWITCHES, &adc_reading));
        int i = 0;
        for (i = 0; i < (int)(sizeof(sw_limits) / sizeof(uint16_t)); i++)
        {
            if (adc_reading < sw_limits[i])
                break;
        }
        i = ~i;
        this->buttonState = i;
        this->movementIsDetected = gpio_get_level(PIN_MOVEMENT);
    }

    void HalLoop()
    {
        TickType_t lastWakeTime = xTaskGetTickCount();
        const TickType_t FREQUENCY = pdMS_TO_TICKS(50);
        int64_t nextBinaryAndAnalogReadout{0};
        while (true)
        {
            xTaskDelayUntil(&lastWakeTime, FREQUENCY);
            int64_t now = GetMillis64_1024();
            if (oneWireBus)
                oneWireBus->Loop(now);
            bme280dev->Loop(now);
                bh1750dev->Loop(now);
                ccs811dev->Loop(now);
            if (GetMillis64() > nextBinaryAndAnalogReadout)
            {
                readBinaryAndAnalogIOs();
                nextBinaryAndAnalogReadout = GetMillis64() + 100;
            }
        }
    }

    void AudioLoop()
    {
        // Waehrend ein Sound laeuft: enge Schleife, Loop() blockiert im DAC-Write und haelt den DMA-Puffer
        // gefuellt (ein getakteter Aufruf liesse ihn leerlaufen). Im Leerlauf kehrt Loop() sofort zurueck,
        // dort schlafen (sonst Task-Watchdog) und den DAC abschalten (sonst wiederholt der DMA den letzten
        // Puffer zyklisch -> Klackern im Lautsprecher).
        while (mp3player)
        {
            mp3player->Loop();
            if (!mp3player->IsEmittingSamples())
            {
                dacCodec->SetPowerState(false);
                this->sound = 0;
                vTaskDelay(pdMS_TO_TICKS(10));
            }
        }
    }

public:
    HAL_Impl() {}

    void DoMonitoring() override
    {
        static int64_t nextOneLineStatus{0};
        int64_t now = GetMillis64();
        if (now < nextOneLineStatus)
            return;
        nextOneLineStatus = now + 5000;
        uint32_t heap = esp_get_free_heap_size();
        bool red = GetButtonRedIsPressed();
        bool yel = GetButtonEncoderIsPressed();
        bool grn = GetButtonGreenIsPressed();
        bool mov = IsMovementDetected();
        float htrTemp{0.f};
        int enc{0};
        GetEncoderValue(&enc);
        int32_t snd{0};
        GetSound(&snd);
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
        ESP_LOGI(TAG, "Heap %6lu  RED %d YEL %d GRN %d MOV %d ENC %i SOUND %ld SUPPLY %4.1f BRGHT %4.1f HEAT %4.1f AIRT %4.1f AIRPRS %5.0f AIRHUM %3.0f CO2 %5.0f",
                 heap, red, yel, grn, mov, enc, snd, spply, bright, htrTemp, airTemp, airPres, airHumid, co2);
    }

    int64_t IRAM_ATTR GetMicros() override
    {
        return esp_timer_get_time();
    }

    uint32_t GetMillis() override
    {
        return (uint32_t)(esp_timer_get_time() / 1000ULL);
    }

    int64_t GetMillis64()
    {
        return esp_timer_get_time() / 1000ULL;
    }

    int64_t GetMillis64_1024()
    {
        return esp_timer_get_time() / 1024ULL;
    }

    ErrorCode GetAnalogInputs(float **voltages) override
    {
        *voltages = this->analogInputsVolt;
        return ErrorCode::OK;
    }

    ErrorCode GetSensorsAsJSON(char *buffer, size_t &maxLenInput_usedLen_Output) override
    {
        auto maxLen = maxLenInput_usedLen_Output;
        size_t used = 0;
        used += snprintf(buffer + used, maxLen - used, "{\"ds18b20\":");
        if (oneWireBus)
            used += this->oneWireBus->FormatJSON(buffer + used, maxLen - used);
        else
            used += snprintf(buffer + used, maxLen - used, "[]");
        used += snprintf(buffer + used, maxLen - used, "}");
        maxLenInput_usedLen_Output = used;
        return ErrorCode::OK;
    }

    ErrorCode GetEncoderValue(int *value) override
    {
        bool isPressed;
        int16_t val;
        ErrorCode err = this->rotenc->GetValue(val, isPressed) == ESP_OK ? ErrorCode::OK : ErrorCode::GENERIC_ERROR;
        *value = val;
        return err;
    }

    ErrorCode SetSound(int32_t soundNumber) override
    {
        if (!mp3player)
        {
            ESP_LOGW(TAG, "Audio Player not initialized!");
            return ErrorCode::OK;
        }
        if (soundNumber < 0 || soundNumber >= (int32_t)(sizeof(SOUNDS) / sizeof(uint8_t *)))
        {
            soundNumber = 0;
        }
        this->sound = soundNumber;
        mp3player->PlayMP3(SOUNDS[soundNumber], SONGS_LEN[soundNumber], 255, true);
        ESP_LOGI(TAG, "Set Sound to %ld", soundNumber);
        return ErrorCode::OK;
    }

    ErrorCode GetSound(int32_t *soundNumber) override
    {
        *soundNumber = this->sound;
        return ErrorCode::OK;
    }

    ErrorCode GetCO2PPM(float *co2PPM) override
    {
        *co2PPM = ccs811dev->HasValidData() ? (float)ccs811dev->Get_eCO2() : std::numeric_limits<float>::quiet_NaN();
        return ErrorCode::OK;
    }

    ErrorCode GetHeaterTemperature(float *degreesCelcius) override
    {
        if (!this->oneWireBus)
        {
            *degreesCelcius = std::numeric_limits<float>::quiet_NaN();
            return ErrorCode::GENERIC_ERROR;
        }
        *degreesCelcius = this->oneWireBus->GetMaxTemp();
        return ErrorCode::OK;
    }

    ErrorCode GetAirTemperature(float *degreesCelcius) override
    {
        return GetAirTemperatureBME280(degreesCelcius);
    }

    ErrorCode GetAirPressure(float *pa) override
    {
        float t, h;
        return ReadBme280(&t, pa, &h);
    }

    ErrorCode GetAirQuality(float *qualityPercent) override
    {
        *qualityPercent = std::numeric_limits<float>::quiet_NaN();
        return ErrorCode::OK;
    }

    ErrorCode GetAirRelHumidity(float *percent) override
    {
        return GetAirRelHumidityBME280(percent);
    }

    ErrorCode GetAirSpeed(float *meterPerSecond) override
    {
        *meterPerSecond = std::numeric_limits<float>::quiet_NaN();
        return ErrorCode::OK;
    }

    ErrorCode GetAmbientBrightness(float *lux) override
    {
        return GetAmbientBrightnessDigital(lux);
    }

    ErrorCode GetWifiRssiDb(float *db) override
    {
        *db = this->wifiRssiDb;
        return ErrorCode::OK;
    }

    ErrorCode SetAnalogOutput(uint8_t outputIndex, float volts) override
    {
        return ErrorCode::OK;
    }

    ErrorCode InitAndRun() override
    {
        //-------------ADC1 Init: Widerstandsleiter der Taster---------------//
        adc_oneshot_unit_init_cfg_t init_config1 = {};
        init_config1.unit_id = ADC_UNIT_1;
        ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));
        adc_oneshot_chan_cfg_t config = {};
        config.bitwidth = ADC_BITWIDTH_12;
        config.atten = ADC_ATTEN_DB_0;
        ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, CHANNEL_SWITCHES, &config));

        // Movement Sensor
        gpio_reset_pin(PIN_MOVEMENT);
        gpio_set_direction(PIN_MOVEMENT, GPIO_MODE_INPUT);
        gpio_set_pull_mode(PIN_MOVEMENT, GPIO_FLOATING);

        // Rotary Encoder Input
        rotenc = new cRotaryEncoder(PIN_ROTENC_A, PIN_ROTENC_B, GPIO_NUM_NC);
        ESP_ERROR_CHECK(rotenc->Init());
        ESP_ERROR_CHECK(rotenc->Start());

        // Relay K3: input (feedback) and output
        gpio_set_direction(PIN_K3_1, GPIO_MODE_INPUT);
        gpio_set_pull_mode(PIN_K3_1, GPIO_FLOATING);
        gpio_set_level(PIN_K3_ON, 0);
        gpio_set_direction(PIN_K3_ON, GPIO_MODE_OUTPUT);

        // MCPWM: je ein Timer+Operator fuer Heizung, Luefter (2 Ausgaenge) und Servo
        ESP_ERROR_CHECK(InitPwm());

        // I2C Master Bus
        i2c_master_bus_config_t i2c_mst_config = {};
        i2c_mst_config.i2c_port = I2C_PORT;
        i2c_mst_config.sda_io_num = PIN_I2C_SDA;
        i2c_mst_config.scl_io_num = PIN_I2C_SCL;
        i2c_mst_config.clk_source = I2C_CLK_SRC_DEFAULT;
        i2c_mst_config.glitch_ignore_cnt = 7;
        i2c_mst_config.flags.enable_internal_pullup = 1;
        ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &i2c_master_handle));
        i2c_discover::Discover(i2c_master_handle);

        bme280dev = new BME280::M(i2c_master_handle, BME280::ADDRESS::PRIM);
        bh1750dev = new BH1750::M(i2c_master_handle, BH1750::ADDRESS::LOW, BH1750::OPERATIONMODE::CONTINU_H_RESOLUTION);
        ccs811dev = new CCS811::M(i2c_master_handle, CCS811::ADDRESS::ADDR0, CCS811::MODE::_1SEC, (gpio_num_t)GPIO_NUM_NC);

        oneWireBus = new OneWire::OneWireBus<PIN_ONEWIRE>();
        if (oneWireBus->Init() != ErrorCode::OK)
        {
            delete oneWireBus;
            oneWireBus = nullptr;
        }

        // Audio ueber den internen DAC (GPIO25)
        dacCodec = new CodecManager::InternalDacWithPotentiometer();
        ERRORCODE_CHECK(dacCodec->Init());
        mp3player = new AudioPlayer::Player(dacCodec);

        // LED Strip
        strip = new led::RgbStrip<LED_NUMBER, led::DeviceType::WS2812>();
        ERRORCODE_CHECK(strip->Begin(SPI3_HOST, PIN_LED_WS2812));
        ERRORCODE_CHECK(strip->Clear(100));

        readBinaryAndAnalogIOs(); // do this while init to avoid race condition (wifimanager is resettet when red and green buttons are pressed during startup)

        xTaskCreate([](void *p)
                    { ((HAL_Impl *)p)->HalLoop(); },
                    "halTask", 4096 * 4, this, 6, nullptr);
        xTaskCreatePinnedToCore([](void *p)
                                { ((HAL_Impl *)p)->AudioLoop(); },
                                "audioTask", 6144 * 4, this, 8, nullptr, 1);
        ESP_LOGI(TAG, "HAL successfully initialized");
        return ErrorCode::OK;
    }

    ErrorCode BeforeLoop() override
    {
        float ht;
        if (GetHeaterTemperature(&ht) == ErrorCode::OK && ht > 85)
        {
            ESP_LOGE(TAG, "Emergency Shutdown. Heater Temperature too high!!!");
            this->SetHeaterDuty(0);
            this->heaterEmergencyShutdown = true;
        }
        return ErrorCode::OK;
    }

    ErrorCode AfterLoop() override
    {
        strip->Refresh(100); // checks internally, whether data is dirty and has to be pushed out
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
        if (ledIndex >= LED_NUMBER)
            return ErrorCode::INDEX_OUT_OF_BOUNDS;
        return strip->SetPixel(LED_NUMBER - ledIndex - 1, colorCRGB);
    }

    ErrorCode UnColorizeAllLed() override
    {
        strip->Clear(1000);
        return ErrorCode::OK;
    }

    ErrorCode SetRelayState(bool state) override
    {
        gpio_set_level(PIN_K3_ON, state);
        return ErrorCode::OK;
    }

    ErrorCode SetHeaterDuty(float dutyInPercent) override
    {
        if (this->heaterEmergencyShutdown)
        {
            heater.SetDutyPercent(0);
            return ErrorCode::EMERGENCY_SHUTDOWN;
        }
        return heater.SetDutyPercent(dutyInPercent) == ESP_OK ? ErrorCode::OK : ErrorCode::GENERIC_ERROR;
    }

    float GetHeaterState() override
    {
        return heater.GetDutyPercent();
    }

    // Index 0 = Servo 1 (nur bei MULTI1 = SERVO1 bestueckt, Standard: nicht vorhanden), Index 1 = Servo 2 (GPIO33)
    ErrorCode SetServoPosition(uint8_t servoIndex, float angle_0_to_180) override
    {
        if (servoIndex != 1)
            return ErrorCode::NONE_AVAILABLE;
        angle_0_to_180 = std::min(SERVO_MAX_DEGREE, std::max(0.0f, angle_0_to_180));
        uint32_t pulsewidthUs = (uint32_t)(SERVO_MIN_PULSEWIDTH + (((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * angle_0_to_180) / SERVO_MAX_DEGREE));
        return servo2.SetPulseWidthUs(pulsewidthUs) == ESP_OK ? ErrorCode::OK : ErrorCode::GENERIC_ERROR;
    }

    ErrorCode SetFanDuty(uint8_t fanIndex, float dutyInPercent) override
    {
        if (fanIndex == 0)
            return fan1.SetDutyPercent(dutyInPercent) == ESP_OK ? ErrorCode::OK : ErrorCode::GENERIC_ERROR;
        if (fanIndex == 1)
            return fan2.SetDutyPercent(dutyInPercent) == ESP_OK ? ErrorCode::OK : ErrorCode::GENERIC_ERROR;
        return ErrorCode::NONE_AVAILABLE;
    }

    ErrorCode GetFanDuty(uint8_t fanIndex, float *dutyInPercent) override
    {
        if (fanIndex == 0)
            *dutyInPercent = fan1.GetDutyPercent();
        else if (fanIndex == 1)
            *dutyInPercent = fan2.GetDutyPercent();
        else
            return ErrorCode::NONE_AVAILABLE;
        return ErrorCode::OK;
    }

    ErrorCode SetLedPowerWhiteDuty(float dutyInpercent) override
    {
        return ErrorCode::OK; // wie bisher nicht umgesetzt
    }

    bool GetButtonRedIsPressed() override
    {
        return GetBitIdx(this->buttonState, (uint8_t)Button::BUT_RED);
    }

    bool GetButtonEncoderIsPressed() override
    {
        return GetBitIdx(this->buttonState, (uint8_t)Button::BUT_ENCODER);
    }

    bool GetButtonGreenIsPressed() override
    {
        return GetBitIdx(this->buttonState, (uint8_t)Button::BUT_GREEN);
    }

    bool IsMovementDetected() override
    {
        return this->movementIsDetected;
    }

    float GetUSBCVoltage() override
    {
        return 20.0; // Rev. 5.x hat keinen USB-PD-Controller
    }

    ErrorCode GreetUserOnStartup() override
    {
        for (int i = 0; i < 3; i++)
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
        SetSound(5); // ready.mp3
        UnColorizeAllLed();
        return ErrorCode::OK;
    }

    ErrorCode GetAmbientBrightnessAnalog(float *lux) override
    {
        *lux = std::numeric_limits<float>::quiet_NaN();
        return ErrorCode::NONE_AVAILABLE;
    }

    ErrorCode GetAmbientBrightnessDigital(float *lux) override
    {
        if (!bh1750dev->HasValidData())
        {
            *lux = std::numeric_limits<float>::quiet_NaN();
            return ErrorCode::GENERIC_ERROR;
        }
        uint16_t temp;
        bh1750dev->Read(temp);
        *lux = temp;
        return ErrorCode::OK;
    }

    ErrorCode GetAirTemperatureDS18B20(float *degreesCelcius) override
    {
        if (!this->oneWireBus)
        {
            *degreesCelcius = std::numeric_limits<float>::quiet_NaN();
            return ErrorCode::GENERIC_ERROR;
        }
        *degreesCelcius = this->oneWireBus->GetMinTemp();
        return ErrorCode::OK;
    }

    ErrorCode GetAirTemperatureAHT21(float *degreesCelcius) override
    {
        *degreesCelcius = std::numeric_limits<float>::quiet_NaN();
        return ErrorCode::NONE_AVAILABLE;
    }

    ErrorCode GetAirTemperatureBME280(float *degreesCelcius) override
    {
        float p, h;
        return ReadBme280(degreesCelcius, &p, &h);
    }

    ErrorCode GetAirRelHumidityAHT21(float *percent) override
    {
        *percent = std::numeric_limits<float>::quiet_NaN();
        return ErrorCode::NONE_AVAILABLE;
    }

    ErrorCode GetAirRelHumidityBME280(float *percent) override
    {
        float t, p;
        return ReadBme280(&t, &p, percent);
    }

    ErrorCode GetDistanceMillimeters(uint16_t *value) override
    {
        *value = 0;
        return ErrorCode::NONE_AVAILABLE; // Rev. 5.x hat keinen Abstandssensor
    }

private:
    ErrorCode ReadBme280(float *tempDegCel, float *pressurePa, float *relHumidityPercent)
    {
        if (!bme280dev->HasValidData())
        {
            *tempDegCel = *pressurePa = *relHumidityPercent = std::numeric_limits<float>::quiet_NaN();
            return ErrorCode::GENERIC_ERROR;
        }
        return bme280dev->GetData(tempDegCel, pressurePa, relHumidityPercent);
    }

    esp_err_t InitPwm()
    {
        // Heizung
        mcpwm_timer_handle_t timerHeater;
        mcpwm_oper_handle_t operHeater;
        ESP_RETURN_ON_ERROR(NewTimerAndOperator(PWM_HEATER_RESOLUTION_HZ, PWM_HEATER_PERIOD_TICKS, &timerHeater, &operHeater), TAG, "heater timer");
        ESP_RETURN_ON_ERROR(heater.Init(operHeater, PIN_HEATER, PWM_HEATER_RESOLUTION_HZ, PWM_HEATER_PERIOD_TICKS), TAG, "heater");
        // Luefter-Index 0 (Heater-Experiment, Modbus) liegt am Stecker "FAN2" (GPIO32) -- dort steckt der Luefter
        // der Platine; Index 1 ist der Stecker "FAN1" (GPIO12). Beide teilen sich Timer und Operator.
        mcpwm_timer_handle_t timerFan;
        mcpwm_oper_handle_t operFan;
        ESP_RETURN_ON_ERROR(NewTimerAndOperator(PWM_FAN_RESOLUTION_HZ, PWM_FAN_PERIOD_TICKS, &timerFan, &operFan), TAG, "fan timer");
        ESP_RETURN_ON_ERROR(fan1.Init(operFan, PIN_FAN2_DRIVE, PWM_FAN_RESOLUTION_HZ, PWM_FAN_PERIOD_TICKS), TAG, "fan1");
        ESP_RETURN_ON_ERROR(fan2.Init(operFan, PIN_FAN1_DRIVE, PWM_FAN_RESOLUTION_HZ, PWM_FAN_PERIOD_TICKS), TAG, "fan2");
        // Servo 2
        mcpwm_timer_handle_t timerServo;
        mcpwm_oper_handle_t operServo;
        ESP_RETURN_ON_ERROR(NewTimerAndOperator(PWM_SERVO_RESOLUTION_HZ, PWM_SERVO_PERIOD_TICKS, &timerServo, &operServo), TAG, "servo timer");
        ESP_RETURN_ON_ERROR(servo2.Init(operServo, PIN_SERVO2, PWM_SERVO_RESOLUTION_HZ, PWM_SERVO_PERIOD_TICKS), TAG, "servo2");
        return ESP_OK;
    }

    static esp_err_t NewTimerAndOperator(uint32_t resolutionHz, uint32_t periodTicks, mcpwm_timer_handle_t *timer, mcpwm_oper_handle_t *oper)
    {
        mcpwm_timer_config_t timerCfg = {};
        timerCfg.group_id = 0;
        timerCfg.clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT;
        timerCfg.resolution_hz = resolutionHz;
        timerCfg.count_mode = MCPWM_TIMER_COUNT_MODE_UP;
        timerCfg.period_ticks = periodTicks;
        ESP_RETURN_ON_ERROR(mcpwm_new_timer(&timerCfg, timer), TAG, "new timer");
        mcpwm_operator_config_t operCfg = {};
        operCfg.group_id = 0;
        ESP_RETURN_ON_ERROR(mcpwm_new_operator(&operCfg, oper), TAG, "new operator");
        ESP_RETURN_ON_ERROR(mcpwm_operator_connect_timer(*oper, *timer), TAG, "connect timer");
        ESP_RETURN_ON_ERROR(mcpwm_timer_enable(*timer), TAG, "enable timer");
        return mcpwm_timer_start_stop(*timer, MCPWM_TIMER_START_NO_STOP);
    }
};
#undef TAG
