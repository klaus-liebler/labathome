#include <inttypes.h>
#include <Wire.h>
#include <owb.h>
#include <include/owb_rmt.h>
#include <include/ds18b20.h>
#include <SparkFunBME280.h>
#include <BH1750.h>
#include <driver/mcpwm.h>
#include <driver/ledc.h>
#include "WS2812.hh"
#include <driver/rmt.h>

typedef uint8_t Pintype;

const Pintype PIN_R3_1 = 36;
const Pintype PIN_MOVEMENT = 39;
const Pintype PIN_SW = 34;
const Pintype PIN_ROTENC_A = 35;
const Pintype PIN_FAN2_DRIVE = 32;
const Pintype PIN_FAN1_SENSE = 33;
const Pintype PIN_SERVO2 = 33;
const Pintype PIN_SPEAKER = 25;
const Pintype PIN_LED_STRIP = 26;

const Pintype PIN_MULTI1 = 27;
const Pintype PIN_ONEWIRE = 14;
const Pintype PIN_FAN1_DRIVE = 12;
const Pintype PIN_LED_POWER_WHITE = 13;

const Pintype PIN_SPI_MOSI = 23;
const Pintype PIN_I2C_SDA = 22;
const Pintype PIN_I2C_SCL = 21;
const Pintype PIN_SPI_MISO = 19;
const Pintype PIN_SPI_CLK = 18;
const Pintype PIN_SPI_IO1 = 5;
const Pintype PIN_HEATER = 17;
const Pintype PIN_MULTI3 = 16;
const Pintype PIN_MULTI2 = 4;
const Pintype PIN_SPI_IO2 = 0;
const Pintype PIN_R3_ON = 2;
const Pintype PIN_ROTENC_B = 15;

const Pintype PIN_SERVO1 = PIN_MULTI1;
const Pintype PIN_I2S_SCK = PIN_MULTI1;
const Pintype PIN_485_DE = PIN_MULTI1;
const Pintype PIN_EXT1 = PIN_MULTI1;

const Pintype PN_485_RO = PIN_MULTI2;
const Pintype PN_CAN_TX = PIN_MULTI2;
const Pintype PN_EXT2 = PIN_MULTI2;
const Pintype PN_I2S_WS = PIN_MULTI2;

const Pintype PN_485_DI = PIN_MULTI3;
const Pintype PN_CAN_RX = PIN_MULTI3;
const Pintype PN_I2S_SD = PIN_MULTI3;
const Pintype PN_EXT3 = PIN_MULTI3;

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

enum class LED : uint8_t
{
    LED_RED,
    LED_YELLOW,
    LED_GREEN,
    LED_3,
    LED_4,
    LED_5,
    LED_6,
    LED_7,
};

enum class Switch : uint8_t
{
    SW_GREEN = 1,
    SW_ROTARY = 0,
    SW_RED = 2,
};

enum class Servo : uint8_t
{
    Servo1 = 0,
    Servo2 = 1,
};

constexpr size_t LED_NUMBER = 8;
constexpr rmt_channel_t RMT_TX_CHANNEL = RMT_CHANNEL_0;
constexpr int MAX_DEVICES=          (8);
constexpr auto DS18B20_RESOLUTION  = (DS18B20_RESOLUTION_12_BIT);


class HAL
{
private:
    const uint16_t sw_limits[7] = {160, 480, 1175, 1762, 2346, 2779, 3202};
    const int SERVO_MIN_PULSEWIDTH = 500;  //Minimum pulse width in microsecond
    const int SERVO_MAX_PULSEWIDTH = 2400; //Maximum pulse width in microsecond
    const int SERVO_MAX_DEGREE = 180;      //Maximum angle in degree upto which servo can rotate
    MODE_IO33 mode_io33;
    MODE_MULTI1_PIN mode_multi1;
    MODE_MULTI_2_3_PINS mode_multi23;
    bool needLedStripUpdate = false;
    uint8_t buttonState = 0;
    byte addr[8];
    uint32_t nextOneWireReadout = UINT32_MAX;
    float lastTemperature=0.0;

    float raw2celsius(uint8_t *data)
    {
        // Convert the data to actual temperature
        // because the result is a 16 bit signed integer, it should
        // be stored to an "int16_t" type, which is always 16 bits
        // even when compiled on a 32 bit processor.
        int16_t raw = (data[1] << 8) | data[0];
       
        byte cfg = (data[4] & 0x60);
        // at lower res, the low bits are undefined, so let's zero them
        if (cfg == 0x00)
            raw = raw & ~7; // 9 bit resolution, 93.75 ms
        else if (cfg == 0x20)
            raw = raw & ~3; // 10 bit res, 187.5 ms
        else if (cfg == 0x40)
            raw = raw & ~1; // 11 bit res, 375 ms
                            //// default is 12 bit resolution, 750 ms conversion time
        return (float)raw / 16.0;
    }

public:
    WS2812_Strip<LED_NUMBER> *strip;
    OneWireBus *owb;

