#include <Arduino.h>
#include "spilcd16bit.hh"
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#include <esp_log.h>
static const char* TAG = "LCD";

const uint16_t PARALLEL_LINES = 16;

SPILCD16bit::SPILCD16bit(spi_host_device_t spi_host, const uint32_t dmaChannel, const int16_t physicalWidth, const int16_t physicalHeigth, const DisplayRotation rotation, const gpio_num_t misopin, const gpio_num_t mosipin, const gpio_num_t clkpin, const gpio_num_t cspin, const gpio_num_t dcpin, const gpio_num_t backlightPin, const gpio_num_t rstpin) :
		_physicalwidth(physicalWidth), _physicalheight(physicalHeigth), spi_host(spi_host), dmaChannel(dmaChannel), rotation(rotation), _miso(misopin), _mosi(mosipin), _clk(clkpin), _cs(cspin), _dc(dcpin), _backlight(backlightPin), _rst(rstpin)
{
	buffer[0] = (uint16_t*)heap_caps_malloc(sizeof(uint16_t)*physicalWidth*PARALLEL_LINES, MALLOC_CAP_DMA);
	assert(buffer[0]!=NULL);
	buffer[1] = (uint16_t*)heap_caps_malloc(sizeof(uint16_t)*physicalWidth*PARALLEL_LINES, MALLOC_CAP_DMA);
	assert(buffer[1]!=NULL);

    for (size_t x=0; x<6; x++) {
        memset(&trans[x], 0, sizeof(spi_transaction_t));
        if ((x&1)==0) {
            //Even transfers are commands
            trans[x].length=8;
            trans[x].user=(void*)(0x00000000+this->_dc); 
        } else {
            //Odd transfers are data
            trans[x].length=8*4;
            trans[x].user=(void*)(0x80000000+this->_dc); 
        }
        trans[x].flags=SPI_TRANS_USE_TXDATA;
    }
}

SPILCD16bit::~SPILCD16bit()
{
	heap_caps_free(buffer[0]);
	heap_caps_free(buffer[1]);
}



void SPILCD16bit::backlight(bool on) {
	gpio_set_level(_backlight, on);
}





//This function is called (in irq context!) just before a transmission starts. It will
//set the D/C line to the value indicated in the user field.
extern "C" void IRAM_ATTR lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int val=(int)t->user;
	gpio_num_t dc_pin = (gpio_num_t)(val & 0x7FFFFFFF);
	bool level = (val & 0x80000000)!=0;
	//ESP_LOGI(TAG, "lcd_spi_pre_transfer_callback, dc_pin=%d, level=%d", dc_pin, level);
	//Serial.printf("lcd_spi_pre_transfer_callback, dc_pin=%d, level=%d\n", dc_pin, level);
    gpio_set_level(dc_pin, level);
}

void SPILCD16bit::end(void) {
	spi_device_release_bus(spi_dev);
}

void SPILCD16bit::begin(void) {
	
	ESP_LOGD(TAG, "Initialize the SPI bus");
    spi_bus_config_t buscfg={};
	buscfg.miso_io_num=(_miso>= GPIO_NUM_0 && _miso<GPIO_NUM_MAX)?_miso:-1;
	buscfg.mosi_io_num=this->_mosi;
	buscfg.sclk_io_num=this->_clk;
	buscfg.quadwp_io_num=-1;
	buscfg.quadhd_io_num=-1;
	buscfg.max_transfer_sz=PARALLEL_LINES*240*2+8;
    ESP_ERROR_CHECK(spi_bus_initialize(this->spi_host, &buscfg, this->dmaChannel));

    ESP_LOGD(TAG, "Attach the LCD to the SPI bus");
    spi_device_interface_config_t devcfg={};
	devcfg.clock_speed_hz=SPI_MASTER_FREQ_10M;    //Clock out at 10 MHz
	devcfg.mode=0;                                //SPI mode 0
    devcfg.spics_io_num=(_cs< GPIO_NUM_0 || _cs>=GPIO_NUM_MAX)?-1:_cs;               //CS pin
    devcfg.queue_size=7;                          //We want to be able to queue 7 transactions at a time
    devcfg.pre_cb=lcd_spi_pre_transfer_callback;  //Specify pre-transfer callback to handle D/C line
    ESP_ERROR_CHECK(spi_bus_add_device(this->spi_host, &devcfg, &this->spi_dev));
    
	//Initialize the LCD
	ESP_ERROR_CHECK(spi_device_acquire_bus(spi_dev, portMAX_DELAY));

    //Initialize non-SPI GPIOs
	ESP_LOGD(TAG, "Set _dc pin to %d", _dc);
	Serial.printf("Set _dc pin to %d\n", _dc);
    ESP_ERROR_CHECK(gpio_set_direction(_dc, GPIO_MODE_OUTPUT)); //always necessary

	if(_backlight>= GPIO_NUM_0 && _backlight<GPIO_NUM_MAX)
	{
		ESP_LOGD(TAG, "Set _backlight pin to %d", _backlight);
		Serial.printf("Set _backlight pin to %d\n", _backlight);
		ESP_ERROR_CHECK(gpio_set_direction(_backlight, GPIO_MODE_OUTPUT)); //not always necessary
		backlight(true);
	}
	
	if(_rst>= GPIO_NUM_0 && _rst<GPIO_NUM_MAX)
	{
		ESP_LOGD(TAG, "Set _rst pin to %d", _rst);
		Serial.printf("Set _rst pin to %d\n", _rst);
		ESP_ERROR_CHECK(gpio_set_direction(_rst, GPIO_MODE_OUTPUT)); //not always necessary
		ESP_ERROR_CHECK(gpio_set_level(_rst, 0));
    	vTaskDelay(100 / portTICK_RATE_MS);
    	ESP_ERROR_CHECK(gpio_set_level(_rst, 1));
    	vTaskDelay(100 / portTICK_RATE_MS);
	}
	
	chipInit();
}

