#include "plcmanager.hh"
#include "functionblocks.hh"
#include "esp_log.h"
#include "labathomeerror.hh"
#include "fbexecutable_generated.h"

static const char* TAG = "plcmanager";

extern const uint8_t defaultfunctionblockdiagram_start[] asm("_binary_defaultfunctionblockdiagram_data_start");
extern const uint8_t defaultfunctionblockdiagram_end[]   asm("_binary_defaultfunctionblockdiagram_data_end");

#define SetBit(A,k)     ( A[(k/32)] |= (1 << (k%32)) )
#define ClearBit(A,k)   ( A[(k/32)] &= ~(1 << (k%32)) )            
#define TestBit(A,k)    ( A[(k/32)] & (1 << (k%32)) )

bool PLCManager::IsBinaryAvailable(size_t index)
{
    return (index-1024)<this->currentExecutable->binariesSize;
}

bool PLCManager::IsDoubleAvailable(size_t index)
{
    return false;
}

bool PLCManager::IsIntegerAvailable(size_t index)
{
    return false;
}

LabAtHomeErrorCode PLCManager::SetBinary(size_t index, bool value)
{
    if(index<1024)
    {
        return this->bsp->setBinaryOutput(index, value);
    }
    index-=1024;
    if(index>=this->currentExecutable->binariesSize)
        return LabAtHomeErrorCode::INDEX_OUT_OF_BOUNDS;
    
    if(value)
    {
        SetBit(this->currentExecutable->binaries, index);
    }
    else
    {
        ClearBit(this->currentExecutable->binaries, index);
    }
    return LabAtHomeErrorCode::OK;
}

bool  PLCManager::GetBinary(size_t index)
{
    bool x=false;
    GetBinaryAsPointer(index, &x);
    return x;
}
LabAtHomeErrorCode  PLCManager::GetBinaryAsPointer(size_t index, bool *value)
{
    if(index<1024)
    {
        return this->bsp->getBinaryInput(index, value);
    }
    index-=1024;
    if(index>=this->currentExecutable->binariesSize)
        return LabAtHomeErrorCode::INDEX_OUT_OF_BOUNDS;
    *value = TestBit(this->currentExecutable->binaries, index);
    
    return LabAtHomeErrorCode::OK;
}
        
LabAtHomeErrorCode PLCManager::CompileProtobufConfig2ExecutableAndEnqueue(char* pb, size_t length)
{
    auto fbExec = labathome::GetFbExecutable(defaultfunctionblockdiagram_start);
    ESP_LOGI(TAG, "ID of STD is %llu", fbExec->id());

    auto configs = fbExec->fbConfig();
    size_t configs_len = configs->size();
    ESP_LOGI(TAG, "Size of Configs is %d", configs_len);
    for(size_t cfgIndex=0;cfgIndex<configs->size();cfgIndex++)
    {
        auto cfg_wrapper = configs->Get(cfgIndex);
        const labathome::FbAnd2Configuration *cfg=nullptr;
        uint16_t inputA=0;
        switch (cfg_wrapper->item_type())
        {
        case labathome::FbConfiguration_FbAnd2Configuration:
            ESP_LOGI(TAG, "FOUND FbConfiguration_FbAnd2Configuration");
            cfg = cfg_wrapper->item_as_FbAnd2Configuration();
            inputA = cfg->inputA();
            ESP_LOGI(TAG, "inputA of And2 is %d",inputA);
            break;
        case labathome::FbConfiguration_FbNotConfiguration:
            ESP_LOGI(TAG, "FOUND FbConfiguration_FbNotConfiguration");
            break;
        case labathome::FbConfiguration_FbRSConfiguration:
            ESP_LOGI(TAG, "FOUND FbConfiguration_FbRSConfiguration");
            break;
        default:
            ESP_LOGE(TAG, "No known configuration found");
            break;
        }
    }
 
    FB_AND2 *fbAnd = new FB_AND2(1, 3, 1);
    FunctionBlock** functionBlocks = new FunctionBlock*[1];
    functionBlocks[0]=fbAnd;
    size_t functionBlocksSize=1;
    uint32_t *binaries = new uint32_t[1];
    size_t binariesSize=1;
    int *integers = new int[1];
    size_t integersSize=1;
    Executable *e = new Executable(functionBlocks, functionBlocksSize, binaries, binariesSize, integers, integersSize);
    ESP_LOGI(TAG, "Created new executable an trying to send to queue");

    if(xQueueSend(this->execQueue, &e, (TickType_t) 10) != pdTRUE)
    {
        ESP_LOGE(TAG, "xQueueSend returned error");
        return LabAtHomeErrorCode::QUEUE_OVERLOAD;
    }
    return LabAtHomeErrorCode::OK;
}

Executable* PLCManager::createInitialExecutable()
{
    FB_RS *fb = new FB_RS(1, 3, 1);
    FunctionBlock** functionBlocks = new FunctionBlock*[1];
    functionBlocks[0]=fb;
    size_t functionBlocksSize=1;
    uint32_t *binaries = new uint32_t[1];
    size_t binariesSize=1;
    int *integers = new int[1];
    size_t integersSize=1;
    Executable *e = new Executable(functionBlocks, functionBlocksSize, binaries, binariesSize, integers, integersSize);
    return e;
}

LabAtHomeErrorCode PLCManager::CheckForNewExecutable()
{
    Executable *e;
    if(xQueueReceive(this->execQueue, &e, 0)==pdFALSE){
        //no new executable available
        return LabAtHomeErrorCode::OK;
    }
    ESP_LOGI(TAG, "New executable available");       
    //new Executable available --> delete all elements of old Executable
    for (size_t i = 0; i < this->currentExecutable->functionBlocksSize; i++)
    {
        this->currentExecutable->functionBlocks[i]->deinit(this);
    }
    delete[] this->currentExecutable->functionBlocks;
    delete[] this->currentExecutable->binaries;
    delete[] this->currentExecutable->integers;
    delete this->currentExecutable;
    ESP_LOGI(TAG, "Removed old executable completely --> CHECK THIS AGAIN!!");
    this->currentExecutable = e;
    for (size_t i = 0; i < this->currentExecutable->functionBlocksSize; i++)
    {
        this->currentExecutable->functionBlocks[i]->initPhase1(this);
    }
    ESP_LOGI(TAG, "New executable Init Phase 1");  
    for (size_t i = 0; i < this->currentExecutable->functionBlocksSize; i++)
    {
        this->currentExecutable->functionBlocks[i]->initPhase2(this);
    }
    ESP_LOGI(TAG, "New executable Init Phase 2");  
    for (size_t i = 0; i < this->currentExecutable->functionBlocksSize; i++)
    {
        this->currentExecutable->functionBlocks[i]->initPhase2(this);
    }
    ESP_LOGI(TAG, "New executable Init Phase 3");  
    return LabAtHomeErrorCode::OK;
}



LabAtHomeErrorCode PLCManager::Loop()
{
    //Check queue for newly arrived Executable
    //Contract: Alle Eingänge sind gesetzt

    
    for (size_t i = 0; i < this->currentExecutable->functionBlocksSize; i++)
    {
        this->currentExecutable->functionBlocks[i]->execute(this);
    }
    return LabAtHomeErrorCode::OK;
}