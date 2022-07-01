#pragma once
#include "HAL.hh"

#include <inttypes.h>
#include <owb.h>
#include <owb_rmt.h>
#include <ds18b20.h>

#include <driver/mcpwm.h>
#include <driver/ledc.h>
#include <driver/adc.h>
#include <driver/i2c.h>
#include <driver/rmt.h>

#include "WS2812.hh"
#include "errorcodes.hh"

#define CHECK_BIT(var, pos) ((var) & (1 << (pos)))
// Predefined on Board
constexpr gpio_num_t PIN_LCD_RST = GPIO_NUM_18;
constexpr gpio_num_t PIN_LCD_CLK = GPIO_NUM_19;
constexpr gpio_num_t PIN_LCD_DC = GPIO_NUM_21;
constexpr gpio_num_t PIN_LCD_CS = GPIO_NUM_22;
constexpr gpio_num_t PIN_LCD_MOSI = GPIO_NUM_23;
constexpr gpio_num_t PIN_LCD_MISO = GPIO_NUM_25;
constexpr gpio_num_t PIN_LED_RED = GPIO_NUM_0;
constexpr gpio_num_t PIN_LED_GREEN = GPIO_NUM_2;
constexpr gpio_num_t PIN_LED_YELLOW = GPIO_NUM_4; // IS BLUE!!!
constexpr gpio_num_t PIN_SD_HOST_D2_PULLED_UP = GPIO_NUM_12;
constexpr gpio_num_t PIN_SD_HOST_D3_PULLED_UP = GPIO_NUM_13;
constexpr gpio_num_t PIN_SD_HOST_CMD_PULLED_UP = GPIO_NUM_15;
constexpr gpio_num_t PIN_SD_HOST_CLK_PULLED_UP = GPIO_NUM_14;
constexpr gpio_num_t PIN_SD_HOST_D0_PULLED_UP = GPIO_NUM_2;
constexpr gpio_num_t PIN_SD_HOST_D1_PULLED_UP = GPIO_NUM_4;
constexpr gpio_num_t PIN_SD_HOST_DETECT_PULLED_UP = GPIO_NUM_21;
constexpr gpio_num_t PIN_JTAG_MTMS = GPIO_NUM_14;
constexpr gpio_num_t PIN_JTAG_MTDO = GPIO_NUM_15;
constexpr gpio_num_t PIN_JTAG_MTDI = GPIO_NUM_12;
constexpr gpio_num_t PIN_JTAG_MTCK = GPIO_NUM_13;

// Available 5,
constexpr gpio_num_t available[] = {GPIO_NUM_5,
                                    GPIO_NUM_16, GPIO_NUM_17,                           /*Maybe used internally for PSRAM*/
                                    GPIO_NUM_19, GPIO_NUM_21, GPIO_NUM_22, GPIO_NUM_23, /*all in this line: if  LCD not in use*/
                                    GPIO_NUM_26, GPIO_NUM_27, GPIO_NUM_32, GPIO_NUM_33,
                                    GPIO_NUM_34, GPIO_NUM_35, GPIO_NUM_36, GPIO_NUM_39 /*all in this line: input only, no pullup/pulldowns*/};

constexpr gpio_num_t PIN_SW_ENCODER = GPIO_NUM_22;
constexpr gpio_num_t PIN_SW_GREEN = GPIO_NUM_19;
constexpr gpio_num_t PIN_SW_RED = GPIO_NUM_21;
constexpr gpio_num_t PIN_ONEWIRE = GPIO_NUM_5;
constexpr gpio_num_t PIN_MOVEMENT = PIN_SW_GREEN;
constexpr gpio_num_t PIN_LED_WS2812 = GPIO_NUM_26;

constexpr size_t LED_NUMBER = 8;
constexpr rmt_channel_t CHANNEL_WS2812 = RMT_CHANNEL_0;
constexpr rmt_channel_t CHANNEL_ONEWIRE_TX = RMT_CHANNEL_1;
constexpr rmt_channel_t CHANNEL_ONEWIRE_RX = RMT_CHANNEL_2;

constexpr uint32_t DEFAULT_VREF = 1100; // Use adc2_vref_to_gpio() to obtain a better estimate

class HAL_wroverkit : public HAL
{
private:
    bool sw_green_is_pressed = false;
    bool sw_red_is_pressed = false;
    bool sw_encoder_is_pressed = false;
    bool movementIsDetected = false;

    bool led_red_on = false;
    bool led_yellow_on = false;
    bool led_green_on = false;

    float heaterState = 0.0;
    float fan1State = 0.0;
    float fan2State = 0.0;

