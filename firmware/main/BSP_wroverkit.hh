#pragma once
#include <stdio.h>
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
#include "BSP.hh"

const gpio_num_t LED_RED = GPIO_NUM_0;
const gpio_num_t LED_GREEN = GPIO_NUM_2;
const gpio_num_t LED_YELLOW = GPIO_NUM_4; //IS BLUE!!!
const gpio_num_t SW_GREEN = GPIO_NUM_25;
const gpio_num_t SW_YELLOW = GPIO_NUM_23;
const gpio_num_t SW_RED = GPIO_NUM_27;



/*
======================
IO-Tabelle
======================
*/
const size_t NULLINPUT_INDEX = 0;
const size_t SW_RED_INDEX = 1;
const size_t SW_YELLOW_INDEX = 2;
const size_t SW_GREEN_INDEX = 3;
const size_t MOVEMENT_INDEX = 4;

const size_t NULLOUTPUT_INDEX = 0;
const size_t LED_RED_INDEX = 1;
const size_t LED_YELLOW_INDEX = 2;
const size_t LED_GREEN_INDEX = 3;

class BSP_wroverkit:public BSP
{
    private:
        uint32_t inputs  = 0;
        uint32_t outputs = 0;
        
    
    public:
        BSP_wroverkit()
        {
            
        }
        esp_err_t init()
        {
            //Boolean Inputs in right sequence
            gpio_pad_select_gpio((uint8_t)SW_RED);
            gpio_set_direction(SW_RED, GPIO_MODE_INPUT);
            gpio_set_pull_mode(SW_RED, GPIO_PULLUP_ONLY);
            
            gpio_pad_select_gpio((uint8_t)SW_YELLOW);
            gpio_set_direction(SW_YELLOW, GPIO_MODE_INPUT);
            gpio_set_pull_mode(SW_YELLOW, GPIO_PULLUP_ONLY);

            gpio_pad_select_gpio((uint8_t)SW_GREEN);
            gpio_set_direction(SW_GREEN, GPIO_MODE_INPUT);
            gpio_set_pull_mode(SW_GREEN, GPIO_PULLUP_ONLY);

            //Boolean Outputs in right sequence
            gpio_set_level(LED_RED, 0);
            gpio_pad_select_gpio((uint8_t)LED_RED);
            gpio_set_direction(LED_RED, GPIO_MODE_OUTPUT);

            gpio_set_level(LED_YELLOW, 0);
            gpio_pad_select_gpio((uint8_t)LED_YELLOW);
            gpio_set_direction(LED_YELLOW, GPIO_MODE_OUTPUT);
            
            gpio_set_level(LED_GREEN, 0);
            gpio_pad_select_gpio((uint8_t)LED_GREEN);
            gpio_set_direction(LED_GREEN, GPIO_MODE_OUTPUT);
            return ESP_OK;

        }

        esp_err_t getBinaryInput(size_t index, bool *value)
        {
            *value = inputs & (1<<index);
            return ESP_OK;
        }

        esp_err_t setBinaryOutput(size_t index, bool value)
        {
            if(value)
            {
                outputs = outputs | (1<<index);
            }
            else
            {
                outputs = outputs & ~(1<<index);
            }
            
            
            return ESP_OK;
        }

        esp_err_t fetchInputs()
        {
            this->inputs =  ((gpio_get_level(SW_RED)  ==0)<<SW_RED_INDEX) | 
                            ((gpio_get_level(SW_YELLOW)==0)<<SW_YELLOW_INDEX) |
                            ((gpio_get_level(SW_GREEN)==0)<<SW_GREEN_INDEX);
            return ESP_OK;
        }

        esp_err_t flushOutputs()
        {
                gpio_set_level(LED_RED,    !(outputs & (1<<LED_RED_INDEX)));
                gpio_set_level(LED_YELLOW, !(outputs & (1<<LED_YELLOW_INDEX)));
                gpio_set_level(LED_GREEN,  !(outputs & (1<<LED_GREEN_INDEX)));
                return ESP_OK;
        }


};