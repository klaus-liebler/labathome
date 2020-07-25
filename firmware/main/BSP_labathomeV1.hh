#pragma once
#include <stdio.h>
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
#include "BSP.hh"
#include "labathomeerror.hh"


const gpio_num_t BRIGHTNESS = GPIO_NUM_34;
const gpio_num_t POTI = GPIO_NUM_35;
const gpio_num_t SW_RED = GPIO_NUM_32;
const gpio_num_t SW_BLACK = GPIO_NUM_33;
const gpio_num_t SW_GREEN = GPIO_NUM_25;
const gpio_num_t LED_RED = GPIO_NUM_26;
const gpio_num_t LED_YELLOW = GPIO_NUM_27;
const gpio_num_t LED_GREEN = GPIO_NUM_14;

const gpio_num_t MOVEMENT = GPIO_NUM_23;
const gpio_num_t I2C_SCL = GPIO_NUM_22;
const gpio_num_t I2C_SDA = GPIO_NUM_21;
const gpio_num_t LED_POWER_WHITE = GPIO_NUM_19;
const gpio_num_t HEATER = GPIO_NUM_17;
const gpio_num_t FAN_DRIVE = GPIO_NUM_16;
const gpio_num_t SERVO_PWM1__I2S_SD = GPIO_NUM_4;
const gpio_num_t SERVO_PWM1 = GPIO_NUM_0;



/*
======================
IO-Tabelle
======================
*/
const size_t NULLINPUT_INDEX = 0;
const size_t SW_RED_INDEX = 1;
const size_t SW_BLACK_INDEX = 2;
const size_t SW_GREEN_INDEX = 3;
const size_t MOVEMENT_INDEX = 4;

const size_t NULLOUTPUT_INDEX = 0;
const size_t LED_RED_INDEX = 1;
const size_t LED_YELLOW_INDEX = 2;
const size_t LED_GREEN_INDEX = 3;

class BSP_labathomeV1:public BSP
{
    private:
        uint32_t inputs  = 0;
        uint32_t outputs = 0;
        
    
    public:
        LabAtHomeErrorCode init()
        {
            //Boolean Inputs in right sequence
            gpio_pad_select_gpio((uint8_t)SW_RED);
            gpio_set_direction(SW_RED, GPIO_MODE_INPUT);
            gpio_set_pull_mode(SW_RED, GPIO_PULLUP_ONLY);
            
            gpio_pad_select_gpio((uint8_t)SW_BLACK);
            gpio_set_direction(SW_BLACK, GPIO_MODE_INPUT);
            gpio_set_pull_mode(SW_BLACK, GPIO_PULLUP_ONLY);

            gpio_pad_select_gpio((uint8_t)SW_GREEN);
            gpio_set_direction(SW_GREEN, GPIO_MODE_INPUT);
            gpio_set_pull_mode(SW_GREEN, GPIO_PULLUP_ONLY);

            gpio_pad_select_gpio((uint8_t)MOVEMENT);
            gpio_set_direction(MOVEMENT, GPIO_MODE_INPUT);

            //FÜR V2 ff: R3_A1
            
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

            //FÜR V2 ff: R3_OVERRIDE und R3_SET

            

            //Integer Inputs
            //TODO: FÜr zwei BMExyz (also Addr 0x76 und 0x77 Temperatur, Luftfeuchtigkeit, Luftdruck, reservieren: Luftqualität, 
            //TODO: Dann: Helligkeitssensor
            //Für V2 ff.: Lautstärke, Geschwindigkeit FA1, POTI, POTI_T


            //Integer Outputs
            gpio_set_level(LED_POWER_WHITE, 0);
            gpio_pad_select_gpio((uint8_t)LED_POWER_WHITE); //TODO: PWM Output
            gpio_set_direction(LED_POWER_WHITE, GPIO_MODE_OUTPUT);

            gpio_set_level(HEATER, 0);
            gpio_pad_select_gpio((uint8_t)HEATER); //TODO: PWM Output
            gpio_set_direction(HEATER, GPIO_MODE_OUTPUT);
            
            gpio_set_level(FAN_DRIVE, 0);
            gpio_pad_select_gpio((uint8_t)FAN_DRIVE); //TODO: PWM Output
            gpio_set_direction(FAN_DRIVE, GPIO_MODE_OUTPUT);
            //Für V2 ff.: Buzzer_FREQ, Buzzer_TIMEMS (Beide Werte können gesetzt werden; Time zählt selbständig zurück. Wenn TIME also 0 ist, dann wird kein Ton mehr gespiel)
            //Hier ggf auch einige "Melodien" vordefinieren Buzzer_MELODY, die sich dann nach deauch selbst nach dem Abspielen auf 0 zurücksetzen. Melody hat Priorität und setzt selbständig Buzzer Freq und Buzzer TIMEMS auf die korrekten Werte
            return LabAtHomeErrorCode::OK;

        }

        LabAtHomeErrorCode getBinaryInput(size_t index, bool *value)
        {
            *value = inputs & (1<<index);
            return LabAtHomeErrorCode::OK;
        }

        LabAtHomeErrorCode setBinaryOutput(size_t index, bool value)
        {
            if(value)
            {
                outputs = outputs | (1<<index);
            }
            else
            {
                outputs = outputs & ~(1<<index);
            }
            
            
            return LabAtHomeErrorCode::OK;
        }

        LabAtHomeErrorCode fetchInputs()
        {
            this->inputs =  ((gpio_get_level(SW_RED)  ==0)<<SW_RED_INDEX) | 
                            ((gpio_get_level(SW_BLACK)==0)<<SW_BLACK_INDEX) |
                            ((gpio_get_level(SW_GREEN)==0)<<SW_GREEN_INDEX) |
                            ((gpio_get_level(MOVEMENT)==0)<<MOVEMENT_INDEX);
            return LabAtHomeErrorCode::OK;
        }

        LabAtHomeErrorCode flushOutputs()
        {
                gpio_set_level(LED_RED,    !(outputs & (1<<LED_RED_INDEX)));
                gpio_set_level(LED_YELLOW, !(outputs & (1<<LED_YELLOW_INDEX)));
                gpio_set_level(LED_GREEN,  !(outputs & (1<<LED_GREEN_INDEX)));
                return LabAtHomeErrorCode::OK;
        }


};