#include "plcmanager.hh"
#include "functionblocks.hh"
#include "esp_log.h"

static const char* TAG = "plcmanager";

#define SetBit(A,k)     ( A[(k/32)] |= (1 << (k%32)) )
#define ClearBit(A,k)   ( A[(k/32)] &= ~(1 << (k%32)) )            
#define TestBit(A,k)    ( A[(k/32)] & (1 << (k%32)) )

bool PLCManager::IsBinaryAvailable(size_t index)
{
    return (index-1024)<this->currentExecutable.binariesSize;
}

bool PLCManager::IsDoubleAvailable(size_t index)
{
    return false;
}

bool PLCManager::IsIntegerAvailable(size_t index)
{
    return false;
}

esp_err_t PLCManager::SetBinary(size_t index, bool value)
{
    if(index<1024)
    {
        return this->bsp->setBinaryOutput(index, value);
    }
    index-=1024;
    if(index>=this->currentExecutable.binariesSize)
        return ESP_FAIL;
    
    if(value)
    {
        SetBit(this->currentExecutable.binaries, index);
    }
    else
    {
        ClearBit(this->currentExecutable.binaries, index);
    }
    return ESP_OK;
}

bool  PLCManager::GetBinary(size_t index)
{
    bool x=false;
    GetBinaryAsPointer(index, &x);
    return x;
}
esp_err_t  PLCManager::GetBinaryAsPointer(size_t index, bool *value)
{
    if(index<1024)
    {
        return this->bsp->getBinaryInput(index, value);
    }
    index-=1024;
    if(index>=this->currentExecutable.binariesSize)
        return ESP_FAIL;
    *value = TestBit(this->currentExecutable.binaries, index);
    
    return ESP_OK;
}
        
esp_err_t PLCManager::CompileProtobufConfig2ExecutableAndEnqueue(char* pb, size_t length)
{
    FB_AND2 *fbAnd = new FB_AND2(1, 3, 1);
    FunctionBlock* functionBlocks = {fbAnd};
    size_t functionBlocksSize=1;
    uint32_t *binaries = new uint32_t[1];
    size_t binariesSize=1;
    int *integers = new int[1];
    size_t integersSize=1;
    Executable e;
    e.functionBlocks = functionBlocks;
    e.functionBlocksSize=functionBlocksSize;
    e.binaries=binaries;
    e.binariesSize=binariesSize;
    e.integers=integers;
    e.integersSize=integersSize;
    ESP_LOGI(TAG, "Created new executable an trying to send to queue");

    if(xQueueSend(this->execQueue, &e, (TickType_t) 10) != pdTRUE)
    {
        ESP_LOGE(TAG, "xQueueSend returned error");
    }
    return ESP_OK;
}

esp_err_t PLCManager::CheckForNewExecutable()
{
    Executable e;
    if(xQueueReceive(this->execQueue, &e, 0)==pdFALSE){
        //no new executable available
        return ESP_OK;
    }
    ESP_LOGI(TAG, "New executable available");       
    //new Executable available --> delete all elements of old Executable
    for (size_t i = 0; i < this->currentExecutable.functionBlocksSize; i++)
    {
        this->currentExecutable.functionBlocks[i].deinit(this);
        delete &(this->currentExecutable.functionBlocks[i]);
    }
    delete this->currentExecutable.functionBlocks;
    delete this->currentExecutable.binaries;
    delete this->currentExecutable.integers;

    this->currentExecutable = e;
    for (size_t i = 0; i < this->currentExecutable.functionBlocksSize; i++)
    {
        this->currentExecutable.functionBlocks[i].initPhase1(this);
    }
    ESP_LOGI(TAG, "New executable Init Phase 1");  
    for (size_t i = 0; i < this->currentExecutable.functionBlocksSize; i++)
    {
        this->currentExecutable.functionBlocks[i].initPhase2(this);
    }
    ESP_LOGI(TAG, "New executable Init Phase 2");  
    for (size_t i = 0; i < this->currentExecutable.functionBlocksSize; i++)
    {
        this->currentExecutable.functionBlocks[i].initPhase2(this);
    }
    ESP_LOGI(TAG, "New executable Init Phase 3");  
    return ESP_OK;
}



esp_err_t PLCManager::Loop()
{
    //Check queue for newly arrived Executable
    //Contract: Alle Eingänge sind gesetzt

    
    for (size_t i = 0; i < this->currentExecutable.functionBlocksSize; i++)
    {
        this->currentExecutable.functionBlocks[i].execute(this);
    }
    return ESP_OK;
}