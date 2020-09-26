#pragma once
#include <stdio.h>
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
#include "HAL.hh"
#include "labathomeerror.hh"
#include "input_output_id.hh"

const gpio_num_t LED_RED = GPIO_NUM_0;
const gpio_num_t LED_GREEN = GPIO_NUM_2;
const gpio_num_t LED_YELLOW = GPIO_NUM_4; //IS BLUE!!!
const gpio_num_t SW_GREEN = GPIO_NUM_25;
const gpio_num_t SW_ENCODER = GPIO_NUM_23;
const gpio_num_t SW_RED = GPIO_NUM_27;



class HAL_wroverkit:public HAL
{
    private:
        bool sw_green_is_pressed  = false;
        bool sw_red_is_pressed  = false;
        bool sw_encoder_is_pressed  = false;

        bool led_red_on = false;
        bool led_yellow_on = false;
        bool led_green_on = false;
        
    public:
        HAL_wroverkit(){}
        LabAtHomeErrorCode Init()
        {
            //Boolean Inputs in right sequence
            gpio_pad_select_gpio((uint8_t)SW_RED);
            gpio_set_direction(SW_RED, GPIO_MODE_INPUT);
            gpio_set_pull_mode(SW_RED, GPIO_PULLUP_ONLY);
            
            gpio_pad_select_gpio((uint8_t)SW_ENCODER);
            gpio_set_direction(SW_ENCODER, GPIO_MODE_INPUT);
            gpio_set_pull_mode(SW_ENCODER, GPIO_PULLUP_ONLY);

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
            return LabAtHomeErrorCode::OK;

        }

        LabAtHomeErrorCode SetLedRed(bool value){
            this->led_red_on=value;
            return LabAtHomeErrorCode::OK;
        }
        LabAtHomeErrorCode SetLedYellow(bool value){
            this->led_yellow_on=value;
            return LabAtHomeErrorCode::OK;
        }
        LabAtHomeErrorCode SetLedGreen(bool value){
            this->led_green_on=value;
            return LabAtHomeErrorCode::OK;
        }
        LabAtHomeErrorCode GetButtonRed(bool *value){
            *value=this->sw_red_is_pressed;
            return LabAtHomeErrorCode::OK;
        }
        LabAtHomeErrorCode GetButtonEncoder(bool *value){
            *value=this->sw_encoder_is_pressed;
            return LabAtHomeErrorCode::OK;
        }
        LabAtHomeErrorCode GetButtonGreen(bool *value){
            *value=this->sw_green_is_pressed;
            return LabAtHomeErrorCode::OK;
        }
        LabAtHomeErrorCode FetchInputs(){
            this->sw_red_is_pressed= !gpio_get_level(SW_RED);
            this->sw_encoder_is_pressed = !gpio_get_level(SW_ENCODER);
            this->sw_green_is_pressed=!gpio_get_level(SW_GREEN);
            return LabAtHomeErrorCode::OK;
        }

        LabAtHomeErrorCode FlushOutputs(){
            gpio_set_level(LED_RED, this->led_red_on);
            gpio_set_level(LED_YELLOW, this->led_yellow_on);
            gpio_set_level(LED_GREEN,  this->led_green_on);
            return LabAtHomeErrorCode::OK;
        }

        int64_t GetMicroseconds()
        {
            return  esp_timer_get_time();
        }


};