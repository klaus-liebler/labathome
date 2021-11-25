#pragma once
#include <stdio.h>
#include <string.h>
#include <esp_log.h>
#include <driver/spi_master.h>
#include <driver/gpio.h>
#include "crgb.hh"
#define TAG "WS2812"


template <size_t LEDSIZE>
class WS2812_Strip{
private:
    //static constexpr size_t  LED_DMA_BUFFER_SIZE =((LEDSIZE * 16 * (24/4)))+1;
    //3.2MHz --> spi bit time = 312,5ns
    //4 spi bits are used to transfer one WS2812 bit
    //data bit time = 1250ns
    //reset impulse = 50us = 40 data bit time (48 to be safe and have a "modulo 8==0" number to improve memory alignment)
    static constexpr size_t  LED_DMA_BUFFER_SIZE =(LEDSIZE * 24 /*data bits per LED*/ +48 /*reset pulse*/) * 4 /*spi bits per data bit*/ / 8 /*bits per byte*/;
    uint16_t* buffer;
    uint32_t table[LEDSIZE];
    spi_device_handle_t spi_device_handle=NULL;

public:
    WS2812_Strip(){
        
    }

    esp_err_t SetPixel(size_t index, CRGB color, bool refresh=false){
        index=index%LEDSIZE;
        this->table[index]=color.raw32 ;
        if(refresh) this->Refresh(1000);
        return ESP_OK;
    }


    esp_err_t Refresh(uint32_t timeout_ms){
        static const uint16_t LedBitPattern[16] = {
            0x8888,
            0x8C88,
            0xC888,
            0xCC88,
            0x888C,
            0x8C8C,
            0xC88C,
            0xCC8C,
            0x88C8,
            0x8CC8,
            0xC8C8,
            0xCCC8,
            0x88CC,
            0x8CCC,
            0xC8CC,
            0xCCCC
        };
        uint32_t i;
        int n = 0;
        for (i = 0; i < LEDSIZE; i++) {
            uint32_t temp = table[i];// Data you want to write to each LEDs, I'm my case it's 95 RGB x 3 color

            //R
            buffer[n++] = LedBitPattern[0x0f & (temp >>12)];
            buffer[n++] = LedBitPattern[0x0f & (temp)>>8];

            //G
            buffer[n++] = LedBitPattern[0x0f & (temp >>4)];
            buffer[n++] = LedBitPattern[0x0f & (temp)];

            //B
            buffer[n++] = LedBitPattern[0x0f & (temp >>20)];
            buffer[n++] = LedBitPattern[0x0f & (temp)>>16];

        }

        spi_transaction_t t={};
        t.length = LED_DMA_BUFFER_SIZE * 8; //length is in bits
        t.tx_buffer = buffer;

        ESP_ERROR_CHECK(spi_device_transmit(this->spi_device_handle, &t));
        return ESP_OK;
   
    }

    esp_err_t Clear(uint32_t timeout_ms){
        CRGB black = CRGB::Black;
        memset(table, black.raw32, LEDSIZE*sizeof(uint32_t));
        return Refresh(timeout_ms);
    }
   
    esp_err_t Init(const spi_host_device_t spi_host, const gpio_num_t gpio, const int dma_channel){
        spi_bus_config_t bus_config={};
        bus_config.miso_io_num=GPIO_NUM_NC;
        bus_config.mosi_io_num=gpio;
        bus_config.sclk_io_num=GPIO_NUM_NC;
        bus_config.quadwp_io_num=GPIO_NUM_NC;
        bus_config.quadhd_io_num=GPIO_NUM_NC;
        bus_config.max_transfer_sz=LED_DMA_BUFFER_SIZE;
        ESP_ERROR_CHECK(spi_bus_initialize(spi_host, &bus_config, dma_channel));
        spi_device_interface_config_t dev_config={};
        dev_config.clock_speed_hz=3.2*1000*1000;
        dev_config.mode=0,
        dev_config.spics_io_num=GPIO_NUM_NC;
        dev_config.queue_size=1;
        dev_config.command_bits=0;
        dev_config.address_bits=0;
        ESP_ERROR_CHECK(spi_bus_add_device(spi_host, &dev_config, &this->spi_device_handle));
        
        this->buffer = static_cast<uint16_t*>(heap_caps_malloc(LED_DMA_BUFFER_SIZE, MALLOC_CAP_DMA)); // Critical to be DMA memory.
        if(this->buffer == NULL){
            ESP_LOGE(TAG, "LED DMA Buffer can not be allocated");
            return ESP_FAIL;
        }
        memset(this->buffer, 0, LED_DMA_BUFFER_SIZE);
        Clear(1000);
        return ESP_OK;
    }
};

#undef TAG