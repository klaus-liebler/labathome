/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usbpd_dpm_core.c
  * @author  MCD Application Team
  * @brief   USBPD dpm core file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#define __USBPD_DPM_CORE_C

/* Includes ------------------------------------------------------------------*/
#include "usbpd_core.h"
#include "usbpd_trace.h"
#include "usbpd_dpm_core.h"
#include "usbpd_dpm_conf.h"
#include "usbpd_dpm_user.h"

#include "cmsis_os.h"
#if (osCMSIS >= 0x20000U)
#include "task.h"
#endif /* osCMSIS >= 0x20000U */

/* Generic STM32 prototypes */
extern uint32_t HAL_GetTick(void);

/* Private function prototypes -----------------------------------------------*/
#if (osCMSIS < 0x20000U)
void USBPD_CAD_Task(void const *argument);
#else /* osCMSIS >= 0x20000U */

void USBPD_CAD_Task(void *argument);
#endif /* osCMSIS < 0x20000U */

/* Private typedef -----------------------------------------------------------*/
#if (osCMSIS < 0x20000U)
#define DPM_STACK_SIZE_ADDON_FOR_CMSIS              1
#else
#define DPM_STACK_SIZE_ADDON_FOR_CMSIS              4
#endif /* osCMSIS < 0x20000U */

#define FREERTOS_PE_PRIORITY                    osPriorityAboveNormal
#define FREERTOS_PE_STACK_SIZE                  (350 * DPM_STACK_SIZE_ADDON_FOR_CMSIS)

#define FREERTOS_CAD_PRIORITY                   osPriorityRealtime
#define FREERTOS_CAD_STACK_SIZE                 (300 * DPM_STACK_SIZE_ADDON_FOR_CMSIS)

#if (osCMSIS < 0x20000U)
osThreadDef(CAD, USBPD_CAD_Task, FREERTOS_CAD_PRIORITY, 0, FREERTOS_CAD_STACK_SIZE);
osMessageQDef(queueCAD, 2, uint16_t);
#else /* osCMSIS >= 0x20000U */

osThreadAttr_t CAD_Thread_Atrr = {
  .name       = "CAD",
  .priority   = FREERTOS_CAD_PRIORITY, /*osPriorityRealtime,*/
  .stack_size = FREERTOS_CAD_STACK_SIZE
};
#endif /* osCMSIS < 0x20000U */

/* Private define ------------------------------------------------------------*/
#if (osCMSIS < 0x20000U)
#define OSTHREAD_PE(__PORT__)       (((__PORT__) == USBPD_PORT_0) ? osThread(PE_0) : osThread(PE_1))
#else
#define OSTHREAD_PE(__PORT__)       (((__PORT__) == USBPD_PORT_0) ? USBPD_PE_Task_P0 : USBPD_PE_Task_P1)
#define OSTHREAD_PE_ATTR(__PORT__)  (((__PORT__) == USBPD_PORT_0) ? &PE0_Thread_Atrr : &PE1_Thread_Atrr)
#endif /* osCMSIS < 0x20000U */

/* Private macro -------------------------------------------------------------*/
#define CHECK_PE_FUNCTION_CALL(_function_)  _retr = _function_;                  \
                                            if(USBPD_OK != _retr) {return _retr;}
#define CHECK_CAD_FUNCTION_CALL(_function_) if(USBPD_CAD_OK != _function_) {return USBPD_ERROR;}

#if defined(_DEBUG_TRACE)
#define DPM_CORE_DEBUG_TRACE(_PORTNUM_, __MESSAGE__)  USBPD_TRACE_Add(USBPD_TRACE_DEBUG, _PORTNUM_, 0u, (uint8_t *)(__MESSAGE__), sizeof(__MESSAGE__) - 1u);
#else
#define DPM_CORE_DEBUG_TRACE(_PORTNUM_, __MESSAGE__)
#endif /* _DEBUG_TRACE */

/* Private variables ---------------------------------------------------------*/
static osMessageQId CADQueueId;

USBPD_ParamsTypeDef   DPM_Params[USBPD_PORT_COUNT];

/* Private function prototypes -----------------------------------------------*/
void USBPD_DPM_CADCallback(uint8_t PortNum, USBPD_CAD_EVENT State, CCxPin_TypeDef Cc);
static void USBPD_DPM_CADTaskWakeUp(void);

/**
  * @brief  Initialize the core stack (port power role, PWR_IF, CAD and PE Init procedures)
  * @retval USBPD status
  */
