#pragma once
#include <stdlib.h>
#include <string.h>
#include <sys/cdefs.h>
#include "esp_log.h"
#include "esp_attr.h"
#include "driver/rmt.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt.h"

#define PrepareColorGRB(rgb) ( (((rgb) << 8) & 0x00FF0000) | (((rgb) >> 8) & 0x0000FF00) | (rgb & 0x000000FF) )
#define PrepareColor PrepareColorGRB



struct CRGB {
	union {
		struct {
            union {
                uint8_t r;
                uint8_t red;
            };
            union {
                uint8_t g;
                uint8_t green;
            };
            union {
                uint8_t b;
                uint8_t blue;
            };
        };
		uint8_t raw[3];
        uint32_t raw32;
	};
    /// Array access operator to index into the crgb object
	inline uint8_t& operator[] (uint8_t x) __attribute__((always_inline))
    {
        return raw[x];
    }

    /// Array access operator to index into the crgb object
    inline const uint8_t& operator[] (uint8_t x) const __attribute__((always_inline))
    {
        return raw[x];
    }

    // default values are UNINITIALIZED
    inline CRGB() __attribute__((always_inline)) = default;

    /// allow construction from R, G, B
    inline CRGB( uint8_t ir, uint8_t ig, uint8_t ib)  __attribute__((always_inline))
        : r(ir), g(ig), b(ib)
    {
    }

    inline CRGB( uint32_t colorcode)  __attribute__((always_inline))
    : r((colorcode >> 16) & 0xFF), g((colorcode >> 8) & 0xFF), b((colorcode >> 0) & 0xFF)
    {
    }

    //inline CRGB( PredefinedColors colorcode)  __attribute__((always_inline))
    //: r(((uint32_t)colorcode >> 16) & 0xFF), g(((uint32_t)colorcode >> 8) & 0xFF), b(((uint32_t)colorcode >> 0) & 0xFF)
    //{
    //}

    /// divide each of the channels by a constant
    inline CRGB& operator/= (uint8_t d )
    {
        r /= d;
        g /= d;
        b /= d;
        return *this;
    }

    /// right shift each of the channels by a constant
    inline CRGB& operator>>= (uint8_t d)
    {
        r >>= d;
        g >>= d;
        b >>= d;
        return *this;
    }

    /// invert each channel
    inline CRGB operator- ()
    {
        CRGB retval;
        retval.r = 255 - r;
        retval.g = 255 - g;
        retval.b = 255 - b;
        return retval;
    }