    BME280 bme280;
    BH1750 bh1750;

    HAL(MODE_IO33 mode_io33, MODE_MULTI1_PIN mode_multi1, MODE_MULTI_2_3_PINS mode_multi23) : mode_io33(mode_io33), mode_multi1(mode_multi1), mode_multi23(mode_multi23)
    {
        strip = new WS2812_Strip<LED_NUMBER>(RMT_TX_CHANNEL);
        oneWire = OneWire(PIN_ONEWIRE);
    }

    int64_t IRAM_ATTR getMillis()
    {
        return (esp_timer_get_time() / 1000ULL);
    }

    float getCurrentTemperature()
    {
        return this->lastTemperature;
    }

    void initBoard()
    {
        pinMode(PIN_R3_1, INPUT);
        pinMode(PIN_MOVEMENT, INPUT);
        pinMode(PIN_SW, ANALOG);
        analogSetPinAttenuation(PIN_SW, ADC_0db);
        pinMode(PIN_ROTENC_A, INPUT);
        pinMode(PIN_FAN2_DRIVE, OUTPUT);
        if (mode_io33 == MODE_IO33::FAN1_SENSE)
        {
            pinMode(PIN_FAN1_SENSE, INPUT_PULLUP);
        }
        else if (mode_io33 == MODE_IO33::SERVO2)
        {
            pinMode(PIN_FAN2_DRIVE, OUTPUT);
        }

        pinMode(PIN_SPEAKER, OUTPUT);

        strip->Init(GPIO_NUM_26);

        // Clear LED strip (turn off all LEDs)
        ESP_ERROR_CHECK(strip->Clear(100));

        //OneWire
        owb_rmt_driver_info rmt_driver_info;
        owb = owb_rmt_initialize(&rmt_driver_info, GPIO_DS18B20_0, RMT_CHANNEL_1, RMT_CHANNEL_0);
        owb_use_crc(owb, true);  // enable CRC check for ROM code
        // Find all connected devices
        printf("Find devices:\n");
        OneWireBus_ROMCode device_rom_codes[MAX_DEVICES] = {0};
        int num_devices = 0;
        OneWireBus_SearchState search_state = {0};
        bool found = false;
        owb_search_first(owb, &search_state, &found);
        while (found)
        {
            char rom_code_s[17];
            owb_string_from_rom_code(search_state.rom_code, rom_code_s, sizeof(rom_code_s));
            printf("  %d : %s\n", num_devices, rom_code_s);
            device_rom_codes[num_devices] = search_state.rom_code;
            ++num_devices;
            owb_search_next(owb, &search_state, &found);
        }
    printf("Found %d device%s\n", num_devices, num_devices == 1 ? "" : "s");

        /*
        oneWire.search(addr);
        delay(250);
        oneWire.reset_search();
        delay(250);
        if ( !oneWire.search(addr)) {
            Serial.println("No more addresses.");
        }
        else
        {
            Serial.print("ROM =");
            for (size_t i = 0; i < 8; i++)
            {
                Serial.write(' ');
                Serial.print(addr[i], HEX);
            }
            oneWire.reset();
            oneWire.select(addr);
            oneWire.write(0x44, 1); // start conversion, with parasite power on at the end
            this->nextOneWireReadout = millis() + 1000UL;
        }
        */
        pinMode(PIN_SERVO1, OUTPUT);

        pinMode(PIN_LED_POWER_WHITE, OUTPUT);
        Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 100000);
        bme280.setI2CAddress(0x76);
        if (bme280.beginI2C())
        {
            Serial.println("BME280 successfully initialized");
        }
        else
        {
            Serial.println("BME280 initialization failed");
        }
        if (bh1750.begin())
        {
            Serial.println("BH1750 successfully initialized");
        }
        else
        {
            Serial.println("BH1750 initialization failed");
        }



