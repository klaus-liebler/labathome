#include "st7789.hh"
#include <algorithm>    // std::max


constexpr uint8_t  ST77XX_NOP        =0x00;
constexpr uint8_t  ST77XX_SWRESET    =0x01;
constexpr uint8_t  ST77XX_RDDID      =0x04;
constexpr uint8_t  ST77XX_RDDST      =0x09;

constexpr uint8_t  ST77XX_SLPIN      =0x10;
constexpr uint8_t  ST77XX_SLPOUT     =0x11;
constexpr uint8_t  ST77XX_PTLON      =0x12;
constexpr uint8_t  ST77XX_NORON      =0x13;

constexpr uint8_t  ST77XX_INVOFF     =0x20;
constexpr uint8_t  ST77XX_INVON      =0x21;
constexpr uint8_t ST77XX_DISPOFF    =0x28;
constexpr uint8_t  ST77XX_DISPON     =0x29;
constexpr uint8_t  ST77XX_CASET      =0x2A;
constexpr uint8_t ST77XX_RASET      =0x2B;
constexpr uint8_t ST77XX_RAMWR      =0x2C;
constexpr uint8_t ST77XX_RAMRD      =0x2E;

constexpr uint8_t ST77XX_PTLAR      =0x30;
constexpr uint8_t ST77XX_COLMOD     =0x3A;
constexpr uint8_t ST77XX_MADCTL     =0x36;

constexpr uint8_t ST7789_IDMOFF = 0x38;
constexpr uint8_t ST7789_IDMON = 0x39;


constexpr uint8_t ST77XX_MADCTL_MY  =0x80;
constexpr uint8_t ST77XX_MADCTL_MX  =0x40;
constexpr uint8_t ST77XX_MADCTL_MV  =0x20;
constexpr uint8_t ST77XX_MADCTL_ML  =0x10;
constexpr uint8_t ST77XX_MADCTL_RGB =0x00;

constexpr uint8_t ST77XX_RDID1      =0xDA;
constexpr uint8_t ST77XX_RDID2      =0xDB;
constexpr uint8_t ST77XX_RDID3      =0xDC;
constexpr uint8_t ST77XX_RDID4      =0xDD;

constexpr int16_t ST7789_240x240_XSTART =0;
constexpr uint16_t ST7789_240x240_YSTART =80;

#define TFT_NOP     0x00
#define TFT_SWRST   0x01

#define TFT_SLPIN   0x10
#define TFT_SLPOUT  0x11
#define TFT_NORON   0x13

#define TFT_INVOFF  0x20
#define TFT_INVON   0x21
#define TFT_DISPOFF 0x28
#define TFT_DISPON  0x29
#define TFT_CASET   0x2A
#define TFT_PASET   0x2B
#define TFT_RAMWR   0x2C
#define TFT_RAMRD   0x2E
#define TFT_MADCTL  0x36
#define TFT_COLMOD  0x3A

// Flags for TFT_MADCTL
#define TFT_MAD_MY  0x80
#define TFT_MAD_MX  0x40
#define TFT_MAD_MV  0x20
#define TFT_MAD_ML  0x10
#define TFT_MAD_RGB 0x00
#define TFT_MAD_BGR 0x08
#define TFT_MAD_MH  0x04
#define TFT_MAD_SS  0x02
#define TFT_MAD_GS  0x01

#ifdef TFT_RGB_ORDER
  #if (TFT_RGB_ORDER == 1)
    #define TFT_MAD_COLOR_ORDER TFT_MAD_RGB
  #else
    #define TFT_MAD_COLOR_ORDER TFT_MAD_BGR
  #endif
#else
  #ifdef CGRAM_OFFSET
    #define TFT_MAD_COLOR_ORDER TFT_MAD_BGR
  #else
    #define TFT_MAD_COLOR_ORDER TFT_MAD_RGB
  #endif
#endif

#define TFT_IDXRD   0x00 // ILI9341 only, indexed control register read

// ST7789 specific commands used in init
#define ST7789_NOP			0x00
#define ST7789_SWRESET		0x01
#define ST7789_RDDID		0x04
#define ST7789_RDDST		0x09

#define ST7789_RDDPM		0x0A      // Read display power mode
#define ST7789_RDD_MADCTL	0x0B      // Read display MADCTL
#define ST7789_RDD_COLMOD	0x0C      // Read display pixel format
#define ST7789_RDDIM		0x0D      // Read display image mode
#define ST7789_RDDSM		0x0E      // Read display signal mode
#define ST7789_RDDSR		0x0F      // Read display self-diagnostic result (ST7789V)

#define ST7789_SLPIN		0x10
#define ST7789_SLPOUT		0x11
#define ST7789_PTLON		0x12
#define ST7789_NORON		0x13

