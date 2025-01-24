#include "functionblocks.hh"
#include "esp_log.h"
#include "errorcodes.hh"
#include <vector>
#include <math.h>
#include "devicemanager.hh"
#include "common.hh"
#include "common-esp32.hh"
#include "../generated/flatbuffers_cpp/functionblock_generated.h"
#include "esp_vfs.h"

constexpr uint32_t TRIGGER_FALLBACK_TIME_MS{10000};
constexpr size_t FILE_PATH_MAX =ESP_VFS_PATH_MAX + CONFIG_SPIFFS_OBJ_NAME_LEN;


constexpr TickType_t xFrequency {pdMS_TO_TICKS(50)};
constexpr const char *TAG = "devicemanager";

#define SetBit(A, k) (A[(k / 32)] |= (1 << (k % 32)))
#define ClearBit(A, k) (A[(k / 32)] &= ~(1 << (k % 32)))
#define TestBit(A, k) (A[(k / 32)] & (1 << (k % 32)))

DeviceManager::DeviceManager(iHAL *hal):hal(hal)
{
    currentExecutable = this->createDummyInitialExecutableAndEnqueue();
    nextExecutable = nullptr;
    heaterPIDController = new PID::Controller<float>(&actualTemperature, &setpointHeater, &setpointTemperature, 0, 100, PID::Mode::OFF, PID::AntiWindup::ON_LIMIT_INTEGRATOR, PID::Direction::DIRECT, 1000);
}

ErrorCode DeviceManager::InitAndRun()
{
    xTaskCreate([](void* p){((DeviceManager*)p)->EternalLoop();}, "plcTask", 4096 * 4, this, 6, NULL);
    return ErrorCode::OK;
}

bool DeviceManager::IsBinaryAvailable(size_t index)
{
    return index < this->currentExecutable->binaries.size();
}

bool DeviceManager::IsIntegerAvailable(size_t index)
{
    return index < this->currentExecutable->integers.size();
}

bool DeviceManager::IsColorAvailable(size_t index)
{
    return index < this->currentExecutable->colors.size();
}

bool DeviceManager::IsFloatAvailable(size_t index)
{
    return index < this->currentExecutable->floats.size();
}

ErrorCode DeviceManager::SetBinary(size_t index, bool value)
{
    if (index >= this->currentExecutable->binaries.size()){
        return ErrorCode::INDEX_OUT_OF_BOUNDS;
    }
    this->currentExecutable->binaries[index] = value;
    return ErrorCode::OK;
}

ErrorCode DeviceManager::SetInteger(size_t index, int value)
{
    if (index >= this->currentExecutable->integers.size()){
        return ErrorCode::INDEX_OUT_OF_BOUNDS;
    }
    this->currentExecutable->integers[index] = value;
    return ErrorCode::OK;
    
}

ErrorCode DeviceManager::SetColor(size_t index, uint32_t value)
{
    if (index >= this->currentExecutable->colors.size())
        return ErrorCode::INDEX_OUT_OF_BOUNDS;
    this->currentExecutable->colors[index] = value;
    return ErrorCode::OK;
}

ErrorCode DeviceManager::SetFloat(size_t index, float value)
{
    if (index >= this->currentExecutable->floats.size())
        return ErrorCode::INDEX_OUT_OF_BOUNDS;
    this->currentExecutable->floats[index] = value;
    return ErrorCode::OK;
}

bool DeviceManager::GetBinary(size_t index)
{
    bool x = false;
    GetBinaryAsPointer(index, &x);
    return x;
}

int DeviceManager::GetInteger(size_t index)
{
    int x = 0;
    GetIntegerAsPointer(index, &x);
    return x;
}

uint32_t DeviceManager::GetColor(size_t index)
{
    uint32_t x = 0;
    GetColorAsPointer(index, &x);
    return x;
}

float DeviceManager::GetFloat(size_t index)
{
    float x = 0;
    GetFloatAsPointer(index, &x);
    return x;
}

ErrorCode DeviceManager::GetBinaryAsPointer(size_t index, bool *value)
{
    if (index >= this->currentExecutable->binaries.size())
        return ErrorCode::INDEX_OUT_OF_BOUNDS;
    *value = this->currentExecutable->binaries[index];
    return ErrorCode::OK;
}

ErrorCode DeviceManager::GetIntegerAsPointer(size_t index, int *value)
{
    if (index >= this->currentExecutable->integers.size())
        return ErrorCode::INDEX_OUT_OF_BOUNDS;
    *value = this->currentExecutable->integers[index];
    return ErrorCode::OK;
}

