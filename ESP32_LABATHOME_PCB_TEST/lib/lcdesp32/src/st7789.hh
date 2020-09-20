#pragma once

#include "gfxfont.h"
#include <stddef.h>
#include <stdint.h>
#include <string>

#include "spilcd16bit.hh"

class TFT_ST7789 : public SPILCD16bit{

 public:
	TFT_ST7789(const spi_host_device_t spi_host, const uint8_t spi_mode, const uint32_t dmaChannel, const int16_t physicalWidth, const int16_t physicalHeigth, const DisplayRotation rotation, const gpio_num_t miso, const gpio_num_t mosi, const gpio_num_t clk, const gpio_num_t cspin, const gpio_num_t dcpin, const gpio_num_t backlightPin, const gpio_num_t rstpin);
	~TFT_ST7789();
	void idleModeOn(const bool onOff) override;
	void displayOn(const bool onOff) override;
	void sleepModeOn(const bool mode) override;
 protected:
	void initDisplay() override;
	void setAddr(uint16_t x_min_incl, uint16_t y_min_incl, uint16_t x_max_incl, uint16_t y_max_incl) override;

 private:
	uint8_t		_Mactrl_Data;
};
