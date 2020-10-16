#pragma once
#include "labathomeerror.hh"
#include <stdio.h>
#include "plcmanager.hh"
#include "math.h"

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

class FB_AmbientBrigthnessSensor:public FunctionBlock{
    size_t output;
    public:
        LabAtHomeErrorCode execute(FBContext *ctx){
            float value;
            ctx->GetHAL()->GetAmbientBrightness(&value);
            ctx->SetInteger(output, value);
            return LabAtHomeErrorCode::OK;
        }
        FB_AmbientBrigthnessSensor(uint32_t IdOnWebApp, size_t output):FunctionBlock(IdOnWebApp), output(output){}
        ~FB_AmbientBrigthnessSensor(){}
};

class FB_HeaterTemperatureSensor:public FunctionBlock{
    size_t output;
public:
    LabAtHomeErrorCode execute(FBContext *ctx){
        float value;
        ctx->GetHAL()->GetHeaterTemperature(&value);
        ctx->SetInteger(output, value);
        return LabAtHomeErrorCode::OK;
    }
    FB_HeaterTemperatureSensor(uint32_t IdOnWebApp, size_t output):FunctionBlock(IdOnWebApp), output(output){}
    ~FB_HeaterTemperatureSensor(){}
};

class FB_MovementSensor:public FunctionBlock{
    size_t output;
public:
    LabAtHomeErrorCode execute(FBContext *ctx){
        bool value=ctx->GetHAL()->IsMovementDetected();
        ctx->SetBinary(output, value);
        return LabAtHomeErrorCode::OK;
    }
    FB_MovementSensor(uint32_t IdOnWebApp, size_t output):FunctionBlock(IdOnWebApp), output(output){}
    ~FB_MovementSensor(){}
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

class FB_LED3: public FunctionBlock{
    size_t input;
public:
    LabAtHomeErrorCode execute(FBContext *ctx){
        ctx->GetHAL()->ColorizeLed(LED::LED_3, ctx->GetColor(input));
        return LabAtHomeErrorCode::OK;
    }
    FB_LED3(uint32_t IdOnWebApp, size_t input):FunctionBlock(IdOnWebApp), input(input){}
    ~FB_LED3(){}
};

class FB_LED4: public FunctionBlock{
    size_t input;
public:
    LabAtHomeErrorCode execute(FBContext *ctx){
        ctx->GetHAL()->ColorizeLed(LED::LED_4, ctx->GetColor(input));
        return LabAtHomeErrorCode::OK;
    }
    FB_LED4(uint32_t IdOnWebApp, size_t input):FunctionBlock(IdOnWebApp), input(input){}
    ~FB_LED4(){}
};

class FB_LED5: public FunctionBlock{
    size_t input;
    public:
        LabAtHomeErrorCode execute(FBContext *ctx){
        ctx->GetHAL()->ColorizeLed(LED::LED_5, ctx->GetColor(input));
        return LabAtHomeErrorCode::OK;
        }
        FB_LED5(uint32_t IdOnWebApp, size_t input):FunctionBlock(IdOnWebApp), input(input){}
        ~FB_LED5(){}
};

class FB_LED6: public FunctionBlock{
    size_t input;
    public:
        LabAtHomeErrorCode execute(FBContext *ctx){
        ctx->GetHAL()->ColorizeLed(LED::LED_6, ctx->GetColor(input));
        return LabAtHomeErrorCode::OK;
        }
        FB_LED6(uint32_t IdOnWebApp, size_t input):FunctionBlock(IdOnWebApp), input(input){}
        ~FB_LED6(){}
};

class FB_LED7: public FunctionBlock{
    size_t input;
    public:
        LabAtHomeErrorCode execute(FBContext *ctx){
            ctx->GetHAL()->ColorizeLed(LED::LED_7, ctx->GetColor(input));
            return LabAtHomeErrorCode::OK;
        }
        FB_LED7(uint32_t IdOnWebApp, size_t input):FunctionBlock(IdOnWebApp), input(input){}
        ~FB_LED7(){}
};

class FB_Relay: public FunctionBlock{
    size_t input;
    public:
        LabAtHomeErrorCode execute(FBContext *ctx){
            ctx->GetHAL()->SetRelayState(ctx->GetBinary(input));
            return LabAtHomeErrorCode::OK;;
        }
        FB_Relay(uint32_t IdOnWebApp, size_t input):FunctionBlock(IdOnWebApp), input(input){}
        ~FB_Relay(){}
};

class FB_Bool2ColorConverter: public FunctionBlock{
    size_t input;
    size_t output;
    uint32_t colorOnTRUE;
    uint32_t colorOnFALSE;
    
