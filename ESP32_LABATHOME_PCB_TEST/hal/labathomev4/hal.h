#include <inttypes.h>
#include <FastLED.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SparkFunBME280.h>
#include <BH1750.h>
#include <driver/mcpwm.h>
#include <driver/ledc.h>

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
const Pintype PIN_SERVO1 = 27;
const Pintype PIN_ONEWIRE = 14;
const Pintype PIN_FAN1_DRIVE = 12;
const Pintype PIN_LED_POWER_WHITE = 13;

const Pintype PIN_LCD_MOSI = 23;
const Pintype PIN_I2C_SDA = 22;
const Pintype PIN_I2C_SCL = 21;
const Pintype MULTI1 = 19;
const Pintype PIN_LCD_CLK = 18;
const Pintype PIN_LCD_DC = 5;
const Pintype PIN_HEATER = 17;
const Pintype PIN_MULTI3 = 16;
const Pintype PIN_MULTI2 = 4;
const Pintype PIN_LCD_RES = 0;
const uint8_t PIN_R3_ON = 2;
const Pintype PIN_ROTENC_B = 15;


enum class MODE_IO33
{
    SERVO2,
    FAN1_SENSE,
};

enum class MODE_MULTI_PINS
{
    I2S,
    RS485,
    CAN,
    UNDEFINED,
};

enum class MODE_SPEAKER
{
    TRANSISTOR,
    AMPLIFIER,
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
    SW_GREEN=1,
    SW_ROTARY=0,
    SW_YELLOW=0, //for compatibility reasons
    SW_RED=2,
};

enum class Servo:uint8_t
{
    Servo1=0,
    Servo2=1,
};

class HAL
{
    private:
        const uint16_t sw_limits[7] = {160, 480, 1175, 1762, 2346, 2779, 3202};
        const int SERVO_MIN_PULSEWIDTH=500; //Minimum pulse width in microsecond
        const int  SERVO_MAX_PULSEWIDTH=2400; //Maximum pulse width in microsecond
        const int  SERVO_MAX_DEGREE=180; //Maximum angle in degree upto which servo can rotate
        MODE_IO33 mode_io33;
        MODE_MULTI_PINS mode_multipins;
        MODE_SPEAKER mode_speaker; 
    
    public:
        HAL(MODE_IO33 mode_io33, MODE_MULTI_PINS mode_multipins, MODE_SPEAKER mode_speaker):mode_io33(mode_io33), mode_multipins(mode_multipins), mode_speaker(mode_speaker)
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
            pinMode(PIN_R3_1, INPUT);
            pinMode(PIN_MOVEMENT, INPUT);
            pinMode(PIN_SW, ANALOG);
            analogSetPinAttenuation(PIN_SW, ADC_0db);
            pinMode(PIN_ROTENC_A, INPUT);
            pinMode(PIN_FAN2_DRIVE, OUTPUT);
            if(mode_io33==MODE_IO33::FAN1_SENSE)
            {
                pinMode(PIN_FAN1_SENSE, INPUT_PULLUP);
            }
            else if(mode_io33==MODE_IO33::SERVO2)
            {
                pinMode(PIN_FAN2_DRIVE, OUTPUT);
            }
            
            pinMode(PIN_SPEAKER, OUTPUT);
            FastLED.addLeds<LED_TYPE,PIN_LED_STRIP,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
            FastLED.setBrightness( BRIGHTNESS );
            pinMode(PIN_SERVO1, OUTPUT);
            //oneWire.begin(PIN_ONEWIRE);
            pinMode(PIN_FAN1_DRIVE, OUTPUT);
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
            
            pinMode(PIN_HEATER, OUTPUT);
            pinMode(PIN_R3_ON, OUTPUT);
            pinMode(PIN_ROTENC_B, INPUT);

            //Servos
            mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, PIN_SERVO1);
            if(mode_io33==MODE_IO33::SERVO2)
            {
                mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, PIN_SERVO2);
            }
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

            switch (mode_speaker)
            {
            case MODE_SPEAKER::TRANSISTOR:
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
                buzzer_channel.gpio_num   = PIN_SPEAKER;
                buzzer_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
                buzzer_channel.hpoint     = 0;
                buzzer_channel.timer_sel  = LEDC_TIMER_2;
                ledc_channel_config(&buzzer_channel);
                break;
            default:
                break;
            }

        }

        void startBuzzer(double freq)
        {
            if(mode_speaker!=MODE_SPEAKER::TRANSISTOR) return;
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
           digitalWrite(PIN_R3_ON, state);
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
            uint16_t analogIn = analogRead(34);  
            int i=0;
            for(i=0; i<sizeof(sw_limits)/sizeof(uint16_t);i++)
            {
                if(analogIn<sw_limits[i]) break;
            }
            i=~i;
            return bitRead(i, (uint8_t)theSwitch);
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

