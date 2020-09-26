#pragma once

#include <stdio.h>
#include <inttypes.h>
/*


*/

const uint16_t BINARY_ALWAYS_FALSE = 0;
const uint16_t BINARY_ALWAYS_TRUE = 1;
const uint16_t BINARY_HW_INP_MIN = 2;
const uint16_t BINARY_HW_INP_MAX = 1024; //exclusive 1024!
const uint16_t BINARY_HW_OUT_MIN = 1024;
const uint16_t BINARY_HW_OUT_MAX = 2048;
const uint16_t BINARY_HW_MAX = BINARY_HW_OUT_MAX;

const uint16_t BINARY_COM_INP_MIN = 2048;
const uint16_t BINARY_COM_INP_MAX = 3072;
const uint16_t BINARY_COM_OUT_MIN = 3072;
const uint16_t BINARY_COM_OUT_MAX = 4096;

const uint16_t BINARY_INT_MIN=16384;
const uint16_t BINARY_INT_MAX=32768;



