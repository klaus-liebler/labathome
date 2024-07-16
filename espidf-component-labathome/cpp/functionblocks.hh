#pragma once
#include "errorcodes.hh"
#include <stdio.h>
#include "devicemanager.hh"
#include "math.h"
#include "crgb.hh"
#include <pid_t1_controller.hh>
//Consts
class FB_ConstTrue: public FunctionBlock{
    size_t output;
    public:
        ErrorCode execute(FBContext *ctx){
            ctx->SetBinary(output, true);
            return ErrorCode::OK;
        }
        FB_ConstTrue(uint32_t IdOnWebApp, size_t output):FunctionBlock(IdOnWebApp), output(output){}
        ~FB_ConstTrue(){}
};

class FB_ConstFalse: public FunctionBlock{
    size_t output;
    public:
        ErrorCode execute(FBContext *ctx){
            ctx->SetBinary(output, false);
            return ErrorCode::OK;
        }
        FB_ConstFalse(uint32_t IdOnWebApp, size_t output):FunctionBlock(IdOnWebApp), output(output){}
        ~FB_ConstFalse(){}
};

class FB_ConstFLOAT: public FunctionBlock{
    size_t output;
    float constant;
    public:
        ErrorCode execute(FBContext *ctx)
        {
            ctx->SetFloat(this->output, this->constant);
            return ErrorCode::OK;
        }
        FB_ConstFLOAT(uint32_t IdOnWebApp, size_t output, float constant):FunctionBlock(IdOnWebApp), output(output), constant(constant){}
        ~FB_ConstFLOAT(){}
};

//Button/Knobs
class FB_GreenButton: public FunctionBlock{
    size_t output;
    public:
        ErrorCode execute(FBContext *ctx){
            bool value=ctx->GetHAL()->GetButtonGreenIsPressed();
            ctx->SetBinary(output, value);
            return ErrorCode::OK;
        }
        FB_GreenButton(uint32_t IdOnWebApp, size_t output):FunctionBlock(IdOnWebApp), output(output){}
        ~FB_GreenButton(){}
};

class FB_EncoderButton: public FunctionBlock{
    size_t output;
    public:
        ErrorCode execute(FBContext *ctx){
            bool value=ctx->GetHAL()->GetButtonEncoderIsPressed();
            ctx->SetBinary(output, value);
            return ErrorCode::OK;
        }
        FB_EncoderButton(uint32_t IdOnWebApp, size_t output):FunctionBlock(IdOnWebApp), output(output){}
        ~FB_EncoderButton(){}
};

class FB_EncoderDetents: public FunctionBlock{
    size_t output;
    int16_t previous=0;
    int count=0;
    const int maxCount=100;
    const int minCount=0;
    public:
        ErrorCode execute(FBContext *ctx){
            int current=0;
            ctx->GetHAL()->GetEncoderValue(&current);
            int16_t change = ((int16_t)current -  previous);
            change/=4;
            if(change!=0){
                count+=change;
                if(count>this->maxCount) count=maxCount;
                if(count<this->minCount) count=minCount;
                previous=current;
            }
            ctx->SetInteger(output, count);
            return ErrorCode::OK;
        }
        FB_EncoderDetents(uint32_t IdOnWebApp, size_t output):FunctionBlock(IdOnWebApp), output(output){}
        ~FB_EncoderDetents(){}
};

class FB_RedButton: public FunctionBlock{
    size_t output;
    public:
        ErrorCode execute(FBContext *ctx){
            bool value=ctx->GetHAL()->GetButtonRedIsPressed();
            ctx->SetBinary(output, value);
            return ErrorCode::OK;
        }
        FB_RedButton(uint32_t IdOnWebApp, size_t output):FunctionBlock(IdOnWebApp), output(output){}
        ~FB_RedButton(){}
};

//Sensors
class FB_AnalogInput0: public FunctionBlock{
    size_t output;
    public:
        ErrorCode execute(FBContext *ctx){
            float *ptr;
            ctx->GetHAL()->GetAnalogInputs(&ptr);
            ctx->SetFloat(output, ptr[0]);
            return ErrorCode::OK;;
        }
        FB_AnalogInput0(uint32_t IdOnWebApp, size_t output):FunctionBlock(IdOnWebApp), output(output){}
        ~FB_AnalogInput0(){}
};

