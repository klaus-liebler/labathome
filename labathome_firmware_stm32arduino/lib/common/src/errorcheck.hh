#pragma once
#include "stm32g4xx_hal.h"
#include <cstdio>
#include "log.h"
#define TAG "ERRCHK"



void _hal_error_check_failed(HAL_StatusTypeDef rc, const char *file, int line, const char *expression)
{
    log_error("Error %d in file %s line %d: %s", (int)rc, file, line, expression);
}

#define HAL_ERROR_CHECK(x)                                             \
    do                                                                 \
    {                                                                  \
        HAL_StatusTypeDef __err_rc = (x);                              \
        if (__err_rc != HAL_OK)                                        \
        {                                                              \
            _hal_error_check_failed(__err_rc, __FILE__, __LINE__, #x); \
        }                                                              \
    } while (0);

#undef TAG