        pinMode(PIN_R3_ON, OUTPUT);
        pinMode(PIN_ROTENC_B, INPUT);


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
        mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config); //Configure PWM0A & PWM0B with above settings

        //Fans
        mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, PIN_FAN1_DRIVE);
        mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1B, PIN_FAN2_DRIVE);
        pwm_config.frequency = 1000;
        pwm_config.cmpr_a = 0;     //duty cycle of PWMxA = 0
        pwm_config.cmpr_b = 0;     //duty cycle of PWMxb = 0
        pwm_config.counter_mode = MCPWM_UP_COUNTER;
        pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
        mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &pwm_config);
        //Heater
        mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM2A, PIN_HEATER);
        pwm_config.frequency = 20;
        pwm_config.cmpr_a = 0;     //duty cycle of PWMxA = 0
        pwm_config.cmpr_b = 0;     //duty cycle of PWMxb = 0
        pwm_config.counter_mode = MCPWM_UP_COUNTER;
        pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
        mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_2, &pwm_config);

        //LED
        ledc_timer_config_t power_ledc_timer;
        power_ledc_timer.duty_resolution = LEDC_TIMER_13_BIT; // resolution of PWM duty
        power_ledc_timer.freq_hz = 5000;                      // frequency of PWM signal
        power_ledc_timer.speed_mode = LEDC_HIGH_SPEED_MODE;   // timer mode
        power_ledc_timer.timer_num = LEDC_TIMER_0;            // timer index
        ledc_timer_config(&power_ledc_timer);

        ledc_channel_config_t ledc_channel;
        ledc_channel.channel = LEDC_CHANNEL_0;
        ledc_channel.duty = 0;
        ledc_channel.gpio_num = PIN_LED_POWER_WHITE;
        ledc_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
        ledc_channel.hpoint = 0;
        ledc_channel.timer_sel = LEDC_TIMER_0;
        ledc_channel_config(&ledc_channel);

        ledc_timer_config_t buzzer_timer;
        buzzer_timer.duty_resolution = LEDC_TIMER_10_BIT; // resolution of PWM duty
        buzzer_timer.freq_hz = 440;                       // frequency of PWM signal
        buzzer_timer.speed_mode = LEDC_HIGH_SPEED_MODE;   // timer mode
        buzzer_timer.timer_num = LEDC_TIMER_2;            // timer index
        ledc_timer_config(&buzzer_timer);

        ledc_channel_config_t buzzer_channel;
        buzzer_channel.channel = LEDC_CHANNEL_2;
        buzzer_channel.duty = 0;
        buzzer_channel.gpio_num = PIN_SPEAKER;
        buzzer_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
        buzzer_channel.hpoint = 0;
        buzzer_channel.timer_sel = LEDC_TIMER_2;
        ledc_channel_config(&buzzer_channel);
    }

    void readInputs()
    {
        uint16_t analogIn = analogRead(PIN_SW);
        int i = 0;
        for (i = 0; i < sizeof(sw_limits) / sizeof(uint16_t); i++)
        {
            if (analogIn < sw_limits[i])
                break;
        }
        i = ~i;
        this->buttonState = i;
        int32_t ms = millis();
        if (ms > nextOneWireReadout)
        {
            byte data[12];
            oneWire.reset();
            oneWire.select(addr);
            oneWire.write(0xBE); // Read Scratchpad
            for (i = 0; i < 9; i++)
            { // we need 9 bytes
                data[i] = oneWire.read();
            }
            this->lastTemperature= raw2celsius(data);
            
            oneWire.reset();
            oneWire.select(addr);
            oneWire.write(0x44, 1); // start conversion, with parasite power on at the end
            nextOneWireReadout += 1000ULL;
        }
    }

    void writeOutputs()
    {
        if (needLedStripUpdate)
        {
            strip->Refresh(100);
            needLedStripUpdate = false;
        }
    }

    void startBuzzer(double freq)
    {
        ledc_timer_config_t buzzer_timer;
        buzzer_timer.duty_resolution = LEDC_TIMER_10_BIT; // resolution of PWM duty
        buzzer_timer.freq_hz = freq;                      // frequency of PWM signal
        buzzer_timer.speed_mode = LEDC_HIGH_SPEED_MODE;   // timer mode
        buzzer_timer.timer_num = LEDC_TIMER_2;            // timer index
        ledc_timer_config(&buzzer_timer);
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2, 512);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2);
    }

    void endBuzzer()
    {
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2, 0);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2);
    }

    void setLEDState(LED led, CRGB state)
    {
        strip->SetPixel((uint8_t)led, state);
        this->needLedStripUpdate = true;
    }

    void setRELAYState(bool state)
    {
        digitalWrite(PIN_R3_ON, state);
    }

    void setHeaterState(float dutyInPercent)
    {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_OPR_A, dutyInPercent);
    }

    void setFan1State(float dutyInPercent)
    {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, dutyInPercent);
    }

    void setFan2State(float dutyInPercent)
    {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B, dutyInPercent);
    }

    void setLED_POWER_WHITE_DUTY(uint8_t percent)
    {
        if (percent > 100)
            percent = 100;
        uint32_t duty = ((1 << 13) * percent) / 100;
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
    }

    bool getButtonIsPressed(Switch theSwitch)
    {
        return bitRead(this->buttonState, (uint8_t)theSwitch);
    }

    bool isMovementDetected()
    {
        return digitalRead(PIN_MOVEMENT);
    }

    void setServo(Servo servo, uint32_t angle_0_to_180)
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
    }
};
