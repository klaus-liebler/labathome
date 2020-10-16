#include "plcmanager.hh"
#include "functionblocks.hh"
#include "esp_log.h"
#include "labathomeerror.hh"
#include <vector>
#include "PID_v1.h"
#include "paths_and_files.hh"
#include <math.h>

constexpr int64_t TRIGGER_FALLBACK_TIME = 3000000;
constexpr const char *TAG = "plcmanager";

#define SetBit(A, k) (A[(k / 32)] |= (1 << (k % 32)))
#define ClearBit(A, k) (A[(k / 32)] &= ~(1 << (k % 32)))
#define TestBit(A, k) (A[(k / 32)] & (1 << (k % 32)))

PLCManager::PLCManager(HAL *hal):hal(hal)
{
    currentExecutable = this->createInitialExecutable();
    nextExecutable = nullptr;
    pid = new PID(&actualTemperature, &setpointHeaterClosedLoop, &setpointTemperature, KP, KI, KD, DIRECT);
    pid->SetMode(MANUAL);
    pid->SetOutputLimits(0, 100.0);
    pid->SetSampleTime(1000);
}

bool PLCManager::IsBinaryAvailable(size_t index)
{
    return index < this->currentExecutable->binaries.size();
}

bool PLCManager::IsIntegerAvailable(size_t index)
{
    return index < this->currentExecutable->integers.size();
}

bool PLCManager::IsColorAvailable(size_t index)
{
    return index < this->currentExecutable->colors.size();
}

bool PLCManager::IsDoubleAvailable(size_t index)
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

LabAtHomeErrorCode PLCManager::SetInteger(size_t index, int value)
{

    if (index < this->currentExecutable->integers.size())
    {
        this->currentExecutable->integers[index] = value;
        return LabAtHomeErrorCode::OK;
    }
    return LabAtHomeErrorCode::INDEX_OUT_OF_BOUNDS;
}

LabAtHomeErrorCode PLCManager::SetColor(size_t index, uint32_t value)
{
    if (index >= this->currentExecutable->colors.size())
        return LabAtHomeErrorCode::INDEX_OUT_OF_BOUNDS;
    this->currentExecutable->colors[index] = value;
    return LabAtHomeErrorCode::OK;
}

bool PLCManager::GetBinary(size_t index)
{
    bool x = false;
    GetBinaryAsPointer(index, &x);
    return x;
}

int PLCManager::GetInteger(size_t index)
{
    int x = 0;
    GetIntegerAsPointer(index, &x);
    return x;
}

uint32_t PLCManager::GetColor(size_t index)
{
    uint32_t x = 0;
    GetColorAsPointer(index, &x);
    return x;
}

LabAtHomeErrorCode PLCManager::GetBinaryAsPointer(size_t index, bool *value)
{
    if (index >= this->currentExecutable->binaries.size())
        return LabAtHomeErrorCode::INDEX_OUT_OF_BOUNDS;
    *value = this->currentExecutable->binaries[index];
    return LabAtHomeErrorCode::OK;
}

LabAtHomeErrorCode PLCManager::GetIntegerAsPointer(size_t index, int *value)
{
    if (index >= this->currentExecutable->integers.size())
        return LabAtHomeErrorCode::INDEX_OUT_OF_BOUNDS;
    *value = this->currentExecutable->integers[index];
    return LabAtHomeErrorCode::OK;
}