/* Send a command to the LCD. Uses spi_device_polling_transmit, which waits
 * until the transfer is complete.
 *
 * Since command transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void SPILCD16bit::writecommand(const uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t={};
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&cmd;               //The data is the cmd itself
    t.user=(void*)(0x00000000+this->_dc);                //D/C needs to be set to 0
    ret=spi_device_polling_transmit(this->spi_dev, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

/* Send data to the LCD. Uses spi_device_polling_transmit, which waits until the
 * transfer is complete.
 *
 * Since data transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void SPILCD16bit::writedata(const uint8_t data)
{
    esp_err_t ret;
    spi_transaction_t t={};
    t.length=8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer=&data;               //Data
    t.user=(void*)(0x80000000+this->_dc);                //D/C needs to be set to 1
    ret=spi_device_polling_transmit(this->spi_dev,  &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

void SPILCD16bit::writedata16(const uint16_t data)
{
    esp_err_t ret;
    spi_transaction_t t{};
    t.length=16;                 //Len is in uint16_t, transaction length is in bits.
    t.tx_buffer=&data;               //Data
    t.user=(void*)(0x80000000+this->_dc);                //D/C needs to be set to 1
    ret=spi_device_polling_transmit(this->spi_dev,  &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

/* To send a set of lines we have to send a command, 2 data bytes, another command, 2 more data bytes and another command
 * before sending the line data itself; a total of 6 transactions. (We can't put all of this in just one transaction
 * because the D/C line needs to be toggled in the middle.)
 * This routine queues these commands up as interrupt transactions so they get
 * sent faster (compared to calling spi_device_transmit several times), and at
 * the mean while the lines for next transactions can get calculated.
 */
void SPILCD16bit::writearea(int16_t xmin, int16_t xmax, int16_t ymin, int16_t ymax, uint16_t *linedata)
{
    esp_err_t ret;
    trans[0].tx_data[0]=0x2A;           //Column Address Set
    trans[1].tx_data[0]=(xmin)>>8;              //Start Col High
    trans[1].tx_data[1]=(xmin)&0xff;              //Start Col Low
    trans[1].tx_data[2]=(xmax)>>8;       //End Col High
    trans[1].tx_data[3]=(xmax)&0xff;     //End Col Low
    trans[2].tx_data[0]=0x2B;           //Page address set
    trans[3].tx_data[0]=ymin>>8;        //Start page high
    trans[3].tx_data[1]=ymin&0xff;      //start page low
    trans[3].tx_data[2]=(ymax)>>8;    //end page high
    trans[3].tx_data[3]=(ymax)&0xff;  //end page low
    trans[4].tx_data[0]=0x2C;           //memory write
    trans[5].tx_buffer=linedata;        //finally send the line data
    trans[5].length=(xmax-xmin)*(ymax-ymin)*2*8;          //Data length, in bits
    trans[5].flags=0; //undo SPI_TRANS_USE_TXDATA flag
    
    //Queue all transactions.
    for (size_t x=0; x<6; x++) {
        ret=spi_device_queue_trans(spi_dev, &trans[x], portMAX_DELAY);
        assert(ret==ESP_OK);
    }
    //When we are here, the SPI driver is busy (in the background) getting the transactions sent. That happens
    //mostly using DMA, so the CPU doesn't have much to do here. We're not going to wait for the transaction to
    //finish because we may as well spend the time calculating the next line. When that is done, we can call
    //send_line_finish, which will wait for the transfers to be done and check their status.
}


