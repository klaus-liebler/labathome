#include "plcmanager.hh"
#include "functionblocks.hh"
#include "esp_log.h"
#include "labathomeerror.hh"
#include <vector>

static const char *TAG = "plcmanager";

#define SetBit(A, k) (A[(k / 32)] |= (1 << (k % 32)))
#define ClearBit(A, k) (A[(k / 32)] &= ~(1 << (k % 32)))
#define TestBit(A, k) (A[(k / 32)] & (1 << (k % 32)))

bool PLCManager::IsBinaryAvailable(size_t index)
{
    return index < this->currentExecutable->binaries.size();
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

    if (index < this->currentExecutable->binaries.size())
    {
        this->currentExecutable->binaries[index] = value;
        return LabAtHomeErrorCode::OK;
    }
    return LabAtHomeErrorCode::INDEX_OUT_OF_BOUNDS;
}

bool PLCManager::GetBinary(size_t index)
{
    bool x = false;
    GetBinaryAsPointer(index, &x);
    return x;
}

LabAtHomeErrorCode PLCManager::GetBinaryAsPointer(size_t index, bool *value)
{

    if (index < this->currentExecutable->binaries.size())
    {
        *value = this->currentExecutable->binaries[index];
        return LabAtHomeErrorCode::OK;
    }
    return LabAtHomeErrorCode::INDEX_OUT_OF_BOUNDS;
}

int64_t PLCManager::GetMicroseconds()
{
    return hal->GetMicros();
}

HAL *PLCManager::GetHAL()
{
    return this->hal;
}

class ParseContext
{
public:
    const uint8_t *buf;
    size_t maxOffset;
    size_t byteOffset;
    uint32_t ReadUint32()
    {
        uint32_t val = *((uint32_t *)(buf + byteOffset));
        byteOffset += 4;
        return val;
    }
    LabAtHomeErrorCode ReadUint8Array(uint8_t *target, size_t len)
    {
        for (size_t i = 0; i < len; i++)
        {
            target[i] = buf[byteOffset + i];
        }
        byteOffset += len;
        return LabAtHomeErrorCode::OK;
    }
};