    typedef enum
    {
    /// Predefined RGB colors
    
        AliceBlue=PrepareColor(0xF0F8FF),
        Amethyst=PrepareColor(0x9966CC),
        AntiqueWhite=PrepareColor(0xFAEBD7),
        Aqua=PrepareColor(0x00FFFF),
        Aquamarine=PrepareColor(0x7FFFD4),
        Azure=PrepareColor(0xF0FFFF),
        Beige=PrepareColor(0xF5F5DC),
        Bisque=PrepareColor(0xFFE4C4),
        Black=PrepareColor(0x000000),
        BlanchedAlmond=PrepareColor(0xFFEBCD),
        Blue=PrepareColor(0x0000FF),
        BlueViolet=PrepareColor(0x8A2BE2),
        Brown=PrepareColor(0xA52A2A),
        BurlyWood=PrepareColor(0xDEB887),
        CadetBlue=PrepareColor(0x5F9EA0),
        Chartreuse=PrepareColor(0x7FFF00),
        Chocolate=PrepareColor(0xD2691E),
        Coral=PrepareColor(0xFF7F50),
        CornflowerBlue=PrepareColor(0x6495ED),
        Cornsilk=PrepareColor(0xFFF8DC),
        Crimson=PrepareColor(0xDC143C),
        Cyan=PrepareColor(0x00FFFF),
        DarkBlue=PrepareColor(0x00008B),
        DarkCyan=PrepareColor(0x008B8B),
        DarkGoldenrod=PrepareColor(0xB8860B),
        DarkGray=PrepareColor(0xA9A9A9),
        DarkGrey=PrepareColor(0xA9A9A9),
        DarkGreen=PrepareColor(0x006400),
        DarkKhaki=PrepareColor(0xBDB76B),
        DarkMagenta=PrepareColor(0x8B008B),
        DarkOliveGreen=PrepareColor(0x556B2F),
        DarkOrange=PrepareColor(0xFF8C00),
        DarkOrchid=PrepareColor(0x9932CC),
        DarkRed=PrepareColor(0x8B0000),
        DarkSalmon=PrepareColor(0xE9967A),
        DarkSeaGreen=PrepareColor(0x8FBC8F),
        DarkSlateBlue=PrepareColor(0x483D8B),
        DarkSlateGray=PrepareColor(0x2F4F4F),
        DarkSlateGrey=PrepareColor(0x2F4F4F),
        DarkTurquoise=PrepareColor(0x00CED1),
        DarkViolet=PrepareColor(0x9400D3),
        DeepPink=PrepareColor(0xFF1493),
        DeepSkyBlue=PrepareColor(0x00BFFF),
        DimGray=PrepareColor(0x696969),
        DimGrey=PrepareColor(0x696969),
        DodgerBlue=PrepareColor(0x1E90FF),
        FireBrick=PrepareColor(0xB22222),
        FloralWhite=PrepareColor(0xFFFAF0),
        ForestGreen=PrepareColor(0x228B22),
        Fuchsia=PrepareColor(0xFF00FF),
        Gainsboro=PrepareColor(0xDCDCDC),
        GhostWhite=PrepareColor(0xF8F8FF),
        Gold=PrepareColor(0xFFD700),
        Goldenrod=PrepareColor(0xDAA520),
        Gray=PrepareColor(0x808080),
        Grey=PrepareColor(0x808080),
        Green=PrepareColor(0x008000),
        GreenYellow=PrepareColor(0xADFF2F),
        Honeydew=PrepareColor(0xF0FFF0),
        HotPink=PrepareColor(0xFF69B4),
        IndianRed=PrepareColor(0xCD5C5C),
        Indigo=PrepareColor(0x4B0082),
        Ivory=PrepareColor(0xFFFFF0),
        Khaki=PrepareColor(0xF0E68C),
        Lavender=PrepareColor(0xE6E6FA),
        LavenderBlush=PrepareColor(0xFFF0F5),
        LawnGreen=PrepareColor(0x7CFC00),
        LemonChiffon=PrepareColor(0xFFFACD),
        LightBlue=PrepareColor(0xADD8E6),
        LightCoral=PrepareColor(0xF08080),
        LightCyan=PrepareColor(0xE0FFFF),
        LightGoldenrodYellow=PrepareColor(0xFAFAD2),
        LightGreen=PrepareColor(0x90EE90),
        LightGrey=PrepareColor(0xD3D3D3),
        LightPink=PrepareColor(0xFFB6C1),
        LightSalmon=PrepareColor(0xFFA07A),
        LightSeaGreen=PrepareColor(0x20B2AA),
        LightSkyBlue=PrepareColor(0x87CEFA),
        LightSlateGray=PrepareColor(0x778899),
        LightSlateGrey=PrepareColor(0x778899),
        LightSteelBlue=PrepareColor(0xB0C4DE),
        LightYellow=PrepareColor(0xFFFFE0),
        Lime=PrepareColor(0x00FF00),
        LimeGreen=PrepareColor(0x32CD32),
        Linen=PrepareColor(0xFAF0E6),
        Magenta=PrepareColor(0xFF00FF),
        Maroon=PrepareColor(0x800000),
        MediumAquamarine=PrepareColor(0x66CDAA),
        MediumBlue=PrepareColor(0x0000CD),
        MediumOrchid=PrepareColor(0xBA55D3),
        MediumPurple=PrepareColor(0x9370DB),
        MediumSeaGreen=PrepareColor(0x3CB371),
        MediumSlateBlue=PrepareColor(0x7B68EE),
        MediumSpringGreen=PrepareColor(0x00FA9A),
        MediumTurquoise=PrepareColor(0x48D1CC),
        MediumVioletRed=PrepareColor(0xC71585),
        MidnightBlue=PrepareColor(0x191970),
        MintCream=PrepareColor(0xF5FFFA),
        MistyRose=PrepareColor(0xFFE4E1),
        Moccasin=PrepareColor(0xFFE4B5),
        NavajoWhite=PrepareColor(0xFFDEAD),
        Navy=PrepareColor(0x000080),
        OldLace=PrepareColor(0xFDF5E6),
        Olive=PrepareColor(0x808000),
        OliveDrab=PrepareColor(0x6B8E23),
        Orange=PrepareColor(0xFFA500),
        OrangeRed=PrepareColor(0xFF4500),
        Orchid=PrepareColor(0xDA70D6),
        PaleGoldenrod=PrepareColor(0xEEE8AA),
        PaleGreen=PrepareColor(0x98FB98),
        PaleTurquoise=PrepareColor(0xAFEEEE),
        PaleVioletRed=PrepareColor(0xDB7093),
        PapayaWhip=PrepareColor(0xFFEFD5),
        PeachPuff=PrepareColor(0xFFDAB9),
        Peru=PrepareColor(0xCD853F),
        Pink=PrepareColor(0xFFC0CB),
        Plaid=PrepareColor(0xCC5533),
        Plum=PrepareColor(0xDDA0DD),
        PowderBlue=PrepareColor(0xB0E0E6),
        Purple=PrepareColor(0x800080),
        Red=PrepareColor(0xFF0000),
        RosyBrown=PrepareColor(0xBC8F8F),
        RoyalBlue=PrepareColor(0x4169E1),
        SaddleBrown=PrepareColor(0x8B4513),
        Salmon=PrepareColor(0xFA8072),
        SandyBrown=PrepareColor(0xF4A460),
        SeaGreen=PrepareColor(0x2E8B57),
        Seashell=PrepareColor(0xFFF5EE),
        Sienna=PrepareColor(0xA0522D),
        Silver=PrepareColor(0xC0C0C0),
        SkyBlue=PrepareColor(0x87CEEB),
        SlateBlue=PrepareColor(0x6A5ACD),
        SlateGray=PrepareColor(0x708090),
        SlateGrey=PrepareColor(0x708090),
        Snow=PrepareColor(0xFFFAFA),
        SpringGreen=PrepareColor(0x00FF7F),
        SteelBlue=PrepareColor(0x4682B4),
        Tan=PrepareColor(0xD2B48C),
        Teal=PrepareColor(0x008080),
        Thistle=PrepareColor(0xD8BFD8),
        Tomato=PrepareColor(0xFF6347),
        Turquoise=PrepareColor(0x40E0D0),
        Violet=PrepareColor(0xEE82EE),
        Wheat=PrepareColor(0xF5DEB3),
        White=PrepareColor(0xFFFFFF),
        WhiteSmoke=PrepareColor(0xF5F5F5),
        Yellow=PrepareColor(0xFFFF00),
        YellowGreen=PrepareColor(0x9ACD32),

        // LED RGB color that roughly approximates
        // the color of incandescent fairy lights,
        // assuming that you're using FastLED
        // color correction on your LEDs (recommended).
        FairyLight=PrepareColor(0xFFE42D),
        // If you are using no color correction, use this
        FairyLightNCC=PrepareColor(0xFF9D2A)

    }HTMLColorCode;
};

