#include <inttypes.h>
#include <FastLED.h>
#include <Wire.h>
#include <SparkFunBME280.h>
#include <BH1750.h>

typedef uint8_t Pintype;

const Pintype PIN_R3_1 = 36;
const Pintype PIN_MOVEMENT = 39;
const Pintype PIN_POTI_T = 34;
const Pintype PIN_POTI = 35;
const Pintype PIN_SW_RED = 32;
const Pintype PIN_FAN_SENSE = 33;
const Pintype PIN_SERVO1 = 25;
const Pintype PIN_LED_RED = 26;
const Pintype PIN_LED_YELLOW = 27;
const Pintype PIN_LED_POWER_WHITE = 14;
const Pintype PIN_FAN_DRIVE = 12;
const Pintype PIN_LED_GREEN = 13;
const Pintype PIN_I2C_SCL = 22;
const Pintype PIN_I2C_SDA = 21;
const Pintype PIN_BUZZER = 5;
const Pintype PIN_R3_ON = 17;
const Pintype PIN_HEATER = 16;
const Pintype PIN_SERVO2 = 4;
const Pintype PIN_SW_GREEN = 0;
const Pintype PIN_SW_BLACK = 15;

const uint8_t PWM_CHANNEL_BUZZER = 10;

enum class LED:uint8_t{
    LED_RED=PIN_LED_RED,
    LED_YELLOW=PIN_LED_YELLOW,
    LED_GREEN=PIN_LED_GREEN,
    

};

enum class Switch:uint8_t{
    SW_RED=PIN_SW_RED,
    SW_YELLOW=PIN_SW_BLACK,
    SW_GREEN=PIN_SW_GREEN,
};

class HAL
{
    public:


        // Pass our oneWire reference to Dallas Temperature. 
        //DallasTemperature sensors(&oneWire);

        BME280 bme280;
        BH1750 bh1750;
        void initBoard()
        {
            pinMode(PIN_R3_1, INPUT);
            pinMode(PIN_MOVEMENT, INPUT);
            pinMode(PIN_POTI_T, ANALOG);
            pinMode(PIN_POTI, ANALOG);
            pinMode(PIN_SW_RED, INPUT_PULLUP);
            pinMode(PIN_FAN_SENSE, INPUT_PULLUP);
            pinMode(PIN_SERVO1, OUTPUT);
            pinMode(PIN_LED_RED, OUTPUT);
            pinMode(PIN_LED_YELLOW, OUTPUT);
            pinMode(PIN_LED_POWER_WHITE, OUTPUT);
            pinMode(PIN_FAN_DRIVE, OUTPUT);
            pinMode(PIN_LED_GREEN, OUTPUT);
            Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 100000);
            bme280.setI2CAddress(0x76); 
            if(bme280.beginI2C() == false) Serial.println("BME280 connect failed");
            if(!bh1750.begin()) Serial.println("BH1750 connect failed");
            pinMode(PIN_BUZZER, OUTPUT);
            ledcSetup(PWM_CHANNEL_BUZZER, 2000, 8);
            ledcAttachPin(PIN_BUZZER, 10);
            ledcWriteTone(PWM_CHANNEL_BUZZER, 440);
            delay(200);
            ledcWriteTone(PWM_CHANNEL_BUZZER, 880);
            delay(200);
            ledcWrite(PWM_CHANNEL_BUZZER, 0);
            pinMode(PIN_R3_ON, OUTPUT);
            pinMode(PIN_HEATER, OUTPUT);
            pinMode(PIN_SERVO2, OUTPUT);
            pinMode(PIN_SW_GREEN, INPUT_PULLUP);
            pinMode(PIN_SW_BLACK, INPUT_PULLUP);

        }
        void setLEDState(LED led, CRGB state)
        {
            digitalWrite((uint8_t)led, (state==CRGB::Black)?HIGH:LOW);
        }
        bool getButtonIsPressed(Switch theSwitch)
        {
            return !digitalRead((uint8_t)theSwitch);
        }

        bool isMovementDetected()
        {
            return digitalRead(PIN_MOVEMENT);
        }
};