LabAtHomeErrorCode PLCManager::GetColorAsPointer(size_t index, uint32_t *value)
{
    if (index >= this->currentExecutable->colors.size())
        return LabAtHomeErrorCode::INDEX_OUT_OF_BOUNDS;
    *value = this->currentExecutable->colors[index];
    return LabAtHomeErrorCode::OK;
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
    uint32_t ReadU32()
    {
        //if(byteOffset%4 !=0 || byteOffset+4>=maxOffset) return 0;
        uint32_t val = *((uint32_t *)(buf + byteOffset));
        byteOffset += 4;
        return val;
    }

    int32_t ReadS32()
    {
        //if(byteOffset%4 !=0 || byteOffset+4>=maxOffset) return 0;
        int32_t val = *((int32_t *)(buf + byteOffset));
        byteOffset += 4;
        return val;
    }
    LabAtHomeErrorCode ReadU8Array(uint8_t *target, size_t len)
    {
        //if(byteOffset+len>=maxOffset) return LabAtHomeErrorCode::INDEX_OUT_OF_BOUNDS;
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
    ESP_LOGI(TAG, "Starting to parse new Executable of length %d", length);
    const uint32_t dataStructureVersion = ctx->ReadU32();
    if(dataStructureVersion!=0xAFFECAFE) return LabAtHomeErrorCode::INCOMPATIBLE_VERSION;
    
    uint32_t hash= ctx->ReadU32();

    //BOOLEAN=0,
    //INTEGER=1,
    //FLOAT=2,
    //COLOR=3,
    const uint32_t booleansCount = ctx->ReadU32();
    const uint32_t integersCount = ctx->ReadU32();
    const uint32_t floatsCount = ctx->ReadU32();
    const uint32_t colorsCount = ctx->ReadU32();
    const uint32_t operatorsCount = ctx->ReadU32();
    ESP_LOGI(TAG, "booleansCount = %d, integersCount = %d, floatsCount = %d, colorsCount = %d, operatorsCount = %d",booleansCount, integersCount, floatsCount, colorsCount, operatorsCount);
    
    std::vector<FunctionBlock *> functionBlocks(operatorsCount);

    for (size_t cfgIndex = 0; cfgIndex < operatorsCount; cfgIndex++)
    {
        const uint32_t operatorType = ctx->ReadU32();
        switch (operatorType)
        {
        case 1:
        {
            ESP_LOGI(TAG, "Found FB_AND2");
            functionBlocks[cfgIndex] = new FB_AND2(ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 2:
        {
            ESP_LOGI(TAG, "Found FB_OR2");
            functionBlocks[cfgIndex] = new FB_OR2(ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 7:
        {
            ESP_LOGI(TAG, "FOUND FB_RS");
            functionBlocks[cfgIndex] = new FB_RS(ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 8:
        {
            ESP_LOGI(TAG, "FOUND FB_NOT");
            functionBlocks[cfgIndex] = new FB_NOT(ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 9:
        {
            ESP_LOGI(TAG, "FOUND FB_GreenButton");
            functionBlocks[cfgIndex] = new FB_GreenButton(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 10:
        {
            ESP_LOGI(TAG, "FOUND FB_EncoderButton");
            functionBlocks[cfgIndex] = new FB_EncoderButton(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 11:
        {
            ESP_LOGI(TAG, "FOUND FB_RedButton");
            functionBlocks[cfgIndex] = new FB_RedButton(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 12:
        {
            ESP_LOGI(TAG, "FOUND FB_MovementSensor");
            functionBlocks[cfgIndex] = new FB_MovementSensor(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 14:
        {
            ESP_LOGI(TAG, "FOUND FB_Relay");
            functionBlocks[cfgIndex] = new FB_Relay(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 15:
        {
            ESP_LOGI(TAG, "FOUND FB_RedLED");
            functionBlocks[cfgIndex] = new FB_RedLED(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 16:
        {
            ESP_LOGI(TAG, "FOUND FB_YellowLED");
            functionBlocks[cfgIndex] = new FB_YellowLED(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 17:
        {
            ESP_LOGI(TAG, "FOUND FB_GreenLED");
            functionBlocks[cfgIndex] = new FB_GreenLED(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 19:
        {
            ESP_LOGI(TAG, "FOUND FB_ConstInteger");
            functionBlocks[cfgIndex] = new FB_ConstInteger(ctx->ReadU32(), ctx->ReadU32(), ctx->ReadS32());
        }
        break;
        case 20:
        {
            ESP_LOGI(TAG, "FOUND FB_TON");
            functionBlocks[cfgIndex] = new FB_TON(ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 21:
        {
            ESP_LOGI(TAG, "FOUND FB_TOF");
            functionBlocks[cfgIndex] = new FB_TOF(ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 22:
        {
            ESP_LOGI(TAG, "FOUND FB_GT");
            functionBlocks[cfgIndex] = new FB_GT(ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 23:
        {
            ESP_LOGI(TAG, "FOUND FB_LT");
            functionBlocks[cfgIndex] = new FB_LT(ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 24:
        {
            ESP_LOGI(TAG, "FOUND FB_AmbientBrigthnessSensor");
            functionBlocks[cfgIndex] = new FB_AmbientBrigthnessSensor(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 25:
        {
            ESP_LOGI(TAG, "FOUND FB_HeaterTemperatureSensor");
            functionBlocks[cfgIndex] = new FB_HeaterTemperatureSensor(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 26:
        {
            ESP_LOGI(TAG, "FOUND FB_Bool2ColorConverter");
            functionBlocks[cfgIndex] = new FB_Bool2ColorConverter(ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 27:
        {
            ESP_LOGI(TAG, "FOUND FB_LED3");
            functionBlocks[cfgIndex] = new FB_LED3(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 28:
        {
            ESP_LOGI(TAG, "FOUND FB_LED4");
            functionBlocks[cfgIndex] = new FB_LED4(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 29:
        {
            ESP_LOGI(TAG, "FOUND FB_LED5");
            functionBlocks[cfgIndex] = new FB_LED5(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 30:
        {
            ESP_LOGI(TAG, "FOUND FB_LED6");
            functionBlocks[cfgIndex] = new FB_LED6(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 31:
        {
            ESP_LOGI(TAG, "FOUND FB_LED7");
            functionBlocks[cfgIndex] = new FB_LED7(ctx->ReadU32(), ctx->ReadU32());
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

    size_t debugSizeBytes = 4 /*Hashcode!*/ +4*(booleansCount+1+integersCount+1+floatsCount+1+colorsCount+1);//jeweils noch ein size_t für die Länge
    
    this->nextExecutable = new Executable(hash, debugSizeBytes, functionBlocks, binaries, integers, floats, colors);
    ESP_LOGI(TAG, "Created new executable and enqueued it");
    return LabAtHomeErrorCode::OK;
}

Executable *PLCManager::createInitialExecutable()
{
    FB_RedButton *button_red = new FB_RedButton(0,2);
    FB_RedLED *led_red = new FB_RedLED(1,2);
    const uint32_t booleansCount = 3;
    const uint32_t integersCount = 2;
    const uint32_t floatsCount = 2;
    const uint32_t colorsCount = 2;
    
    std::vector<FunctionBlock *> functionBlocks(2);
    std::vector<bool> binaries(booleansCount);
    std::vector<int> integers(integersCount);
    std::vector<double> floats(floatsCount);
    std::vector<uint32_t> colors(colorsCount);
    functionBlocks[0] = button_red;
    functionBlocks[1] = led_red;
    uint32_t hash = 0;
   
    size_t debugSizeBytes = 4 /*Hashcode!*/ +4*(booleansCount+1+integersCount+1+floatsCount+1+colorsCount+1);//jeweils noch ein size_t für die Länge
    Executable *e = new Executable(hash, debugSizeBytes, functionBlocks, binaries, integers, floats, colors);
    return e;
}

LabAtHomeErrorCode PLCManager::GetDebugInfoSize(size_t *sizeInBytes){
    *sizeInBytes=this->currentExecutable->debugSizeBytes;
    return LabAtHomeErrorCode::OK;
}


LabAtHomeErrorCode PLCManager::GetDebugInfo(uint8_t *buffer, size_t maxSizeInByte){
    int *bufAsINT32 = (int*)buffer;
    float *bufAsFLOAT = (float*)buffer;
    uint32_t *bufAsUINT32 = (uint32_t*)buffer;
    size_t offset32 = 0;
    if(maxSizeInByte<this->currentExecutable->debugSizeBytes) return LabAtHomeErrorCode::INDEX_OUT_OF_BOUNDS;
    
    //First, the Hash
    bufAsUINT32[offset32]=this->currentExecutable->hash;
    offset32++;
    
    //Then bools
    bufAsUINT32[offset32]=this->currentExecutable->binaries.size();
    offset32++;
    for (size_t vecPos = 0; vecPos < this->currentExecutable->binaries.size(); vecPos++) {
        if (this->currentExecutable->binaries[vecPos]) {
            bufAsUINT32[vecPos+offset32] = 1;
        }
        else
        {
            bufAsUINT32[vecPos+offset32] = 0;
        }
    }
    offset32+=this->currentExecutable->binaries.size();

    //then S32
    bufAsUINT32[offset32]=this->currentExecutable->integers.size();
    offset32++;
    for (size_t vecPos = 0; vecPos < this->currentExecutable->integers.size(); vecPos++) {
        bufAsINT32[vecPos+offset32] = this->currentExecutable->binaries[vecPos];
    }
    offset32+=this->currentExecutable->integers.size();

    //then f32
    bufAsUINT32[offset32]=this->currentExecutable->floats.size();
    offset32++;
    for (size_t vecPos = 0; vecPos < this->currentExecutable->floats.size(); vecPos++) {
        bufAsFLOAT[vecPos+offset32] = this->currentExecutable->floats[vecPos];
    }
    offset32+=this->currentExecutable->floats.size();
    
    //then colors
    bufAsUINT32[offset32]=this->currentExecutable->colors.size();
    offset32++;
    for (size_t vecPos = 0; vecPos < this->currentExecutable->colors.size(); vecPos++) {
        bufAsUINT32[vecPos+offset32] = this->currentExecutable->colors[vecPos];
    }
    offset32+=this->currentExecutable->colors.size();
    
    return LabAtHomeErrorCode::OK;
}

LabAtHomeErrorCode PLCManager::Init()
{
    FILE *fd = NULL;
    struct stat file_stat;
    ESP_LOGI(TAG, "Trying to open %s", Paths::DEFAULT_FBD_BIN_FILENAME);
    if (stat(Paths::DEFAULT_FBD_BIN_FILENAME, &file_stat) == -1) {
        ESP_LOGI(TAG, "Default PLC file %s does not exist. Using factory default instead", Paths::DEFAULT_FBD_BIN_FILENAME);
        return LabAtHomeErrorCode::OK;
    }
    fd = fopen(Paths::DEFAULT_FBD_BIN_FILENAME, "r");
    if (!fd) {
        ESP_LOGE(TAG, "Failed to read existing file : %s", Paths::DEFAULT_FBD_BIN_FILENAME);
        return LabAtHomeErrorCode::FILE_SYSTEM_ERROR;
    }
    uint8_t buf[file_stat.st_size];
    size_t size_read = fread(buf, 1, file_stat.st_size, fd);
    if(size_read!=file_stat.st_size){
        ESP_LOGE(TAG, "Unable to read file completely : %s", Paths::DEFAULT_FBD_BIN_FILENAME);
        return LabAtHomeErrorCode::FILE_SYSTEM_ERROR;
    }
    ESP_LOGI(TAG, "Successfully read %s", Paths::DEFAULT_FBD_BIN_FILENAME);
    return this->ParseNewExecutableAndEnqueue(buf, size_read);
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
    int64_t nowUsSteady = hal->GetMicros();
    if(experimentMode != ExperimentMode::functionblock && nowUsSteady-this->lastExperimentTrigger>TRIGGER_FALLBACK_TIME)
    {
        //auto fallback
        experimentMode = ExperimentMode::functionblock;
        ESP_LOGI(TAG, "Auto fallback to experimentMode = ExperimentMode::functionblock;");
        hal->SetFan1State(0);
        hal->SetFan2State(0);
        hal->SetHeaterState(0);
    }

    if(experimentMode == ExperimentMode::functionblock)
    {
        for (const auto &i : this->currentExecutable->functionBlocks)
        {
            i->execute(this);
        }
    }
    else if(experimentMode==ExperimentMode::openloop){
        if(pid->GetMode()!=MANUAL)
        {
            pid->SetMode(MANUAL);
        }
        hal->SetFan1State(this->setpointFan);
        hal->SetFan2State(this->setpointFan);
        hal->SetHeaterState(this->setpointHeaterOpenloop);
    }
    else if(experimentMode==ExperimentMode::closedloop){
        if(pid->GetMode()!=AUTOMATIC)
        {
            ESP_LOGI(TAG, "pid->SetMode(AUTOMATIC);");
            pid->SetMode(AUTOMATIC);
        }
        if(pid->GetKd()!=this->KD || pid->GetKi()!=this->KI || pid->GetKp()!=this->KP)
        {
            ESP_LOGI(TAG, "pid->SetTunings(KP, KI, KD); %f %f %f", KP, KI, KD);
            pid->SetTunings(KP, KI, KD);
        }
        float act =0;
        hal->GetHeaterTemperature(&act);
        this->actualTemperature=act;

        bool newResult = pid->Compute();
        if(newResult)
        {
            ESP_LOGI(TAG, "if(newResult): %F", this->setpointHeaterClosedLoop);
            hal->SetHeaterState(this->setpointHeaterClosedLoop);
        }
        hal->SetFan1State(this->setpointFan);
        hal->SetFan2State(this->setpointFan);
    }
    
    return LabAtHomeErrorCode::OK;
}

LabAtHomeErrorCode PLCManager::TriggerHeaterExperimentClosedLoop(double setpointTemperature, double setpointFan, double KP, double KI, double KD, ExperimentData *data){
    this->lastExperimentTrigger=hal->GetMicros();
    this->experimentMode=ExperimentMode::closedloop;
    this->setpointTemperature=setpointTemperature;
    this->setpointFan=setpointFan;
    this->KP=KP;
    this->KI=KI;
    this->KD=KD;
    data->Fan=hal->GetFan1State();
    data->Heater=hal->GetHeaterState();
    hal->GetHeaterTemperature(&(data->ActualTemperature));
    data->SetpointTemperature=this->setpointTemperature;
    return LabAtHomeErrorCode::OK;
}
LabAtHomeErrorCode PLCManager::TriggerHeaterExperimentOpenLoop(double setpointHeater, double setpointFan, ExperimentData *data){
    this->lastExperimentTrigger=hal->GetMicros();
    this->experimentMode=ExperimentMode::openloop;
    this->setpointHeaterOpenloop=setpointHeater;
    this->setpointFan=setpointFan;
    data->Fan=hal->GetFan1State();
    data->Heater=hal->GetHeaterState();
    hal->GetHeaterTemperature(&(data->ActualTemperature));
    data->SetpointTemperature=0;
    return LabAtHomeErrorCode::OK;
}
LabAtHomeErrorCode PLCManager::TriggerHeaterExperimentFunctionblock(ExperimentData *data){
    this->lastExperimentTrigger=hal->GetMicros();
    this->experimentMode=ExperimentMode::functionblock;
    this->setpointHeaterOpenloop=0;
    this->setpointFan=0;
    data->Fan=hal->GetFan1State();
    data->Heater=hal->GetHeaterState();
    hal->GetHeaterTemperature(&(data->ActualTemperature));
    data->SetpointTemperature=0;
    return LabAtHomeErrorCode::OK;
}