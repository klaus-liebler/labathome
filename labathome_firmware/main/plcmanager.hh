#pragma once
#include <stdio.h>
#include "esp_system.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "HAL.hh"
#include "labathomeerror.hh"
#include <vector>
#include <cstring>


class FunctionBlock;
class PID;

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
        
        virtual LabAtHomeErrorCode  GetBinaryAsPointer(size_t index, bool *value)=0;
        virtual LabAtHomeErrorCode  SetBinary(size_t index, bool value)=0;
        virtual bool  GetBinary(size_t index)=0;
        

        virtual LabAtHomeErrorCode  GetIntegerAsPointer(size_t index, int *value)=0;
        virtual LabAtHomeErrorCode  SetInteger(size_t index, int value)=0;
        virtual int  GetInteger(size_t index)=0;
        
        virtual LabAtHomeErrorCode  GetColorAsPointer(size_t index, uint32_t *value)=0;
        virtual LabAtHomeErrorCode  SetColor(size_t index, uint32_t value)=0;
        virtual uint32_t  GetColor(size_t index)=0;
        

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
        PID *heaterPIDController;
        PID *airspeedPIDController;
        Executable *currentExecutable;
        Executable *nextExecutable;
        Executable* createInitialExecutable();
        int64_t lastExperimentTrigger=0;
        ExperimentMode experimentMode;
        double heaterKP=0; double heaterKI=0; double heaterKD=0;
        double airspeedKP=0; double airspeedKI=0; double airspeedKD=0;
        double actualTemperature=0;
        double setpointTemperature=0;
        double actualAirspeed=0;
        double setpointAirspeed=0;
        double setpointFan1=0;
        double setpointFan2=0;
        double setpointServo1=0;
        double setpointHeaterOpenLoop=0;
        double setpointHeaterClosedLoop=0;
        
    public:
        bool IsBinaryAvailable(size_t index);
        bool IsIntegerAvailable(size_t index);
        bool IsDoubleAvailable(size_t index);
        bool IsColorAvailable(size_t index);
        LabAtHomeErrorCode  GetBinaryAsPointer(size_t index, bool *value);
        LabAtHomeErrorCode  SetBinary(size_t index, bool value);
        bool  GetBinary(size_t index);
        
        LabAtHomeErrorCode  GetIntegerAsPointer(size_t index, int *value);
        LabAtHomeErrorCode  SetInteger(size_t index, int value);
        int  GetInteger(size_t index);
        
        LabAtHomeErrorCode  GetColorAsPointer(size_t index, uint32_t *value);
        LabAtHomeErrorCode  SetColor(size_t index, uint32_t value);
        uint32_t GetColor(size_t index);
        
       
        int64_t GetMicroseconds();
        LabAtHomeErrorCode ParseNewExecutableAndEnqueue(const uint8_t  *buffer, size_t length);
        HAL *GetHAL();
        
        PLCManager(HAL *hal);
        LabAtHomeErrorCode Init();
 
        LabAtHomeErrorCode CheckForNewExecutable();
        LabAtHomeErrorCode Loop();
        LabAtHomeErrorCode TriggerAirspeedExperimentClosedLoop(double setpointAirspeed, double setpointServo1, double KP, double KI, double KD, AirspeedExperimentData *data);
        LabAtHomeErrorCode TriggerAirspeedExperimentOpenLoop(double setpointFan2, double setpointServo1, AirspeedExperimentData *data);
        LabAtHomeErrorCode TriggerAirspeedExperimentFunctionblock(AirspeedExperimentData *data);
        LabAtHomeErrorCode TriggerHeaterExperimentClosedLoop(double setpointTemperature, double setpointFan, double KP, double KI, double KD, HeaterExperimentData *data);
        LabAtHomeErrorCode TriggerHeaterExperimentOpenLoop(double setpointHeater, double setpointFan, HeaterExperimentData *data);
        LabAtHomeErrorCode TriggerHeaterExperimentFunctionblock(HeaterExperimentData *data);
        LabAtHomeErrorCode GetDebugInfoSize(size_t *sizeInBytes);
        LabAtHomeErrorCode GetDebugInfo(uint8_t *buffer, size_t maxSizeInByte);
        
        
};