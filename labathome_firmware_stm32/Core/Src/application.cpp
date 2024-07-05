
#include "stm32g4xx_hal.h"
#include <cinttypes>
#include <cstring>
#include <cstdio>
#include "cmsis_os.h"
#include "errorcheck.hh"
#include "log.h"

extern "C" void app_main();
extern "C" void DoEachMs();
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern DAC_HandleTypeDef hdac1;
extern I2C_HandleTypeDef hi2c1;
extern TIM_HandleTypeDef htim1;//3Phase Driver
extern TIM_HandleTypeDef htim2;//Rotary Encoder
extern TIM_HandleTypeDef htim3; //Hall Sensors
extern TIM_HandleTypeDef htim4; //4.4 WHITE LED, 
extern TIM_HandleTypeDef htim15; //15.1->Servo 1, 15.2->Servo 2
extern TIM_HandleTypeDef htim16; //16.1->Servo 3
extern TIM_HandleTypeDef htim17; //17.1 ->FAN2
extern UART_HandleTypeDef huart4;
extern PCD_HandleTypeDef hpcd_USB_FS;

//TX
constexpr size_t TX_BUFFER_SIZE{12};
uint8_t tx_buf[TX_BUFFER_SIZE]; //"my" perspective! from slave to master
constexpr size_t STATUS_POS{0};
constexpr size_t BTN_MOVEMENT_BLFAULT_POS{1};
constexpr size_t ROTENC_POS{2};
constexpr size_t BRIGHTNESS_POS{4};
constexpr size_t USBPD_VOLTAGE_IS_POS{6};
constexpr size_t ADC1_POS{8};
constexpr size_t ADC2_POS{10};

//RX
constexpr size_t RX_BUFFER_SIZE{18};
uint8_t rx_buf[RX_BUFFER_SIZE]; // from master to slave
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





void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    HAL_I2C_EnableListen_IT(hi2c);
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    HAL_I2C_EnableListen_IT(hi2c);
}

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
    UNUSED(AddrMatchCode);
    if (TransferDirection == I2C_DIRECTION_RECEIVE) // TransferDirection is master perspective
    {
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

void app_main()
{
    SetupAndStartI2CSlave(&hi2c1);
    HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
    while (1)
    {
        uint16_t cnt = TIM2->CNT;
        //log_info("TxCplt = %lu, RxCplt = %lu, Addr = %lu ,Listen = %lu; ram content %d %d %d %d\r\n", cntSlaveTxCpltCallback, cntSlaveRxCpltCallback, cntAddrCallback, cntListenCallback, ram[0], ram[1], ram[2], ram[3]);
        log_info("encoder=%d", cnt);
        osDelay(1000);
    }
}

void DoEachMs(){
    //Heater; Wert ist von 0-255; Zyklus dauert 255*4clock
    static uint32_t startOfCycle=uwTick;
    uint32_t passedTime=uwTick-startOfCycle;
    if(passedTime>=(255*4)){
        startOfCycle=uwTick;
        if(rx_buf[HEATER_POS]>0){
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);
        }
    }
    if(passedTime>=4*rx_buf[HEATER_POS]){
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);
    }
}
