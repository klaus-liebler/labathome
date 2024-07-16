#include "functionblocks.hh"
#include "esp_log.h"
#include "errorcodes.hh"
#include <vector>
#include "common_in_project.hh"
#include <math.h>
#include "winfactboris_messages.hh"
#include "devicemanager.hh"

constexpr uint32_t TRIGGER_FALLBACK_TIME_MS = 3000;
constexpr const char *TAG = "devicemanager";

#define SetBit(A, k) (A[(k / 32)] |= (1 << (k % 32)))
#define ClearBit(A, k) (A[(k / 32)] &= ~(1 << (k % 32)))
#define TestBit(A, k) (A[(k / 32)] & (1 << (k % 32)))

DeviceManager::DeviceManager(HAL *hal):hal(hal)
{
    currentExecutable = this->createInitialExecutable();
    nextExecutable = nullptr;
    heaterPIDController = new PID_T1::Controller<float>(&actualTemperature, &setpointHeater, &setpointTemperature, 0, 100, PID_T1::Mode::OFF, PID_T1::AntiWindup::ON_SWICH_OFF_INTEGRATOR, PID_T1::Direction::DIRECT, 1000);
    airspeedPIDController = new PID_T1::Controller<float>(&actualAirspeed, &setpointFan2, &setpointAirspeed, 0, 100, PID_T1::Mode::OFF, PID_T1::AntiWindup::ON_SWICH_OFF_INTEGRATOR, PID_T1::Direction::DIRECT, 1000);
    ptnPIDController = new PID_T1::Controller<float>(&actualPtn, &setpointVoltageOut, &setpointPtn, 0, 3.3, PID_T1::Mode::OFF, PID_T1::AntiWindup::ON_SWICH_OFF_INTEGRATOR, PID_T1::Direction::DIRECT, 1000);
}

