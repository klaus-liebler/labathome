/*
	Based on https://github.com/sumotoy/TFT_ILI9163C
	Based on https://github.com/adafruit/Adafruit-GFX-Library
*/
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "gfxfont.h"


static constexpr uint8_t SPI_LCD_DELAY_SIGN = 0x80;
constexpr uint8_t PARAM_BASE=1;
constexpr uint8_t DELAY_0ms   = 0x00;
constexpr uint8_t DELAY_10ms  = 0x40;
constexpr uint8_t DELAY_150ms = 0x80;
constexpr uint8_t DELAY_500ms = 0xC0;

#define Swap2Bytes(val) ( (((val) >> 8) & 0x00FF) | (((val) << 8) & 0xFF00) )
#define RgbTo16bit(r, g, b) ( (((r)<<11)&0xF800) | (((g)<<5)&0x07E0) | ((b)&0x001F))
#define PrepareColorESP32(r,g,b) RgbTo16bit((r),(g),(b))
#define PrepareColor PrepareColorESP32

#include "spilcd16bit_settings.hh"

constexpr uint_fast16_t TEST = ( (((1)<<11)&0xF800) | (((1)<<5)&0x07E0) | ((1)&0x001F));

constexpr uint16_t	BLACK =  	 PrepareColor(0,0,0);
constexpr uint16_t	NAVY  =      PrepareColor(0,0,128);     /*   0,   0, 128 */
constexpr uint16_t	DARKGREEN =  PrepareColor(0,128,0)  ;    /*   0, 128,   0 */
constexpr uint16_t	DARKCYAN =   PrepareColor(0,128,128) ;     /*   0, 128, 128 */
constexpr uint16_t	MAROON =     PrepareColor(128,0,0);      /* 128,   0,   0 */
constexpr uint16_t	PURPLE =     PrepareColor(128,0,128)  ;    /* 128,   0, 128 */
constexpr uint16_t	OLIVE   =    PrepareColor(128,128,0) ;     /* 128, 128,   0 */
constexpr uint16_t	LIGHTGREY =  PrepareColor(192,192,192) ;     /* 192, 192, 192 */
constexpr uint16_t	DARKGREY=    PrepareColor(128,128,128) ;     /* 128, 128, 128 */
constexpr uint16_t	BLUE    =    PrepareColor(0,0,255) ;     /*   0,   0, 255 */
constexpr uint16_t	GREEN   =    PrepareColor(0,255,0) ;     /*   0, 255,   0 */
constexpr uint16_t	CYAN    =    PrepareColor(0,255,255) ;     /*   0, 255, 255 */
constexpr uint16_t	RED     =    PrepareColor(255,0,0) ;     /* 255,   0,   0 */
constexpr uint16_t	MAGENTA  =   PrepareColor(255,0,255)   ;   /* 255,   0, 255 */
constexpr uint16_t	YELLOW  =    PrepareColor(255,255,0) ;     /* 255, 255,   0 */
constexpr uint16_t	WHITE  =     PrepareColor(255,255,255);      /* 255, 255, 255 */
constexpr uint16_t	ORANGE  =    PrepareColor(255,165,0) ;     /* 255, 165,   0 */
constexpr uint16_t	GREENYELLOW= PrepareColor(173,255,47) ;     /* 173, 255,  47 */



enum class DisplayRotation:uint8_t {
//	ROT0    =0b00000000,
//	ROT90CW =0b01100000,
//	ROT180CW=0b11000000,
//	ROT270CW=0b10100000,
	ROT0    =0,
	ROT90CW =1,
	ROT180CW=2,
	ROT270CW=3,
};



enum class BufferfillerMode:uint8_t{
	NONE,
	RECT,
	STRING,
};

enum class Anchor:uint8_t{
	TOP_LEFT,
	BOTTOM_LEFT,
	TOP_RIGHT,
	BOTTOM_RIGHT,
};

enum class PrintStringError
{
	OK=0,
	PARAM_ASEERTION_ERROR,
	LAYOUT_NOT_IMPLEMENTED,
	BUFFER_TOO_SMALL,
	NO_FONT_SET,
	TOO_LONG,
};