static uint32_t ws2812_t0h_ticks;
static uint32_t ws2812_t1h_ticks;
static uint32_t ws2812_t0l_ticks;
static uint32_t ws2812_t1l_ticks;

/**
 * @brief Convert RGB data to RMT format.
 *
 * @note For WS2812, R,G,B each contains 256 different choices (i.e. uint8_t)
 *
 * @param[in] src: source data, to converted to RMT format
 * @param[in] dest: place where to store the convert result
 * @param[in] src_size: size of source data
 * @param[in] wanted_num: number of RMT items that want to get
 * @param[out] translated_size: number of source data that got converted
 * @param[out] item_num: number of RMT items which are converted from source data
 */
static void IRAM_ATTR ws2812_rmt_adapter(const void *src, rmt_item32_t *dest, size_t src_size,
        size_t wanted_num, size_t *translated_size, size_t *item_num)
{
    if (src == NULL || dest == NULL) {
        *translated_size = 0;
        *item_num = 0;
        return;
    }
    const rmt_item32_t bit0 = {{{ ws2812_t0h_ticks, 1, ws2812_t0l_ticks, 0 }}}; //Logical 0
    const rmt_item32_t bit1 = {{{ ws2812_t1h_ticks, 1, ws2812_t1l_ticks, 0 }}}; //Logical 1
    size_t size = 0;
    size_t num = 0;
    uint8_t *psrc = (uint8_t *)src;
    rmt_item32_t *pdest = dest;
    while (size < src_size && num < wanted_num) {
        for (int i = 0; i < 8; i++) {
            // MSB first
            if (*psrc & (1 << (7 - i))) {
                pdest->val =  bit1.val;
            } else {
                pdest->val =  bit0.val;
            }
            num++;
            pdest++;
        }
        size++;
        psrc++;
    }
    *translated_size = size;
    *item_num = num;
}

inline bool operator== (const CRGB& lhs, const CRGB& rhs)
{
    return (lhs.r == rhs.r) && (lhs.g == rhs.g) && (lhs.b == rhs.b);
}