LabAtHomeErrorCode PLCManager::ParseNewExecutableAndEnqueue(const uint8_t  *buffer, size_t length)
{
    ParseContext *ctx = new ParseContext();
    ctx->buf = buffer;
    ctx->byteOffset = 0;
    ctx->maxOffset=length;

    const uint32_t dataStructureVersion = ctx->ReadUint32();

    uint8_t uuid[16];
    ctx->ReadUint8Array(uuid, 16);

    //BOOLEAN=0,
    //INTEGER=1,
    //FLOAT=2,
    //COLOR=3,
    const uint32_t booleansCount = ctx->ReadUint32();
    const uint32_t integersCount = ctx->ReadUint32();
    const uint32_t floatsCount = ctx->ReadUint32();
    const uint32_t colorsCount = ctx->ReadUint32();

    const uint32_t operatorsCount = ctx->ReadUint32();
    std::vector<FunctionBlock *> functionBlocks(operatorsCount);

    for (size_t cfgIndex = 0; cfgIndex < operatorsCount; cfgIndex++)
    {
        const uint32_t operatorType = ctx->ReadUint32();
        switch (operatorType)
        {
        case 1:
        {
            ESP_LOGI(TAG, "Found FB_AND2");
            functionBlocks[cfgIndex] = new FB_AND2(ctx->ReadUint32(), ctx->ReadUint32(), ctx->ReadUint32(), ctx->ReadUint32());
        }
        break;
        case 2:
        {
            ESP_LOGI(TAG, "Found FB_OR2");
            functionBlocks[cfgIndex] = new FB_OR2(ctx->ReadUint32(), ctx->ReadUint32(), ctx->ReadUint32(), ctx->ReadUint32());
        }
        break;
        case 7:
        {
            ESP_LOGI(TAG, "FOUND FB_RS");
            functionBlocks[cfgIndex] = new FB_RS(ctx->ReadUint32(), ctx->ReadUint32(), ctx->ReadUint32(), ctx->ReadUint32());
        }
        break;
        case 8:
        {
            ESP_LOGI(TAG, "FOUND FB_NOT");
            functionBlocks[cfgIndex] = new FB_NOT(ctx->ReadUint32(), ctx->ReadUint32(), ctx->ReadUint32());
        }
        break;
        case 9:
        {
            ESP_LOGI(TAG, "FOUND FB_GreenButton");
            functionBlocks[cfgIndex] = new FB_GreenButton(ctx->ReadUint32(), ctx->ReadUint32());
        }
        break;
        case 10:
        {
            ESP_LOGI(TAG, "FOUND FB_EncoderButton");
            functionBlocks[cfgIndex] = new FB_EncoderButton(ctx->ReadUint32(), ctx->ReadUint32());
        }
        break;
        case 11:
        {
            ESP_LOGI(TAG, "FOUND FB_RedButton");
            functionBlocks[cfgIndex] = new FB_RedButton(ctx->ReadUint32(), ctx->ReadUint32());
        }
        break;
        case 15:
        {
            ESP_LOGI(TAG, "FOUND FB_RedLED");
            functionBlocks[cfgIndex] = new FB_RedLED(ctx->ReadUint32(), ctx->ReadUint32());
        }
        break;
        case 16:
        {
            ESP_LOGI(TAG, "FOUND FB_YellowLED");
            functionBlocks[cfgIndex] = new FB_YellowLED(ctx->ReadUint32(), ctx->ReadUint32());
        }
        break;
        case 17:
        {
            ESP_LOGI(TAG, "FOUND FB_GreenLED");
            functionBlocks[cfgIndex] = new FB_GreenLED(ctx->ReadUint32(), ctx->ReadUint32());
        }
        break;
        default:
            ESP_LOGE(TAG, "Unknown Operator Type found");
            return LabAtHomeErrorCode::INVALID_NEW_FBD;
        }
    }

    delete ctx;

    if (this->nextExecutable != nullptr)
    {
        return LabAtHomeErrorCode::QUEUE_OVERLOAD;
    }

    std::vector<bool> binaries(booleansCount);
    std::vector<int> integers(integersCount);
    std::vector<double> floats(floatsCount);
    std::vector<uint32_t> colors(colorsCount);
    
    this->nextExecutable = new Executable(functionBlocks, binaries, integers, floats, colors);
    ESP_LOGI(TAG, "Created new executable and enqueued it");
    return LabAtHomeErrorCode::OK;
}

Executable *PLCManager::createInitialExecutable()
{
    FB_RedButton *button_red = new FB_RedButton(0,2);
    FB_RedLED *led_red = new FB_RedLED(1,2);
    
    std::vector<FunctionBlock *> functionBlocks(2);
    std::vector<bool> binaries(3);
    std::vector<int> integers(1);
    std::vector<double> floats(1);
    std::vector<uint32_t> colors(1);
    functionBlocks[0] = button_red;
    functionBlocks[1] = led_red;
    Executable *e = new Executable(functionBlocks, binaries, integers, floats, colors);
    return e;
}

LabAtHomeErrorCode PLCManager::CheckForNewExecutable()
{
    if (this->nextExecutable == nullptr)
    {
        //no new executable available
        return LabAtHomeErrorCode::OK;
    }
    ESP_LOGI(TAG, "New executable available");
    //new Executable available --> delete all elements of old Executable
    for (const auto &i : this->currentExecutable->functionBlocks)
    {
        i->deinit(this);
    }
    delete this->currentExecutable; //Destructor takes care for all internal data structures
    ESP_LOGI(TAG, "Removed old executable completely --> CHECK THIS AGAIN!!");
    this->currentExecutable = this->nextExecutable;
    this->nextExecutable = nullptr;
    for (const auto &i : this->currentExecutable->functionBlocks)
    {
        i->initPhase1(this);
    }
    ESP_LOGI(TAG, "New executable Init Phase 1");
    for (const auto &i : this->currentExecutable->functionBlocks)
    {
        i->initPhase2(this);
    }
    ESP_LOGI(TAG, "New executable Init Phase 2");
    for (const auto &i : this->currentExecutable->functionBlocks)
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

    for (const auto &i : this->currentExecutable->functionBlocks)
    {
        i->execute(this);
    }
    return LabAtHomeErrorCode::OK;
}