#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_log.h"

const uint32_t DEFAULT_VREF=1100;        //Use adc2_vref_to_gpio() to obtain a better estimate
static esp_adc_cal_characteristics_t *adc_chars;
const adc1_channel_t channel = ADC1_CHANNEL_6;     //GPIO34 if ADC1
const adc_bits_width_t width = ADC_WIDTH_BIT_12;
const adc_atten_t atten = ADC_ATTEN_DB_11;


extern "C"{
    void app_main();
    void managementTask( void *);
}

const gpio_num_t LED_RED = GPIO_NUM_0;
const gpio_num_t LED_GRN = GPIO_NUM_2;
const gpio_num_t LED_BLU = GPIO_NUM_4;
const gpio_num_t SW_BLCK = GPIO_NUM_15;


static void check_efuse(void)
{
    //Check TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("eFuse Two Point: NOT supported\n");
    }

    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    } else {
        printf("eFuse Vref: NOT supported\n");
    }
}

static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
}


// Perform an action every 10 ticks.
void managementTask( void * pvParameters )
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 10;

    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount ();
    //Einschaltverzögerung
    uint32_t presettime = 10;
    uint32_t elapsedtime = 0;
    int last_button_state = 1;
    while(true)
    {
        // Wait for the next cycle.
        vTaskDelayUntil( &xLastWakeTime, xFrequency );
        int current_button_state = gpio_get_level(SW_BLCK);
        if(last_button_state==1 && current_button_state==0)
        {
            //button pressed!!
            printf("pressed\n");
            int adc_reading = adc1_get_raw((adc1_channel_t)channel);
            uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
            printf("Raw: %d\tVoltage: %dmV\n", adc_reading, voltage);
        }
        else if(last_button_state==0 && current_button_state==1)
        {
            //button released
            elapsedtime=0;
            gpio_set_level(LED_RED, 0);
            printf("released\n");
        }
        else if(current_button_state==0)
        {
            //button is held down
            if(elapsedtime<presettime)
            {
                elapsedtime++;
            }
            else if(elapsedtime==presettime)
            {
                gpio_set_level(LED_RED, 1);
                printf("switched\n");
                elapsedtime++;
            }
        }
        last_button_state=current_button_state;
    }
}

void app_main(void)
{

    printf("Hello world!\n");
    
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
	
    check_efuse();

    adc1_config_width(width);
    adc1_config_channel_atten(channel, atten);

    //Characterize ADC1 (ADC2 used by Wifi)
    adc_chars = (esp_adc_cal_characteristics_t*)calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, atten, width, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);

    gpio_pad_select_gpio((uint8_t)LED_RED);
    gpio_pad_select_gpio((uint8_t)LED_GRN);
    gpio_pad_select_gpio((uint8_t)LED_BLU);
    gpio_pad_select_gpio((uint8_t)SW_BLCK);
    gpio_set_direction(LED_RED, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_GRN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_BLU, GPIO_MODE_OUTPUT);
    gpio_set_direction(SW_BLCK, GPIO_MODE_INPUT);
    gpio_set_pull_mode(SW_BLCK, GPIO_PULLUP_ONLY);
    gpio_set_level(LED_GRN, 0);
    gpio_set_level(LED_BLU, 0);

    
    esp_log_level_set("LAB@HOME", ESP_LOG_INFO);
    xTaskCreate(managementTask, "managementTask", 1024 * 2, NULL, 5, NULL);

    int i=0;

while (true) {
        printf("Start was %d seconds ago\n", i);
        gpio_set_level(LED_BLU, 1);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        gpio_set_level(LED_BLU, 0);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        i++;
    }


    while (true) {
        printf("Start was %d seconds ago\n", i);
        gpio_set_level(LED_RED, 1);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        gpio_set_level(LED_RED, 0);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        i++;
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