class FB_AnalogInput1: public FunctionBlock{
    size_t output;
    public:
        ErrorCode execute(FBContext *ctx){
            float *ptr;
            ctx->GetHAL()->GetAnalogInputs(&ptr);
            ctx->SetFloat(output, ptr[1]);
            return ErrorCode::OK;;
        }
        FB_AnalogInput1(uint32_t IdOnWebApp, size_t output):FunctionBlock(IdOnWebApp), output(output){}
        ~FB_AnalogInput1(){}
};

class FB_AnalogInput2: public FunctionBlock{
    size_t output;
    public:
        ErrorCode execute(FBContext *ctx){
            float *ptr;
            ctx->GetHAL()->GetAnalogInputs(&ptr);
            ctx->SetFloat(output, ptr[2]);
            return ErrorCode::OK;;
        }
        FB_AnalogInput2(uint32_t IdOnWebApp, size_t output):FunctionBlock(IdOnWebApp), output(output){}
        ~FB_AnalogInput2(){}
};

class FB_AnalogInput3: public FunctionBlock{
    size_t output;
    public:
        ErrorCode execute(FBContext *ctx){
            float *ptr;
            ctx->GetHAL()->GetAnalogInputs(&ptr);
            ctx->SetFloat(output, ptr[3]);
            return ErrorCode::OK;;
        }
        FB_AnalogInput3(uint32_t IdOnWebApp, size_t output):FunctionBlock(IdOnWebApp), output(output){}
        ~FB_AnalogInput3(){}
};

class FB_AmbientBrigthnessSensor:public FunctionBlock{
    size_t output;
    public:
        ErrorCode execute(FBContext *ctx){
            float value;
            ctx->GetHAL()->GetAmbientBrightness(&value);
            ctx->SetFloat(output, value);
            return ErrorCode::OK;
        }
        FB_AmbientBrigthnessSensor(uint32_t IdOnWebApp, size_t output):FunctionBlock(IdOnWebApp), output(output){}
        ~FB_AmbientBrigthnessSensor(){}
};

class FB_AirTemperatureSensor:public FunctionBlock{
    size_t output;
    public:
        ErrorCode execute(FBContext *ctx){
            float value;
            ctx->GetHAL()->GetAirTemperature(&value);
            ctx->SetFloat(output, value);
            return ErrorCode::OK;
        }
        FB_AirTemperatureSensor(uint32_t IdOnWebApp, size_t output):FunctionBlock(IdOnWebApp), output(output){}
        ~FB_AirTemperatureSensor(){}
};

class FB_AirHumiditySensor:public FunctionBlock{
    size_t output;
    public:
        ErrorCode execute(FBContext *ctx){
            float value;
            ctx->GetHAL()->GetAirRelHumidity(&value);
            ctx->SetFloat(output, value);
            return ErrorCode::OK;
        }
        FB_AirHumiditySensor(uint32_t IdOnWebApp, size_t output):FunctionBlock(IdOnWebApp), output(output){}
        ~FB_AirHumiditySensor(){}
};

class FB_AirPressureSensor:public FunctionBlock{
    size_t output;
    public:
        ErrorCode execute(FBContext *ctx){
            float value;
            ctx->GetHAL()->GetAirPressure(&value);
            ctx->SetFloat(output, value);
            return ErrorCode::OK;
        }
        FB_AirPressureSensor(uint32_t IdOnWebApp, size_t output):FunctionBlock(IdOnWebApp), output(output){}
        ~FB_AirPressureSensor(){}
};

class FB_AirCO2Sensor:public FunctionBlock{
    size_t output;
    public:
        ErrorCode execute(FBContext *ctx){
            float value;
            ctx->GetHAL()->GetCO2PPM(&value);
            ctx->SetFloat(output, value);
            return ErrorCode::OK;
        }
        FB_AirCO2Sensor(uint32_t IdOnWebApp, size_t output):FunctionBlock(IdOnWebApp), output(output){}
        ~FB_AirCO2Sensor(){}
};

class FB_AirQualitySensor:public FunctionBlock{
    size_t output;
    public:
        ErrorCode execute(FBContext *ctx){
            float value;
            ctx->GetHAL()->GetAirQuality(&value);
            ctx->SetFloat(output, value);
            return ErrorCode::OK;
        }
        FB_AirQualitySensor(uint32_t IdOnWebApp, size_t output):FunctionBlock(IdOnWebApp), output(output){}
        ~FB_AirQualitySensor(){}
};

