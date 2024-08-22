
#include "stm32g4xx_hal.h"
#include "stm32g4xx_ll_gpio.h"
#include <cinttypes>
#include <cstring>
#include <cstdio>

#include "errorcheck.hh"
#include "common.hh"
#include "log.h"
#include "main.h"

#include "led_manager.hh"

extern "C" void app_setup();
extern "C" void app_loop();
extern "C" void app_loop_1ms_irq_context();

extern I2C_HandleTypeDef hi2c1;
//TIM1.1+2+3->3Phase Driver
//TIM2.1+2->Rotary Encoder
extern TIM_HandleTypeDef htim2;
//TIM3.2+3+4->Hall Sensors
//TIM4.4->WHITE LED,
extern TIM_HandleTypeDef htim4;
//TIM15.1->Servo 1, TIM15.2->Servo 2
extern TIM_HandleTypeDef htim15;
//TIM16.1->Servo 3
extern TIM_HandleTypeDef htim16;
//TIM17.1 ->FAN
extern TIM_HandleTypeDef htim17;

extern PCD_HandleTypeDef hpcd_USB_FS;


extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern DAC_HandleTypeDef hdac1;

#include "stm32_esp32_buffer_definitions.hh"

uint8_t stm2esp_buf[STM2ESP_SIZE]={0};
uint8_t esp2stm_buf[ESP2STM_SIZE]={0};


constexpr time_t TIMEOUT_FOR_FAILSAFE{3000};
bool noDataWarningAlreadyPrinted{false};
bool gotDataInfoAlreadyPrinted{false};

//ADC
/* Variables for ADC conversion data */
__IO uint32_t  adc1Data32[2];//ADC1.5 (ADC1), Temp, Vbat, Vrefint
__IO uint16_t* adc1Data16 = (uint16_t*)adc1Data32;
__IO uint32_t  adc2Data32[1];//ADC2.12 (Brightness) und ADC2.15 (ADC2)
__IO uint16_t* adc2Data16 = (uint16_t*)adc2Data32;

//Time related
__IO time_t millisI64{0};
time_t timeToPutActorsInFailsafe{TIMEOUT_FOR_FAILSAFE};

//Manager
LED::M *led{nullptr};



void SetServoAngle(uint8_t servo_1_2_3, uint8_t angle_0_180)
{
    if(angle_0_180>180){
        return;
    }
    float pulse_length = 1.0 + (angle_0_180 / 180.0);  // 1.0 ms - 2.0 ms Impulsweite
    uint32_t pulse = (uint32_t)(1000 * pulse_length);
    switch (servo_1_2_3)
    {
    case 1:
        __HAL_TIM_SET_COMPARE(&htim15, TIM_CHANNEL_1, pulse);
        break;
    case 2:
        __HAL_TIM_SET_COMPARE(&htim15, TIM_CHANNEL_2, pulse);
        break;
    case 3:
        __HAL_TIM_SET_COMPARE(&htim16, TIM_CHANNEL_1, pulse);
        break;
    default:
        break;
    }
}

void SetFanSpeed(uint8_t power_0_100)
{
    if(power_0_100>100){
        power_0_100=100;
    }
    uint32_t pulse = power_0_100*10; 
    __HAL_TIM_SET_COMPARE(&htim17, TIM_CHANNEL_1, pulse);
}

void SetLedPowerPower(uint8_t power_0_100)
{
    if(power_0_100>100){
        power_0_100=100;
    }
    uint32_t pulse = power_0_100*10; 
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, pulse);
}