    public:
        LabAtHomeErrorCode execute(FBContext *ctx)
        {
            ctx->SetColor(this->output, ctx->GetBinary(input)?colorOnTRUE:colorOnFALSE);
            return LabAtHomeErrorCode::OK;;
        }
        FB_Bool2ColorConverter(uint32_t IdOnWebApp, size_t input, size_t output, uint32_t colorOnTRUE, uint32_t colorOnFALSE=0):
            FunctionBlock(IdOnWebApp), input(input), output(output), colorOnTRUE(colorOnTRUE), colorOnFALSE(colorOnFALSE){}
        ~FB_Bool2ColorConverter(){}
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

class FB_GT: public FunctionBlock{
    size_t inputA;
    size_t inputB;
    size_t output;
    
    public:
        LabAtHomeErrorCode execute(FBContext *ctx)
        {
            ctx->SetBinary(this->output, ctx->GetInteger(inputA) > ctx->GetInteger(inputB));
            return LabAtHomeErrorCode::OK;;
        }
        FB_GT(uint32_t IdOnWebApp, size_t inputA, size_t inputB, size_t output):FunctionBlock(IdOnWebApp), inputA(inputA), inputB(inputB), output(output){}
        ~FB_GT(){}
};

class FB_LT: public FunctionBlock{
    size_t inputA;
    size_t inputB;
    size_t output;
    
    public:
        LabAtHomeErrorCode execute(FBContext *ctx)
        {
            ctx->SetBinary(this->output, ctx->GetInteger(inputA) < ctx->GetInteger(inputB));
            return LabAtHomeErrorCode::OK;;
        }
        FB_LT(uint32_t IdOnWebApp, size_t inputA, size_t inputB, size_t output):FunctionBlock(IdOnWebApp), inputA(inputA), inputB(inputB), output(output){}
        ~FB_LT(){}
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
    size_t inputTrigger;
    size_t inputPresetTime_msecs;
    size_t output;
    size_t outputElapsedTime_msecs;
    

    int64_t inputPositiveEdge = INT64_MAX;
    bool lastInputValue;
    
    public:
        LabAtHomeErrorCode execute(FBContext *ctx)
        {
            bool currentInputValue = ctx->GetBinary(this->inputTrigger);
            int presetTime_msecs = ctx->GetInteger(this->inputPresetTime_msecs);
            auto now = ctx->GetMicroseconds();
            if(lastInputValue==false && currentInputValue==true)
            {
                inputPositiveEdge=now;
            }
            else if(currentInputValue==false)
            {
                inputPositiveEdge=INT64_MAX;
            }
            lastInputValue=currentInputValue;
            auto elapsed = (now-inputPositiveEdge)/1000;
            ctx->SetBinary(output, elapsed>=presetTime_msecs);
            ctx->SetInteger(outputElapsedTime_msecs, elapsed);
            return LabAtHomeErrorCode::OK;
        }
        FB_TON(uint32_t IdOnWebApp, size_t inputTrigger, size_t inputPresetTime_msecs, size_t output, size_t outputElapsedTime_msecs):FunctionBlock(IdOnWebApp), inputTrigger(inputTrigger), inputPresetTime_msecs(inputPresetTime_msecs), output(output), outputElapsedTime_msecs(outputElapsedTime_msecs){}
        ~FB_TON(){}
};

class FB_TOF: public FunctionBlock{
    size_t inputTrigger;
    size_t inputPresetTime_msecs;
    size_t output;
    size_t outputElapsedTime_msecs;
    

    int64_t inputNegativeEdge = INT64_MAX;
    bool lastInputValue;
    
    public:
        LabAtHomeErrorCode execute(FBContext *ctx)
        {
            bool currentInputValue = ctx->GetBinary(this->inputTrigger);
            int presetTime_msecs = ctx->GetInteger(this->inputPresetTime_msecs);
            auto now = ctx->GetMicroseconds();
            if(lastInputValue==true && currentInputValue==false)
            {
                inputNegativeEdge=now;
            }
            else if(currentInputValue==false)
            {
                inputNegativeEdge=INT64_MIN;
            }
            lastInputValue=currentInputValue;
            auto elapsed = (now-inputNegativeEdge)/1000;
            elapsed=std::min(elapsed, (long long)presetTime_msecs);
            ctx->SetBinary(output, currentInputValue | (elapsed<presetTime_msecs));
            ctx->SetInteger(outputElapsedTime_msecs, elapsed);
            return LabAtHomeErrorCode::OK;
        }
        FB_TOF(uint32_t IdOnWebApp, size_t inputTrigger, size_t inputPresetTime_msecs, size_t output, size_t outputElapsedTime_msecs):FunctionBlock(IdOnWebApp), inputTrigger(inputTrigger), inputPresetTime_msecs(inputPresetTime_msecs), output(output), outputElapsedTime_msecs(outputElapsedTime_msecs){}
        ~FB_TOF(){}
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

class FB_ConstInteger: public FunctionBlock{
    size_t output;
    int constant;
    public:
        LabAtHomeErrorCode execute(FBContext *ctx)
        {
            ctx->SetInteger(this->output, this->constant);
            return LabAtHomeErrorCode::OK;
        }
        FB_ConstInteger(uint32_t IdOnWebApp, size_t output, int constant):FunctionBlock(IdOnWebApp), output(output), constant(constant){}
        ~FB_ConstInteger(){}
};


