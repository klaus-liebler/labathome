#pragma once

#include <stdio.h>
#include <inttypes.h>
/*
Variablen werden FÜR JEDEN DATENTYP UNABHÄNGIG mit einer 16bit ID identifiziert
Für Bool gilt
- Index 0 liefert immer False (dies ist auch der Standardwert für "nicht angeschlossen)
- Index 1 liefert immer True
- Index 2...1023 ist Hardware-Input
- Index 1024...2047 ist Hardware-Output
- Index 2048...3071 ist Kommunikations-Input
- Index 3072...4095 ist Kommunikations-Output
- Variablen mit Index 16384..32767 sind "Zwischenvariablen"
- Variablen mit Index >=32768 sind reserviert (ggf für Retained?)
Für Integer gilt
- Index 0 liefert immer 0
- Index 1 liefert immer 1
- tbc
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



