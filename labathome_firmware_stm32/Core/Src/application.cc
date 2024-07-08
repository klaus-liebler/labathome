
#include "stm32g4xx_hal.h"
#include "stm32g4xx_ll_gpio.h"
#include <cinttypes>
#include <cstring>
#include <cstdio>

#include "errorcheck.hh"
#include "common.hh"
#include "log.h"
#include "main.h"

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
//TIM17.1 ->FAN2
extern TIM_HandleTypeDef htim17;

extern PCD_HandleTypeDef hpcd_USB_FS;


extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern DAC_HandleTypeDef hdac1;

//TX
constexpr size_t TX_BUFFER_SIZE{12};
uint8_t tx_buf[TX_BUFFER_SIZE]={0}; //"my" perspective! from slave to master
constexpr size_t STATUS_POS{0};
constexpr size_t BTN_MOVEMENT_BLFAULT_POS{1};
constexpr size_t ROTENC_POS{2};
constexpr size_t BRIGHTNESS_POS{4};
constexpr size_t USBPD_VOLTAGE_IS_POS{6};
constexpr size_t ADC1_POS{8};
constexpr size_t ADC2_POS{10};

//RX
constexpr size_t RX_BUFFER_SIZE{18};
uint8_t rx_buf[RX_BUFFER_SIZE]={0}; // from master to slave
constexpr size_t ADDRESS_POINTER_POS{0};
constexpr size_t RELAY_BLRESET_POS{1};
constexpr size_t SERVO1_POS{2};
constexpr size_t SERVO2_POS{3};
constexpr size_t SERVO3_POS{4};
constexpr size_t FAN_POS{5};
constexpr size_t USBPD_VOLTAGE_SHOULD_POS{6};
constexpr size_t DAC1_POS{8};
constexpr size_t DAC2_POS{10};
constexpr size_t HEATER_POS{12};
constexpr size_t LED_POWER_POS{13};
constexpr size_t THREEPHASE_MODE_POS{14};
constexpr size_t THREEPHASE_P1_DUTY_POS{15};
constexpr size_t THREEPHASE_P2_DUTY_POS{16};
constexpr size_t THREEPHASE_P3_DUTY_POS{17};

//ADC
/* Variables for ADC conversion data */
__IO uint32_t  adc1Data32[2];//ADC1.5 (ADC1), Temp, Vbat, Vrefint
__IO uint16_t* adc1Data16 = (uint16_t*)adc1Data32;
__IO uint32_t  adc2Data32[1];//ADC2.12 (Brightness) und ADC2.15 (ADC2)
__IO uint16_t* adc2Data16 = (uint16_t*)adc2Data32;


void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    HAL_I2C_EnableListen_IT(hi2c);
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    //TODO: Dieser Aufruf passiert nur, wenn der komplette buffer übermittelt wird
    //Falls später mal nur relevante Teile des Buffers übertragen werden sollen, dann muss das intelligenter passieren, also auch bei einem Abbruch
    //write rx_buffer to my outputs
    if(ParseU8(rx_buf, RELAY_BLRESET_POS)&& 0b1){
        LL_GPIO_SetOutputPin(RELAY_GPIO_Port, RELAY_Pin);
    }else{
        LL_GPIO_ResetOutputPin(RELAY_GPIO_Port, RELAY_Pin);
    }

    if(ParseU8(rx_buf, RELAY_BLRESET_POS)&& 0b10){
        LL_GPIO_SetOutputPin(BL_RESET_GPIO_Port, BL_RESET_Pin);
    }else{
        LL_GPIO_ResetOutputPin(BL_RESET_GPIO_Port, BL_RESET_Pin);
    }
    
    SetServoAngle(1, ParseU8(rx_buf, SERVO1_POS));
    SetServoAngle(2, ParseU8(rx_buf, SERVO2_POS));
    SetServoAngle(3, ParseU8(rx_buf, SERVO3_POS));
    SetFanSpeed(ParseU8(rx_buf, FAN_POS));
    //TODO USBC
    HAL_DAC_SetValue(&hdac1, DAC1_CHANNEL_1, DAC_ALIGN_12B_R, ParseU16(rx_buf, DAC1_POS));
    HAL_DAC_SetValue(&hdac1, DAC1_CHANNEL_2, DAC_ALIGN_12B_R, ParseU16(rx_buf, DAC2_POS));
    SetHeaterPower(ParseU8(rx_buf, HEATER_POS));
    SetLedPowerPower(ParseU8(rx_buf, LED_POWER_POS));
    //TODO Brushless Mode
    //TODO Brushless Speeds
    //Start the whole process
    HAL_I2C_EnableListen_IT(hi2c);
}

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
    UNUSED(AddrMatchCode);
    if (TransferDirection == I2C_DIRECTION_RECEIVE) // TransferDirection is master perspective
    {
        //write my inputs to tx buffer
        WriteU8(0, tx_buf, STATUS_POS);
        uint8_t singleBitInputs = 
            (LL_GPIO_IsInputPinSet(BTN_RED_GPIO_Port, BTN_RED_Pin)<<0) || 
            (LL_GPIO_IsInputPinSet(BTN_YEL_GPIO_Port, BTN_YEL_Pin)<<1) || 
            (LL_GPIO_IsInputPinSet(MOVEMENT_SENSOR_GPIO_Port, MOVEMENT_SENSOR_Pin)<<2) 
            ||(LL_GPIO_IsInputPinSet(BL_FAULT_GPIO_Port, BL_FAULT_Pin)<<3); 
        WriteU8(singleBitInputs,tx_buf, BTN_MOVEMENT_BLFAULT_POS);
        WriteU16(TIM2->CNT, tx_buf, ROTENC_POS);
        WriteU16(adc2Data16[0], tx_buf, BRIGHTNESS_POS);
        //USB_PD as is
        WriteU16(adc1Data16[0], tx_buf, ADC1_POS);
        WriteU16(adc2Data16[1], tx_buf, ADC2_POS);
        HAL_ERROR_CHECK(HAL_I2C_Slave_Seq_Transmit_IT(hi2c, (uint8_t *)tx_buf, TX_BUFFER_SIZE, I2C_FIRST_AND_LAST_FRAME));
    }
    else
    {
        HAL_ERROR_CHECK(HAL_I2C_Slave_Seq_Receive_IT(hi2c, (uint8_t *)rx_buf, RX_BUFFER_SIZE, I2C_FIRST_AND_LAST_FRAME));
    }
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
    HAL_I2C_EnableListen_IT(hi2c);
}

