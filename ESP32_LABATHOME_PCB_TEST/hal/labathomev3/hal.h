#pragma once
#include <inttypes.h>
#include <FastLED.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SparkFunBME280.h>
#include <BH1750.h>
#include <driver/mcpwm.h>
#include <driver/ledc.h>
#include <esp32-hal-ledc.h>


typedef uint8_t Pintype;
const Pintype PIN_R3_1 = 36;
const Pintype PIN_MOVEMENT = 39;
const Pintype PIN_POTI_T = 34;
const Pintype PIN_POTI = 35;
const Pintype PIN_SW_RED = 32;
const Pintype PIN_FAN_SENSE = 33;
const Pintype PIN_SERVO1 = 25;
const Pintype PIN_LED_STRIP = 26;
const Pintype PIN_SERVO2 = 27;
const Pintype PIN_ONEWIRE = 27;
const Pintype PIN_MULTI_I2S_SCK = 14;
const Pintype PIN_FAN_DRIVE = 12;
const Pintype PIN_LED_POWER_WHITE = 13;
const Pintype PIN_LCD_MOSI = 23;
const Pintype PIN_I2C_SCL = 22;
const Pintype PIN_I2C_SDA = 21;
const Pintype PIN_LCD_RES = 19;
const Pintype PIN_LCD_CLK = 18;
const Pintype PIN_LCD_DC = 5;
const Pintype PIN_BUZZER = 17;
const Pintype PIN_R3_ON = 17;
const Pintype PIN_MULTI_I2S_SD = 16;
const Pintype PIN_MULTI_I2S_WS = 4;
const Pintype PIN_SW_GREEN = 0;
const Pintype PIN_HEATER = 2;
const Pintype PIN_SW_YELLOW = 15;


#define SERVO_MIN_PULSEWIDTH 500 //Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH 2400 //Maximum pulse width in microsecond
#define SERVO_MAX_DEGREE 180 //Maximum angle in degree upto which servo can rotate

enum class IO17_MODE
{
    RELAY,
    BUZZER,
};

enum class IO4_MODE{
    I2S_WS,
    RS485_RO,
    SPECIAL_SPECIAL_RELAY3,
};

enum class LED:uint8_t{
    LED_RED,
    LED_YELLOW,
    LED_GREEN,
    LED_3,
    LED_4,
    LED_5,
    LED_6,
    LED_7,
    

};

enum class Switch:uint8_t{
    SW_RED=PIN_SW_RED,
    SW_YELLOW=PIN_SW_YELLOW,
    SW_GREEN=PIN_SW_GREEN,
};

enum class Servo:uint8_t
{
    Servo1=0,
    Servo2=1,
};

class HAL
{    
    private:
        IO17_MODE io17;
        IO4_MODE io4;
    public:
        HAL(IO17_MODE io17, IO4_MODE io4):io17(io17), io4(io4)
        {

        }


        #define NUM_LEDS 8

        #define LED_TYPE WS2812B
        #define COLOR_ORDER GRB
        const uint8_t BRIGHTNESS = 50;
        const uint8_t FRAMES_PER_SECOND = 60;
        CRGBArray<NUM_LEDS> leds;
        // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
        OneWire oneWire();

        // Pass our oneWire reference to Dallas Temperature. 
        //DallasTemperature sensors(&oneWire);

        BME280 bme280;
        BH1750 bh1750;
        void initBoard()
        {
            
            pinMode(PIN_MOVEMENT, INPUT);
            pinMode(PIN_R3_1, INPUT);
            pinMode(PIN_POTI_T, ANALOG);
            pinMode(PIN_POTI, ANALOG);
            pinMode(PIN_SW_RED, INPUT_PULLUP);
            pinMode(PIN_FAN_SENSE, INPUT_PULLUP);
            FastLED.addLeds<LED_TYPE,PIN_LED_STRIP,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
            FastLED.setBrightness( BRIGHTNESS );
            //oneWire.begin(27);
            pinMode(PIN_FAN_DRIVE, OUTPUT);
            pinMode(PIN_LED_POWER_WHITE, OUTPUT);
            Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 100000);
            bme280.setI2CAddress(0x76); 
            if(bme280.beginI2C()){
                Serial.println("BME280 successfully initialized");
            }
            else
            {
                Serial.println("BME280 initialization failed");
            }
            if(bh1750.begin()){
                Serial.println("BH1750 successfully initialized");
            }
            else
            {
                Serial.println("BH1750 initialization failed");
            }
            
            
            
            
            pinMode(PIN_SW_GREEN, INPUT_PULLUP);
            pinMode(PIN_HEATER, OUTPUT);
            pinMode(PIN_SW_YELLOW, INPUT_PULLUP);