ErrorCode DeviceManager::InitAndRun(){
    xTaskCreate(DeviceManager::plcTask, "plcTask", 4096 * 4, this, 6, NULL);
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

HAL *DeviceManager::GetHAL()
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


void DeviceManager::plcTask(void *pvParameters)
{
    DeviceManager *plc = (DeviceManager *)pvParameters;
    plc->EternalLoop();
}

void DeviceManager::EternalLoop(){   
    ESP_LOGI(TAG, "PLC Manager started");
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 10;
    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();
    this->FindInitialExecutable();
    hal->GreetUserOnStartup();

    ESP_LOGD(TAG, "devicemanager main loop starts");
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

#define S(x, ...) do {                 \
        ESP_LOGI(TAG, "Found " #x);                      \
        functionBlocks[cfgIndex] = new x ( __VA_ARGS__ );\
    } while(0);\
    break;\

ErrorCode DeviceManager::ParseNewExecutableAndEnqueue(const uint8_t  *buffer, size_t length)
{
    ParseContext *ctx = new ParseContext();
    ctx->buf = buffer;
    ctx->byteOffset = 0;
    ctx->maxOffset=length;
    ESP_LOGI(TAG, "Starting to parse new Executable of length %d", length);
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
        case 22: S(FB_Limit, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(),ctx->ReadU32(),ctx->ReadU32());
        case 23: S(FB_LimitMonitor, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(),ctx->ReadU32(),ctx->ReadU32(), ctx->ReadU32());
//#pragma endregion Arithmetic
//#pragma region Converter
        case 24:S(FB_Bool2ColorConverter, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32()); 
        case 25:S(FB_Bool2IntConverter, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadS32(), ctx->ReadS32()); 
        case 26:S(FB_Bool2FloatConverter, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadF32(), ctx->ReadF32()); 
        case 27:S(FB_Int2BoolConverter, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32(), ctx->ReadS32()); 
        case 28:S(FB_Int2FloatConverter, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32()); 
        case 29:S(FB_Float2IntConverter, ctx->ReadU32(), ctx->ReadU32(), ctx->ReadU32());     
//#pragma endregion Converter
//#pragma region Input       
        case 30:S(FB_GreenButton, ctx->ReadU32(), ctx->ReadU32());
        case 31:S(FB_EncoderButton, ctx->ReadU32(), ctx->ReadU32());
        case 32:S(FB_EncoderDetents, ctx->ReadU32(), ctx->ReadU32());
        case 33:S(FB_RedButton, ctx->ReadU32(), ctx->ReadU32());
        case 34:S(FB_AnalogInput0, ctx->ReadU32(), ctx->ReadU32());
        case 35:S(FB_AnalogInput1, ctx->ReadU32(), ctx->ReadU32());
        case 36:S(FB_AnalogInput2, ctx->ReadU32(), ctx->ReadU32());
        case 37:S(FB_AnalogInput3, ctx->ReadU32(), ctx->ReadU32());
//#pragma endregion Input
//#pragma region Sensor
        case 38: S(FB_MovementSensor, ctx->ReadU32(), ctx->ReadU32());
        case 39: S(FB_AirTemperatureSensor, ctx->ReadU32(), ctx->ReadU32());
        case 40: S(FB_AirHumiditySensor, ctx->ReadU32(), ctx->ReadU32());
        case 41: S(FB_AirPressureSensor, ctx->ReadU32(), ctx->ReadU32());
        case 42: S(FB_AirCO2Sensor, ctx->ReadU32(), ctx->ReadU32());
        case 43: S(FB_AirQualitySensor, ctx->ReadU32(), ctx->ReadU32());
        case 44: S(FB_AmbientBrigthnessSensor, ctx->ReadU32(), ctx->ReadU32());
        case 45: S(FB_HeaterTemperatureSensor, ctx->ReadU32(), ctx->ReadU32());
//#pragma endregion Sensor
//#pragma region Output
        case 48: S(FB_Relay,ctx->ReadU32(), ctx->ReadU32());
        case 49: S(FB_RedLED, ctx->ReadU32(), ctx->ReadU32());
        case 50: S(FB_YellowLED, ctx->ReadU32(), ctx->ReadU32());
        case 51: S(FB_GreenLED, ctx->ReadU32(), ctx->ReadU32());
        case 52: S(FB_LED3, ctx->ReadU32(), ctx->ReadU32());
        case 53: S(FB_FAN, ctx->ReadU32(), ctx->ReadU32());
        //case 54: S(FB_FAN2, ctx->ReadU32(), ctx->ReadU32());
        case 55: S(FB_PowerLED, ctx->ReadU32(), ctx->ReadU32());
        case 56: S(FB_AnalogOutput0, ctx->ReadU32(), ctx->ReadU32());

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

Executable *DeviceManager::createInitialExecutable()
{
#if TARGET==LABATHOME_V10
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
    Executable *e = new Executable(hash, debugSizeBytes, functionBlocks, binaries, integers, floats, colors);

#elif TARGET==PTNCHEN_V2
    FB_RedButton *button_red = new FB_RedButton(0,2);
    FB_Bool2FloatConverter * conv = new FB_Bool2FloatConverter(1, 2, 2, 3.3, 0);
    FB_AnalogOutput0 *analog_output = new FB_AnalogOutput0(2,2);
    const uint32_t booleansCount = 3;
    const uint32_t integersCount = 2;
    const uint32_t floatsCount = 3;
    const uint32_t colorsCount = 2;
    
    std::vector<FunctionBlock *> functionBlocks(1);
    std::vector<bool> binaries(booleansCount);
    std::vector<int> integers(integersCount);
    std::vector<float> floats(floatsCount);
    std::vector<uint32_t> colors(colorsCount);
    functionBlocks[0] = button_red;
    functionBlocks[1] = conv;
    functionBlocks[2] = analog_output;
    uint32_t hash = 0;
    size_t debugSizeBytes = 4 /*Hashcode!*/ +4*(booleansCount+1+integersCount+1+floatsCount+1+colorsCount+1);//jeweils noch ein size_t für die Länge
    Executable *e = new Executable(hash, debugSizeBytes, functionBlocks, binaries, integers, floats, colors);
#else
    #error "No createInitialExecutable available for this target defined. See devicemanager.cc"
#endif
    return e;
}

ErrorCode DeviceManager::GetDebugInfoSize(size_t *sizeInBytes){
    *sizeInBytes=this->currentExecutable->debugSizeBytes;
    return ErrorCode::OK;
}

ErrorCode DeviceManager::GetDebugInfo(uint8_t *buffer, size_t maxSizeInByte){
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
        bufAsINT32[vecPos+offset32] = this->currentExecutable->integers[vecPos];
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

ErrorCode DeviceManager::FindInitialExecutable()
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
        hal->SetFanDuty(0);
        hal->SetHeaterDuty(0);
        hal->SetServo1Position(0);
        //hal->SetAnalogOutput(0); do not set a out voltage here as this may interfere with MP3 play
        this->setpointAirspeed=0;
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
        heaterPIDController->SetMode(PID_T1::Mode::OFF, nowMsSteady);
        hal->SetHeaterDuty(this->setpointHeater);
        hal->SetFanDuty(this->setpointFan2);
    }
    else if(experimentMode==ExperimentMode::closedloop_heater){
        if(heaterReset){
            heaterPIDController->Reset();
        }
        heaterPIDController->SetMode(PID_T1::Mode::CLOSEDLOOP, nowMsSteady);
        if(heaterPIDController->SetKpTnTv(heaterKP, heaterTN_secs*1000, heaterTV_secs*1000, heaterTV_secs*200)==ErrorCode::OK)
        {
            ESP_LOGI(TAG, "SetKpTnTv to %F %F %F", heaterKP, heaterTN_secs, heaterTV_secs);
        }
        float act =0;
        hal->GetHeaterTemperature(&act);
        this->actualTemperature=act;
        if(heaterPIDController->Compute(nowMsSteady)==ErrorCode::OK){ //OK means: Value changed
             ESP_LOGI(TAG, "Computed a new  setpointHeater %F", setpointHeater);
        }
        hal->SetHeaterDuty(this->setpointHeater);
        hal->SetFanDuty(this->setpointFan2);
    }
    else if(experimentMode==ExperimentMode::openloop_ptn){
        ptnPIDController->SetMode(PID_T1::Mode::OFF, nowMsSteady);
        float *voltages;
        hal->GetAnalogInputs(&voltages);
        hal->ColorizeLed(0, CRGB::FromTemperature(0, 3.3, voltages[0]));
        hal->ColorizeLed(1, CRGB::FromTemperature(0, 3.3, voltages[1]));
        hal->ColorizeLed(2, CRGB::FromTemperature(0, 3.3, voltages[2]));
        hal->ColorizeLed(3, CRGB::FromTemperature(0, 3.3, voltages[3]));
        hal->SetAnalogOutput(this->setpointVoltageOut);
    }
    else if(experimentMode==ExperimentMode::closedloop_ptn){
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
        hal->SetAnalogOutput(this->setpointVoltageOut);
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

ErrorCode DeviceManager::TriggerHeaterExperimentClosedLoop(float setpointTemperature, float setpointFan, float KP, float TN, float TV, bool reset, HeaterExperimentData *data){
    //Trigger
    this->lastExperimentTrigger=hal->GetMillis();
    this->heaterReset=reset;
    this->experimentMode=ExperimentMode::closedloop_heater;
    //New Setpoints
    this->setpointTemperature=setpointTemperature;
    this->setpointFan2=setpointFan;
    this->heaterKP=KP;
    this->heaterTN_secs=TN;
    this->heaterTV_secs=TV;
    //Fill return data
    data->Fan=hal->GetFanState();
    data->Heater=hal->GetHeaterState();
    hal->GetHeaterTemperature(&(data->ActualTemperature));
    data->SetpointTemperature=this->setpointTemperature;
    return ErrorCode::OK;
}
ErrorCode DeviceManager::TriggerHeaterExperimentOpenLoop(float setpointHeater, float setpointFan, HeaterExperimentData *data){
    //Trigger
    this->lastExperimentTrigger=hal->GetMillis();
    this->experimentMode=ExperimentMode::openloop_heater;
    //New Setpoints
    this->setpointHeater=setpointHeater;
    this->setpointFan2=setpointFan;
    //Fill return data
    data->Fan=hal->GetFanState();
    data->Heater=hal->GetHeaterState();
    hal->GetHeaterTemperature(&(data->ActualTemperature));
    data->SetpointTemperature=0;
    return ErrorCode::OK;
}
ErrorCode DeviceManager::TriggerHeaterExperimentFunctionblock(HeaterExperimentData *data){
    //Trigger
    this->lastExperimentTrigger=hal->GetMillis();
    this->experimentMode=ExperimentMode::functionblock;
    //Fill return data
    data->Fan=hal->GetFanState();
    data->Heater=hal->GetHeaterState();
    hal->GetHeaterTemperature(&(data->ActualTemperature));
    data->SetpointTemperature=0;
    return ErrorCode::OK;
}
ErrorCode DeviceManager::TriggerPtnExperimentClosedLoop(float setpoint, float KP, float TN, float TV, bool reset, float **data){
    //Trigger
    this->lastExperimentTrigger=hal->GetMillis();
    this->ptnReset=reset;
    this->experimentMode=ExperimentMode::closedloop_ptn;
    //New Setpoints
    this->setpointPtn=setpoint;
    this->ptnKP=KP;
    this->ptnTN_secs=TN;
    this->ptnTV_secs=TV;
    //Fill return data
    hal->GetAnalogInputs(data);
    return ErrorCode::OK;
}
ErrorCode DeviceManager::TriggerPtnExperimentOpenLoop(float setpoint, float **data){
    //Trigger
    this->lastExperimentTrigger=hal->GetMillis();
    this->experimentMode=ExperimentMode::openloop_ptn;
    //New Setpoints
    this->setpointVoltageOut=setpoint;
    
    //Fill return data
    hal->GetAnalogInputs(data);
    return ErrorCode::OK;
}
ErrorCode DeviceManager::TriggerPtnExperimentFunctionblock(float **data){
    //Trigger
    this->lastExperimentTrigger=hal->GetMillis();
    this->experimentMode=ExperimentMode::functionblock;
    //Fill return data
    hal->GetAnalogInputs(data);
    return ErrorCode::OK;
}
ErrorCode DeviceManager::TriggerAirspeedExperimentClosedLoop(float setpointAirspeed, float setpointServo1, float KP, float TN, float TV, bool reset, AirspeedExperimentData *data){
    //Trigger
    this->lastExperimentTrigger=hal->GetMillis();
    this->airspeedReset=reset;
    this->experimentMode=ExperimentMode::closedloop_airspeed;
    //New Setpoints
    this->setpointAirspeed=setpointAirspeed;
    this->setpointServo1=setpointServo1;
    this->airspeedKP=KP;
    this->airspeedTN_secs=TN;
    this->airspeedTV_secs=TV;
    //Fill return data 
    data->Fan=hal->GetFanState();
    hal->GetAirSpeed(&(data->ActualAirspeed));
    data->SetpointAirspeed=setpointAirspeed;
    data->Servo=this->setpointServo1;
    return ErrorCode::OK;
}
ErrorCode DeviceManager::TriggerAirspeedExperimentOpenLoop(float setpointFan2, float setpointServo1, AirspeedExperimentData *data){
    //Trigger
    this->lastExperimentTrigger=hal->GetMillis();
    this->experimentMode=ExperimentMode::openloop_heater;
    //New Setpoints
    this->setpointFan2=setpointFan2;
    this->setpointServo1=setpointServo1;
    //Fill return data
    data->Fan=hal->GetFanState();
    hal->GetAirSpeed(&(data->ActualAirspeed));
    data->SetpointAirspeed=setpointAirspeed;
    data->Servo=this->setpointServo1;
    return ErrorCode::OK;
}
ErrorCode DeviceManager::TriggerAirspeedExperimentFunctionblock(AirspeedExperimentData *data){
    //Trigger
    this->lastExperimentTrigger=hal->GetMillis();
    this->experimentMode=ExperimentMode::functionblock;
    //Fill return data
    data->Fan=hal->GetFanState();
    hal->GetAirSpeed(&(data->ActualAirspeed));
    data->SetpointAirspeed=setpointAirspeed;
    data->Servo=this->setpointServo1;
    return ErrorCode::OK;
}
ErrorCode DeviceManager::TriggerBorisUDP(uint8_t *requestU8, size_t requestLen, uint8_t* responseU8, size_t& responseLen){
    //Trigger
    this->lastExperimentTrigger=hal->GetMillis();
    this->experimentMode=ExperimentMode::boris_udp;
    if((uint32_t)requestU8%4!=0){
        ESP_LOGE(TAG, "Data Buffer not aligned property");
        return ErrorCode::GENERIC_ERROR;
    }
    if((uint32_t)responseU8%4!=0){
        ESP_LOGE(TAG, "Response Buffer not aligned property");
        return ErrorCode::GENERIC_ERROR;
    }

    if(responseLen<sizeof(MessageInputDataLabAtHome)){
        ESP_LOGE(TAG, "Response Buffer (%d) is too small for the response(%d)", responseLen, sizeof(MessageInputDataLabAtHome));
        return ErrorCode::GENERIC_ERROR;
    }
   

    uint32_t* requestU32 = (uint32_t*) requestU8;
    uint32_t messageType = requestU32[0];
    
    switch (messageType)
    {
        case MESSAGE_TYPE_CONFIG:{
            hal->UpdatePinConfiguration(requestU8, requestLen);
        }
        break;
        case MESSAGE_TYPE_OUTPUTDATA_LABATHOME:{
             if(requestLen!=sizeof(MessageOutputDataLabAtHome)){
                ESP_LOGE(TAG, "requestLen %d!=sizeof(MessageOutputDataLabAtHome)", requestLen);
                return ErrorCode::INDEX_OUT_OF_BOUNDS;
            }
            MessageOutputDataLabAtHome* output = (MessageOutputDataLabAtHome*)requestU8;
            
            LOGD(TAG, "Got a MESSAGE_TYPE_OUTPUTDATA_LABATHOME with DutyFan1 %f RelayK3 %d LED0 %lu", output->DutyFan1Percent, output->RelayK3, output->LED0);
           

            if(!std::isnan(output->AnalogOutputVolts)){
                hal->SetAnalogOutput(output->AnalogOutputVolts);
            }
            if(!std::isnan(output->AngleServo1Degress)){
                hal->SetServo1Position(output->AngleServo1Degress);
            }
            if(!std::isnan(output->AngleServo2Degress)){
                hal->SetServo1Position(output->AngleServo2Degress);
            }
            if(!std::isnan(output->DutyFan1Percent)){
                hal->SetFanDuty(output->DutyFan1Percent);
            }
            
            
            if(!std::isnan(output->DutyFan2Percent)){
                //Data from WinFact still has this field hal->SetFan2Duty(output->DutyFan2Percent);
            }
            
            if(!std::isnan(output->DutyHeaterPercent)){
                hal->SetHeaterDuty(output->DutyHeaterPercent);
            }
            if(!std::isnan(output->DutyPowerLedPercent)){
                hal->SetLedPowerWhiteDuty(output->DutyPowerLedPercent);
            }
            hal->ColorizeLed(0, output->LED0);
            hal->ColorizeLed(1, output->LED1);
            hal->ColorizeLed(2, output->LED2);
            hal->ColorizeLed(3, output->LED3);
            if(output->RelayK3!=0xFF){
                hal->SetRelayState(output->RelayK3);
            }
            if(output->SoundIsValid == VALID){
                hal->SetSound(output->SoundValue);
            }
            if(!std::isnan(output->UsbSupplyVoltageVolts)){
                ESP_LOGE(TAG, "output->UsbSupplyVoltageVolts not yet implemented");
            }
            MessageInputDataLabAtHome* input = (MessageInputDataLabAtHome*)responseU8;
            float* analogInputs{nullptr};
            hal->GetCO2PPM(&input->AirCo2PPM);
            hal->GetAirPressure(&input->AirPressurePa);
            hal->GetAirPressure(&input->AirQualityPercent);
            hal->GetAmbientBrightness(&input->AmbientBrightnessLux);
            hal->GetAnalogInputs(&analogInputs);
            input->AnalogInputVolt=analogInputs[0];
            input->ButtonGreen=hal->GetButtonGreenIsPressed();
            input->ButtonRed=hal->GetButtonRedIsPressed();
            input->ButtonYellow=hal->GetButtonEncoderIsPressed();
            //hal->GetFanRpm(&input->Fan1RotationsRpM);
            hal->GetHeaterTemperature(&input->HeaterTemperatureDegCel);
            hal->GetEncoderValue(&input->IncrementalEncoderDetents);
            input->IncrementalEncoderIsValid=0x01;
            input->MessageType=MESSAGE_TYPE_INPUTDATA_LABATHOME;
            input->MovementSensor=hal->IsMovementDetected();
            input->SoundIsValid=0x01;
            int32_t songNumber;
            hal->GetSound(&songNumber);
            input->SoundValue=(BorisInt)songNumber;
            input->UsbSupplyVoltageVolts=hal->GetUSBCVoltage();
            hal->GetWifiRssiDb(&input->WifiSignalStrengthDB);
            responseLen=sizeof(MessageInputDataLabAtHome);
         
        }
        break;
        case MESSAGE_TYPE_OUTPUTDATA_PTNCHEN:{
             if(requestLen!=sizeof(MessageOutputDataPtnchen)){
                ESP_LOGE(TAG, "requestLen %d!=sizeof(MessageOutputDataPtnchen)", requestLen);
                return ErrorCode::INDEX_OUT_OF_BOUNDS;
            }
            MessageOutputDataPtnchen* output = (MessageOutputDataPtnchen*)requestU8;
            
            LOGD(TAG, "Got a MESSAGE_TYPE_OUTPUTDATA_Ptnchen with Output %f LED0 %lu", output->AnalogOutputVolts, output->LED0);
            if(!std::isnan(output->AnalogOutputVolts)){
                hal->SetAnalogOutput(output->AnalogOutputVolts);
            }
            hal->ColorizeLed(0, output->LED0);
            hal->ColorizeLed(1, output->LED1);
            hal->ColorizeLed(2, output->LED2);
            hal->ColorizeLed(3, output->LED3);
            
            MessageInputDataPtnchen* input = (MessageInputDataPtnchen*)responseU8;
            float* analogInputs{nullptr};
            hal->GetAnalogInputs(&analogInputs);
            bool button = hal->GetButtonRedIsPressed();
            input->Button=button;
            input->INPUT=analogInputs[0];
            input->MessageType=MESSAGE_TYPE_INPUTDATA_PTNCHEN;
            input->PTN1=analogInputs[1];
            input->PTN2=analogInputs[2];
            input->PTN3=analogInputs[3];
            responseLen=sizeof(MessageInputDataPtnchen);
        }
        break;
    }
    return ErrorCode::OK;
}