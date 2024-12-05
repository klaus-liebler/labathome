#pragma once
#include <stdio.h>
#include "esp_system.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "HAL.hh"
#include "errorcodes.hh"
#include <vector>
#include <cstring>
#include <pid_t1_controller.hh>
#include <pidcontroller.hh>

#include "flatbuffers/flatbuffers.h"
#include "../generated/flatbuffers_cpp/heaterexperiment_generated.h"

//see Flowchart.ts -values must be the same
constexpr const char *FBDSTORE_BASE_DIRECTORY = "/spiffs/fbdstore/";    
constexpr const char *DEFAULT_FBD_FILEPATH =  "/spiffs/default.fbd";
constexpr const char *TEMP_FBD_FILEPATH = "/spiffs/temp.fbd";

class FunctionBlock;
class Executable
{
    public:
        uint32_t hash;
        size_t debugSizeBytes;
        std::vector<FunctionBlock*> functionBlocks;
        std::vector<bool> binaries;
        std::vector<int> integers;
        std::vector<float> floats;
        std::vector<uint32_t> colors;
        Executable(uint32_t hash, size_t debugSizeBytes, std::vector<FunctionBlock*> functionBlocks, std::vector<bool> binaries, std::vector<int> integers, std::vector<float> floats, std::vector<uint32_t> colors):
            hash(hash), debugSizeBytes(debugSizeBytes), functionBlocks(functionBlocks), binaries(binaries), integers(integers), floats(floats), colors(colors)
        {
        }
        ~Executable()
        {
            functionBlocks.clear();
            functionBlocks.shrink_to_fit();
            binaries.clear();
            binaries.shrink_to_fit();
            integers.clear();
            integers.shrink_to_fit();
            floats.clear();
            floats.shrink_to_fit();
            colors.clear();
            colors.shrink_to_fit();
        }
};


class FBContext{
    public:
        virtual bool IsBinaryAvailable(size_t index)=0;
        virtual bool IsIntegerAvailable(size_t index)=0;
        virtual bool IsFloatAvailable(size_t index)=0;
        virtual bool IsColorAvailable(size_t index)=0;
        
        virtual ErrorCode  GetBinaryAsPointer(size_t index, bool *value)=0;
        virtual ErrorCode  SetBinary(size_t index, bool value)=0;
        virtual bool  GetBinary(size_t index)=0;
        

        virtual ErrorCode  GetIntegerAsPointer(size_t index, int *value)=0;
        virtual ErrorCode  SetInteger(size_t index, int value)=0;
        virtual int  GetInteger(size_t index)=0;
        
        virtual ErrorCode  GetColorAsPointer(size_t index, uint32_t *value)=0;
        virtual ErrorCode  SetColor(size_t index, uint32_t value)=0;
        virtual uint32_t  GetColor(size_t index)=0;

        virtual ErrorCode  GetFloatAsPointer(size_t index, float *value)=0;
        virtual ErrorCode  SetFloat(size_t index, float value)=0;
        virtual float  GetFloat(size_t index)=0;
        

        virtual int64_t GetMicroseconds()=0;
        virtual HAL *GetHAL()=0;
};

class FunctionBlock {
   public:
      virtual ErrorCode initPhase1(FBContext *ctx){return ErrorCode::OK;};
      virtual ErrorCode initPhase2(FBContext *ctx){return ErrorCode::OK;};
      virtual ErrorCode initPhase3(FBContext *ctx){return ErrorCode::OK;};
      virtual ErrorCode execute(FBContext *ctx)=0;
      virtual ErrorCode deinit(FBContext *ctx){return ErrorCode::OK;}
      const uint32_t IdOnWebApp;
      FunctionBlock(uint32_t IdOnWebApp):IdOnWebApp(IdOnWebApp){}
      virtual ~FunctionBlock(){};
};

struct HeaterExperimentData{
    float Heater;
    float Fan;
    float ActualTemperature;
    float SetpointTemperature;
};

struct AirspeedExperimentData{
    float Fan;
    float Servo;
    float SetpointAirspeed;
    float ActualAirspeed;
};

struct FFTExperimentData
{
    float Magnitudes[64];
};



enum class ExperimentMode
{
    functionblock,
    openloop_heater,
    closedloop_heater,
    openloop_ptn,
    closedloop_ptn,
    closedloop_airspeed,
    boris_udp,
};

class DeviceManager:public FBContext
{
 private:
        HAL *hal;
        PID::Controller<float> *heaterPIDController;
        Executable *currentExecutable;
        Executable *nextExecutable;
        Executable* createDummyInitialExecutableAndEnqueue();
        uint32_t lastExperimentTrigger=0;
        ExperimentMode experimentMode;
        float heaterKP=0, heaterTN_secs=0, heaterTV_secs=0; bool heaterReset=true;
        float actualTemperature=0;
        float setpointTemperature=0;
        float heaterWorkingPointOffset{0};



        float setpointFan=0;
        float setpointServo1=0;
        float setpointHeater=0;
        float setpointVoltageOut=0;

        void EternalLoop();
        ErrorCode CheckForNewExecutable();
        ErrorCode Loop();
        
    public:
        bool IsBinaryAvailable(size_t index);
        bool IsIntegerAvailable(size_t index);
        bool IsFloatAvailable(size_t index);
        bool IsColorAvailable(size_t index);
    
        ErrorCode  GetBinaryAsPointer(size_t index, bool *value);
        ErrorCode  SetBinary(size_t index, bool value);
        bool  GetBinary(size_t index);
        
        ErrorCode  GetIntegerAsPointer(size_t index, int *value);
        ErrorCode  SetInteger(size_t index, int value);
        int  GetInteger(size_t index);
        
        ErrorCode  GetColorAsPointer(size_t index, uint32_t *value);
        ErrorCode  SetColor(size_t index, uint32_t value);
        uint32_t GetColor(size_t index);

        ErrorCode  GetFloatAsPointer(size_t index, float *value);
        ErrorCode  SetFloat(size_t index, float value);
        float GetFloat(size_t index);
        
       
        int64_t GetMicroseconds();
        ErrorCode ParseNewExecutableAndEnqueue(const char* path);
        HAL *GetHAL();
        
        DeviceManager(HAL *hal);
        ErrorCode InitAndRun();
        ErrorCode TriggerHeaterExperiment(const heaterexperiment::RequestHeater *r, flatbuffers::FlatBufferBuilder &b);
        ErrorCode GetDebugInfoSize(size_t *sizeInBytes);
        ErrorCode GetDebugInfo(flatbuffers::FlatBufferBuilder& b);      
};