ErrorCode DeviceManager::GetColorAsPointer(size_t index, uint32_t *value)
{
    if (index >= this->currentExecutable->colors.size())
        return ErrorCode::INDEX_OUT_OF_BOUNDS;
    *value = this->currentExecutable->colors[index];
    return ErrorCode::OK;
}

ErrorCode DeviceManager::GetFloatAsPointer(size_t index, float *value)
{
    if (index >= this->currentExecutable->floats.size())
        return ErrorCode::INDEX_OUT_OF_BOUNDS;
    *value = this->currentExecutable->floats[index];
    return ErrorCode::OK;
}

int64_t DeviceManager::GetMicroseconds()
{
    return hal->GetMicros();
}

iHAL *DeviceManager::GetHAL()
{
    return this->hal;
}

void DeviceManager::EternalLoop(){   
    ESP_LOGI(TAG, "PLC Manager started");
    TickType_t xLastWakeTime;
    
    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();
    if(this->ParseNewExecutableAndEnqueue(DEFAULT_FBD_FILEPATH)!=ErrorCode::OK){
        ESP_LOGW(TAG, "No default.fbd found. Continuing with factory dummy fbd");
    }
    hal->GreetUserOnStartup();

    ESP_LOGD(TAG, "devicemanager main loop starts");
    while (true)
    {
        xTaskDelayUntil(&xLastWakeTime, xFrequency);
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

class ParseContext
{
public:
    const uint8_t *buf;
    size_t maxOffset;
    size_t byteOffset;
    uint32_t ReadU32()
    {
        uint32_t val = ParseU32(buf, byteOffset);
        byteOffset += 4;
        return val;
    }

    float ReadF32()
    {
        float val = ParseF32(buf, byteOffset);
        byteOffset += 4;
        return val;
    }

    int32_t ReadS32()
    {
        int32_t val = ParseI32(buf, byteOffset);
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

#define S(x, ...) do {                 \
        ESP_LOGI(TAG, "Found " #x);                      \
        functionBlocks[cfgIndex] = new x ( __VA_ARGS__ );\
    } while(0);\
    break;\

ErrorCode DeviceManager::ParseNewExecutableAndEnqueue(const char* path)
{
    FILE *fd = NULL;
    struct stat file_stat;
    
    if (stat(path, &file_stat) == -1) {
        ESP_LOGI(TAG, "FBD file %s does not exist.", path);
        return ErrorCode::OK;
    }
    fd = fopen(path, "r");
    if (!fd) {
        ESP_LOGE(TAG, "Failed to read existing file : %s", path);
        return ErrorCode::FILE_SYSTEM_ERROR;
    }
    ESP_LOGI(TAG, "Opening FBD %s was successful", path);
    size_t sizeOfBinaryData{0};
    size_t size_read = fread(&sizeOfBinaryData, 4, 1, fd);
    if(size_read!=4){
        ESP_LOGE(TAG, "Unable to read sizeOfBinaryData : %s", path);
        return ErrorCode::FILE_SYSTEM_ERROR;
    }
    ESP_LOGI(TAG, "sizeOfBinaryData in FBD %s is %u", path, sizeOfBinaryData);
    uint8_t buffer[sizeOfBinaryData];
    size_read = fread(&buffer, 1, sizeOfBinaryData, fd);
    if(size_read!=file_stat.st_size){
        ESP_LOGE(TAG, "Unable to read file completely : %s", path);
        return ErrorCode::FILE_SYSTEM_ERROR;
    }
    ESP_LOGI(TAG, "Successfully read flatbufferArea of %s", path);

    
    ParseContext *ctx = new ParseContext();
    ctx->buf = buffer;
    ctx->byteOffset = 0;
    ctx->maxOffset=sizeOfBinaryData;
    ESP_LOGI(TAG, "Starting to parse new Executable of length %d", sizeOfBinaryData);
    const uint32_t dataStructureVersion = ctx->ReadU32();
    if(dataStructureVersion!=0xAFFECAFE){
        ESP_LOGE(TAG, "dataStructureVersion!=0xAFFECAFE");
        return ErrorCode::INCOMPATIBLE_VERSION;
    }
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
    ESP_LOGI(TAG, "booleansCount = %lu, integersCount = %lu, floatsCount = %lu, colorsCount = %lu, operatorsCount = %lu",booleansCount, integersCount, floatsCount, colorsCount, operatorsCount);
    
    std::vector<FunctionBlock *> functionBlocks(operatorsCount);

    for (size_t cfgIndex = 0; cfgIndex < operatorsCount; cfgIndex++)
    {
        const uint32_t operatorType = ctx->ReadU32();
        switch (operatorType)
        {
//#pragma region Basic
        case 1: S(FB_AND2, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        case 2: S(FB_OR2, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        case 3: S(FB_XOR2, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        case 4: S(FB_NOT, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        case 5: S(FB_RS, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        case 6: S(FB_SR, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        case 7: S(FB_ConstTrue, ctx->ReadU32(), ctx->ReadU32());
        case 8: S(FB_ConstFalse, ctx->ReadU32(), ctx->ReadU32());
        case 9: S(FB_CNT, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        case 10: S(FB_Timekeeper, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        case 11: S(FB_TON, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        case 12: S(FB_TOF, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
//#pragma endregion Basic
//#pragma region Arithmetic
        case 13: S(FB_ADD2, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());   
        case 14: S(FB_SUB2, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        case 15: S(FB_MULTIPLY, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());        
        case 16: S(FB_DIVIDE, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        case 17: S(FB_MAX, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());       
        case 18: S(FB_MIN, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        case 19: S(FB_GT, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        case 20: S(FB_LT, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());
        case 21: S(FB_ConstFLOAT, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadF32());
        case 22: S(FB_ConstINTEGER, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadS32());
        case 23: S(FB_Limit, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(),ctx->ReadU32(),ctx->ReadU32());
        case 24: S(FB_LimitMonitor, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(),ctx->ReadU32(),ctx->ReadU32(), ctx->ReadU32());
//#pragma endregion Arithmetic
//#pragma region Converter
        case 25:S(FB_Bool2ColorConverter, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32()); 
        case 26:S(FB_Bool2IntConverter, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadS32(), ctx->ReadS32()); 
        case 27:S(FB_Bool2FloatConverter, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadF32(), ctx->ReadF32()); 
        case 28:S(FB_Int2BoolConverter, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadS32()); 
        case 29:S(FB_Int2FloatConverter, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32()); 
        case 30:S(FB_Float2IntConverter, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());     
//#pragma endregion Converter
//#pragma region Input       
        case 31:S(FB_GreenButton, ctx->ReadU32(), ctx->ReadU32());
        case 32:S(FB_EncoderButton, ctx->ReadU32(), ctx->ReadU32());
        case 33:S(FB_EncoderDetents, ctx->ReadU32(), ctx->ReadU32());
        case 34:S(FB_RedButton, ctx->ReadU32(), ctx->ReadU32());
        case 35:S(FB_AnalogInput0, ctx->ReadU32(), ctx->ReadU32());
        case 36:S(FB_AnalogInput1, ctx->ReadU32(), ctx->ReadU32());
        case 37:S(FB_AnalogInput2, ctx->ReadU32(), ctx->ReadU32());
        case 38:S(FB_AnalogInput3, ctx->ReadU32(), ctx->ReadU32());
//#pragma endregion Input
//#pragma region Sensor
        case 39: S(FB_MovementSensor, ctx->ReadU32(), ctx->ReadU32());
        case 40: S(FB_AirTemperatureSensor, ctx->ReadU32(), ctx->ReadU32());
        case 41: S(FB_AirHumiditySensor, ctx->ReadU32(), ctx->ReadU32());
        case 42: S(FB_AirPressureSensor, ctx->ReadU32(), ctx->ReadU32());
        case 43: S(FB_AirCO2Sensor, ctx->ReadU32(), ctx->ReadU32());
        case 44: S(FB_AirQualitySensor, ctx->ReadU32(), ctx->ReadU32());
        case 45: S(FB_AmbientBrigthnessSensor, ctx->ReadU32(), ctx->ReadU32());
        case 46: S(FB_HeaterTemperatureSensor, ctx->ReadU32(), ctx->ReadU32());
//#pragma endregion Sensor
//#pragma region Output
        case 49: S(FB_Relay,ctx->ReadU32(), ctx->ReadU32());
        case 50: S(FB_RedLED, ctx->ReadU32(), ctx->ReadU32());
        case 51: S(FB_YellowLED, ctx->ReadU32(), ctx->ReadU32());
        case 52: S(FB_GreenLED, ctx->ReadU32(), ctx->ReadU32());
        case 53: S(FB_LED3, ctx->ReadU32(), ctx->ReadU32());
        case 54: S(FB_FAN, ctx->ReadU32(), ctx->ReadU32());
        //case 55: S(FB_FAN2, ctx->ReadU32(), ctx->ReadU32());
        case 56: S(FB_PowerLED, ctx->ReadU32(), ctx->ReadU32());
        case 57: S(FB_AnalogOutput0, ctx->ReadU32(), ctx->ReadU32());

//region Specials
        case 58: S(FB_Sound, ctx->ReadU32(), ctx->ReadU32(),ctx->ReadU32());
        case 59: S(FB_PIDSimple, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadF32()/*float kp*/, ctx->ReadU32()/*int32_t tn_msecs*/, ctx->ReadU32() /*int32_t tv_msecs*/, ctx->ReadF32()/*float minOutput*/, ctx->ReadF32()/*float maxOutput*/, ctx->ReadU32()/*bool directionInverse*/);
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
            ESP_LOGE(TAG, "Unknown Operator Type %lu found!", operatorType);
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
    std::vector<float> floats(floatsCount);
    std::vector<uint32_t> colors(colorsCount);

    size_t debugSizeBytes = 4 /*Hashcode!*/ +4*(booleansCount+1+integersCount+1+floatsCount+1+colorsCount+1);//jeweils noch ein size_t für die Länge
    
    this->nextExecutable = new Executable(hash, debugSizeBytes, functionBlocks, binaries, integers, floats, colors);
    ESP_LOGI(TAG, "Created new executable and enqueued it");
    return ErrorCode::OK;
}

Executable* DeviceManager::createDummyInitialExecutableAndEnqueue()
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
    std::vector<float> floats(floatsCount);
    std::vector<uint32_t> colors(colorsCount);
    functionBlocks[0] = button_red;
    functionBlocks[1] = led_red;
    uint32_t hash = 0;
   
    size_t debugSizeBytes = 4 /*Hashcode!*/ +4*(booleansCount+1+integersCount+1+floatsCount+1+colorsCount+1);//jeweils noch ein size_t für die Länge
    return new Executable(hash, debugSizeBytes, functionBlocks, binaries, integers, floats, colors);
}

ErrorCode DeviceManager::GetDebugInfoSize(size_t *sizeInBytes){
    *sizeInBytes=this->currentExecutable->debugSizeBytes;
    return ErrorCode::OK;
}

ErrorCode DeviceManager::GetDebugInfo(flatbuffers::FlatBufferBuilder& b){
    std::vector<uint8_t>bools;
    for (size_t vecPos = 0; vecPos < this->currentExecutable->binaries.size(); vecPos++) {
        bools.push_back(this->currentExecutable->binaries[vecPos]?1:0);
    }
    std::vector<int32_t> integers;
    for (size_t vecPos = 0; vecPos < this->currentExecutable->integers.size(); vecPos++) {
        integers.push_back(this->currentExecutable->integers[vecPos]);
    }
    std::vector<float> floats;
    for (size_t vecPos = 0; vecPos < this->currentExecutable->floats.size(); vecPos++) {
        floats.push_back(this->currentExecutable->floats[vecPos]);
    }
   std::vector<uint32_t> colors;
    for (size_t vecPos = 0; vecPos < this->currentExecutable->colors.size(); vecPos++) {
        colors.push_back(this->currentExecutable->colors[vecPos]);
    }
    b.Finish(
        functionblock::CreateResponseWrapper(
            b,
            functionblock::Responses::Responses_ResponseDebugData,
            functionblock::CreateResponseDebugDataDirect(b, this->currentExecutable->hash, &bools, &integers, &floats, &colors ).Union()
        )
    );
    return ErrorCode::OK;
}

ErrorCode DeviceManager::CheckForNewExecutable()
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
    ESP_LOGI(TAG, "Removed old executable completely");
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

ErrorCode DeviceManager::Loop()
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
        hal->SetFanDuty(0, 0.);
        hal->SetHeaterDuty(0);
        hal->SetServoPosition(0, 0.);
        //hal->SetAnalogOutput(0); do not set a out voltage here as this may interfere with MP3 play
        //this->setpointAirspeed=0;
        //this->setpointFan=0.0;
        //this->setpointHeater=0.0;
        //this->setpointServo1=0;
        //this->setpointTemperature=0;
    }
    
    if(experimentMode == ExperimentMode::functionblock)
    {
        for (const auto &i : this->currentExecutable->functionBlocks)
        {
            i->execute(this);
        }
    }
    else if(experimentMode==ExperimentMode::openloop_heater){
        heaterPIDController->SetMode(PID::Mode::OFF, nowMsSteady);
        hal->SetHeaterDuty(this->setpointHeater);
        hal->SetFanDuty(0, this->setpointFan);
    }
    else if(experimentMode==ExperimentMode::closedloop_heater){
        if(heaterReset){
            heaterPIDController->Reset();
        }
        heaterPIDController->SetMode(PID::Mode::CLOSEDLOOP, nowMsSteady);
        heaterPIDController->SetWorkingPointOffset(this->heaterWorkingPointOffset);
        heaterPIDController->SetKpTnTv(heaterKP, heaterTN_secs*1000, heaterTV_secs*1000/*, heaterTV_secs*200*/);
        float act =0;
        hal->GetHeaterTemperature(&act);
        this->actualTemperature=act;
        if(heaterPIDController->Compute(nowMsSteady)==ErrorCode::OK){ //OK means: Value changed
             ESP_LOGI(TAG, "Computed a new setpointHeater %F", setpointHeater);
        }
        hal->SetHeaterDuty(this->setpointHeater);
        hal->SetFanDuty(0, this->setpointFan);
    }
    else if(experimentMode==ExperimentMode::openloop_ptn){
        /*
        ptnPIDController->SetMode(PID_T1::Mode::OFF, nowMsSteady);
        float *voltages;
        hal->GetAnalogInputs(&voltages);
        hal->ColorizeLed(0, CRGB::FromTemperature(0, 3.3, voltages[0]));
        hal->ColorizeLed(1, CRGB::FromTemperature(0, 3.3, voltages[1]));
        hal->ColorizeLed(2, CRGB::FromTemperature(0, 3.3, voltages[2]));
        hal->ColorizeLed(3, CRGB::FromTemperature(0, 3.3, voltages[3]));
        hal->SetAnalogOutput(0, this->setpointVoltageOut);
        */
    }
    else if(experimentMode==ExperimentMode::closedloop_ptn){
        /*
        if(ptnReset){
            ptnPIDController->Reset();
        }
        ptnPIDController->SetMode(PID_T1::Mode::CLOSEDLOOP, nowMsSteady);
        if(ptnPIDController->SetKpTnTv(ptnKP, ptnTN_secs*1000, ptnTV_secs*1000, ptnTV_secs*200)!=ErrorCode::OBJECT_NOT_CHANGED)
        {
            ESP_LOGI(TAG, "SetKpTnTv to %F %F %F", ptnKP, ptnTN_secs, ptnTV_secs);
        }
        float *voltages;
        hal->GetAnalogInputs(&voltages);
        hal->ColorizeLed(0, CRGB::FromTemperature(0, 3.3, voltages[0]));
        hal->ColorizeLed(1, CRGB::FromTemperature(0, 3.3, voltages[1]));
        hal->ColorizeLed(2, CRGB::FromTemperature(0, 3.3, voltages[2]));
        hal->ColorizeLed(3, CRGB::FromTemperature(0, 3.3, voltages[3]));
        this->actualPtn=voltages[3];
        if(ptnPIDController->Compute(nowMsSteady)==ErrorCode::OK){ //OK means: Value changed
             ESP_LOGI(TAG, "Computed a new  setpointPtn %F", setpointVoltageOut);
        }
        hal->SetAnalogOutput(0, this->setpointVoltageOut);
        */
    }
    else if(experimentMode==ExperimentMode::closedloop_airspeed){
        /*File
        if(airspeedReset){
            airspeedPIDController->Reset();
        }
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
            hal->SetFan2Duty(this->setpointFan2);
        }
        hal->SetFan2Duty(this->setpointFan2);
        */
    }
    else if(experimentMode==ExperimentMode::boris_udp){
        //do nothing
    }
    previousExperimentMode = this->experimentMode;
    return ErrorCode::OK;
}

ErrorCode DeviceManager::TriggerHeaterExperiment(const heaterexperiment::RequestHeater* r, flatbuffers::FlatBufferBuilder &b){
    if(r->mode()==heaterexperiment::Mode::Mode_FunctionBlock) return ErrorCode::GENERIC_ERROR;
    this->lastExperimentTrigger=hal->GetMillis();
    this->heaterReset=r->regulator_reset();
    this->experimentMode=r->mode()==heaterexperiment::Mode::Mode_ClosedLoop?ExperimentMode::closedloop_heater:ExperimentMode::openloop_heater;
    
    //New Setpoints
    this->setpointTemperature=r->setpoint_temperature_degrees();
    this->setpointFan=r->fan_speed_percent();
    this->heaterKP=r->kp();
    this->heaterTN_secs=r->tn();
    this->heaterTV_secs=r->tv();
    this->heaterWorkingPointOffset=r->heater_power_working_point_percent();
    float heaterTemp{0.0};
    float fanDuty{0.0};
    hal->GetFanDuty(0, &fanDuty);
    hal->GetHeaterTemperature(&heaterTemp);
    b.Finish(
        //There is only one message type for Request and one for Response. So there is no Wrapper necessary
        heaterexperiment::CreateResponseHeater(b, setpointTemperature, heaterTemp, hal->GetHeaterState(), fanDuty)
    );
    return ErrorCode::OK;
}

