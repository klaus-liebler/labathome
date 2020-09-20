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

class Executable
{
    public:
        std::vector<FunctionBlock*> functionBlocks;
        std::vector<bool> binaries;
        std::vector<int> integers;
        Executable(std::vector<FunctionBlock*> functionBlocks, std::vector<bool> binaries, std::vector<int> integers):
            functionBlocks(functionBlocks), binaries(binaries), integers(integers)
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

class PLCManager:public FBContext
{
 private:
        HAL *hal;
        Executable *currentExecutable;
        Executable *nextExecutable;
        Executable* createInitialExecutable();
    public:
        bool IsBinaryAvailable(size_t index);
        bool IsIntegerAvailable(size_t index);
        bool IsDoubleAvailable(size_t index);
        LabAtHomeErrorCode  GetBinaryAsPointer(size_t index, bool *value);
        bool  GetBinary(size_t index);
        LabAtHomeErrorCode  SetBinary(size_t index, bool value);
        int64_t GetMicroseconds();
        LabAtHomeErrorCode CompileExampleConfig2ExecutableAndEnqueue(char* pb, size_t length);
        HAL *GetHAL();
        
        PLCManager(HAL *hal): hal(hal)
        {
            currentExecutable = this->createInitialExecutable();
            nextExecutable = nullptr;
        }
        LabAtHomeErrorCode CheckForNewExecutable();
        LabAtHomeErrorCode Loop();
};