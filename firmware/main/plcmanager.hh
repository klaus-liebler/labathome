#pragma once
#include <stdio.h>
#include "esp_system.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "BSP.hh"

class FunctionBlock;

struct Executable
{
    FunctionBlock *functionBlocks;
    size_t functionBlocksSize;
    uint32_t *binaries; //beyond inpuits/outputs
    size_t binariesSize; //beyond inpuits/outputs
    int *integers;
    size_t integersSize; //beyond inpuits/outputs
};


class FBContext{
    public:
        virtual bool IsBinaryAvailable(size_t index)=0;
        virtual bool IsIntegerAvailable(size_t index)=0;
        virtual bool IsDoubleAvailable(size_t index)=0;
        virtual esp_err_t  GetBinaryAsPointer(size_t index, bool *value)=0;
        virtual bool  GetBinary(size_t index)=0;
        virtual esp_err_t  SetBinary(size_t index, bool value)=0;
};

class FunctionBlock {
   public:
      virtual esp_err_t initPhase1(FBContext *ctx){return ESP_OK;};
      virtual esp_err_t initPhase2(FBContext *ctx){return ESP_OK;};
      virtual esp_err_t initPhase3(FBContext *ctx){return ESP_OK;};
      virtual esp_err_t execute(FBContext *ctx)=0;
      virtual esp_err_t deinit(FBContext *ctx){return ESP_OK;}
      virtual ~FunctionBlock(){};
};

class PLCManager:public FBContext
{
 private:
        QueueHandle_t execQueue;
        Executable currentExecutable;
        BSP *bsp;
    public:
        bool IsBinaryAvailable(size_t index);
        bool IsIntegerAvailable(size_t index);
        bool IsDoubleAvailable(size_t index);
        esp_err_t  GetBinaryAsPointer(size_t index, bool *value);
        bool  GetBinary(size_t index);
        esp_err_t  SetBinary(size_t index, bool value);

        esp_err_t CompileProtobufConfig2ExecutableAndEnqueue(char* pb, size_t length);

        
        PLCManager(BSP *bsp): bsp(bsp)
        {
            execQueue=xQueueCreate( 2, sizeof(Executable) );
            
        }
        esp_err_t CheckForNewExecutable();
        esp_err_t Loop();
};