void SetPhysicalOutputs(){
    //write rx_buffer to my outputs
    if(ParseU8(esp2stm_buf, RELAY_BLRESET_POS)&& 0b1){
        LL_GPIO_SetOutputPin(RELAY_GPIO_Port, RELAY_Pin);
    }else{
        LL_GPIO_ResetOutputPin(RELAY_GPIO_Port, RELAY_Pin);
    }

    if(ParseU8(esp2stm_buf, RELAY_BLRESET_POS)&& 0b10){
        LL_GPIO_SetOutputPin(BL_RESET_GPIO_Port, BL_RESET_Pin);
    }else{
        LL_GPIO_ResetOutputPin(BL_RESET_GPIO_Port, BL_RESET_Pin);
    }
    
    SetServoAngle(1, ParseU8(esp2stm_buf, SERVO1_POS));
    SetServoAngle(2, ParseU8(esp2stm_buf, SERVO2_POS));
    SetServoAngle(3, ParseU8(esp2stm_buf, SERVO3_POS));
    SetFanSpeed(ParseU8(esp2stm_buf, FAN_POS));
    //TODO USBC
    HAL_DAC_SetValue(&hdac1, DAC1_CHANNEL_1, DAC_ALIGN_12B_R, ParseU16(esp2stm_buf, DAC1_POS));
    HAL_DAC_SetValue(&hdac1, DAC1_CHANNEL_2, DAC_ALIGN_12B_R, ParseU16(esp2stm_buf, DAC2_POS));
    //SetHeaterPower: loop1ms automatically fetches correct value from buffer
    SetLedPowerPower(ParseU8(esp2stm_buf, LED_POWER_POS));
    //TODO Brushless Mode
    //TODO Brushless Speeds
}

void setActorsToSafeSetting(){
    memset(esp2stm_buf, 0, ESP2STM_SIZE);
    SetPhysicalOutputs();
}

void SetupAndStartI2CSlave(I2C_HandleTypeDef *hi2c)
{
    memset(stm2esp_buf, 0, STM2ESP_SIZE);
    setActorsToSafeSetting();
    HAL_ERROR_CHECK(HAL_I2C_EnableListen_IT(hi2c));
}

LED::BlinkPattern WAITING_FOR_CONNECTION(500, 500);
LED::BlinkPattern UNDER_CONTROL_FROM_MASTER(100, 900);
LED::BlinkPattern PROBLEM(100, 100);

void app_setup()
{
    
    SetupAndStartI2CSlave(&hi2c1);
    HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);
    HAL_TIM_PWM_Start(&htim15, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim15, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim17, TIM_CHANNEL_1);
    HAL_NVIC_DisableIRQ(DMA1_Channel1_IRQn);
    HAL_NVIC_DisableIRQ(DMA1_Channel2_IRQn);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc1Data32, 4);
    led = new LED::M(GPIO::Pin::PB03, true, &WAITING_FOR_CONNECTION);
    led->Begin(millisI64, &WAITING_FOR_CONNECTION, 0);
    log_info("Init completed");
    stm2esp_buf[STATUS_POS]=0x01;
}

uint8_t CollectBitInputs(){
     return 
            (LL_GPIO_IsInputPinSet(BTN_RED_GPIO_Port, BTN_RED_Pin)<<0) | 
            ((!LL_GPIO_IsInputPinSet(BTN_YEL_GPIO_Port, BTN_YEL_Pin))<<1) | 
            (LL_GPIO_IsInputPinSet(MOVEMENT_SENSOR_GPIO_Port, MOVEMENT_SENSOR_Pin)<<2) |
            ((!LL_GPIO_IsInputPinSet(BL_FAULT_GPIO_Port, BL_FAULT_Pin))<<3); 
}

void app_loop20ms(time_t now){
    led->Loop(now);
    if(now>=timeToPutActorsInFailsafe){
        if(!noDataWarningAlreadyPrinted){
            log_warn("No Data from ESP32 - goto failsafe!");
            noDataWarningAlreadyPrinted=true;
        }
        gotDataInfoAlreadyPrinted=false;
        setActorsToSafeSetting();
        SetBitIdx(stm2esp_buf[STATUS_POS], 2);
        timeToPutActorsInFailsafe=INT64_MAX;
    }else{
        if(!gotDataInfoAlreadyPrinted){
            log_info("Got Data from ESP32");
            gotDataInfoAlreadyPrinted=true;
        }
        noDataWarningAlreadyPrinted=false;
    }  
}

void app_loop1000ms(time_t now){
    (void)now;
    log_info("stat=0x%02X, input_bits=0x%02X, enco=%d heater=%d adc1=%lu, temp=%lu, vbat=%lu, vref=%lu", stm2esp_buf[STATUS_POS], CollectBitInputs(), TIM2->CNT, esp2stm_buf[HEATER_POS], adc1Data16[0], adc1Data16[1], adc1Data16[2], adc1Data16[3]);
}