class SPILCD16bit{

 public:
	SPILCD16bit(const spi_host_device_t spi_host, const uint8_t spi_mode, const uint32_t dmaChannel, const int16_t physicalWidth, const int16_t physicalHeigth, const DisplayRotation rotation, const gpio_num_t miso, const gpio_num_t mosi, const gpio_num_t clk, const gpio_num_t cspin, const gpio_num_t dcpin, const gpio_num_t backlightPin, const gpio_num_t rstpin);
	~SPILCD16bit();
	PrintStringError printString(int16_t cursor_x, int16_t cursor_y, int16_t xWindowStart, int16_t xWindowEnd, int16_t yWindowStart, int16_t yWindowEnd, Anchor anchorType, const char *format, ...);
	void			begin(void),
					end(void),
					fillScreen(uint16_t color),
					printString(int16_t x, int16_t y, Anchor anchor, const char *str, ...),
					printStringCb(),
					drawPixel(int16_t x, int16_t y),
					drawFastVLine(int16_t x, int16_t y, int16_t h),
					drawFastHLine(int16_t x, int16_t y, int16_t w),
					drawLine(int16_t x0, int16_t y0,int16_t x1, int16_t y1),
					drawRect(int16_t x, int16_t y, int16_t w, int16_t h),
					fillRect(int16_t x, int16_t y, int16_t w, int16_t h),
					fillRectCb(),
					spiTxCompleteCallback(),
					setColors(uint16_t foregroundColor, uint16_t backgroundColor),
					setFont(const GFXfont *font),
					backlightOn(bool on);
	bool			DMAIdle();
	virtual void	idleModeOn(bool onOff)=0;
	virtual void 	displayOn(bool onOff)=0;
	virtual void 	sleepModeOn(bool mode)=0;
 protected:
	void			processInitCommands(const uint8_t* cmdStructure);
	void			processInitCommandsCompact(const uint8_t* startAddr);
	void			writearea(int16_t xmin, int16_t xmax, int16_t ymin, int16_t ymax, uint16_t *linedata);
	void			writearea_wait_for_completion();
	void 			writecommand(const uint8_t cmd);
	void			writedata(const uint8_t data);
	void			writedata16(const uint16_t data);
	virtual void 	initDisplay()=0;
	virtual void 	setAddr(uint16_t x_min_incl, uint16_t y_min_incl, uint16_t x_max_incl, uint16_t y_max_incl)=0;
	uint16_t		_width=0, _height=0;
	uint16_t		_physicalwidth, _physicalheight;
	bool			sleep=false;
 private:

	void	 		charBounds(char c, int16_t *x);
	int16_t 		getTextPixelLength(const char *str);
	void 			getTextBounds(const char *str, int16_t x, int16_t y, Anchor anchorType, int16_t *x1, int16_t *y1, int16_t *x2, int16_t *y2);
	bool 			boundaryCheck(int16_t x,int16_t y);
	const spi_host_device_t spi_host;
	const uint8_t spi_mode;
	const uint32_t 	dmaChannel;
	const DisplayRotation rotation;
	const gpio_num_t	_miso, _mosi, _clk, _cs, _dc, _backlight, _rst;//dc HIGH -->DATA

	spi_device_handle_t spi_dev;
	int16_t		cursor_x=0, cursor_y=0;
	char 		strBuffer[STRING_BUFFER_SIZE_CHARS];
	size_t		strLength=0;
	int16_t 	xWindowStart=0, xWindowEnd=0, yWindowStart=0, yWindowEnd=0;
	uint16_t	*buffer[2];
	spi_transaction_t trans[6];
	int32_t		bufferStep=0; //kann vom bufferFiller genutzt werden, um sich zu merken, wo beim nächsten Callback weiterzumachen ist
	BufferfillerMode bufferFillerMode=BufferfillerMode::NONE;
	uint16_t	foregroundColor=BLACK;
	uint16_t	backgroundColor=WHITE;
	const GFXfont*	font=NULL;
};