#define ST7789_INVOFF		0x20
#define ST7789_INVON		0x21
#define ST7789_GAMSET		0x26      // Gamma set
#define ST7789_DISPOFF		0x28
#define ST7789_DISPON		0x29
#define ST7789_CASET		0x2A
#define ST7789_RASET		0x2B
#define ST7789_RAMWR		0x2C
#define ST7789_RGBSET		0x2D      // Color setting for 4096, 64K and 262K colors
#define ST7789_RAMRD		0x2E

#define ST7789_PTLAR		0x30
#define ST7789_VSCRDEF		0x33      // Vertical scrolling definition (ST7789V)
#define ST7789_TEOFF		0x34      // Tearing effect line off
#define ST7789_TEON			0x35      // Tearing effect line on
#define ST7789_MADCTL		0x36      // Memory data access control
#define ST7789_IDMOFF		0x38      // Idle mode off
#define ST7789_IDMON		0x39      // Idle mode on
#define ST7789_RAMWRC		0x3C      // Memory write continue (ST7789V)
#define ST7789_RAMRDC		0x3E      // Memory read continue (ST7789V)
#define ST7789_COLMOD		0x3A

#define ST7789_RAMCTRL		0xB0      // RAM control
#define ST7789_RGBCTRL		0xB1      // RGB control
#define ST7789_PORCTRL		0xB2      // Porch control
#define ST7789_FRCTRL1		0xB3      // Frame rate control
#define ST7789_PARCTRL		0xB5      // Partial mode control
#define ST7789_GCTRL		0xB7      // Gate control
#define ST7789_GTADJ		0xB8      // Gate on timing adjustment
#define ST7789_DGMEN		0xBA      // Digital gamma enable
#define ST7789_VCOMS		0xBB      // VCOMS setting
#define ST7789_LCMCTRL		0xC0      // LCM control
#define ST7789_IDSET		0xC1      // ID setting
#define ST7789_VDVVRHEN		0xC2      // VDV and VRH command enable
#define ST7789_VRHS			0xC3      // VRH set
#define ST7789_VDVSET		0xC4      // VDV setting
#define ST7789_VCMOFSET		0xC5      // VCOMS offset set
#define ST7789_FRCTR2		0xC6      // FR Control 2
#define ST7789_CABCCTRL		0xC7      // CABC control
#define ST7789_REGSEL1		0xC8      // Register value section 1
#define ST7789_REGSEL2		0xCA      // Register value section 2
#define ST7789_PWMFRSEL		0xCC      // PWM frequency selection
#define ST7789_PWCTRL1		0xD0      // Power control 1
#define ST7789_VAPVANEN		0xD2      // Enable VAP/VAN signal output
#define ST7789_CMD2EN		0xDF      // Command 2 enable
#define ST7789_PVGAMCTRL	0xE0      // Positive voltage gamma control
#define ST7789_NVGAMCTRL	0xE1      // Negative voltage gamma control
#define ST7789_DGMLUTR		0xE2      // Digital gamma look-up table for red
#define ST7789_DGMLUTB		0xE3      // Digital gamma look-up table for blue
#define ST7789_GATECTRL		0xE4      // Gate control
#define ST7789_SPI2EN		0xE7      // SPI2 enable
#define ST7789_PWCTRL2		0xE8      // Power control 2
#define ST7789_EQCTRL		0xE9      // Equalize time control
#define ST7789_PROMCTRL		0xEC      // Program control
#define ST7789_PROMEN		0xFA      // Program mode enable
#define ST7789_NVMSET		0xFC      // NVM setting
#define ST7789_PROMACT		0xFE      // Program action


constexpr uint8_t init_cmds1[] =
{
		9,                              //  9 commands in list:
		    ST77XX_SWRESET,   SPI_LCD_DELAY_SIGN, //  1: Software reset, no args, w/delay
		      200,                          //    150 ms delay
		    ST77XX_SLPOUT ,   SPI_LCD_DELAY_SIGN, //  2: Out of sleep mode, no args, w/delay
		      255,                          //     255 = 500 ms delay
		    ST77XX_COLMOD , 1+SPI_LCD_DELAY_SIGN, //  3: Set color mode, 1 arg + delay:
		      0x55,                         //     16-bit color
		      10,                           //     10 ms delay
		    ST77XX_MADCTL , 1,              //  4: Mem access ctrl (directions), 1 arg:
		      0x08,                         //     Row/col addr, bottom-top refresh
		    ST77XX_CASET  , 4,              //  5: Column addr set, 4 args, no delay:
		      0x00,
		      ST7789_240x240_XSTART,        //     XSTART = 0
		      (240+ST7789_240x240_XSTART)>>8,
		      (240+ST7789_240x240_XSTART)&0xFF,  //     XEND = 240
		    ST77XX_RASET  , 4,              //  6: Row addr set, 4 args, no delay:
		      0x00,
		      ST7789_240x240_YSTART,             //     YSTART = 0
		      (240+ST7789_240x240_YSTART)>>8,
		      (240+ST7789_240x240_YSTART)&0xFF,  //     YEND = 240
		    ST77XX_INVON  ,   SPI_LCD_DELAY_SIGN,  //  7: hack
		      10,
		    ST77XX_NORON  ,   SPI_LCD_DELAY_SIGN, //  8: Normal display on, no args, w/delay
		      10,                           //     10 ms delay
		    ST77XX_DISPON ,   SPI_LCD_DELAY_SIGN, //  9: Main screen turn on, no args, delay
		    255 };