    const int SERVO_MIN_PULSEWIDTH = 500;  // Minimum pulse width in microsecond
    const int SERVO_MAX_PULSEWIDTH = 2400; // Maximum pulse width in microsecond
    const int SERVO_MAX_DEGREE = 180;      // Maximum angle in degree upto which servo can rotate
    bool needLedStripUpdate = false;
    int buttonState = 0;

    int64_t nextOneWireReadout = INT64_MAX;
    float lastTemperature = 0.0;

    WS2812_Strip<LED_NUMBER> *strip = NULL;

    owb_rmt_driver_info rmt_driver_info;
    OneWireBus *owb = NULL;
    DS18B20_Info *ds18b20_info = NULL;

public:
    HAL_wroverkit() {}

    int64_t IRAM_ATTR GetMicros()
    {
        return esp_timer_get_time();
    }

    uint32_t GetMillis()
    {
        return (uint32_t)(esp_timer_get_time() / 1000ULL);
    }

    ErrorCode GetHeaterTemperature(float *degrees)
    {
        *degrees = this->lastTemperature;
        return ErrorCode::OK;
    }

    ErrorCode FreeFanAndHeater(bool free)
    {
        return ErrorCode::OK;
    }

    ErrorCode GetEncoderValue(int *s)
    {
        *s = 0;
        return ErrorCode::OK;
    }

    ErrorCode GetCO2PPM(unsigned short *a)
    {
        *a = 0;
        return ErrorCode::OK;
    }

    ErrorCode GetAmbientBrightness(float *s)
    {
        *s = 0;
        return ErrorCode::OK;
    }

    ErrorCode GetAirTemperature(float *s)
    {
        *s = 0;
        return ErrorCode::OK;
    }

    ErrorCode GetAirPressure(float *s)
    {
        *s = 0;
        return ErrorCode::OK;
    }

    ErrorCode GetAirRelHumidity(float *s)
    {
        *s = 0;
        return ErrorCode::OK;
    }

    ErrorCode GetAirSpeed(float *s)
    {
        *s = 0;
        return ErrorCode::OK;
    }

    ErrorCode GetAnalogInputs(float **s)
    {
        return ErrorCode::OK;
    }

    ErrorCode SetSound(unsigned int)
    {
        return ErrorCode::OK;
    }

    ErrorCode GetFFT64(float *)
    {
        return ErrorCode::OK;
    }

    ErrorCode InitAndRun()
    {
        // Boolean Inputs in right sequence
        gpio_set_level(PIN_LCD_RST, 0);
        gpio_set_direction(PIN_LCD_RST, GPIO_MODE_OUTPUT);

        gpio_set_direction(PIN_SW_RED, GPIO_MODE_INPUT);
        gpio_set_pull_mode(PIN_SW_RED, GPIO_PULLUP_ONLY);

        gpio_set_direction(PIN_SW_ENCODER, GPIO_MODE_INPUT);
        gpio_set_pull_mode(PIN_SW_ENCODER, GPIO_PULLUP_ONLY);

        gpio_set_direction(PIN_SW_GREEN, GPIO_MODE_INPUT);
        gpio_set_pull_mode(PIN_SW_GREEN, GPIO_PULLUP_ONLY);

        // Boolean Outputs in right sequence
        gpio_set_level(PIN_LED_RED, 0);
        gpio_set_direction(PIN_LED_RED, GPIO_MODE_OUTPUT);

        gpio_set_level(PIN_LED_YELLOW, 0);
        gpio_set_direction(PIN_LED_YELLOW, GPIO_MODE_OUTPUT);

        gpio_set_level(PIN_LED_GREEN, 0);
        gpio_set_direction(PIN_LED_GREEN, GPIO_MODE_OUTPUT);

        // OneWire
        owb = owb_rmt_initialize(&rmt_driver_info, PIN_ONEWIRE, CHANNEL_ONEWIRE_TX, CHANNEL_ONEWIRE_RX);
        owb_use_crc(owb, true); // enable CRC check for ROM code
        // Find all connected devices
        OneWireBus_ROMCode rom_code;
        owb_status status = owb_read_rom(owb, &rom_code);
        if (status == OWB_STATUS_OK)
        {
            ESP_LOGI(TAG, "Single device  present");
            this->ds18b20_info = ds18b20_malloc(); // heap allocation
            ds18b20_init_solo(ds18b20_info, owb);  // only one device on bus
            ds18b20_use_crc(ds18b20_info, true);   // enable CRC check on all reads
            ds18b20_set_resolution(ds18b20_info, DS18B20_RESOLUTION_12_BIT);
            ds18b20_convert_all(owb);
            nextOneWireReadout = GetMillis() + 1000;
        }
        else
        {
            ESP_LOGE(TAG, "An error occurred reading ROM code: %d", status);
        }

        // LED Strip
        strip = new WS2812_Strip<LED_NUMBER>();
        ESP_ERROR_CHECK(strip->Init(VSPI_HOST, PIN_LED_WS2812, 2));
        ESP_ERROR_CHECK(strip->Clear(100));

        return ErrorCode::OK;
    }

