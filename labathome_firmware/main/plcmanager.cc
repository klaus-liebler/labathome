#include "plcmanager.hh"
#include "functionblocks.hh"
#include "esp_log.h"
#include "errorcodes.hh"
#include <vector>
#include "pidcontroller.hh"
#include "common_in_project.hh"
#include <math.h>

constexpr uint32_t TRIGGER_FALLBACK_TIME_MS = 3000;
constexpr const char *TAG = "plcmanager";

#define SetBit(A, k) (A[(k / 32)] |= (1 << (k % 32)))
#define ClearBit(A, k) (A[(k / 32)] &= ~(1 << (k % 32)))
#define TestBit(A, k) (A[(k / 32)] & (1 << (k % 32)))

PLCManager::PLCManager(HAL *hal):hal(hal)
{
    currentExecutable = this->createInitialExecutable();
    nextExecutable = nullptr;
    heaterPIDController = new PIDController(&actualTemperature, &setpointHeater, &setpointTemperature, 0, 100, Mode::OFF, Direction::DIRECT, 1000);
    airspeedPIDController = new PIDController(&actualAirspeed, &setpointFan2, &setpointFan2, 0, 100, Mode::OFF, Direction::DIRECT, 1000);
}

ErrorCode PLCManager::InitAndRun(){
    xTaskCreate(PLCManager::plcTask, "plcTask", 4096 * 4, this, 6, NULL);
    return ErrorCode::OK;
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

ErrorCode PLCManager::SetBinary(size_t index, bool value)
{
    if (index < this->currentExecutable->binaries.size())
    {
        this->currentExecutable->binaries[index] = value;
        return ErrorCode::OK;
    }
    return ErrorCode::INDEX_OUT_OF_BOUNDS;
}

ErrorCode PLCManager::SetInteger(size_t index, int value)
{

    if (index < this->currentExecutable->integers.size())
    {
        this->currentExecutable->integers[index] = value;
        return ErrorCode::OK;
    }
    return ErrorCode::INDEX_OUT_OF_BOUNDS;
}

ErrorCode PLCManager::SetColor(size_t index, uint32_t value)
{
    if (index >= this->currentExecutable->colors.size())
        return ErrorCode::INDEX_OUT_OF_BOUNDS;
    this->currentExecutable->colors[index] = value;
    return ErrorCode::OK;
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

ErrorCode PLCManager::GetBinaryAsPointer(size_t index, bool *value)
{
    if (index >= this->currentExecutable->binaries.size())
        return ErrorCode::INDEX_OUT_OF_BOUNDS;
    *value = this->currentExecutable->binaries[index];
    return ErrorCode::OK;
}

ErrorCode PLCManager::GetIntegerAsPointer(size_t index, int *value)
{
    if (index >= this->currentExecutable->integers.size())
        return ErrorCode::INDEX_OUT_OF_BOUNDS;
    *value = this->currentExecutable->integers[index];
    return ErrorCode::OK;
}

ErrorCode PLCManager::GetColorAsPointer(size_t index, uint32_t *value)
{
    if (index >= this->currentExecutable->colors.size())
        return ErrorCode::INDEX_OUT_OF_BOUNDS;
    *value = this->currentExecutable->colors[index];
    return ErrorCode::OK;
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
    ErrorCode ReadU8Array(uint8_t *target, size_t len)
    {
        //if(byteOffset+len>=maxOffset) return ErrorCode::INDEX_OUT_OF_BOUNDS;
        for (size_t i = 0; i < len; i++)
        {
            target[i] = buf[byteOffset + i];
        }
        byteOffset += len;
        return ErrorCode::OK;
    }
};


void PLCManager::plcTask(void *pvParameters)
{
    PLCManager *plc = (PLCManager *)pvParameters;
    plc->EternalLoop();
}

void PLCManager::EternalLoop(){   
    ESP_LOGI(TAG, "PLC Manager started");
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 10;
    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();
    this->FindInitialExecutable();
    
    for(int i=0;i<3;i++)
    {
        hal->ColorizeLed(LED::LED_RED, CRGB::DarkRed);
        hal->ColorizeLed(LED::LED_GREEN, CRGB::DarkGreen);
        hal->ColorizeLed(LED::LED_YELLOW, CRGB::Yellow);
        hal->ColorizeLed(LED::LED_3, CRGB::DarkBlue);
        hal->AfterLoop();
        vTaskDelay(pdMS_TO_TICKS(150));
        hal->ColorizeLed(LED::LED_RED, CRGB::DarkBlue);
        hal->ColorizeLed(LED::LED_GREEN, CRGB::Yellow);
        hal->ColorizeLed(LED::LED_YELLOW, CRGB::DarkGreen);
        hal->ColorizeLed(LED::LED_3, CRGB::DarkRed);
        hal->AfterLoop();
        vTaskDelay(pdMS_TO_TICKS(150));
    }    
    
    hal->PlaySong(0);
    ESP_LOGD(TAG, "plcmanager main loop starts");
    while (true)
    {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        ESP_LOGD(TAG, "CheckForNewExecutable");
        CheckForNewExecutable();
        ESP_LOGD(TAG, "BeforeLoop");
        hal->BeforeLoop();
        ESP_LOGD(TAG, "Loop");
        Loop();
        ESP_LOGD(TAG, "AfterLoop");
        hal->AfterLoop();
    }
}

ErrorCode PLCManager::ParseNewExecutableAndEnqueue(const uint8_t  *buffer, size_t length)
{
    ParseContext *ctx = new ParseContext();
    ctx->buf = buffer;
    ctx->byteOffset = 0;
    ctx->maxOffset=length;
    ESP_LOGI(TAG, "Starting to parse new Executable of length %d", length);
    const uint32_t dataStructureVersion = ctx->ReadU32();
    if(dataStructureVersion!=0xAFFECAFE) return ErrorCode::INCOMPATIBLE_VERSION;
    
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
//#pragma region Basic
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
        case 3:
        {
            ESP_LOGI(TAG, "Found FB_XOR2");
            functionBlocks[cfgIndex] = new FB_XOR2(ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 4:
        {
            ESP_LOGI(TAG, "FOUND FB_NOT");
            functionBlocks[cfgIndex] = new FB_NOT(ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 5:
        {
            ESP_LOGI(TAG, "FOUND FB_RS");
            functionBlocks[cfgIndex] = new FB_RS(ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 6:
        {
            ESP_LOGI(TAG, "FOUND FB_SR");
            functionBlocks[cfgIndex] = new FB_SR(ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 7:
        {
            ESP_LOGI(TAG, "FOUND FB_ConstTrue");
            functionBlocks[cfgIndex] = new FB_ConstTrue(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 8:
        {
            ESP_LOGI(TAG, "FOUND FB_ConstFalse");
            functionBlocks[cfgIndex] = new FB_ConstFalse(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 9:
        {
            ESP_LOGI(TAG, "FOUND FB_CNT");
            functionBlocks[cfgIndex] = new FB_CNT(ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        // case 10:
        // {
        //     ESP_LOGI(TAG, "FOUND FB_Timekeeper");
        //     functionBlocks[cfgIndex] = new FB_Timekeeper(ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        // }
        // break;
        case 11:
        {
            ESP_LOGI(TAG, "FOUND FB_TON");
            functionBlocks[cfgIndex] = new FB_TON(ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 12:
        {
            ESP_LOGI(TAG, "FOUND FB_TOF");
            functionBlocks[cfgIndex] = new FB_TOF(ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        }
        break;
//#pragma endregion Basic
//#pragma region Arithmetic
        case 13:
        {
            ESP_LOGI(TAG, "Found FB_ADD2");
            functionBlocks[cfgIndex] = new FB_ADD2(ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        }
        break;   
        case 14:
        {
            ESP_LOGI(TAG, "Found FB_SUB2");
            functionBlocks[cfgIndex] = new FB_SUB2(ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 15:
        {
            ESP_LOGI(TAG, "Found FB_MULTIPLY");
            functionBlocks[cfgIndex] = new FB_MULTIPLY(ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        }
        break;        
        case 16:
        {
            ESP_LOGI(TAG, "Found FB_DIVIDE");
            functionBlocks[cfgIndex] = new FB_DIVIDE(ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 17:
        {
            ESP_LOGI(TAG, "Found FB_MAX");
            functionBlocks[cfgIndex] = new FB_MAX(ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        }
        break;        
        case 18:
        {
            ESP_LOGI(TAG, "Found FB_MIN");
            functionBlocks[cfgIndex] = new FB_MIN(ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 19:
        {
            ESP_LOGI(TAG, "FOUND FB_GT");
            functionBlocks[cfgIndex] = new FB_GT(ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 20:
        {
            ESP_LOGI(TAG, "FOUND FB_LT");
            functionBlocks[cfgIndex] = new FB_LT(ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 21:
        {
            ESP_LOGI(TAG, "FOUND FB_ConstInteger");
            functionBlocks[cfgIndex] = new FB_ConstInteger(ctx->ReadU32(), ctx->ReadU32(), ctx->ReadS32());
        }
        break;
//#pragma endregion Arithmetic
//#pragma region Converter
        case 24:
        {
            ESP_LOGI(TAG, "FOUND FB_Bool2ColorConverter");
            functionBlocks[cfgIndex] = new FB_Bool2ColorConverter(ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        }
        break;       
//#pragma endregion Converter
//#pragma region Input       
        case 30:
        {
            ESP_LOGI(TAG, "FOUND FB_GreenButton");
            functionBlocks[cfgIndex] = new FB_GreenButton(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 31:
        {
            ESP_LOGI(TAG, "FOUND FB_EncoderButton");
            functionBlocks[cfgIndex] = new FB_EncoderButton(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 33:
        {
            ESP_LOGI(TAG, "FOUND FB_RedButton");
            functionBlocks[cfgIndex] = new FB_RedButton(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
//#pragma endregion Input
//#pragma region Sensor
        case 34:
        {
            ESP_LOGI(TAG, "FOUND FB_MovementSensor");
            functionBlocks[cfgIndex] = new FB_MovementSensor(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 36:
        {
            ESP_LOGI(TAG, "FOUND FB_AirTemperatureBMESensor");
            functionBlocks[cfgIndex] = new FB_AirTemperatureBMESensor(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        
        case 41:
        {
            ESP_LOGI(TAG, "FOUND FB_AmbientBrigthnessSensor");
            functionBlocks[cfgIndex] = new FB_AmbientBrigthnessSensor(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 44:
        {
            ESP_LOGI(TAG, "FOUND FB_HeaterTemperatureSensor");
            functionBlocks[cfgIndex] = new FB_HeaterTemperatureSensor(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
//#pragma endregion Sensor
//#pragma region Output
        case 45:
        {
            ESP_LOGI(TAG, "FOUND FB_Relay");
            functionBlocks[cfgIndex] = new FB_Relay(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 46:
        {
            ESP_LOGI(TAG, "FOUND FB_RedLED");
            functionBlocks[cfgIndex] = new FB_RedLED(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 47:
        {
            ESP_LOGI(TAG, "FOUND FB_YellowLED");
            functionBlocks[cfgIndex] = new FB_YellowLED(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 48:
        {
            ESP_LOGI(TAG, "FOUND FB_GreenLED");
            functionBlocks[cfgIndex] = new FB_GreenLED(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 49:
        {
            ESP_LOGI(TAG, "FOUND FB_LED3");
            functionBlocks[cfgIndex] = new FB_LED3(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 50:
        {
            ESP_LOGI(TAG, "FOUND FB_LED4");
            functionBlocks[cfgIndex] = new FB_LED4(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 51:
        {
            ESP_LOGI(TAG, "FOUND FB_LED5");
            functionBlocks[cfgIndex] = new FB_LED5(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 52:
        {
            ESP_LOGI(TAG, "FOUND FB_LED6");
            functionBlocks[cfgIndex] = new FB_LED6(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 53:
        {
            ESP_LOGI(TAG, "FOUND FB_LED7");
            functionBlocks[cfgIndex] = new FB_LED7(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 54:
        {
            ESP_LOGI(TAG, "FOUND FB_FAN1");
            functionBlocks[cfgIndex] = new FB_FAN1(ctx->ReadU32(), ctx->ReadU32());
        }
        break;
        case 55:
        {
            ESP_LOGI(TAG, "FOUND FB_FAN2");
            functionBlocks[cfgIndex] = new FB_FAN2(ctx->ReadU32(), ctx->ReadU32());
        }
        break;

//region Specials
        case 57:
        {
            ESP_LOGI(TAG, "FOUND FB_Melody");
            functionBlocks[cfgIndex] = new FB_Melody(ctx->ReadU32(), ctx->ReadU32(),ctx->ReadU32());
        }
        break;
        /*case 33:
        {
            ESP_LOGI(TAG, "FOUND FB_MQTT");
            //welche broker URL (aktuell nicht genutzt), welches Topic template, immer nach wie vielen Millisekunden senden
            functionBlocks[cfgIndex] = new FB_MQTT(ctx->ReadU32(), ctx->ReadU32(),ctx->ReadU32()); //FIXME
        }
        break;
*/
//endregion Specials
        default:
            ESP_LOGE(TAG, "Unknown Operator Type found");
            return ErrorCode::INVALID_NEW_FBD;
        }
    }

    delete ctx;

    if (this->nextExecutable != nullptr)
    {
        return ErrorCode::QUEUE_OVERLOAD;
    }

    std::vector<bool> binaries(booleansCount);
    std::vector<int> integers(integersCount);
    std::vector<double> floats(floatsCount);
    std::vector<uint32_t> colors(colorsCount);

    size_t debugSizeBytes = 4 /*Hashcode!*/ +4*(booleansCount+1+integersCount+1+floatsCount+1+colorsCount+1);//jeweils noch ein size_t für die Länge
    
    this->nextExecutable = new Executable(hash, debugSizeBytes, functionBlocks, binaries, integers, floats, colors);
    ESP_LOGI(TAG, "Created new executable and enqueued it");
    return ErrorCode::OK;
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

ErrorCode PLCManager::GetDebugInfoSize(size_t *sizeInBytes){
    *sizeInBytes=this->currentExecutable->debugSizeBytes;
    return ErrorCode::OK;
}

ErrorCode PLCManager::GetDebugInfo(uint8_t *buffer, size_t maxSizeInByte){
    int *bufAsINT32 = (int*)buffer;
    float *bufAsFLOAT = (float*)buffer;
    uint32_t *bufAsUINT32 = (uint32_t*)buffer;
    size_t offset32 = 0;
    if(maxSizeInByte<this->currentExecutable->debugSizeBytes) return ErrorCode::INDEX_OUT_OF_BOUNDS;
    
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
    
    return ErrorCode::OK;
}

ErrorCode PLCManager::FindInitialExecutable()
{
    FILE *fd = NULL;
    struct stat file_stat;
    ESP_LOGI(TAG, "Trying to open %s", labathome::config::paths::DEFAULT_FBD_BIN_FILENAME);
    if (stat(labathome::config::paths::DEFAULT_FBD_BIN_FILENAME, &file_stat) == -1) {
        ESP_LOGI(TAG, "Default PLC file %s does not exist. Using factory default instead", labathome::config::paths::DEFAULT_FBD_BIN_FILENAME);
        return ErrorCode::OK;
    }
    fd = fopen(labathome::config::paths::DEFAULT_FBD_BIN_FILENAME, "r");
    if (!fd) {
        ESP_LOGE(TAG, "Failed to read existing file : %s", labathome::config::paths::DEFAULT_FBD_BIN_FILENAME);
        return ErrorCode::FILE_SYSTEM_ERROR;
    }
    uint8_t buf[file_stat.st_size];
    size_t size_read = fread(buf, 1, file_stat.st_size, fd);
    if(size_read!=file_stat.st_size){
        ESP_LOGE(TAG, "Unable to read file completely : %s", labathome::config::paths::DEFAULT_FBD_BIN_FILENAME);
        return ErrorCode::FILE_SYSTEM_ERROR;
    }
    ESP_LOGI(TAG, "Successfully read %s", labathome::config::paths::DEFAULT_FBD_BIN_FILENAME);
    return this->ParseNewExecutableAndEnqueue(buf, size_read);
}

ErrorCode PLCManager::CheckForNewExecutable()
{
    if (this->nextExecutable == nullptr)
    {
        //no new executable available
        return ErrorCode::OK;
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
    return ErrorCode::OK;
}


ErrorCode PLCManager::Loop()
{
    static ExperimentMode previousExperimentMode = ExperimentMode::functionblock; //set in last line of this method
    uint32_t nowMsSteady = hal->GetMillis();
    if(experimentMode != ExperimentMode::functionblock && nowMsSteady-this->lastExperimentTrigger>TRIGGER_FALLBACK_TIME_MS)
    {
        //auto fallback
        experimentMode = ExperimentMode::functionblock;
        ESP_LOGI(TAG, "Auto fallback to experimentMode = ExperimentMode::functionblock;");
    }
    if(this->experimentMode!=previousExperimentMode){
        //Safe settings on mode change!
        hal->SetFan1State(0);
        hal->SetFan2State(0);
        hal->SetHeaterState(0);
        hal->SetServo1Position(0);
        this->setpointAirspeed=0;
        this->setpointFan1=0.0;
        this->setpointFan2=0.0;
        this->setpointHeater=0.0;
        this->setpointServo1=0;
        this->setpointTemperature=0;
    }
    
    if(experimentMode == ExperimentMode::functionblock)
    {
        for (const auto &i : this->currentExecutable->functionBlocks)
        {
            i->execute(this);
        }
    }
    else if(experimentMode==ExperimentMode::openloop_heater){
        heaterPIDController->SetMode(Mode::OFF, nowMsSteady);
        hal->SetHeaterState(this->setpointHeater);
        hal->SetFan1State(this->setpointFan1);
        hal->SetFan2State(this->setpointFan1);
    }
    else if(experimentMode==ExperimentMode::closedloop_heater){
        heaterPIDController->SetMode(Mode::CLOSEDLOOP, nowMsSteady);
        if(heaterPIDController->SetKpTnTv(heaterKP, heaterTN, heaterTV)!=ErrorCode::OBJECT_NOT_CHANGED)
        {
            ESP_LOGI(TAG, "SetKpTnTv to %F %F %F", heaterKP, heaterTN, heaterTV);
        }
        float act =0;
        hal->GetHeaterTemperature(&act);
        this->actualTemperature=act;
        if(heaterPIDController->Compute(nowMsSteady)==ErrorCode::OK){ //OK means: Value changed
             ESP_LOGI(TAG, "Computed a new  setpointHeater %F", setpointHeater);
        }
        hal->SetHeaterState(this->setpointHeater);
        hal->SetFan1State(this->setpointFan1);
        hal->SetFan2State(this->setpointFan1);
    }
    else if(experimentMode==ExperimentMode::closedloop_airspeed){
        /*
        if(airspeedPIDController->GetMode()!=AUTOMATIC)
        {
            ESP_LOGI(TAG, "airspeedPIDController->SetMode(AUTOMATIC);");
            airspeedPIDController->SetMode(AUTOMATIC);
        }
        if(airspeedPIDController->GetKd()!=this->airspeedKD || airspeedPIDController->GetKi()!=this->airspeedKI || airspeedPIDController->GetKp()!=this->airspeedKP)
        {
            ESP_LOGI(TAG, "airspeedPIDController->SetTunings(KP, KI, KD); %f %f %f", airspeedKP, airspeedKI, airspeedKD);
            airspeedPIDController->SetTunings(airspeedKP, airspeedKI, airspeedKD);
        }
        float act =0;
        hal->GetAirSpeed(&act);
        this->actualAirspeed=act;

        bool newResult = airspeedPIDController->Compute();
        if(newResult)
        {
            ESP_LOGI(TAG, "airspeedPIDController if(newResult): %F", this->setpointFan2);
            hal->SetFan2State(this->setpointFan2);
        }
        hal->SetFan2State(this->setpointFan2);
        */
    }
    previousExperimentMode = this->experimentMode;
    return ErrorCode::OK;
}

ErrorCode PLCManager::TriggerHeaterExperimentClosedLoop(double setpointTemperature, double setpointFan1, double KP, double TN, double TV, HeaterExperimentData *data){
    //Trigger
    this->lastExperimentTrigger=hal->GetMillis();
    this->experimentMode=ExperimentMode::closedloop_heater;
    //New Setpoints
    this->setpointTemperature=setpointTemperature;
    this->setpointFan1=setpointFan1;
    this->heaterKP=KP;
    this->heaterTN=TN;
    this->heaterTV=TV;
    //Fill return data
    data->Fan=hal->GetFan1State();
    data->Heater=hal->GetHeaterState();
    hal->GetHeaterTemperature(&(data->ActualTemperature));
    data->SetpointTemperature=this->setpointTemperature;
    return ErrorCode::OK;
}
ErrorCode PLCManager::TriggerHeaterExperimentOpenLoop(double setpointHeater, double setpointFan1, HeaterExperimentData *data){
    //Trigger
    this->lastExperimentTrigger=hal->GetMillis();
    this->experimentMode=ExperimentMode::openloop_heater;
    //New Setpoints
    this->setpointHeater=setpointHeater;
    this->setpointFan1=setpointFan1;
    //Fill return data
    data->Fan=hal->GetFan1State();
    data->Heater=hal->GetHeaterState();
    hal->GetHeaterTemperature(&(data->ActualTemperature));
    data->SetpointTemperature=0;
    return ErrorCode::OK;
}
ErrorCode PLCManager::TriggerHeaterExperimentFunctionblock(HeaterExperimentData *data){
    //Trigger
    this->lastExperimentTrigger=hal->GetMillis();
    this->experimentMode=ExperimentMode::functionblock;
    //Fill return data
    data->Fan=hal->GetFan1State();
    data->Heater=hal->GetHeaterState();
    hal->GetHeaterTemperature(&(data->ActualTemperature));
    data->SetpointTemperature=0;
    return ErrorCode::OK;
}
ErrorCode PLCManager::TriggerAirspeedExperimentClosedLoop(double setpointAirspeed, double setpointServo1, double KP, double TN, double TV, AirspeedExperimentData *data){
    //Trigger
    this->lastExperimentTrigger=hal->GetMillis();
    this->experimentMode=ExperimentMode::closedloop_airspeed;
    //New Setpoints
    this->setpointAirspeed=setpointAirspeed;
    this->setpointServo1=setpointServo1;
    this->airspeedKP=KP;
    this->airspeedTN=TN;
    this->airspeedTV=TV;
    //Fill return data 
    data->Fan=hal->GetFan2State();
    hal->GetAirSpeed(&(data->ActualAirspeed));
    data->SetpointAirspeed=setpointAirspeed;
    data->Servo=this->setpointServo1;
    return ErrorCode::OK;
}
ErrorCode PLCManager::TriggerAirspeedExperimentOpenLoop(double setpointFan2, double setpointServo1, AirspeedExperimentData *data){
    //Trigger
    this->lastExperimentTrigger=hal->GetMillis();
    this->experimentMode=ExperimentMode::openloop_heater;
    //New Setpoints
    this->setpointFan2=setpointFan2;
    this->setpointServo1=setpointServo1;
    //Fill return data
    data->Fan=hal->GetFan2State();
    hal->GetAirSpeed(&(data->ActualAirspeed));
    data->SetpointAirspeed=setpointAirspeed;
    data->Servo=this->setpointServo1;
    return ErrorCode::OK;
}
ErrorCode PLCManager::TriggerAirspeedExperimentFunctionblock(AirspeedExperimentData *data){
    //Trigger
    this->lastExperimentTrigger=hal->GetMillis();
    this->experimentMode=ExperimentMode::functionblock;
    //Fill return data
    data->Fan=hal->GetFan2State();
    hal->GetAirSpeed(&(data->ActualAirspeed));
    data->SetpointAirspeed=setpointAirspeed;
    data->Servo=this->setpointServo1;
    return ErrorCode::OK;
}