class FB_HeaterTemperatureSensor:public FunctionBlock{
    size_t output;
public:
    ErrorCode execute(FBContext *ctx){
        float value;
        ctx->GetHAL()->GetHeaterTemperature(&value);
        ctx->SetFloat(output, value);
        return ErrorCode::OK;
    }
    FB_HeaterTemperatureSensor(uint32_t IdOnWebApp, size_t output):FunctionBlock(IdOnWebApp), output(output){}
    ~FB_HeaterTemperatureSensor(){}
};

class FB_MovementSensor:public FunctionBlock{
    size_t output;
public:
    ErrorCode execute(FBContext *ctx){
        bool value=ctx->GetHAL()->IsMovementDetected();
        ctx->SetBinary(output, value);
        return ErrorCode::OK;
    }
    FB_MovementSensor(uint32_t IdOnWebApp, size_t output):FunctionBlock(IdOnWebApp), output(output){}
    ~FB_MovementSensor(){}
};

//Actors
class FB_RedLED: public FunctionBlock{
    size_t input;
public:
    ErrorCode execute(FBContext *ctx){
        ctx->GetHAL()->ColorizeLed(0, ctx->GetBinary(input)?CRGB::DarkRed:CRGB::Black);
        return ErrorCode::OK;;
    }
    FB_RedLED(uint32_t IdOnWebApp, size_t input):FunctionBlock(IdOnWebApp), input(input){}
    ~FB_RedLED(){}
};

class FB_YellowLED: public FunctionBlock{
    size_t input;
public:
    ErrorCode execute(FBContext *ctx){
        ctx->GetHAL()->ColorizeLed(1, ctx->GetBinary(input)?CRGB::Yellow:CRGB::Black);
        return ErrorCode::OK;;
    }
    FB_YellowLED(uint32_t IdOnWebApp, size_t input):FunctionBlock(IdOnWebApp), input(input){}
    ~FB_YellowLED(){}
};

class FB_GreenLED: public FunctionBlock{
    size_t input;
public:
    ErrorCode execute(FBContext *ctx){
        ctx->GetHAL()->ColorizeLed(2, ctx->GetBinary(input)?CRGB::DarkGreen:CRGB::Black);
        return ErrorCode::OK;;
    }
    FB_GreenLED(uint32_t IdOnWebApp, size_t input):FunctionBlock(IdOnWebApp), input(input){}
    ~FB_GreenLED(){}
};

class FB_LED3: public FunctionBlock{
    size_t input;
public:
    ErrorCode execute(FBContext *ctx){
        ctx->GetHAL()->ColorizeLed(3, ctx->GetColor(input));
        return ErrorCode::OK;
    }
    FB_LED3(uint32_t IdOnWebApp, size_t input):FunctionBlock(IdOnWebApp), input(input){}
    ~FB_LED3(){}
};

class FB_Sound: public FunctionBlock{
    size_t input;
    uint32_t melodyIndex;
    bool lastInputState = false;
public:
    ErrorCode execute(FBContext *ctx){
        bool newInputState=ctx->GetBinary(input);
        if(!lastInputState && newInputState){
            ctx->GetHAL()->SetSound(melodyIndex);
        }
        lastInputState=newInputState;

        return ErrorCode::OK;;
    }
    FB_Sound(uint32_t IdOnWebApp, size_t input, uint32_t melodyIndex):FunctionBlock(IdOnWebApp), input(input), melodyIndex(melodyIndex){}
    ~FB_Sound(){}
};

class FB_Relay: public FunctionBlock{
    size_t input;
    public:
        ErrorCode execute(FBContext *ctx){
            ctx->GetHAL()->SetRelayState(ctx->GetBinary(input));
            return ErrorCode::OK;;
        }
        FB_Relay(uint32_t IdOnWebApp, size_t input):FunctionBlock(IdOnWebApp), input(input){}
        ~FB_Relay(){}
};

class FB_FAN: public FunctionBlock{
    size_t input;
    public:
        ErrorCode execute(FBContext *ctx){
            ctx->GetHAL()->SetFanDuty(ctx->GetFloat(input));
            return ErrorCode::OK;;
        }
        FB_FAN(uint32_t IdOnWebApp, size_t input):FunctionBlock(IdOnWebApp), input(input){}
        ~FB_FAN(){}
};