constexpr uint8_t init_cmds[] = {
		PARAM_BASE+0, 0x13, // partial mode off
		PARAM_BASE+0, 0x21, // display inversion off
		PARAM_BASE+1, 0x36,0x08,	// memory access 0xc0 for 180 degree flipped
		PARAM_BASE+1, 0x3a,0x55,	// pixel format; 5=RGB565
		PARAM_BASE+2, 0x37,0x00,0x00, //
		PARAM_BASE+5, 0xb2,0x0c,0x0c,0x00,0x33,0x33, // Porch control
		PARAM_BASE+1, 0xb7,0x35,	// gate control
		PARAM_BASE+1, 0xbb,0x1a,	// VCOM
		PARAM_BASE+1, 0xc0,0x2c,	// LCM
		PARAM_BASE+1, 0xc2,0x01,	// VDV & VRH command enable
		PARAM_BASE+1, 0xc3,0x0b,	// VRH set
		PARAM_BASE+1, 0xc4,0x20,	// VDV set
		PARAM_BASE+1, 0xc6,0x0f,	// FR control 2
		PARAM_BASE+2, 0xd0, 0xa4, 0xa1, 	// Power control 1
		PARAM_BASE+14, 0xe0, 0x00,0x19,0x1e,0x0a,0x09,0x15,0x3d,0x44,0x51,0x12,0x03,
		0x00,0x3f,0x3f, 	// gamma 1
		PARAM_BASE+14, 0xe1, 0x00,0x18,0x1e,0x0a,0x09,0x25,0x3f,0x43,0x52,0x33,0x03,
		0x00,0x3f,0x3f,		// gamma 2
		PARAM_BASE+0, 0x29,	// display on
	0
};

/*
 * Creates a lcd display object tailored for an ST7789 controller 
 *
 * @param spi_host Defines whether to use HSPI or VSPI
 * @param spi_mode Defines the SPI mode to use (basically idle levels of signal lines). This is either "0" (if display has a CS line) or "3" (if display does not have a CS line)
 * @param spi_dma_channel Defines which DMA channel (1 or 2) to use. We always test with 2...
 */
TFT_ST7789::TFT_ST7789(const spi_host_device_t spi_host, const uint8_t spi_mode, const uint32_t spi_dma_channel, const int16_t physicalWidth, const int16_t physicalHeigth, const DisplayRotation rotation, const gpio_num_t miso, const gpio_num_t mosi, const gpio_num_t clk, const gpio_num_t cspin, const gpio_num_t dcpin, const gpio_num_t backlightPin, const gpio_num_t rstpin)
:SPILCD16bit(spi_host, spi_mode, spi_dma_channel, physicalWidth, physicalHeigth, rotation, miso, mosi, clk, cspin, dcpin, backlightPin, rstpin)
{

	switch (rotation) {
	case DisplayRotation::ROT0:
		_Mactrl_Data = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST77XX_MADCTL_RGB;
		_width = physicalWidth;
		_height = physicalHeigth;
		break;
	case DisplayRotation::ROT180CW:
		_Mactrl_Data = ST77XX_MADCTL_RGB;
		_width = physicalWidth;
		_height = physicalHeigth;
		break;
	case DisplayRotation::ROT90CW:
		_Mactrl_Data =  ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
		_width = physicalHeigth;
		_height = physicalWidth;
		break;
	case DisplayRotation::ROT270CW:
		_Mactrl_Data = ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
		_width = physicalHeigth;
		_height = physicalWidth;
		break;
	}
}
TFT_ST7789::~TFT_ST7789()
{
	//TODO: Wird hier eigentlich auch der Destruktor der Superklasse aufgerufen?
}

