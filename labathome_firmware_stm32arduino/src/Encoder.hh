#pragma once
#include "Arduino.h"
#include "errorcheck.hh"

namespace ROTARY_ENCODER
{
	enum class MODE
	{
		SINGLE = 0,
		HALFQUAD = 1,
		FULLQUAD = 2,
	};

	class M
	{
	public:
		M() {}
		bool Setup()
		{

			__HAL_RCC_TIM2_CLK_ENABLE();
    		__HAL_RCC_GPIOA_CLK_ENABLE();
			GPIO_InitTypeDef GPIO_InitStruct = {0};
			GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
			GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
			GPIO_InitStruct.Pull = GPIO_NOPULL;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
			GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

			TIM_Encoder_InitTypeDef sConfig = {0};
  			TIM_MasterConfigTypeDef sMasterConfig = {0};
			htim2.Instance = TIM2;
			htim2.Init.Prescaler = 0;
			htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
			htim2.Init.Period = 65535;
			htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
			htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
			sConfig.EncoderMode = TIM_ENCODERMODE_TI1;
			sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
			sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
			sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
			sConfig.IC1Filter = 15;
			sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
			sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
			sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
			sConfig.IC2Filter = 15;
			HAL_ERROR_CHECK(HAL_TIM_Encoder_Init(&htim2, &sConfig));

			sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
			sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
			HAL_ERROR_CHECK(HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig));
			HAL_ERROR_CHECK(HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL));
			return true;
		}

		int GetTicks()
		{
			return LL_TIM_GetCounter(htim2.Instance);
		}

	private:
		TIM_HandleTypeDef htim2;
	};
}