void SPILCD16bit::writearea_wait_for_completion()
{
    spi_transaction_t *rtrans;
    esp_err_t ret;
    //Wait for all 6 transactions to be done and get back the results.
    for (int x=0; x<6; x++) {
        ret=spi_device_get_trans_result(spi_dev, &rtrans, portMAX_DELAY);
        assert(ret==ESP_OK);
        //We could inspect rtrans now if we received any info back. The LCD is treated as write-only, though.
    }
}


void SPILCD16bit::fillScreen(uint16_t color) {
	for(size_t i = 0; i < PARALLEL_LINES*_physicalwidth;i++ )
	{
		buffer[0][i]=color;
	}
	for(int line=0; line<_physicalheight;line+=16)
	{
		writearea(0, _physicalwidth, line, line+16, buffer[0]);
		ESP_LOGI(TAG, "Fill Screen for lines %d till %d", line, line+16);
		Serial.printf("Fill Screen for lines %d till %d\n", line, line+16);
		writearea_wait_for_completion();
	}
}

void SPILCD16bit::drawPixel(int16_t x, int16_t y) {
	if (boundaryCheck(x, y))
		return;
	if ((x < 0) || (y < 0))
		return;
	setAddr(x, y, x, y);
	//TODO: Version von SUMOTOY setAddr(x, y, x + 1, y + 1);
	writedata16(this->foregroundColor);
}

void SPILCD16bit::drawFastVLine(int16_t x, int16_t y, int16_t h) {
	// Rudimentary clipping
	if (boundaryCheck(x, y))
		return;
	if (((y + h) - 1) >= _height)
		h = _height - y;
	setAddr(x, y, x, (y + h) - 1);
	while (h-- > 0) {
		writedata16(this->foregroundColor);
	}
}

bool SPILCD16bit::boundaryCheck(int16_t x, int16_t y) {
	if ((x >= _width) || (y >= _height))
		return true;
	return false;
}

void SPILCD16bit::drawFastHLine(int16_t x, int16_t y, int16_t w) {
	// Rudimentary clipping
	if (boundaryCheck(x, y))
		return;
	if (((x + w) - 1) >= _width)
		w = _width - x;
	setAddr(x, y, (x + w) - 1, y);
	while (w-- > 0) {

		writedata16(foregroundColor);
	}

}

// fill a rectangle
void SPILCD16bit::fillRect(int16_t x, int16_t y, int16_t w, int16_t h) {
	if (boundaryCheck(x, y))
		return;
	if (((x + w) - 1) >= _width)
		w = _width - x;
	if (((y + h) - 1) >= _height)
		h = _height - y;

	while (this->bufferFillerMode != BufferfillerMode::NONE)
		vTaskDelay(10/portTICK_RATE_MS); //wait till a previous process has been finished;
	setAddr(x, y, (x + w) - 1, (y + h) - 1);
	this->bufferFillerMode = BufferfillerMode::RECT;
	this->bufferStep = w * h;
	//while (LL_SPI_IsActiveFlag_BSY(spi)) //as long as the setAddr last byte is transmitted
	//		__NOP();
	//Gpio::Set(_dc, true);
	//Gpio::Set(_cs, false);
	this->fillRectCb();

//	for (y = h; y > 0; y--) {
//		for (x = w; x > 0; x--) {
//			writedata16(color);
//		}
//	}

}
//bufferStep zählt, wie viele 16bit-Farben noch zu senden sind
void SPILCD16bit::fillRectCb() {
	size_t length = 0;
	uint16_t *buffer16 = (uint16_t *) buffer0;
	while (bufferStep > 0 && length < BUFFER_SIZE_BYTES / 2) {
		buffer16[length] = this->foregroundColor;
		length++;
		bufferStep--;
	}
	length *= 2;
	//LL_DMA_SetDataLength(DMA1, this->dmaChannel, length);
	//LL_DMA_EnableChannel(DMA1, this->dmaChannel);
}



void SPILCD16bit::setColors(uint16_t foregroundColor, uint16_t backgroundColor)
{
	this->foregroundColor= Swap2Bytes(foregroundColor); //Because of DMA byte order
	this->backgroundColor= Swap2Bytes(backgroundColor);
}

