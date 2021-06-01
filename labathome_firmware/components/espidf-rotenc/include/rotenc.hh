#pragma once

#include <stdio.h>
#include "esp_log.h"
#include "sdkconfig.h"
#include "esp_err.h"
#include "driver/pcnt.h"

class cRotaryEncoder{
public:
    pcnt_unit_t pcnt_unit;
	gpio_num_t phase_a_gpio_num;     /*!< Phase A GPIO number */
    gpio_num_t phase_b_gpio_num;     /*!< Phase B GPIO number */
	int minCount;
	int maxCount;
	
	
	
	esp_err_t Init();


    /**
     * @brief Start rotary encoder
     *
     * @param encoder Rotary encoder handle
     * @return
     *      - ESP_OK: Start rotary encoder successfully
     *      - ESP_FAIL: Start rotary encoder failed because of other error
     */
    esp_err_t Start();

    /**
     * @brief Stop rotary encoder
     *
     * @param encoder Rotary encoder handle
     * @return
     *      - ESP_OK: Stop rotary encoder successfully
     *      - ESP_FAIL: Stop rotary encoder failed because of other error
     */
    esp_err_t Stop();


    /**
     * @brief Get rotary encoder counter value
     *
     * @param encoder Rotary encoder handle
     * @return Current counter value (the sign indicates the direction of rotation)
     */
    esp_err_t GetValue(int16_t *value);

	/**
	 * @brief Create rotary encoder instance for EC11
	 *
	 * @param config Rotary encoder configuration
	 * @param ret_encoder Returned rotary encoder handle
	 * @return
	 *      - ESP_OK: Create rotary encoder instance successfully
	 *      - ESP_ERR_INVALID_ARG: Create rotary encoder instance failed because of some invalid argument
	 *      - ESP_ERR_NO_MEM: Create rotary encoder instance failed because there's no enough capable memory
	 *      - ESP_FAIL: Create rotary encoder instance failed because of other error
	 */
	cRotaryEncoder(pcnt_unit_t pcnt_unit, gpio_num_t phase_a_gpio_num, gpio_num_t phase_b_gpio_num, int minCount, int maxCount);
};