class FB_PowerLED: public FunctionBlock{
    size_t input;
    public:
        ErrorCode execute(FBContext *ctx){
            ctx->GetHAL()->SetLedPowerWhiteDuty(ctx->GetFloat(input));
            return ErrorCode::OK;;
        }
        FB_PowerLED(uint32_t IdOnWebApp, size_t input):FunctionBlock(IdOnWebApp), input(input){}
        ~FB_PowerLED(){}
};

class FB_AnalogOutput0: public FunctionBlock{
    size_t input;
    public:
        ErrorCode execute(FBContext *ctx){
            ctx->GetHAL()->SetAnalogOutput(ctx->GetFloat(input));
            return ErrorCode::OK;;
        }
        FB_AnalogOutput0(uint32_t IdOnWebApp, size_t input):FunctionBlock(IdOnWebApp), input(input){}
        ~FB_AnalogOutput0(){}
};

//Converters
class FB_Bool2ColorConverter: public FunctionBlock{
    size_t input;
    size_t output;
    uint32_t colorOnTRUE;
    uint32_t colorOnFALSE;
    
    public:
        ErrorCode execute(FBContext *ctx)
        {
            ctx->SetColor(this->output, ctx->GetBinary(input)?colorOnTRUE:colorOnFALSE);
            return ErrorCode::OK;;
        }
        FB_Bool2ColorConverter(uint32_t IdOnWebApp, size_t input, size_t output, uint32_t colorOnTRUE, uint32_t colorOnFALSE=0):
            FunctionBlock(IdOnWebApp), input(input), output(output), colorOnTRUE(colorOnTRUE<<8), colorOnFALSE(colorOnFALSE<<8){}
        ~FB_Bool2ColorConverter(){}
};

class FB_Bool2IntConverter: public FunctionBlock{
    size_t input;
    size_t output;
    uint32_t intOnTRUE;
    uint32_t intOnFALSE;
    
    public:
        ErrorCode execute(FBContext *ctx)
        {
            ctx->SetInteger(this->output, ctx->GetBinary(input)?intOnTRUE:intOnFALSE);
            return ErrorCode::OK;;
        }
        FB_Bool2IntConverter(uint32_t IdOnWebApp, size_t input, size_t output, int32_t intOnTRUE, int32_t intOnFALSE=0):
            FunctionBlock(IdOnWebApp), input(input), output(output), intOnTRUE(intOnTRUE), intOnFALSE(intOnFALSE){}
        ~FB_Bool2IntConverter(){}
};

class FB_Bool2FloatConverter: public FunctionBlock{
    size_t input;
    size_t output;
    float floatOnTRUE;
    float floatOnFALSE;
    
    public:
        ErrorCode execute(FBContext *ctx)
        {
            ctx->SetFloat(this->output, ctx->GetBinary(input)?floatOnTRUE:floatOnFALSE);
            return ErrorCode::OK;;
        }
        FB_Bool2FloatConverter(uint32_t IdOnWebApp, size_t input, size_t output, float floatOnTRUE=1.0, float floatOnFALSE=0.0):
            FunctionBlock(IdOnWebApp), input(input), output(output), floatOnTRUE(floatOnTRUE), floatOnFALSE(floatOnFALSE){}
        ~FB_Bool2FloatConverter(){}
};

class FB_Int2BoolConverter: public FunctionBlock{
    size_t input;
    size_t output;
    int32_t limitfor1;
    
    public:
        ErrorCode execute(FBContext *ctx)
        {
            ctx->SetBinary(this->output, ctx->GetInteger(input)>=limitfor1);
            return ErrorCode::OK;;
        }
        FB_Int2BoolConverter(uint32_t IdOnWebApp, size_t input, size_t output, int32_t limitfor1=1.0):
            FunctionBlock(IdOnWebApp), input(input), output(output), limitfor1(limitfor1){}
        ~FB_Int2BoolConverter(){}
};

class FB_Int2FloatConverter: public FunctionBlock{
    size_t input;
    size_t output;
    
    public:
        ErrorCode execute(FBContext *ctx)
        {
            ctx->SetFloat(this->output, ctx->GetInteger(input));
            return ErrorCode::OK;;
        }
        FB_Int2FloatConverter(uint32_t IdOnWebApp, size_t input, size_t output):
            FunctionBlock(IdOnWebApp), input(input), output(output){}
        ~FB_Int2FloatConverter(){}
};

class FB_Float2IntConverter: public FunctionBlock{
    size_t input;
    size_t output;
    