void app_loop_1ms_irq_context(){
    //Heater; Wert ist von 0-100; Zyklus dauert 1000ms
    millisI64++;
    static time_t startOfCycle=0;
    time_t passedTime=millisI64-startOfCycle;
    if(passedTime>=(10*100)){
        startOfCycle=millisI64;
        if(esp2stm_buf[HEATER_POS]>0){
            //log_info("NEW ON");
            LL_GPIO_SetOutputPin(BL_ENABLE_OR_LED_GPIO_Port, BL_ENABLE_OR_LED_Pin);
        }else{
            //log_info("NEW OFF");
        }
    }
    else if(passedTime>=10*esp2stm_buf[HEATER_POS] && LL_GPIO_IsOutputPinSet(BL_ENABLE_OR_LED_GPIO_Port, BL_ENABLE_OR_LED_Pin)){
        LL_GPIO_ResetOutputPin(BL_ENABLE_OR_LED_GPIO_Port, BL_ENABLE_OR_LED_Pin);
        //log_info("OFF after %lu", passedTime);
    }
}


void app_loop(){
    static time_t last20ms=millisI64;
    static time_t last1000ms=millisI64;
    time_t now=millisI64;
    time_t timePassed20=now-last20ms;
    if(timePassed20>=20){
        last20ms=now;
        app_loop20ms(now);
    }
    time_t timePassed1000=now-last1000ms;
    if(timePassed1000>=1000){
        last1000ms=now;
        app_loop1000ms(now);
    }
}



//I2C callbacks
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    HAL_I2C_EnableListen_IT(hi2c);
}


void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    SetPhysicalOutputs();
    timeToPutActorsInFailsafe=millisI64+3000;
    ClearBitIdx(stm2esp_buf[STATUS_POS], 2);
    HAL_I2C_EnableListen_IT(hi2c);
}

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
    if(timeToPutActorsInFailsafe)
    
    UNUSED(AddrMatchCode);
    if (TransferDirection == I2C_DIRECTION_RECEIVE) // TransferDirection is master perspective
    {
        //write my inputs to tx buffer     
        WriteU8(CollectBitInputs(),stm2esp_buf, BTN_MOVEMENT_BLFAULT_POS);
        WriteU16(TIM2->CNT, stm2esp_buf, ROTENC_POS);
        WriteU16(adc2Data16[0], stm2esp_buf, BRIGHTNESS_POS);
        //USB_PD as is
        WriteU16(adc1Data16[0], stm2esp_buf, ADC1_POS);
        WriteU16(adc2Data16[1], stm2esp_buf, ADC2_POS);
        HAL_ERROR_CHECK(HAL_I2C_Slave_Seq_Transmit_IT(hi2c, (uint8_t *)stm2esp_buf, STM2ESP_SIZE, I2C_FIRST_AND_LAST_FRAME));
    }
    else
    {
        HAL_ERROR_CHECK(HAL_I2C_Slave_Seq_Receive_IT(hi2c, (uint8_t *)esp2stm_buf, ESP2STM_SIZE, I2C_FIRST_AND_LAST_FRAME));
    }
    timeToPutActorsInFailsafe=millisI64+5000;
}

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
    HAL_I2C_EnableListen_IT(hi2c);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    /** Error_Handler() function is called when error occurs.
     * 1- When Slave doesn't acknowledge its address, Master restarts communication.
     * 2- When Master doesn't acknowledge the last data transferred, Slave doesn't care in this example.
     */
    // HAL_I2C_ERROR_NONE       0x00000000U    /*!< No error           */
    // HAL_I2C_ERROR_BERR       0x00000001U    /*!< BERR error         */
    // HAL_I2C_ERROR_ARLO       0x00000002U    /*!< ARLO error         */
    // HAL_I2C_ERROR_AF         0x00000004U    /*!< Ack Failure error  */
    // HAL_I2C_ERROR_OVR        0x00000008U    /*!< OVR error          */
    // HAL_I2C_ERROR_DMA        0x00000010U    /*!< DMA transfer error */
    // HAL_I2C_ERROR_TIMEOUT    0x00000020U    /*!< Timeout Error      */
    uint32_t error_code = HAL_I2C_GetError(hi2c);
    if (error_code != HAL_I2C_ERROR_AF)
    {
        log_error("I2C Error %d", error_code);
    }
    SetPhysicalOutputs();
    HAL_I2C_EnableListen_IT(hi2c);
}