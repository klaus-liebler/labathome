#pragma once
#include "labathomeerror.hh"
#include <stdio.h>
#include "plcmanager.hh"

class FB_GreenButton: public FunctionBlock{
    size_t output;
    public:
        LabAtHomeErrorCode execute(FBContext *ctx){
            bool value=ctx->GetHAL()->GetButtonGreenIsPressed();
            ctx->SetBinary(output, value);
            return LabAtHomeErrorCode::OK;
        }
        FB_GreenButton(uint32_t IdOnWebApp, size_t output):FunctionBlock(IdOnWebApp), output(output){}
        ~FB_GreenButton(){}
};

class FB_EncoderButton: public FunctionBlock{
    size_t output;
    public:
        LabAtHomeErrorCode execute(FBContext *ctx){
            bool value=ctx->GetHAL()->GetButtonEncoderIsPressed();
            ctx->SetBinary(output, value);
            return LabAtHomeErrorCode::OK;
        }
        FB_EncoderButton(uint32_t IdOnWebApp, size_t output):FunctionBlock(IdOnWebApp), output(output){}
        ~FB_EncoderButton(){}
};

class FB_RedButton: public FunctionBlock{
    size_t output;
    public:
        LabAtHomeErrorCode execute(FBContext *ctx){
            bool value=ctx->GetHAL()->GetButtonRedIsPressed();
            ctx->SetBinary(output, value);
            return LabAtHomeErrorCode::OK;
        }
        FB_RedButton(uint32_t IdOnWebApp, size_t output):FunctionBlock(IdOnWebApp), output(output){}
        ~FB_RedButton(){}
};

class FB_GreenLED: public FunctionBlock{
    size_t input;
    public:
        LabAtHomeErrorCode execute(FBContext *ctx){
            ctx->GetHAL()->ColorizeLed(LED::LED_GREEN, ctx->GetBinary(input)?CRGB::DarkGreen:CRGB::Black);
            return LabAtHomeErrorCode::OK;;
        }
        FB_GreenLED(uint32_t IdOnWebApp, size_t input):FunctionBlock(IdOnWebApp), input(input){}
        ~FB_GreenLED(){}
};

class FB_YellowLED: public FunctionBlock{
    size_t input;
    public:
        LabAtHomeErrorCode execute(FBContext *ctx){
            ctx->GetHAL()->ColorizeLed(LED::LED_YELLOW, ctx->GetBinary(input)?CRGB::Yellow:CRGB::Black);
            return LabAtHomeErrorCode::OK;;
        }
        FB_YellowLED(uint32_t IdOnWebApp, size_t input):FunctionBlock(IdOnWebApp), input(input){}
        ~FB_YellowLED(){}
};

class FB_RedLED: public FunctionBlock{
    size_t input;
    public:
        LabAtHomeErrorCode execute(FBContext *ctx){
            ctx->GetHAL()->ColorizeLed(LED::LED_RED, ctx->GetBinary(input)?CRGB::DarkRed:CRGB::Black);
            return LabAtHomeErrorCode::OK;;
        }
        FB_RedLED(uint32_t IdOnWebApp, size_t input):FunctionBlock(IdOnWebApp), input(input){}
        ~FB_RedLED(){}
};

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
        FB_AND2(uint32_t IdOnWebApp, size_t inputA, size_t inputB, size_t output):FunctionBlock(IdOnWebApp), inputA(inputA), inputB(inputB), output(output){}
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
        FB_OR2(uint32_t IdOnWebApp, size_t inputA, size_t inputB, size_t output):FunctionBlock(IdOnWebApp), inputA(inputA), inputB(inputB), output(output){}
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
        FB_RS(uint32_t IdOnWebApp, size_t inputR, size_t inputS, size_t output):FunctionBlock(IdOnWebApp), inputR(inputR), inputS(inputS), output(output), state(false){}
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
        FB_TON(uint32_t IdOnWebApp, size_t input, size_t output, uint32_t presetTime_secs):FunctionBlock(IdOnWebApp), input(input), output(output), presetTime_secs(presetTime_secs){}
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
        FB_NOT(uint32_t IdOnWebApp, size_t input, size_t output):FunctionBlock(IdOnWebApp), input(input), output(output){}
        ~FB_NOT(){}
};