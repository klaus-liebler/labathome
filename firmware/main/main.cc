#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
include "FastLED.h" CRGBPalette16 currentPalette;
TBlendType currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 IRAM_ATTR myRedWhiteBluePalette_p;

#include "palettes.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "BSP.hh"
#include "BSP_wroverkit.hh"
#include "functionblocks.hh"

#define NUM_LEDS 30
#define DATA_PIN 13
#define BRIGHTNESS 80
#define LED_TYPE WS2812B
#define COLOR_ORDER RGB
CRGB leds[NUM_LEDS];

#include "esp_log.h"
static const char *TAG = "main";

const uint32_t DEFAULT_VREF = 1100; //Use adc2_vref_to_gpio() to obtain a better estimate
static esp_adc_cal_characteristics_t *adc_chars;
const adc1_channel_t channel = ADC1_CHANNEL_6; //GPIO34 if ADC1
const adc_bits_width_t width = ADC_WIDTH_BIT_12;
const adc_atten_t atten = ADC_ATTEN_DB_11;

BSP *bsp = new BSP_wroverkit();
PLCManager *manager = new PLCManager(bsp);

extern "C"
{
    void app_main();
    void managementTask(void *);
    void plcTask(void *);
    void testModelCreatorTask(void *);
}

static void check_efuse(void)
{
    //Check TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK)
    {
        printf("eFuse Two Point: Supported\n");
    }
    else
    {
        printf("eFuse Two Point: NOT supported\n");
    }

    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK)
    {
        printf("eFuse Vref: Supported\n");
    }
    else
    {
        printf("eFuse Vref: NOT supported\n");
    }
}

static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP)
    {
        printf("Characterized using Two Point Value\n");
    }
    else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF)
    {
        printf("Characterized using eFuse Vref\n");
    }
    else
    {
        printf("Characterized using Default Vref\n");
    }
}

// Perform an action every 10 ticks.
void managementTask(void *pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 10;

    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();
    //Einschaltverzögerung
    uint32_t presettime = 10;
    uint32_t elapsedtime = 0;
    int last_button_state = 1;
    while (true)
    {
        // Wait for the next cycle.
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        int current_button_state = gpio_get_level(SW_YELLOW);
        if (last_button_state == 1 && current_button_state == 0)
        {
            //button pressed!!
            printf("pressed\n");
            int adc_reading = adc1_get_raw((adc1_channel_t)channel);
            uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
            printf("Raw: %d\tVoltage: %dmV\n", adc_reading, voltage);
        }
        else if (last_button_state == 0 && current_button_state == 1)
        {
            //button released
            elapsedtime = 0;
            gpio_set_level(LED_YELLOW, 0);
            printf("released\n");
        }
        else if (current_button_state == 0)
        {
            //button is held down
            if (elapsedtime < presettime)
            {
                elapsedtime++;
            }
            else if (elapsedtime == presettime)
            {
                gpio_set_level(LED_YELLOW, 1);
                printf("switched\n");
                elapsedtime++;
            }
        }
        last_button_state = current_button_state;
    }
}

void testModelCreatorTask(void *pvParameters)
{
    ESP_LOGI(TAG, "testModelCreatorTask started");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    manager->CompileProtobufConfig2ExecutableAndEnqueue(NULL, 0);
}

// Main PLC Task
void plcTask(void *pvParameters)
{
    ESP_LOGI(TAG, "plcTask started");
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 10;
    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();
    while (true)
    {
        // Wait for the next cycle.
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        ESP_LOGI(TAG, "CheckForNewExecutable");
        manager->CheckForNewExecutable();
        ESP_LOGI(TAG, "fetchInputs");
        bsp->fetchInputs();
        ESP_LOGI(TAG, "Loop");
        manager->Loop();
        ESP_LOGI(TAG, "flushOutputs");
        bsp->flushOutputs();
    }
}

#define N_COLORS 17
CRGB colors[N_COLORS] = {
    CRGB::AliceBlue,
    CRGB::ForestGreen,
    CRGB::Lavender,
    CRGB::MistyRose,
    CRGB::DarkOrchid,
    CRGB::DarkOrange,
    CRGB::Black,
    CRGB::Red,
    CRGB::Green,
    CRGB::Blue,
    CRGB::White,
    CRGB::Teal,
    CRGB::Violet,
    CRGB::Lime,
    CRGB::Chartreuse,
    CRGB::BlueViolet,
    CRGB::Aqua};

void blinkLeds_simple(void *pvParameters)
{

    while (1)
    {

        for (int j = 0; j < N_COLORS; j++)
        {
            printf("blink leds\n");

            for (int i = 0; i < NUM_LEDS; i++)
            {
                leds[i] = colors[j];
            }
            FastLED.show();
            delay(1000);
        };
    }
};

void app_main(void)
{
    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU cores, WiFi%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Free heap: %d\n", esp_get_free_heap_size());
    printf("=======================================================\n");

    printf(" entering app main, call add leds\n");
    FastLED.addLeds<LED_TYPE, DATA_PIN>(leds, NUM_LEDS);

    printf(" set max power\n");
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 1000);

    // change the task below to one of the functions above to try different patterns
    printf("create task for led blinking\n");
    xTaskCreatePinnedToCore(&blinkLeds_simple, "blinkLeds", 4000, NULL, 5, NULL, 0);
    while (true)
    {
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }

    check_efuse();

    adc1_config_width(width);
    adc1_config_channel_atten(channel, atten);

    //Characterize ADC1 (ADC2 used by Wifi)
    adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, atten, width, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);

    bsp->init();
    esp_log_level_set(TAG, ESP_LOG_INFO);
    //xTaskCreate(managementTask, "managementTask", 1024 * 2, NULL, 5, NULL);
    xTaskCreate(plcTask, "plcTask", 4096 * 2, NULL, 6, NULL);
    xTaskCreate(testModelCreatorTask, "testModelCreatorTask", 4096 * 2, NULL, 6, NULL);
    int i = 0;

    while (true)
    {
        printf("Start was %d seconds ago\n", i);
        i += 5;
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