    ErrorCode HardwareTest()
    {
        return ErrorCode::OK;
    }

    ErrorCode SetServo1Position(uint32_t a)
    {
        return ErrorCode::OK;
    }

    ErrorCode SetServo2Position(uint32_t a)
    {
        return ErrorCode::OK;
    }

    ErrorCode BeforeLoop()
    {
        this->movementIsDetected = gpio_get_level(PIN_MOVEMENT);
        this->sw_red_is_pressed = !gpio_get_level(PIN_SW_RED);
        this->sw_encoder_is_pressed = !gpio_get_level(PIN_SW_ENCODER);
        this->sw_green_is_pressed = !gpio_get_level(PIN_SW_GREEN);
        ESP_LOGI(TAG, "Before int32_t ms = GetMillis();");
        int32_t ms = GetMillis();
        if (ms > nextOneWireReadout)
        {
            float temp = 0.0;
            ESP_LOGI(TAG, "ds18b20_read_temp(this->ds18b20_info, &(this->lastTemperature));");
            ds18b20_read_temp(ds18b20_info, &temp);
            ESP_LOGI(TAG, "Before ds18b20_convert_all(owb);");
            ds18b20_convert_all(owb);
            ESP_LOGI(TAG, "Temperatur gemessen %.1f", temp);
            nextOneWireReadout = GetMillis() + 1000;
        }
        return ErrorCode::OK;
    }

    ErrorCode AfterLoop()
    {
        gpio_set_level(PIN_LED_RED, this->led_red_on);
        gpio_set_level(PIN_LED_YELLOW, this->led_yellow_on);
        gpio_set_level(PIN_LED_GREEN, this->led_green_on);
        return ErrorCode::OK;
        if (needLedStripUpdate)
        {
            strip->Refresh(100);
            needLedStripUpdate = false;
        }
        return ErrorCode::OK;
    }

    ErrorCode StartBuzzer(double freqHz)
    {
        return ErrorCode::OK;
    }

    ErrorCode EndBuzzer()
    {

        return ErrorCode::OK;
    }

    ErrorCode ColorizeLed(LED led, uint32_t color)
    {
        strip->SetPixel((uint8_t)led, color);
        this->needLedStripUpdate = true;
        switch (led)
        {
        case LED::LED_RED:
            this->led_red_on = color != 0;
            break;
        case LED::LED_YELLOW:
            this->led_yellow_on = color != 0;
            break;
        case LED::LED_GREEN:
            this->led_green_on = color != 0;
            break;
        default:
            break;
        }
        return ErrorCode::OK;
    }

    ErrorCode UnColorizeAllLed()
    {
        this->led_red_on = false;
        this->led_yellow_on = false;
        this->led_green_on = false;
        return ErrorCode::OK;
    }

    ErrorCode SetRelayState(bool state)
    {
        return ErrorCode::OK;
    }

    ErrorCode SetHeaterDuty(float dutyInPercent)
    {
        heaterState = dutyInPercent;
        return ErrorCode::OK;
    }

    float GetHeaterState()
    {
        return heaterState;
    }

    ErrorCode SetFan1Duty(float dutyInPercent)
    {
        fan1State = dutyInPercent;
        return ErrorCode::OK;
    }

    float GetFan1State()
    {
        return fan1State;
    }

    ErrorCode SetFan2Duty(float dutyInPercent)
    {
        fan2State = dutyInPercent;
        return ErrorCode::OK;
    }

    float GetFan2State()
    {
        return fan2State;
    }

    ErrorCode SetLedPowerWhiteDuty(uint8_t dutyInpercent)
    {
        return ErrorCode::OK;
    }

    bool GetButtonRedIsPressed()
    {
        return sw_red_is_pressed;
    }

    bool GetButtonEncoderIsPressed()
    {
        return sw_encoder_is_pressed;
    }

    bool GetButtonGreenIsPressed()
    {
        return sw_green_is_pressed;
    }

    bool IsMovementDetected()
    {
        return this->movementIsDetected;
    }

    ErrorCode SetServoPosition(Servo servo, uint32_t angle_0_to_180)
    {

        return ErrorCode::OK;
    }
};