    public:
        ErrorCode execute(FBContext *ctx)
        {
            ctx->SetInteger(this->output, ctx->GetFloat(input));
            return ErrorCode::OK;;
        }
        FB_Float2IntConverter(uint32_t IdOnWebApp, size_t input, size_t output):
            FunctionBlock(IdOnWebApp), input(input), output(output){}
        ~FB_Float2IntConverter(){}
};

//Basic Boolean Functions
class FB_NOT: public FunctionBlock{
    size_t input;
    size_t output;
    
    public:
        ErrorCode execute(FBContext *ctx)
        {
            ctx->SetBinary(this->output, !ctx->GetBinary(input));
            return ErrorCode::OK;
        }
        FB_NOT(uint32_t IdOnWebApp, size_t input, size_t output):FunctionBlock(IdOnWebApp), input(input), output(output){}
        ~FB_NOT(){}
};

class FB_AND2: public FunctionBlock{
    size_t inputA;
    size_t inputB;
    size_t output;
    
    public:
        ErrorCode execute(FBContext *ctx)
        {
            ctx->SetBinary(this->output, ctx->GetBinary(inputA) & ctx->GetBinary(inputB));
            return ErrorCode::OK;;
        }
        FB_AND2(uint32_t IdOnWebApp, size_t inputA, size_t inputB, size_t output):FunctionBlock(IdOnWebApp), inputA(inputA), inputB(inputB), output(output){}
        ~FB_AND2(){}
};

class FB_OR2: public FunctionBlock{
    size_t inputA;
    size_t inputB;
    size_t output;
    
    public:
        ErrorCode execute(FBContext *ctx)
        {
            ctx->SetBinary(this->output, ctx->GetBinary(inputA) | ctx->GetBinary(inputB));
            return ErrorCode::OK;;
        }
        FB_OR2(uint32_t IdOnWebApp, size_t inputA, size_t inputB, size_t output):FunctionBlock(IdOnWebApp), inputA(inputA), inputB(inputB), output(output){}
        ~FB_OR2(){}
};

class FB_XOR2: public FunctionBlock{
    size_t inputA;
    size_t inputB;
    size_t output;
    
    public:
        ErrorCode execute(FBContext *ctx)
        {
            ctx->SetBinary(this->output, ctx->GetBinary(inputA) != ctx->GetBinary(inputB));
            return ErrorCode::OK;;
        }
        FB_XOR2(uint32_t IdOnWebApp, size_t inputA, size_t inputB, size_t output):FunctionBlock(IdOnWebApp), inputA(inputA), inputB(inputB), output(output){}
        ~FB_XOR2(){}
};

class FB_RS: public FunctionBlock{
    size_t inputR;
    size_t inputS;
    size_t output;
    bool state;
    
    public:
        ErrorCode execute(FBContext *ctx)
        {
            if(ctx->GetBinary(inputR)) state=false;
            else if(ctx->GetBinary(inputS)) state = true;
            ctx->SetBinary(this->output, state);
            return ErrorCode::OK;
        }
        FB_RS(uint32_t IdOnWebApp, size_t inputR, size_t inputS, size_t output):FunctionBlock(IdOnWebApp), inputR(inputR), inputS(inputS), output(output), state(false){}
        ~FB_RS(){}
};

class FB_SR: public FunctionBlock{
    size_t inputR;
    size_t inputS;
    size_t output;
    bool state;
    
    public:
        ErrorCode execute(FBContext *ctx)
        {
            if(ctx->GetBinary(inputS)) state = true;
            else if(ctx->GetBinary(inputR)) state=false;
            ctx->SetBinary(this->output, state);
            return ErrorCode::OK;
        }
        FB_SR(uint32_t IdOnWebApp, size_t inputR, size_t inputS, size_t output):FunctionBlock(IdOnWebApp), inputR(inputR), inputS(inputS), output(output), state(false){}
        ~FB_SR(){}
};

//Basic Number Functions
class FB_ADD2: public FunctionBlock{
    size_t inputA;
    size_t inputB;
    size_t output;
    
    public:
        ErrorCode execute(FBContext *ctx)
        {
            ctx->SetFloat(this->output, ctx->GetFloat(inputA) + ctx->GetFloat(inputB));
            return ErrorCode::OK;;
        }
        FB_ADD2(uint32_t IdOnWebApp, size_t inputA, size_t inputB, size_t output):FunctionBlock(IdOnWebApp), inputA(inputA), inputB(inputB), output(output){}
        ~FB_ADD2(){}
};

