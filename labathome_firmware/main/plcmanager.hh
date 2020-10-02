#pragma once
#include <stdio.h>
#include "esp_system.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "HAL.hh"
#include "labathomeerror.hh"
#include "vector"


class FunctionBlock;
class PID;

class Executable
{
    public:
        std::vector<FunctionBlock*> functionBlocks;
        std::vector<bool> binaries;
        std::vector<int> integers;
        std::vector<double> floats;
        std::vector<uint32_t> colors;
        Executable(std::vector<FunctionBlock*> functionBlocks, std::vector<bool> binaries, std::vector<int> integers, std::vector<double> floats, std::vector<uint32_t> colors):
            functionBlocks(functionBlocks), binaries(binaries), integers(integers), floats(floats), colors(colors)
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
        virtual LabAtHomeErrorCode  GetBinaryAsPointer(size_t index, bool *value)=0;
        virtual bool  GetBinary(size_t index)=0;
        virtual LabAtHomeErrorCode  SetBinary(size_t index, bool value)=0;
        virtual int64_t GetMicroseconds()=0;
        virtual HAL *GetHAL()=0;
};

class FunctionBlock {
   public:
      virtual LabAtHomeErrorCode initPhase1(FBContext *ctx){return LabAtHomeErrorCode::OK;};
      virtual LabAtHomeErrorCode initPhase2(FBContext *ctx){return LabAtHomeErrorCode::OK;};
      virtual LabAtHomeErrorCode initPhase3(FBContext *ctx){return LabAtHomeErrorCode::OK;};
      virtual LabAtHomeErrorCode execute(FBContext *ctx)=0;
      virtual LabAtHomeErrorCode deinit(FBContext *ctx){return LabAtHomeErrorCode::OK;}
      const uint32_t IdOnWebApp;
      FunctionBlock(uint32_t IdOnWebApp):IdOnWebApp(IdOnWebApp){}
      virtual ~FunctionBlock(){};
};

struct ExperimentData{
    float Heater;
    float Fan;
    float ActualTemperature;
    float SetpointTemperature;
};


enum class ExperimentMode
{
    functionblock,
    openloop,
    closedloop,
};

class PLCManager:public FBContext
{
 private:
        HAL *hal;
        PID *pid;
        Executable *currentExecutable;
        Executable *nextExecutable;
        Executable* createInitialExecutable();
        int64_t lastExperimentTrigger=0;
        ExperimentMode experimentMode;
        double KP=0; double KI=0; double KD=0;
        double actualTemperature=0;
        double setpointTemperature=0;
        double setpointFan=0;
        double setpointHeaterOpenloop=0;
        double setpointHeaterClosedLoop=0;
        
    public:
        bool IsBinaryAvailable(size_t index);
        bool IsIntegerAvailable(size_t index);
        bool IsDoubleAvailable(size_t index);
        LabAtHomeErrorCode  GetBinaryAsPointer(size_t index, bool *value);
        bool  GetBinary(size_t index);
        LabAtHomeErrorCode  SetBinary(size_t index, bool value);
        int64_t GetMicroseconds();
        LabAtHomeErrorCode ParseNewExecutableAndEnqueue(const uint8_t  *buffer, size_t length);
        HAL *GetHAL();
        
        PLCManager(HAL *hal);
        LabAtHomeErrorCode CheckForNewExecutable();
        LabAtHomeErrorCode Loop();
        LabAtHomeErrorCode TriggerHeaterExperimentClosedLoop(double setpointTemperature, double setpointFan, double KP, double KI, double KD, ExperimentData *data);
        LabAtHomeErrorCode TriggerHeaterExperimentOpenLoop(double setpointHeater, double setpointFan, ExperimentData *data);
        LabAtHomeErrorCode TriggerHeaterExperimentFunctionblock(ExperimentData *data);
};