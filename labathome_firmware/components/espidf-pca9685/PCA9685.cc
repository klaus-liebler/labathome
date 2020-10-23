
#include "include/PCA9685.hh"
#include <i2c.hh>

#define LEDn_ON_L(n) ((uint16_t)(0x06 + ((uint8_t)(n)) * 4))
#define LEDn_ON_H(n) (uint16_t)(0x07 + (n)*4)
#define LEDn_OFF_L(n) (uint16_t)(0x08 + (n)*4)
#define LEDn_OFF_H(n) (uint16_t)(0x09 + (n)*4)

esp_err_t cPCA9685::SetupStatic(i2c_port_t i2c, uint8_t address7bit, ePCA9685_InvOutputs inv, ePCA9685_OutputDriver outdrv, ePCA9685_OutputNotEn outne, ePCA9685_Frequency freq)
{
	uint8_t data = 1 << MODE1_SLEEP;
	if (I2C::WriteReg(i2c, address7bit, MODE1, &data, 1) != ESP_OK)
	{
		return ESP_ERR_NOT_FOUND;
	}

	/* PRE_SCALE Register:
	 * Set to value specified in PCA9685_InitStruct->PWMFrequency;
	 * Has to be set when device is in sleep mode
	 */
	data = (uint8_t)(freq);
	if (I2C::WriteReg(i2c, address7bit, PRE_SCALE, &data, 1) != ESP_OK)
	{
		return ESP_ERR_NOT_FOUND;
	}

	/* MODE1 Register:
	 * Internal clock, not external
	 * Register Auto-Increment enabled
	 * Normal mode (not sleep)
	 * Does not respond to subaddresses
	 * Does not respond to All Call I2C-bus address
	 */
	data = (1 << MODE1_AI);
	if (I2C::WriteReg(i2c, address7bit, MODE1, &data, 1) != ESP_OK)
	{
		return ESP_FAIL;
	}

	/* MODE2 Register:
	 * Outputs change on STOP command
	 */
	data = ((uint8_t)(inv) << MODE2_INVRT) | ((uint8_t)(outdrv) << MODE2_OUTDRV) | ((uint8_t)(outne) << MODE2_OUTNE0);
	if (I2C::WriteReg(i2c, address7bit, MODE2, &data, 1) != ESP_OK)
	{
		return ESP_FAIL;
	}

	//Switch all off
	SetAllOutputs(0);
	return true;
}

esp_err_t cPCA9685::Init(ePCA9685_InvOutputs inv, ePCA9685_OutputDriver outdrv, ePCA9685_OutputNotEn outne, ePCA9685_Frequency freq)
{
	if (SetupStatic(this->i2c_num, (uint8_t)this->adress, this->inv, this->outdrv, outne, freq) != ESP_OK)
	{
		return ESP_FAIL;
	}
	return ESP_OK;
}

esp_err_t cPCA9685::SetOutputFull(ePCA9685Output Output, bool on)
{
	uint8_t data = 0xF0;
	return I2C::WriteReg(this->i2c_num, (uint8_t)this->adress, (uint16_t)(0x06 + 4 * (uint8_t)Output + on ? 1 : 3), &data, 1);
}

/**
 * @brief	Sets a specific output for a PCA9685
 * @param	Address: The address to the PCA9685
 * @param	Output: The output to set
 * @param	OnValue: The value at which the output will turn on
 * @param	OffValue: The value at which the output will turn off
 * @retval	None
 */
esp_err_t cPCA9685::SetOutput(ePCA9685Output Output, uint16_t OnValue, uint16_t OffValue)
{
	// Optional: PCA9685_I2C_SlaveAtAddress(Address), might make things slower
	uint8_t data[4] = {(uint8_t)(OnValue & 0xFF), (uint8_t)((OnValue >> 8) & 0x1F), (uint8_t)(OffValue & 0xFF), (uint8_t)((OffValue >> 8) & 0x1F)};
	return I2C::WriteReg(this->i2c_num, (uint8_t)this->adress, LEDn_ON_L(Output), data, 4);
}

esp_err_t cPCA9685::SetAllOutputs(uint16_t dutyCycle)
{
	uint16_t offValue;
	uint16_t onValue;
	if (dutyCycle == UINT16_MAX)
	{
		onValue = cPCA9685::MAX_OUTPUT_VALUE;
		offValue = 0;
	}
	else if (dutyCycle == 0)
	{
		onValue = 0;
		offValue = cPCA9685::MAX_OUTPUT_VALUE;
	}
	else
	{
		onValue = 0;				 //((uint16_t)Output)*0xFF; //for phase shift to reduce EMI
		offValue = (dutyCycle >> 4); // + onValue; //to make a 12bit-Value
	}

	uint8_t data[4] = {(uint8_t)(onValue & 0xFF), (uint8_t)((onValue >> 8) & 0x1F), (uint8_t)(offValue & 0xFF), (uint8_t)((offValue >> 8) & 0x1F)};
	return I2C::WriteReg(this->i2c_num, (uint8_t)this->adress, ALL_LED_ON_L, data, 4);
}