void TFT_ST7789::initDisplay() {
	writecommand(ST77XX_SLPOUT);   // Sleep out
  	vTaskDelay(120/portTICK_RATE_MS);

	writecommand(ST77XX_NORON);    // Normal display mode on

  	//------------------------------display and color format setting--------------------------------//
  	writecommand(ST77XX_MADCTL);
  	writedata(_Mactrl_Data);

  	// JLX240 display datasheet
  	writecommand(0xB6);
  	writedata(0x0A);
  	writedata(0x82);

  	writecommand(ST77XX_COLMOD);
  	writedata(0x55);
  	vTaskDelay(10 / portTICK_RATE_MS);

  	//--------------------------------ST7789V Frame rate setting----------------------------------//
  	writecommand(ST7789_PORCTRL);
  	writedata(0x0c);
  	writedata(0x0c);
  	writedata(0x00);
  	writedata(0x33);
  	writedata(0x33);

  	writecommand(ST7789_GCTRL);      // Voltages: VGH / VGL
  	writedata(0x35);

  	//---------------------------------ST7789V Power setting--------------------------------------//
  	writecommand(ST7789_VCOMS);
  	writedata(0x28);		// JLX240 display datasheet

  	writecommand(ST7789_LCMCTRL);
  	writedata(0x0C);

  	writecommand(ST7789_VDVVRHEN);
  	writedata(0x01);
  	writedata(0xFF);

  	writecommand(ST7789_VRHS);       // voltage VRHS
  	writedata(0x10);

  	writecommand(ST7789_VDVSET);
  	writedata(0x20);

  	writecommand(ST7789_FRCTR2);
  	writedata(0x0f);

  	writecommand(ST7789_PWCTRL1);
  	writedata(0xa4);
  	writedata(0xa1);

  	//--------------------------------ST7789V gamma setting---------------------------------------//
  	writecommand(ST7789_PVGAMCTRL);
  	writedata(0xd0);
  	writedata(0x00);
  	writedata(0x02);
  	writedata(0x07);
  	writedata(0x0a);
  	writedata(0x28);
  	writedata(0x32);
  	writedata(0x44);
  	writedata(0x42);
  	writedata(0x06);
  	writedata(0x0e);
  	writedata(0x12);
  	writedata(0x14);
  	writedata(0x17);

  	writecommand(ST7789_NVGAMCTRL);
  	writedata(0xd0);
  	writedata(0x00);
  	writedata(0x02);
  	writedata(0x07);
  	writedata(0x0a);
  	writedata(0x28);
  	writedata(0x31);
  	writedata(0x54);
  	writedata(0x47);
  	writedata(0x0e);
  	writedata(0x1c);
  	writedata(0x17);
  	writedata(0x1b);
  	writedata(0x1e);

  	writecommand(ST7789_INVON);

  	writecommand(ST7789_CASET);    // Column address set
  	writedata(0x00);
  	writedata(0x00);
  	writedata(0x00);
  	writedata(0xE5);    // 239

  	writecommand(ST7789_RASET);    // Row address set
  	writedata(0x00);
  	writedata(0x00);
  	writedata(0x01);
  	writedata(0x3F);    // 319

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  	vTaskDelay(120/portTICK_RATE_MS);

  	writecommand(ST7789_DISPON);    //Display on
  	vTaskDelay(120/portTICK_RATE_MS);
	
	fillScreen(RED);
}

void TFT_ST7789::setAddr(uint16_t x_min_incl, uint16_t y_min_incl, uint16_t x_max_incl, uint16_t y_max_incl) {
	writecommand(ST77XX_CASET); // Column
	writedata(x_min_incl>>8);
	writedata(x_min_incl&0xff);
	writedata(x_max_incl>>8);
	writedata(x_max_incl&0xff);
	writecommand(ST77XX_RASET); // Page
	writedata(y_min_incl>>8);
	writedata(y_min_incl&0xff);
	writedata(y_max_incl>>8);
	writedata(y_max_incl&0xff);
	
	writecommand(ST77XX_RAMWR); //Into RAM
}

void TFT_ST7789::displayOn(bool on) {
	writecommand(on ? ST77XX_DISPON : ST77XX_DISPOFF);
}

void TFT_ST7789::idleModeOn(bool on) {

	writecommand(on ? ST7789_IDMON : ST7789_IDMOFF);
}

void TFT_ST7789::sleepModeOn(bool sleepIn) {
	if (sleepIn) {
		if (sleep == 1)
			return; //already sleeping
		sleep = 1;
		writecommand(ST77XX_SLPIN);
		vTaskDelay(10/portTICK_RATE_MS);
	} else {
		if (sleep == 0)
			return; //Already awake
		sleep = 0;
		writecommand(ST77XX_SLPOUT);
		vTaskDelay(120/portTICK_RATE_MS);
	}
}





