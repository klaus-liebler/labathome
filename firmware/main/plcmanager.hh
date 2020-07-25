#pragma once
#include <stdio.h>
#include "esp_system.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "BSP.hh"
#include "labathomeerror.hh"

class FunctionBlock;

class Executable
{
    public:
        FunctionBlock **functionBlocks;
        size_t functionBlocksSize;
        uint32_t *binaries; //beyond inpuits/outputs
        size_t binariesSize; //beyond inpuits/outputs
        int *integers;
        size_t integersSize; //beyond inpuits/outputs
        Executable(FunctionBlock **functionBlocks, size_t functionBlocksSize, uint32_t *binaries, size_t binariesSize, int *integers, size_t integersSize):
            functionBlocks(functionBlocks), functionBlocksSize(functionBlocksSize), 
            binaries(binaries),binariesSize(binariesSize),
            integers(integers), integersSize(integersSize)
        {

        }
        ~Executable()
        {
            delete[] functionBlocks;
            delete[] binaries;
            delete[] integers;
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
};

class FunctionBlock {
   public:
      virtual LabAtHomeErrorCode initPhase1(FBContext *ctx){return LabAtHomeErrorCode::OK;};
      virtual LabAtHomeErrorCode initPhase2(FBContext *ctx){return LabAtHomeErrorCode::OK;};
      virtual LabAtHomeErrorCode initPhase3(FBContext *ctx){return LabAtHomeErrorCode::OK;};
      virtual LabAtHomeErrorCode execute(FBContext *ctx)=0;
      virtual LabAtHomeErrorCode deinit(FBContext *ctx){return LabAtHomeErrorCode::OK;}
      virtual ~FunctionBlock(){};
};

class PLCManager:public FBContext
{
 private:
        QueueHandle_t execQueue;
        Executable *currentExecutable;
        BSP *bsp;
        Executable* createInitialExecutable();
    public:
        bool IsBinaryAvailable(size_t index);
        bool IsIntegerAvailable(size_t index);
        bool IsDoubleAvailable(size_t index);
        LabAtHomeErrorCode  GetBinaryAsPointer(size_t index, bool *value);
        bool  GetBinary(size_t index);
        LabAtHomeErrorCode  SetBinary(size_t index, bool value);

        LabAtHomeErrorCode CompileProtobufConfig2ExecutableAndEnqueue(char* pb, size_t length);

        
        PLCManager(BSP *bsp): bsp(bsp)
        {
            execQueue=xQueueCreate( 2, sizeof(Executable*) );
            currentExecutable = this->createInitialExecutable();
        }
        LabAtHomeErrorCode CheckForNewExecutable();
        LabAtHomeErrorCode Loop();
};