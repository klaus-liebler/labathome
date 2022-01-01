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

#define ALL4  __attribute__ ((aligned (16)))


class FunctionBlock;
class PIDController;

class Executable
{
    public:
        uint32_t hash;
        size_t debugSizeBytes;
        std::vector<FunctionBlock*> functionBlocks;
        std::vector<bool> binaries;
        std::vector<int> integers;
        std::vector<double> floats;
        std::vector<uint32_t> colors;
        Executable(uint32_t hash, size_t debugSizeBytes, std::vector<FunctionBlock*> functionBlocks, std::vector<bool> binaries, std::vector<int> integers, std::vector<double> floats, std::vector<uint32_t> colors):
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
        virtual bool IsDoubleAvailable(size_t index)=0;
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
    closedloop_airspeed,
};

class PLCManager:public FBContext
{
 private:
        HAL *hal;
        PIDController *heaterPIDController;
        PIDController *airspeedPIDController;
        Executable *currentExecutable;
        Executable *nextExecutable;
        Executable* createInitialExecutable();
        uint32_t lastExperimentTrigger=0;
        ExperimentMode experimentMode;
        double heaterKP=0; double heaterTN=0; double heaterTV=0;
        double airspeedKP=0; double airspeedTN=0; double airspeedTV=0;
        double actualTemperature=0;
        double setpointTemperature=0;
        double actualAirspeed=0;
        double setpointAirspeed=0;
        double setpointFan1=0;
        double setpointFan2=0;
        double setpointServo1=0;
        double setpointHeater=0;

        void LoopPID(int64_t nowUs);
        
    public:
        bool IsBinaryAvailable(size_t index);
        bool IsIntegerAvailable(size_t index);
        bool IsDoubleAvailable(size_t index);
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
        
       
        int64_t GetMicroseconds();
        ErrorCode ParseNewExecutableAndEnqueue(const uint8_t  *buffer, size_t length);
        HAL *GetHAL();
        
        PLCManager(HAL *hal);
        ErrorCode Init();
 
        ErrorCode CheckForNewExecutable();
        ErrorCode Loop();
        ErrorCode TriggerAirspeedExperimentClosedLoop(double setpointAirspeed, double setpointServo1, double KP, double TN, double TV, AirspeedExperimentData *data);
        ErrorCode TriggerAirspeedExperimentOpenLoop(double setpointFan2, double setpointServo1, AirspeedExperimentData *data);
        ErrorCode TriggerAirspeedExperimentFunctionblock(AirspeedExperimentData *data);
        ErrorCode TriggerHeaterExperimentClosedLoop(double setpointTemperature, double setpointFan, double KP, double TN, double TV, HeaterExperimentData *data);
        ErrorCode TriggerHeaterExperimentOpenLoop(double setpointHeater, double setpointFan, HeaterExperimentData *data);
        ErrorCode TriggerHeaterExperimentFunctionblock(HeaterExperimentData *data);
        ErrorCode GetDebugInfoSize(size_t *sizeInBytes);
        ErrorCode GetDebugInfo(uint8_t *buffer, size_t maxSizeInByte);
        
        
};