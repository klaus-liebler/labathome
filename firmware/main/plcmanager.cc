#include "plcmanager.hh"
#include "functionblocks.hh"
#include "esp_log.h"
#include "labathomeerror.hh"
#include "fbexecutable_generated.h"
#include <vector>
#include "input_output_id.hh"

static const char* TAG = "plcmanager";

extern const uint8_t defaultfunctionblockdiagram_start[] asm("_binary_defaultfunctionblockdiagram_data_start");
extern const uint8_t defaultfunctionblockdiagram_end[]   asm("_binary_defaultfunctionblockdiagram_data_end");

#define SetBit(A,k)     ( A[(k/32)] |= (1 << (k%32)) )
#define ClearBit(A,k)   ( A[(k/32)] &= ~(1 << (k%32)) )            
#define TestBit(A,k)    ( A[(k/32)] & (1 << (k%32)) )

bool PLCManager::IsBinaryAvailable(size_t index)
{
    if(index>=BINARY_INT_MIN && index < BINARY_INT_MAX)
    {
        index-=BINARY_INT_MIN;
        return index<this->currentExecutable->binaries.size();
    }
    else if(index >= BINARY_HW_MAX && index < BINARY_HW_MAX)
    {
        return this->bsp->IsBinaryAvailable(index);
    }
    else
    {
        return false;
    }
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
    if(index>=BINARY_INT_MIN && index < BINARY_INT_MAX)
    {
        index-=BINARY_INT_MIN;
        if(index<this->currentExecutable->binaries.size())
        {
            this->currentExecutable->binaries[index]=value;
            return LabAtHomeErrorCode::OK;
        }
    }
    else if(index >= BINARY_HW_MAX && index < BINARY_HW_MAX)
    {
        return this->bsp->setBinaryOutput(index, value);
    }
    return LabAtHomeErrorCode::INDEX_OUT_OF_BOUNDS;
}

bool  PLCManager::GetBinary(size_t index)
{
    bool x=false;
    GetBinaryAsPointer(index, &x);
    return x;
}
LabAtHomeErrorCode  PLCManager::GetBinaryAsPointer(size_t index, bool *value)
{
    if(index>=BINARY_INT_MIN && index < BINARY_INT_MAX)
    {
        index-=BINARY_INT_MIN;
        if(index<this->currentExecutable->binaries.size())
        {
            *value = this->currentExecutable->binaries[index];
            return LabAtHomeErrorCode::OK;
        }
    }
    else if(index >= BINARY_HW_MAX && index < BINARY_HW_MAX)
    {
        return this->bsp->getBinaryInput(index, value);
    }
    return LabAtHomeErrorCode::INDEX_OUT_OF_BOUNDS;
}

int64_t PLCManager::GetMicroseconds()
{
    return bsp->GetMicroseconds();
}

LabAtHomeErrorCode PLCManager::CompileProtobufConfig2ExecutableAndEnqueue(char* pb, size_t length)
{
    auto fbExec = labathome::GetFbExecutable(defaultfunctionblockdiagram_start);
    ESP_LOGI(TAG, "ID of STD is %llu", fbExec->id());

    auto configs = fbExec->fbConfig();
    size_t configs_len = configs->size();
    std::vector<FunctionBlock*> functionBlocks(configs_len);
    for(size_t cfgIndex=0;cfgIndex<configs_len;cfgIndex++)
    {
        auto cfg_wrapper = configs->Get(cfgIndex);
    
        switch (cfg_wrapper->item_type())
        {
        case labathome::FbConfiguration_FbAnd2Configuration:
            {
                ESP_LOGI(TAG, "FOUND FbConfiguration_FbAnd2Configuration");
                auto *cfg = cfg_wrapper->item_as_FbAnd2Configuration();
                functionBlocks[cfgIndex]=new FB_AND2(cfg->inputA(), cfg->inputB(), cfg->output());
            }
            break;
        case labathome::FbConfiguration_FbNotConfiguration:
            {
                ESP_LOGI(TAG, "FOUND FbConfiguration_FbNotConfiguration");
                auto *cfg = cfg_wrapper->item_as_FbNotConfiguration();
                functionBlocks[cfgIndex]=new FB_NOT(cfg->input(), cfg->output());
            }
            break;
        case labathome::FbConfiguration_FbRSConfiguration:
            {
                ESP_LOGI(TAG, "FOUND FbConfiguration_FbRSConfiguration");
                auto *cfg = cfg_wrapper->item_as_FbRSConfiguration();
                functionBlocks[cfgIndex]=new FB_RS(cfg->inputR(), cfg->inputS(), cfg->output());
            }
            break;
        case labathome::FbConfiguration_FbTonConfiguration:
            {
                ESP_LOGI(TAG, "FOUND FbConfiguration_FbTonConfiguration");
                auto *cfg = cfg_wrapper->item_as_FbTonConfiguration();
                functionBlocks[cfgIndex]=new FB_TON(cfg->input(), cfg->output(), cfg->presetTime_secs());
            }
            break;
        default:
            ESP_LOGE(TAG, "No known configuration found");
            break;
        }
    }
    if(this->nextExecutable!=nullptr)
    {
        return LabAtHomeErrorCode::QUEUE_OVERLOAD;
    }


    std::vector<bool> binaries(fbExec->maxBinaryIndex()-(BINARY_INT_MIN-1));
    std::vector<int> integers(100); //TODO
    this->nextExecutable = new Executable(functionBlocks, binaries, integers);
    ESP_LOGI(TAG, "Created new executable and enqueued it");
    return LabAtHomeErrorCode::OK;
}

Executable* PLCManager::createInitialExecutable()
{
    FB_RS *fb = new FB_RS(1, 3, 1);
    std::vector<FunctionBlock*> functionBlocks(1);
    std::vector<bool> binaries(1);
    std::vector<int> integers(1);
    functionBlocks[0]=fb;
    Executable *e = new Executable(functionBlocks, binaries, integers);
    return e;
}

LabAtHomeErrorCode PLCManager::CheckForNewExecutable()
{
    if(this->nextExecutable==nullptr){
        //no new executable available
        return LabAtHomeErrorCode::OK;
    }
    ESP_LOGI(TAG, "New executable available");       
    //new Executable available --> delete all elements of old Executable
    for (const auto& i:this->currentExecutable->functionBlocks)
    {
        i->deinit(this);
    }
    delete this->currentExecutable; //Destructor takes care for all internal data structures
    ESP_LOGI(TAG, "Removed old executable completely --> CHECK THIS AGAIN!!");
    this->currentExecutable = this->nextExecutable;
    this->nextExecutable=nullptr;
    for (const auto& i:this->currentExecutable->functionBlocks)
    {
        i->initPhase1(this);
    }
    ESP_LOGI(TAG, "New executable Init Phase 1");  
    for (const auto& i:this->currentExecutable->functionBlocks)
    {
        i->initPhase2(this);
    }
    ESP_LOGI(TAG, "New executable Init Phase 2");  
    for (const auto& i:this->currentExecutable->functionBlocks)
    {
        i->initPhase3(this);
    }
    ESP_LOGI(TAG, "New executable Init Phase 3");  
    return LabAtHomeErrorCode::OK;
}



LabAtHomeErrorCode PLCManager::Loop()
{
    //Check queue for newly arrived Executable
    //Contract: Alle Eingänge sind gesetzt

    
    for (const auto& i:this->currentExecutable->functionBlocks)
    {
        i->execute(this);
    }
    return LabAtHomeErrorCode::OK;
}