class FB_SUB2: public FunctionBlock{
    size_t inputA;
    size_t inputB;
    size_t output;
    
    public:
        ErrorCode execute(FBContext *ctx)
        {
            ctx->SetFloat(this->output, ctx->GetFloat(inputA) - ctx->GetFloat(inputB));
            return ErrorCode::OK;;
        }
        FB_SUB2(uint32_t IdOnWebApp, size_t inputA, size_t inputB, size_t output):FunctionBlock(IdOnWebApp), inputA(inputA), inputB(inputB), output(output){}
        ~FB_SUB2(){}
};

class FB_MULTIPLY: public FunctionBlock{
    size_t inputA;
    size_t inputB;
    size_t output;
    
    public:
        ErrorCode execute(FBContext *ctx)
        {
            ctx->SetFloat(this->output, ctx->GetFloat(inputA) * ctx->GetFloat(inputB));
            return ErrorCode::OK;;
        }
        FB_MULTIPLY(uint32_t IdOnWebApp, size_t inputA, size_t inputB, size_t output):FunctionBlock(IdOnWebApp), inputA(inputA), inputB(inputB), output(output){}
        ~FB_MULTIPLY(){}
};

class FB_DIVIDE: public FunctionBlock{
    size_t inputA;
    size_t inputB;
    size_t output;
    
    public:
        ErrorCode execute(FBContext *ctx)
        {
            ctx->SetFloat(this->output, ctx->GetFloat(inputA) / ctx->GetFloat(inputB));
            return ErrorCode::OK;;
        }
        FB_DIVIDE(uint32_t IdOnWebApp, size_t inputA, size_t inputB, size_t output):FunctionBlock(IdOnWebApp), inputA(inputA), inputB(inputB), output(output){}
        ~FB_DIVIDE(){}
};

class FB_MAX: public FunctionBlock{
    size_t inputA;
    size_t inputB;
    size_t output;
    
    public:
        ErrorCode execute(FBContext *ctx)
        {
            ctx->SetFloat(this->output, std::max(ctx->GetFloat(inputA), ctx->GetFloat(inputB)));
            return ErrorCode::OK;;
        }
        FB_MAX(uint32_t IdOnWebApp, size_t inputA, size_t inputB, size_t output):FunctionBlock(IdOnWebApp), inputA(inputA), inputB(inputB), output(output){}
        ~FB_MAX(){}
};

class FB_MIN: public FunctionBlock{
    size_t inputA;
    size_t inputB;
    size_t output;
    
    public:
        ErrorCode execute(FBContext *ctx)
        {
            ctx->SetFloat(this->output, std::min(ctx->GetFloat(inputA), ctx->GetFloat(inputB)));
            return ErrorCode::OK;;
        }
        FB_MIN(uint32_t IdOnWebApp, size_t inputA, size_t inputB, size_t output):FunctionBlock(IdOnWebApp), inputA(inputA), inputB(inputB), output(output){}
        ~FB_MIN(){}
};

class FB_GT: public FunctionBlock{
    size_t inputA;
    size_t inputB;
    size_t output;
    
    public:
        ErrorCode execute(FBContext *ctx)
        {
            ctx->SetBinary(this->output, ctx->GetFloat(inputA) > ctx->GetFloat(inputB));
            return ErrorCode::OK;;
        }
        FB_GT(uint32_t IdOnWebApp, size_t inputA, size_t inputB, size_t output):FunctionBlock(IdOnWebApp), inputA(inputA), inputB(inputB), output(output){}
        ~FB_GT(){}
};

class FB_LT: public FunctionBlock{
    size_t inputA;
    size_t inputB;
    size_t output;
    
    public:
        ErrorCode execute(FBContext *ctx)
        {
            ctx->SetBinary(this->output, ctx->GetFloat(inputA) < ctx->GetFloat(inputB));
            return ErrorCode::OK;;
        }
        FB_LT(uint32_t IdOnWebApp, size_t inputA, size_t inputB, size_t output):FunctionBlock(IdOnWebApp), inputA(inputA), inputB(inputB), output(output){}
        ~FB_LT(){}
};

class FB_Limit: public FunctionBlock{
    size_t inputMinimum;
    size_t input;
    size_t inputMaximum;
    size_t output;
    
