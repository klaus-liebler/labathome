#pragma once

class FunctionBlock;

struct Executable
{
    FunctionBlock *functionBlocks;
    size_t functionBlocksSize;
    int *binaries;
    size_t binariesSize;
};

class FBContext{
    private:
        QueueHandle_t xQueue1;
        Executable *currentExecutable=0;

    public:
        bool IsBinaryAvailable(size_t index);
        bool IsIntegerAvailable(size_t index);
        bool IsDoubleAvailable(size_t index);

        esp_err_t CompileProtobufConfig2ExecutableAndEnqueue(char* pb, size_t length);

        bool GetBinary(size_t index);
        void SetBinary(size_t index, bool value);
        FBContext()
        {
            xQueue1=xQueueCreate( 10, sizeof( struct Executable * ) );
        }
        esp_err_t Init();
        esp_err_t Loop();
};


class FunctionBlock {
   public:
      virtual esp_err_t initPhase1(FBContext *ctx){return ESP_OK;};
      virtual esp_err_t initPhase2(FBContext *ctx){return ESP_OK;};
      virtual esp_err_t initPhase3(FBContext *ctx){return ESP_OK;};
      virtual esp_err_t execute(FBContext *ctx)=0;
};

class FB_AND2: public FunctionBlock{
    size_t inputA;
    size_t inputB;
    size_t output;
    
    public:
        esp_err_t execute(FBContext *ctx)
        {
            ctx->SetBinary(this->output, ctx->GetBinary(inputA) & ctx->GetBinary(inputB));
            return ESP_OK;
        }
        FB_AND2(size_t inputA, size_t inputB, size_t output):inputA(inputA), inputB(inputB), output(output){}
};

class FB_OR2: public FunctionBlock{
    size_t inputA;
    size_t inputB;
    size_t output;
    
    public:
        esp_err_t execute(FBContext *ctx)
        {
            ctx->SetBinary(this->output, ctx->GetBinary(inputA) | ctx->GetBinary(inputB));
            return ESP_OK;
        }
        FB_OR2(size_t inputA, size_t inputB, size_t output):inputA(inputA), inputB(inputB), output(output){}
};

class FB_RS: public FunctionBlock{
    size_t inputR;
    size_t inputS;
    size_t output;
    bool state;
    
    public:
        esp_err_t execute(FBContext *ctx)
        {
            if(inputR) state=false;
            else if(inputS) state = true;
            ctx->SetBinary(this->output, state);
            return ESP_OK;
        }
        FB_RS(size_t inputR, size_t inputS, size_t output):inputR(inputR), inputS(inputS), output(output), state(false){}
};

class FB_TON: public FunctionBlock{
    size_t input;
    size_t output;
    uint32_t presetTime;
    bool state;
    
    public:
        esp_err_t execute(FBContext *ctx)
        {
            return ESP_OK;
        }
        FB_TON(size_t input, size_t output, uint32_t presetTime):input(input), output(output), presetTime(presetTime), state(false){}
};