            //Servos
            mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, PIN_SERVO1);
            mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, PIN_SERVO2);
            mcpwm_config_t pwm_config;
            pwm_config.frequency = 50;    //frequency = 50Hz, i.e. for every servo motor time period should be 20ms
            pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
            pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
            pwm_config.counter_mode = MCPWM_UP_COUNTER;
            pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
            mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings   

            //LED
            ledc_timer_config_t power_ledc_timer;
            power_ledc_timer.duty_resolution = LEDC_TIMER_13_BIT; // resolution of PWM duty
            power_ledc_timer.freq_hz = 5000;                      // frequency of PWM signal
            power_ledc_timer.speed_mode = LEDC_HIGH_SPEED_MODE;           // timer mode
            power_ledc_timer.timer_num = LEDC_TIMER_0;            // timer index
            ledc_timer_config(&power_ledc_timer);

            ledc_channel_config_t ledc_channel;
            ledc_channel.channel    = LEDC_CHANNEL_0;
            ledc_channel.duty       = 0;
            ledc_channel.gpio_num   = PIN_LED_POWER_WHITE;
            ledc_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
            ledc_channel.hpoint     = 0;
            ledc_channel.timer_sel  = LEDC_TIMER_0;
            ledc_channel_config(&ledc_channel);
            

           


            switch (io17)
            {
            case IO17_MODE::RELAY:
                pinMode(PIN_R3_ON, OUTPUT);
                break;
            case IO17_MODE::BUZZER:
                 //BUZZER
                ledc_timer_config_t buzzer_timer;
                buzzer_timer.duty_resolution = LEDC_TIMER_10_BIT; // resolution of PWM duty
                buzzer_timer.freq_hz = 440;                      // frequency of PWM signal
                buzzer_timer.speed_mode = LEDC_HIGH_SPEED_MODE;           // timer mode
                buzzer_timer.timer_num = LEDC_TIMER_2;            // timer index
                ledc_timer_config(&buzzer_timer);

                ledc_channel_config_t buzzer_channel;
                buzzer_channel.channel    = LEDC_CHANNEL_2;
                buzzer_channel.duty       = 0;
                buzzer_channel.gpio_num   = PIN_BUZZER;
                buzzer_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
                buzzer_channel.hpoint     = 0;
                buzzer_channel.timer_sel  = LEDC_TIMER_2;
                ledc_channel_config(&buzzer_channel);
                break;
            default:
                break;
            }

            switch (io4)
            {
            case IO4_MODE::SPECIAL_SPECIAL_RELAY3:
                pinMode(PIN_MULTI_I2S_WS, OUTPUT);
                break;
            default:
                Serial.println("IO4_MODE not yet supported");
            }
        }

        void startBuzzer(double freq)
        {
            if(io17!=IO17_MODE::BUZZER) return;
            ledc_timer_config_t buzzer_timer;
            buzzer_timer.duty_resolution = LEDC_TIMER_10_BIT; // resolution of PWM duty
            buzzer_timer.freq_hz = freq;                      // frequency of PWM signal
            buzzer_timer.speed_mode = LEDC_HIGH_SPEED_MODE;           // timer mode
            buzzer_timer.timer_num = LEDC_TIMER_2;            // timer index
            ledc_timer_config(&buzzer_timer);
            ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2, 512);
            ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2);
        }

        void endBuzzer()
        {
            //ledcWrite(2, 0);
            ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2, 0);
            ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2);
        }
        
        void setLEDState(LED led, CRGB state)
        {
            leds[(uint8_t)led]=state;
            FastLED.show();
        }

        void setRELAYState(bool state)
        {
            if(io17==IO17_MODE::RELAY)
            {
                digitalWrite(PIN_R3_ON, state);
            }
            if(io4==IO4_MODE::SPECIAL_SPECIAL_RELAY3)
            {
                digitalWrite(PIN_MULTI_I2S_WS, state);
            }
        }

        void setLED_POWER_WHITE_DUTY(uint8_t percent)
        {
            if(percent>100) percent=100;
            uint32_t duty =((1<<13)*percent)/100;
            ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
            ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
        }
        bool getButtonIsPressed(Switch theSwitch)
        {
            return !digitalRead((uint8_t)theSwitch);
        }

        bool isMovementDetected()
        {
            return digitalRead(PIN_MOVEMENT);
        }

        void setServo (Servo servo, uint32_t angle_0_to_180)
        {
            uint32_t cal_pulsewidth = (SERVO_MIN_PULSEWIDTH + (((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * (angle_0_to_180)) / (SERVO_MAX_DEGREE)));
            switch(servo)
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