    public:
        ErrorCode execute(FBContext *ctx)
        {
            int i = ctx->GetFloat(this->input);
            int min = ctx->GetFloat(this->inputMinimum);
            int max = ctx->GetFloat(this->inputMaximum);
            ctx->SetFloat(this->output, i>max?max:i<min?min:i);
            return ErrorCode::OK;;
        }
        FB_Limit(uint32_t IdOnWebApp, size_t inputMinimum, size_t input, size_t inputMaximum, size_t output):FunctionBlock(IdOnWebApp), inputMinimum(inputMinimum), input(input), inputMaximum(inputMaximum), output(output){}
        ~FB_Limit(){}
};

class FB_LimitMonitor: public FunctionBlock{
    size_t inputMinimum;
    size_t input;
    size_t inputMaximum;
    size_t inputHysteresis;
    size_t outputLLE;
    size_t outputULE;
    
    public:
        ErrorCode execute(FBContext *ctx)
        {
            int i = ctx->GetFloat(this->input);
            int min = ctx->GetFloat(this->inputMinimum);
            int max = ctx->GetFloat(this->inputMaximum);
            int h = ctx->GetFloat(this->inputHysteresis);
            if(i>max){
                ctx->SetBinary(this->outputULE, true);
            }else if(i<=max-h){
                ctx->SetBinary(this->outputULE, false);
            }
            if(i<min){
                ctx->SetBinary(this->outputLLE, true);
            } else if(i>=min+h){
                ctx->SetBinary(this->outputLLE, false);
            }
            return ErrorCode::OK;;
        }
        FB_LimitMonitor(uint32_t IdOnWebApp, size_t inputMinimum, size_t input, size_t inputMaximum, size_t inputHysteresis ,size_t outputLLE, size_t outputULE):FunctionBlock(IdOnWebApp), inputMinimum(inputMinimum), input(input), inputMaximum(inputMaximum), inputHysteresis(inputHysteresis), outputLLE(outputLLE), outputULE(outputULE){}
        ~FB_LimitMonitor(){}
};

class FB_CNT: public FunctionBlock{
    size_t inputTrigger;
    size_t inputReset;
    size_t inputPreset;
    size_t output;
    size_t outputCurrentValue;
    

    int _CurrentValue=0;
    bool lastInputValue=false;
    
    public:
        ErrorCode execute(FBContext *ctx)
        {
            bool iv = ctx->GetBinary(this->inputTrigger);
            bool PV = ctx->GetBinary(this->inputPreset);
            if(ctx->GetBinary(this->inputReset)){
                this->_CurrentValue=0;
            }else if (this->lastInputValue==false && iv==true && this->_CurrentValue<PV){
                this->_CurrentValue++;
            }
            ctx->SetBinary(this->output, this->_CurrentValue>=PV);
            ctx->SetInteger(this->outputCurrentValue, this->_CurrentValue);
            return ErrorCode::OK;
        }
        FB_CNT(uint32_t IdOnWebApp, size_t inputTrigger, size_t inputReset, size_t inputPreset, size_t output, size_t outputCurrentValue):FunctionBlock(IdOnWebApp), inputTrigger(inputTrigger), inputReset(inputReset), inputPreset(inputPreset), output(output), outputCurrentValue(outputCurrentValue){}
        ~FB_CNT(){}
};

//Timing
class FB_Timekeeper: public FunctionBlock{
    size_t inputCountUp;
    size_t inputReset;
    size_t inputPresetTime_msecs;
    size_t output;
    size_t outputCurrentValue_msecs;
    

    int64_t currentValueUS = 0;
    bool lastInput{false};//for edge detection
    int64_t lastCallTimeUS{0};
    
    public:
        ErrorCode execute(FBContext *ctx)
        {
            if(ctx->GetBinary(this->inputReset)){
                this->currentValueUS=0;
                this->lastCallTimeUS=0;
                this->lastInput=false;
                ctx->SetBinary(this->output, false);
                return ErrorCode::OK;
            }
            bool currentInput = ctx->GetBinary(this->inputCountUp);
            if(currentInput){
                int64_t now = ctx->GetMicroseconds();
                if(lastInput){
                    this->currentValueUS+=(now-lastCallTimeUS);
                }
                lastCallTimeUS=now; 
            }
            int64_t currentValueMS = currentValueUS/1000;
            ctx->SetBinary(this->output,currentValueMS>=ctx->GetInteger(inputPresetTime_msecs));
            ctx->SetInteger(this->outputCurrentValue_msecs, currentValueMS);
            lastInput=currentInput;
            return ErrorCode::OK;
        }
        FB_Timekeeper(uint32_t IdOnWebApp, size_t inputCountUp, size_t inputReset, size_t inputPresetTime_msecs, size_t output, size_t outputCurrentValue_msecs):
            FunctionBlock(IdOnWebApp), inputCountUp(inputCountUp), inputReset(inputReset), inputPresetTime_msecs(inputPresetTime_msecs), output(output), outputCurrentValue_msecs(outputCurrentValue_msecs){}
        ~FB_Timekeeper(){}
};

