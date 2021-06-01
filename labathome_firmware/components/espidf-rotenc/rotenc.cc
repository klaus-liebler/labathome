#include "include/rotenc.hh"
#include <stdlib.h>
#include <string.h>
#include <sys/cdefs.h>
#include "esp_compiler.h"
#include "esp_log.h"
#include "driver/pcnt.h"



esp_err_t cRotaryEncoder::Start()
{
    return pcnt_counter_resume(this->pcnt_unit);
}

esp_err_t cRotaryEncoder::Stop()
{
    return pcnt_counter_pause(this->pcnt_unit);
}

esp_err_t cRotaryEncoder::GetValue(int16_t *value)
{
    return pcnt_get_counter_value(this->pcnt_unit, value);
}


cRotaryEncoder::cRotaryEncoder(pcnt_unit_t pcnt_unit, gpio_num_t phase_a_gpio_num, gpio_num_t phase_b_gpio_num, int minCount, int maxCount)
	:pcnt_unit(pcnt_unit), phase_a_gpio_num(phase_a_gpio_num), phase_b_gpio_num(phase_b_gpio_num), minCount(minCount), maxCount(maxCount){

}

esp_err_t cRotaryEncoder::Init()
{
	//Set up the IO state of hte pin
	gpio_pad_select_gpio(phase_a_gpio_num);
	gpio_pad_select_gpio(phase_b_gpio_num);
	gpio_set_direction(phase_a_gpio_num, GPIO_MODE_INPUT);
	gpio_set_direction(phase_b_gpio_num, GPIO_MODE_INPUT);
	gpio_pullup_en(phase_a_gpio_num);
	gpio_pullup_en(phase_b_gpio_num);
	
	// Configure channel 0
    pcnt_config_t dev_config;
    dev_config.pulse_gpio_num = this->phase_a_gpio_num;
    dev_config.ctrl_gpio_num = this->phase_b_gpio_num;
    dev_config.channel = PCNT_CHANNEL_0;
    dev_config.unit = this->pcnt_unit;
    dev_config.pos_mode = PCNT_COUNT_DEC;
    dev_config.neg_mode = PCNT_COUNT_INC;
    dev_config.lctrl_mode = PCNT_MODE_REVERSE;
    dev_config.hctrl_mode = PCNT_MODE_KEEP;
    dev_config.counter_h_lim = this->maxCount;
    dev_config.counter_l_lim = this->minCount;
    
    ESP_ERROR_CHECK(pcnt_unit_config(&dev_config));

    // Configure channel 1
    dev_config.pulse_gpio_num = this->phase_b_gpio_num;
    dev_config.ctrl_gpio_num = this->phase_a_gpio_num;
    dev_config.channel = PCNT_CHANNEL_1;
    dev_config.pos_mode = PCNT_COUNT_INC;//TODO DIS
    dev_config.neg_mode = PCNT_COUNT_DEC; //TODO DIS
	//dev_config.lctrl_mode = PCNT_MODE_DISABLE;
    //dev_config.hctrl_mode = PCNT_MODE_DISABLE;
    ESP_ERROR_CHECK(pcnt_unit_config(&dev_config));

    // PCNT pause and reset value
    pcnt_counter_pause(this->pcnt_unit);
    pcnt_counter_clear(this->pcnt_unit);

	// Filter out bounces and noise
	pcnt_set_filter_value(this->pcnt_unit, 250);  // Filter Runt Pulses
	pcnt_filter_enable(this->pcnt_unit);
    return ESP_OK;
}