inline bool operator!= (const CRGB& lhs, const CRGB& rhs)
{
    return !(lhs == rhs);
}

template <size_t LEDSIZE>
class WS2812_Strip{
private:
    const char *TAG = "WS2812_Strip";
    const uint32_t WS2812_T0H_NS =(350);
    const uint32_t WS2812_T0L_NS =(1000);
    const uint32_t WS2812_T1H_NS =(1000);
    const uint32_t WS2812_T1L_NS =(350);
    const uint32_t WS2812_RESET_US =(280);
    


    uint8_t buffer[3*LEDSIZE];
    rmt_channel_t channel;

    

public:


    WS2812_Strip(rmt_channel_t channel):channel(channel)
    {
        memset(this->buffer, 0, 3*LEDSIZE);
    }


    esp_err_t SetPixel(size_t index, CRGB color){
        if(index>=LEDSIZE)
        {
            ESP_LOGE(TAG, "index out of the maximum number of leds");
            return ESP_ERR_INVALID_ARG;
        }
        this->buffer[3*index+0]=color.r;
        this->buffer[3*index+1]=color.g;
        this->buffer[3*index+2]=color.b;
        
        return ESP_OK;
    }


    esp_err_t Refresh(uint32_t timeout_ms)
    {
        ESP_ERROR_CHECK(rmt_write_sample(this->channel, this->buffer, 3*LEDSIZE, true));
        return rmt_wait_tx_done(this->channel, pdMS_TO_TICKS(timeout_ms));
    }

    esp_err_t Clear(uint32_t timeout_ms)
    {
        memset(this->buffer, 0, 3*LEDSIZE);
        return Refresh(timeout_ms);
    }
   

    esp_err_t Init(gpio_num_t gpio)
    {
        rmt_config_t config;
        config.rmt_mode=RMT_MODE_TX;
        config.channel=this->channel;
        config.gpio_num=gpio;
        config.clk_div=2;
        config.mem_block_num=1;
        config.tx_config.carrier_freq_hz=38000;
        config.tx_config.carrier_level=RMT_CARRIER_LEVEL_HIGH;
        config.tx_config.idle_level=RMT_IDLE_LEVEL_LOW;
        config.tx_config.carrier_duty_percent=33,
        config.tx_config.carrier_en=false;
        config.tx_config.loop_en=false;
        config.tx_config.idle_output_en=true;  
        ESP_ERROR_CHECK(rmt_config(&config));
        ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));
        
        uint32_t counter_clk_hz = 40000000;
        // ns -> ticks
        float ratio = (float)counter_clk_hz / 1e9;
        ws2812_t0h_ticks = (uint32_t)(ratio * WS2812_T0H_NS);
        ws2812_t0l_ticks = (uint32_t)(ratio * WS2812_T0L_NS);
        ws2812_t1h_ticks = (uint32_t)(ratio * WS2812_T1H_NS);
        ws2812_t1l_ticks = (uint32_t)(ratio * WS2812_T1L_NS);
        // set ws2812 to rmt adapter
        rmt_translator_init(this->channel, ws2812_rmt_adapter);
        return ESP_OK;
    }

    static CRGB hsv2rgb(uint32_t h, uint32_t s, uint32_t v)
    {
        uint32_t r;
        uint32_t g;
        uint32_t b;

        h %= 360; // h -> [0,360]
        uint32_t rgb_max = v * 2.55f;
        uint32_t rgb_min = rgb_max * (100 - s) / 100.0f;

        uint32_t i = h / 60;
        uint32_t diff = h % 60;

        // RGB adjustment amount by hue
        uint32_t rgb_adj = (rgb_max - rgb_min) * diff / 60;

        switch (i) {
        case 0:
            r = rgb_max;
            g = rgb_min + rgb_adj;
            b = rgb_min;
            break;
        case 1:
            r = rgb_max - rgb_adj;
            g = rgb_max;
            b = rgb_min;
            break;
        case 2:
            r = rgb_min;
            g = rgb_max;
            b = rgb_min + rgb_adj;
            break;
        case 3:
            r = rgb_min;
            g = rgb_max - rgb_adj;
            b = rgb_max;
            break;
        case 4:
            r = rgb_min + rgb_adj;
            g = rgb_min;
            b = rgb_max;
            break;
        default:
            r = rgb_max;
            g = rgb_min;
            b = rgb_max - rgb_adj;
            break;
        }
        return CRGB(r,g,b);
    }

};