class FB_TON: public FunctionBlock{
    size_t inputTrigger;
    size_t inputPresetTime_msecs;
    size_t output;
    size_t outputElapsedTime_msecs;
    

    int64_t inputPositiveEdge = INT64_MAX;
    bool lastInputValue;
    
    public:
        ErrorCode execute(FBContext *ctx)
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
            return ErrorCode::OK;
        }
        FB_TON(uint32_t IdOnWebApp, size_t inputTrigger, size_t inputPresetTime_msecs, size_t output, size_t outputElapsedTime_msecs):FunctionBlock(IdOnWebApp), inputTrigger(inputTrigger), inputPresetTime_msecs(inputPresetTime_msecs), output(output), outputElapsedTime_msecs(outputElapsedTime_msecs){}
        ~FB_TON(){}
};

class FB_TOF: public FunctionBlock{
    size_t inputTrigger;
    size_t inputPresetTime_msecs;
    size_t output;
    size_t outputElapsedTime_msecs;
    

    int64_t inputNegativeEdge = INT64_MIN;
    bool lastInputValue;
    
    public:
        ErrorCode execute(FBContext *ctx)
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
            ctx->SetBinary(output, currentInputValue || (elapsed<presetTime_msecs));
            ctx->SetInteger(outputElapsedTime_msecs, elapsed);
            return ErrorCode::OK;
        }
        FB_TOF(uint32_t IdOnWebApp, size_t inputTrigger, size_t inputPresetTime_msecs, size_t output, size_t outputElapsedTime_msecs):FunctionBlock(IdOnWebApp), inputTrigger(inputTrigger), inputPresetTime_msecs(inputPresetTime_msecs), output(output), outputElapsedTime_msecs(outputElapsedTime_msecs){}
        ~FB_TOF(){}
};

//Complex Functions
class FB_PIDSimple: public FunctionBlock{
    size_t inputSetpoint;
    size_t inputFeedback;
    size_t output;
    float setpointValue;
    float feedbackValue;
    float outputValue;

    float kp;
    int64_t tn_msecs;
    int64_t tv_msecs;
    float minOutput;
    float maxOutput;
    bool directionInverse;

    PID_T1::Controller<float> *pid;
    
    public:
        ErrorCode initPhase1(FBContext *ctx) override
        {
            pid = new PID_T1::Controller<float>(&feedbackValue, &outputValue, &setpointValue, minOutput, maxOutput, PID_T1::Mode::CLOSEDLOOP, PID_T1::AntiWindup::ON_SWICH_OFF_INTEGRATOR, directionInverse?PID_T1::Direction::REVERSE:PID_T1::Direction::DIRECT, 1000);
            pid->SetKpTnTv(kp, tn_msecs, tv_msecs, tv_msecs*0.2);
            return ErrorCode::OK;
        }

        ErrorCode execute(FBContext *ctx) override
        {
            ctx->GetFloatAsPointer(inputSetpoint, &setpointValue);
            ctx->GetFloatAsPointer(inputFeedback, &feedbackValue);
            pid->Compute(ctx->GetMicroseconds()/1000);
            ctx->SetFloat(output, outputValue);
            return ErrorCode::OK;
        }
        FB_PIDSimple(uint32_t IdOnWebApp, size_t inputSetpoint, size_t inputFeedback, size_t output, float kp, int32_t tn_msecs, int32_t tv_msecs, float minOutput, float maxOutput, bool directionInverse):
            FunctionBlock(IdOnWebApp), 
            inputSetpoint(inputSetpoint), 
            inputFeedback(inputFeedback), 
            output(output),
            kp(kp), 
            tn_msecs(tn_msecs),  
            tv_msecs(tv_msecs),
            minOutput(minOutput),
            maxOutput(maxOutput),
            directionInverse(directionInverse){}
        ~FB_PIDSimple(){
            delete pid;
        }
};