USBPD_StatusTypeDef USBPD_DPM_InitCore(void)
{
  /* variable to get dynamique memory allocated by usbpd stack */
  uint32_t stack_dynamemsize;
  USBPD_StatusTypeDef _retr = USBPD_OK;

  /* Check the lib selected */
  if (USBPD_TRUE != USBPD_PE_CheckLIB(_LIB_ID))
  {
    return USBPD_ERROR;
  }

  /* to get how much memory are dynamically allocated by the stack
     the memory return is corresponding to 2 ports so if the application
     managed only one port divide the value return by 2                   */
  stack_dynamemsize = USBPD_PE_GetMemoryConsumption();

  /* done to avoid warning */
  (void)stack_dynamemsize;

  DPM_Params[USBPD_PORT_0].PE_PowerRole     = DPM_Settings[USBPD_PORT_0].PE_DefaultRole;

  {
    static const USBPD_CAD_Callbacks CAD_cbs = { USBPD_DPM_CADCallback, USBPD_DPM_CADTaskWakeUp };
  /* Init CAD */
    CHECK_CAD_FUNCTION_CALL(USBPD_CAD_Init(USBPD_PORT_0, &CAD_cbs, (USBPD_SettingsTypeDef *)&DPM_Settings[USBPD_PORT_0],
                                           &DPM_Params[USBPD_PORT_0]));

  /* Enable CAD on Port 0 */
  USBPD_CAD_PortEnable(USBPD_PORT_0, USBPD_CAD_ENABLE);
  }
  return _retr;
}

/**
  * @brief  Initialize the OS parts (task, queue,... )
  * @retval USBPD status
  */
USBPD_StatusTypeDef USBPD_DPM_InitOS(void)
{

  return USBPD_OK;
}

/**
  * @brief  Initialize the OS parts (port power role, PWR_IF, CAD and PE Init procedures)
  * @retval None
  */
void USBPD_DPM_Run(void)
{
#if (osCMSIS >= 0x20000U)
  osKernelInitialize();
#endif /* osCMSIS >= 0x20000U */
  osKernelStart();
}

/**
  * @brief  WakeUp CAD task
  * @retval None
  */
static void USBPD_DPM_CADTaskWakeUp(void)
{
#if (osCMSIS < 0x20000U)
  (void)osMessagePut(CADQueueId, 0xFFFF, 0);
#else
  uint32_t event = 0xFFFFU;
  (void)osMessageQueuePut(CADQueueId, &event, 0U, 0U);
#endif /* osCMSIS < 0x20000U */
}

/**
  * @brief  Main task for CAD layer
  * @param  argument Not used
  * @retval None
  */
#if (osCMSIS < 0x20000U)
void USBPD_CAD_Task(void const *argument)
#else
void USBPD_CAD_Task(void *argument)
#endif /* osCMSIS < 0x20000U */
{
  for (;;)
  {
#if (osCMSIS < 0x20000U)
    osMessageGet(CADQueueId, USBPD_CAD_Process());
#else
    uint32_t event;
    (void)osMessageQueueGet(CADQueueId, &event, NULL, USBPD_CAD_Process());
#endif /* osCMSIS < 0x20000U */
  }
}

/**
  * @brief  CallBack reporting events on a specified port from CAD layer.
  * @param  PortNum   The handle of the port
  * @param  State     CAD state
  * @param  Cc        The Communication Channel for the USBPD communication
  * @retval None
  */
void USBPD_DPM_CADCallback(uint8_t PortNum, USBPD_CAD_EVENT State, CCxPin_TypeDef Cc)
{
#ifdef _TRACE
  USBPD_TRACE_Add(USBPD_TRACE_CADEVENT, PortNum, (uint8_t)State, NULL, 0);
#endif /* _TRACE */

  switch (State)
  {
  case USBPD_CAD_EVENT_ATTEMC :
  case USBPD_CAD_EVENT_ATTACHED :
    {
      DPM_Params[PortNum].ActiveCCIs = Cc;
      USBPD_DPM_UserCableDetection(PortNum, State);
      break;
    }
  case USBPD_CAD_EVENT_DETACHED :
  case USBPD_CAD_EVENT_EMC :
    {
      USBPD_DPM_UserCableDetection(PortNum, State);
      DPM_Params[PortNum].ActiveCCIs = CCNONE;
      break;
    }
  default :
    /* nothing to do */
    break;
  }
}
