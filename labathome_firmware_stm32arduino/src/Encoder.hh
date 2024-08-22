#pragma once
#include "Arduino.h"

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
		M(int pinA, int pinB, MODE mode = MODE::SINGLE) : pinA(pinA), pinB(pinB), mode(mode) {}
		bool Setup()
		{

			pinMode(pinA, INPUT_PULLUP);
			pinMode(pinB, INPUT_PULLUP);

			pin_function(digitalPinToPinName(pinA), pinmap_function(digitalPinToPinName(pinA), PinMap_TIM));
			pin_function(digitalPinToPinName(pinB), pinmap_function(digitalPinToPinName(pinB), PinMap_TIM));

			TIM_HandleTypeDef Encoder_Handle;
			TIM_Encoder_InitTypeDef sEncoderConfig;

			Encoder_Handle.Init.Period = 65535;
			if (mode == MODE::SINGLE)
			{
				Encoder_Handle.Init.Prescaler = 1;
			}
			else
			{
				Encoder_Handle.Init.Prescaler = 0;
			}
			
			Encoder_Handle.Init.ClockDivision = 0;
			Encoder_Handle.Init.CounterMode = TIM_COUNTERMODE_UP;
			Encoder_Handle.Init.RepetitionCounter = 0;
			Encoder_Handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

			if (mode == MODE::FULLQUAD)
			{
				sEncoderConfig.EncoderMode = TIM_ENCODERMODE_TI12;
			}
			else
			{
				sEncoderConfig.EncoderMode = TIM_ENCODERMODE_TI1;
			}

			sEncoderConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
			sEncoderConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
			sEncoderConfig.IC1Prescaler = TIM_ICPSC_DIV1;
			sEncoderConfig.IC1Filter = 0;

			sEncoderConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
			sEncoderConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
			sEncoderConfig.IC2Prescaler = TIM_ICPSC_DIV1;
			sEncoderConfig.IC2Filter = 0;

			Encoder_Handle.Instance = (TIM_TypeDef *)pinmap_peripheral(digitalPinToPinName(pinA), PinMap_TIM);
			enableTimerClock(&Encoder_Handle);
			if (HAL_TIM_Encoder_Init(&Encoder_Handle, &sEncoderConfig) != HAL_OK)
				return false;
			HAL_TIM_Encoder_Start(&Encoder_Handle, TIM_CHANNEL_ALL);
			return true;
		}
		int GetTicks()
		{
			return LL_TIM_GetCounter((TIM_TypeDef *)pinmap_peripheral(digitalPinToPinName(pinA), PinMap_TIM));
		}

	private:
		int pinA;
		int pinB;
		MODE mode;
		int32_t _ticks = 0;
		int32_t _prevTicks = 0;
	};
}
