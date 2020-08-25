#pragma once
#include "labathomeerror.hh"
#include <stdio.h>
#include "plcmanager.hh"


class FB_AND2: public FunctionBlock{
    size_t inputA;
    size_t inputB;
    size_t output;
    
    public:
        LabAtHomeErrorCode execute(FBContext *ctx)
        {
            ctx->SetBinary(this->output, ctx->GetBinary(inputA) & ctx->GetBinary(inputB));
            return LabAtHomeErrorCode::OK;;
        }
        FB_AND2(size_t inputA, size_t inputB, size_t output):inputA(inputA), inputB(inputB), output(output){}
        ~FB_AND2(){}
};

class FB_OR2: public FunctionBlock{
    size_t inputA;
    size_t inputB;
    size_t output;
    
    public:
        LabAtHomeErrorCode execute(FBContext *ctx)
        {
            ctx->SetBinary(this->output, ctx->GetBinary(inputA) | ctx->GetBinary(inputB));
            return LabAtHomeErrorCode::OK;;
        }
        FB_OR2(size_t inputA, size_t inputB, size_t output):inputA(inputA), inputB(inputB), output(output){}
        ~FB_OR2(){}
};

class FB_RS: public FunctionBlock{
    size_t inputR;
    size_t inputS;
    size_t output;
    bool state;
    
    public:
        LabAtHomeErrorCode execute(FBContext *ctx)
        {
            if(ctx->GetBinary(inputR)) state=false;
            else if(ctx->GetBinary(inputS)) state = true;
            ctx->SetBinary(this->output, state);
            return LabAtHomeErrorCode::OK;
        }
        FB_RS(size_t inputR, size_t inputS, size_t output):inputR(inputR), inputS(inputS), output(output), state(false){}
        ~FB_RS(){}
};

class FB_TON: public FunctionBlock{
    //TODO: Elapsed Output!
    size_t input;
    size_t output;
    uint32_t presetTime_secs;
    int64_t switchOnAtMicrosecond = INT64_MAX;
    bool lastInputValue;
    
    public:
        LabAtHomeErrorCode execute(FBContext *ctx)
        {
            bool currentInputValue = ctx->GetBinary(this->input);
            auto time = ctx->GetMicroseconds();
            if(lastInputValue==false && currentInputValue==true)
            {
                switchOnAtMicrosecond = time + 1000000*this->presetTime_secs;
            }
            else if(currentInputValue==false)
            {
                switchOnAtMicrosecond = INT64_MAX;
            }
            lastInputValue=currentInputValue;
            ctx->SetBinary(output, time>=switchOnAtMicrosecond);
            return LabAtHomeErrorCode::OK;
        }
        FB_TON(size_t input, size_t output, uint32_t presetTime_secs):input(input), output(output), presetTime_secs(presetTime_secs){}
        ~FB_TON(){}
};

class FB_NOT: public FunctionBlock{
    size_t input;
    size_t output;
    
    public:
        LabAtHomeErrorCode execute(FBContext *ctx)
        {
            ctx->SetBinary(this->output, !ctx->GetBinary(input));
            return LabAtHomeErrorCode::OK;
        }
        FB_NOT(size_t input, size_t output):input(input), output(output){}
        ~FB_NOT(){}
};