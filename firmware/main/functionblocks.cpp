#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_err.h"

#include "functionblocks.hpp"
#include "fbexecutable.pb.h"



bool FBContext::IsBinaryAvailable(size_t index)
{
    if(this->currentExecutable==0) return false;
    return index<this->currentExecutable->binariesSize;
}

bool FBContext::IsDoubleAvailable(size_t index)
{
    return false;
}

bool FBContext::IsIntegerAvailable(size_t index)
{
    return false;
}

esp_err_t FBContext::CompileProtobufConfig2ExecutableAndEnqueue(char* pb, size_t length)
{
    SearchRequest sr;
    sr.ParseFromArray(pb, length);
    return ESP_OK;
}

esp_err_t FBContext::Init()
{
    for (size_t i = 0; i < this->currentExecutable->functionBlocksSize; i++)
    {
        this->currentExecutable->functionBlocks[i].initPhase1(this);
    }
    for (size_t i = 0; i < this->currentExecutable->functionBlocksSize; i++)
    {
        this->currentExecutable->functionBlocks[i].initPhase2(this);
    }
    for (size_t i = 0; i < this->currentExecutable->functionBlocksSize; i++)
    {
        this->currentExecutable->functionBlocks[i].initPhase2(this);
    }
    return ESP_OK;
}

esp_err_t FBContext::Loop()
{
    //Contract: Alle Eingänge sind gesetzt

    
    for (size_t i = 0; i < this->currentExecutable->functionBlocksSize; i++)
    {
        this->currentExecutable->functionBlocks[i].execute(this);
    }
    return ESP_OK;
}