/**************************************************************************/
/*!
    @brief    Helper to determine size of a character with current font/size.
    @param    c     The ascii character in question
    @param    x     Pointer to x location of character
    @param    y     Pointer to y location of character
*/
/**************************************************************************/
void SPILCD16bit::charBounds(char c, int16_t *x) {
	if(c == '\n') { // Newline?
		*x  = 0;    // Reset x to zero, advance y by one line
		return;
	}
	if(c == '\r') { // Not a carriage return; is normal char
		return;
	}
	uint8_t first = font->first;
	uint8_t last  = font->last;
	if((c < first) && (c > last)) { // Char present in this font?
		return;
	}
	GFXglyph *glyph = &((font->glyph)[c - first]);
	*x += glyph->xAdvance;
}

void SPILCD16bit::getTextBounds(const char *str, int16_t x, int16_t y, Anchor anchorType, int16_t *x1, int16_t *y1, int16_t *x2, int16_t *y2) {
	if(anchorType!=Anchor::BOTTOM_LEFT) 
		while (1) 
		{
			//__asm__ __volatile__ ("bkpt #0"); //NOT Implemented
		}
	*x1 = x;
	*x2 = x;
	*y2 = y;
	*y1 = y;

    size_t l = strlen(str);
    for(size_t i=0;i<l; i++)
    {
    	char c = str[i];
    	if(c == '\n' || c == '\r') { // Newline?
			continue;
		}

		uint8_t first = font->first;
		uint8_t last  = font->last;
		if((c < first) && (c > last)) { // Char present in this font?
			continue;
		}
		GFXglyph *glyph = &((font->glyph)[c - first]);
		*x2 += glyph->xAdvance;
		*y1 = std::min((int16_t)*y1, (int16_t)(y+glyph->yOffset));
		*y2 = std::max((int16_t)*y1, (int16_t)(y+glyph->yOffset+glyph->height));
    }
}

int16_t SPILCD16bit::getTextPixelLength(const char *str) {

	int16_t x2=0;

    size_t l = strlen(str);
    for(size_t i=0;i<l; i++)
    {
    	char c = str[i];
    	if(c == '\n' || c == '\r') { // Newline?
			continue;
		}

		uint8_t first = font->first;
		uint8_t last  = font->last;
		if((c < first) && (c > last)) { // Char present in this font?
			continue;
		}
		GFXglyph *glyph = &((font->glyph)[c - first]);
		x2 += glyph->xAdvance;

    }
    return x2;
}

void SPILCD16bit::setFont(const GFXfont* font)
{
	this->font=font;
}



PrintStringError SPILCD16bit::printString(int16_t cursor_x, int16_t cursor_y, int16_t xWindowStart, int16_t xWindowEnd, int16_t yWindowStart, int16_t yWindowEnd, Anchor anchorType, const char *format, ...) {

	//X und Y definieren eine Ankerposition. In welche Richtung ab dort der Text geschrieben wird, bestimmt anchorType
	if(!this->font) return PrintStringError::NO_FONT_SET;
	if(anchorType!=Anchor::BOTTOM_LEFT && anchorType!=Anchor::BOTTOM_RIGHT) return PrintStringError::LAYOUT_NOT_IMPLEMENTED;
	if(xWindowEnd<=xWindowStart) return PrintStringError::PARAM_ASEERTION_ERROR;
	if(yWindowEnd<=yWindowStart) return PrintStringError::PARAM_ASEERTION_ERROR;
	if(BUFFER_SIZE_BYTES/2 < this->_width) return PrintStringError::BUFFER_TOO_SMALL;


	while (this->bufferFillerMode != BufferfillerMode::NONE)
		vTaskDelay(10/portTICK_RATE_MS);
	this->xWindowStart=std::max((int16_t)0, xWindowStart);
	this->xWindowEnd=std::min((int16_t)_width,xWindowEnd);
	this->yWindowStart=std::max((int16_t)0,yWindowStart);
	this->yWindowEnd=std::min((int16_t)_height, yWindowEnd);
	this->bufferStep = yWindowEnd-yWindowStart;
	//va_list va;
	//va_start(va, format);
	//this->strLength=vsnprintf(strBuffer, STRING_BUFFER_SIZE_CHARS, format, va);
	//va_end(va);
	if(anchorType==Anchor::BOTTOM_LEFT)
	{
		this->cursor_x = cursor_x;
		this->cursor_y = cursor_y;
	}
	else if(anchorType==Anchor::BOTTOM_RIGHT)
	{
		this->cursor_x = cursor_x-getTextPixelLength(strBuffer);
		this->cursor_y = cursor_y;
	}
	setAddr(xWindowStart, yWindowStart, xWindowEnd-1, yWindowEnd-1);
	this->bufferFillerMode = BufferfillerMode::STRING;


	//while (LL_SPI_IsActiveFlag_BSY(spi)) //as long as the setAddr last byte is transmitted
	//		__NOP();
	//Gpio::Set(_dc, true);
	//Gpio::Set(_cs, false);
	this->printStringCb();
    return PrintStringError::OK;
}


