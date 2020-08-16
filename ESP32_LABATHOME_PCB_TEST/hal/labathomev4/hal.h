#include <inttypes.h>
#include <FastLED.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SparkFunBME280.h>
#include <BH1750.h>

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

const uint8_t PWM_CHANNEL_BUZZER = 10;

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
    SW_GREEN=0,
    SW_ROTARY=1,
    SW_RED=2,
};

class HAL
{
    private:
        const uint16_t sw_limits[7] = {160, 480, 1175, 1762, 2346, 2779, 3202};
    
    public:
        


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
            pinMode(PIN_FAN1_SENSE, INPUT_PULLUP);
            pinMode(PIN_SPEAKER, OUTPUT);
            FastLED.addLeds<LED_TYPE,PIN_LED_STRIP,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
            FastLED.setBrightness( BRIGHTNESS );
            pinMode(PIN_SERVO1, OUTPUT);
            //oneWire.begin(PIN_ONEWIRE);
            pinMode(PIN_FAN1_DRIVE, OUTPUT);
            pinMode(PIN_LED_POWER_WHITE, OUTPUT);
            Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 100000);
            bme280.setI2CAddress(0x76); 
            if(bme280.beginI2C() == false) Serial.println("BME280 connect failed");
            if(!bh1750.begin()) Serial.println("BH1750 connect failed");
            
            pinMode(PIN_HEATER, OUTPUT);
            pinMode(PIN_R3_ON, OUTPUT);
            pinMode(PIN_ROTENC_B, INPUT);
        }
        void setLEDState(LED led, CRGB state)
        {
            leds[(uint8_t)led]=state;
            FastLED.show();
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
};