void SetupAndStartI2CSlave(I2C_HandleTypeDef *hi2c)
{
    memset(rx_buf, 0, RX_BUFFER_SIZE);
    memset(tx_buf, 0, TX_BUFFER_SIZE);
    HAL_ERROR_CHECK(HAL_I2C_EnableListen_IT(hi2c));
}

void SetServoAngle(uint8_t servo_1_2_3, uint8_t angle_0_180)
{
    if(angle_0_180>180){
        return;
    }
    float pulse_length = 1.0 + (angle_0_180 / 180.0);  // 1.0 ms - 2.0 ms Impulsweite
    uint32_t pulse = (uint32_t)(1000 * pulse_length); 
    if(servo_1_2_3==1){ 
        __HAL_TIM_SET_COMPARE(&htim15, TIM_CHANNEL_1, pulse);

    }
    else if(servo_1_2_3==2){
        __HAL_TIM_SET_COMPARE(&htim15, TIM_CHANNEL_2, pulse);
    } else if(servo_1_2_3==3){
        __HAL_TIM_SET_COMPARE(&htim16, TIM_CHANNEL_1, pulse);
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

void SetHeaterPower(uint8_t power_0_100)
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
}

void app_loop20ms(){

    
   
}

void app_loop1000ms(){
    log_info("enco=%d heater=%d adc1=%lu, temp=%lu, vbat=%lu, vref=%lu", TIM2->CNT, rx_buf[HEATER_POS], adc1Data[0], adc1Data[1], adc1Data[2], adc1Data[3]);
}

void app_loop_1ms_irq_context(){
    //Heater; Wert ist von 0-100; Zyklus dauert 1000ms
    static uint32_t startOfCycle=uwTick;
    uint32_t passedTime=uwTick-startOfCycle;
    if(passedTime>=(10*100)){
        startOfCycle=uwTick;
        if(rx_buf[HEATER_POS]>0){
            //log_info("NEW ON");
            LL_GPIO_SetOutputPin(BL_ENABLE_OR_LED_GPIO_Port, BL_ENABLE_OR_LED_Pin);
        }else{
            //log_info("NEW OFF");
        }
    }
    else if(passedTime>=10*rx_buf[HEATER_POS] && LL_GPIO_IsOutputPinSet(BL_ENABLE_OR_LED_GPIO_Port, BL_ENABLE_OR_LED_Pin)){
        LL_GPIO_ResetOutputPin(BL_ENABLE_OR_LED_GPIO_Port, BL_ENABLE_OR_LED_Pin);
        //log_info("OFF after %lu", passedTime);
    }
}


void app_loop(){
    static uint32_t last20ms=0;
    static uint32_t last1000ms=0;
    uint32_t now=uwTick;
    uint32_t timePassed20=now-last20ms;
    if(timePassed20>=20){
        last20ms=now;
        app_loop20ms();
    }
    uint32_t timePassed1000=now-last1000ms;
    if(timePassed1000>=1000){
        last1000ms=now;
        app_loop1000ms();
    }
}