void SPILCD16bit::printString(int16_t cursor_x, int16_t cursor_y, Anchor anchorType, const char *format, ...) {

	//X und Y definieren eine Ankerposition. In welche Richtung ab dort der Text geschrieben wird, bestimmt anchorType
	if(!this->font) return;
	if(anchorType!=Anchor::BOTTOM_LEFT) 
		while (1)
		{
			// __asm__ __volatile__ ("bkpt #0"); //NOT Implemented
		}

	while (this->bufferFillerMode != BufferfillerMode::NONE)
		vTaskDelay(10/portTICK_RATE_MS); //wait till a previous process has been finished;
	if(BUFFER_SIZE_BYTES/2 < this->_width)//as we write line by line, we need at least one line in the buffer
	{
		while (1) {
				//__asm__ __volatile__ ("bkpt #0");
				
			}
	}


	//va_list va;
	//va_start(va, format);
	//this->strLength=vsnprintf(strBuffer, STRING_BUFFER_SIZE_CHARS, format, va);
	//va_end(va);

	int16_t x1, y1, x2, y2;

	getTextBounds(strBuffer, cursor_x, cursor_y, anchorType, &x1, &y1, &x2, &y2);
	this->cursor_x = cursor_x;
	this->cursor_y = cursor_y;
	this->xWindowStart=std::max((int16_t)0, x1);
	this->xWindowEnd=std::min((int16_t)_width,x2);
	this->yWindowStart=std::max((int16_t)0,y1);
	this->yWindowEnd=std::min((int16_t)_height, y2);


	//this->distanceToBaseline = y1-cursor_y; //negativer Wert, wie glyph->yoffset

	setAddr(xWindowStart, yWindowStart, xWindowEnd-1, yWindowEnd-1);
	this->bufferFillerMode = BufferfillerMode::STRING;
	this->bufferStep = y2-y1;

	//while (LL_SPI_IsActiveFlag_BSY(spi)) //as long as the setAddr last byte is transmitted
	//		__NOP();
	//Gpio::Set(_dc, true);
	//Gpio::Set(_cs, false);
	this->printStringCb();
    return;
}

void SPILCD16bit::printStringCb() {
	uint16_t *buffer16 = (uint16_t *) buffer0;
	size_t length = 0;
	size_t cidx = 0;
	for(int16_t u=0;u<cursor_x-xWindowStart;u++)
	{
		buffer16[length] = this->backgroundColor;
		length++;
	}
	while(cidx<strLength)
	{
		uint8_t c = strBuffer[cidx];
		GFXglyph *glyph  = &(font->glyph[c-font->first]);
		uint8_t  *bitmap = font->bitmap;
		uint16_t bo = glyph->bitmapOffset;
		uint8_t  w  = glyph->width, //of bitmap
				 h  = glyph->height; //of bitmap
		int8_t   xo = glyph->xOffset, //of bitmap
				 yo = glyph->yOffset, //negativ!!
					adv = glyph->xAdvance;


		//BufferStep gibt an, wie viele Zeilen noch fehlen bis zur letzten Zeile

		for(int16_t xx=0; xx<adv;xx++)
		{
			volatile bool bit=false;
			//Prüfung, ob wir innerhalb der Bitmap sind
			int16_t distanceToBaseline= this->yWindowEnd - this->bufferStep - this->cursor_y;
			if(xx>=xo && xx<xo+w && distanceToBaseline >= yo && distanceToBaseline < yo+h)
			{
				volatile int line_in_bitmap = distanceToBaseline-yo;
				int bitindex = line_in_bitmap*w+(xx-xo);
				int byteindex = bitindex >> 3;
				int bitinbyte = bitindex & 0x7;
				bit = (bitmap[bo+byteindex] << bitinbyte) & 0x80;
			}
			buffer16[length] = bit?this->foregroundColor:this->backgroundColor;
			length++;
		}



		cidx++;
	}
	while(length<xWindowEnd-xWindowStart)
	{
		buffer16[length] = this->backgroundColor;
		length++;
	}

	this->bufferStep--;
	length *= 2;
	//LL_DMA_SetDataLength(DMA1, this->dmaChannel, length);
	//LL_DMA_EnableChannel(DMA1, this->dmaChannel);
}

