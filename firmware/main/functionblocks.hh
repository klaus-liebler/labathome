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
    size_t input;
    size_t output;
    uint32_t presetTime;
    bool state;
    
    public:
        LabAtHomeErrorCode execute(FBContext *ctx)
        {
            return LabAtHomeErrorCode::OK;
        }
        FB_TON(size_t input, size_t output, uint32_t presetTime):input(input), output(output), presetTime(presetTime), state(false){}
        ~FB_TON(){}
};