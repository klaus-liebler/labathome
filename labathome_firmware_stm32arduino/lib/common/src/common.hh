#pragma once
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <array>


#define ALL4  __attribute__ ((aligned (16)))

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef int64_t tms_t;

template <typename T>
void SetBitIdx(T &value, const int bitIdx) {
    value |= (1 << bitIdx);
}

template <typename T>
void SetBitMask(T &value, const int bitMask) {
    value |= bitMask;
}

template <typename T>
bool GetBitIdx(const T value, const int bitIdx) {
    return value & (1L << bitIdx);
}

template <typename T>
bool GetBitMask(const T value, const int bitMask) {
    return value & bitMask;
}

template <typename T>
inline void ClearBitIdx(T &value, const int bitIdx)
{
    value &= ~(1L << bitIdx);
}

template <typename T>
inline void ClearBitMask(T &value, const int bitMask)
{
    value &= ~(bitMask);
}

template <typename T> 
bool IntervalIntersects(const T& aLow, const T& aHigh, const T& bLow, const T& bHigh) {
    T min = std::max(aLow, bLow);
    T max =  std::min(aHigh, bHigh);
    return  max>=min;
}

template <size_t K>
bool GetBitInU8Array(const std::array<uint8_t, K> *arr, size_t offset, size_t bitIdx){
    uint8_t b = (*arr)[offset + (bitIdx>>3)];
    uint32_t bitpos = bitIdx & 0b111;
    return b & (1<<bitpos);
}



template<class T>
constexpr const T clamp_kl( const T v, const T lo, const T hi)
{
    return v<lo?lo:v>hi?hi:v;
}

template <typename T>
T clip(const T& n, const T& lower, const T& upper) {
  return std::max(lower, std::min(n, upper));
}

size_t byteBuf2hexCharBuf(char* charBuf, size_t charBufLen, const uint8_t* byteBuf, size_t byteBufLen);



bool GetBitInU8Buf(const uint8_t *buf, size_t offset, size_t bitIdx);

void WriteI8(int8_t value, uint8_t *buffer, uint32_t offset);
void WriteI16(int16_t value, uint8_t *buffer, uint32_t offset);
void WriteI32(int32_t value, uint8_t *buffer, uint32_t offset);
void WriteI64(int64_t value, uint8_t *buffer, uint32_t offset);

int16_t ParseI16(const uint8_t * const buffer, uint32_t offset);
int32_t ParseI32(const uint8_t * const buffer, uint32_t offset);

void WriteU8(uint8_t value, uint8_t *buffer, uint32_t offset);
void WriteU16(uint16_t value, uint8_t *buffer, uint32_t offset);
void WriteU32(uint32_t value, uint8_t *buffer, uint32_t offset);

uint8_t ParseU8(const uint8_t * const buffer, uint32_t offset);
uint16_t ParseU16(const uint8_t * const buffer, uint32_t offset);
uint32_t ParseU32(const uint8_t * const buffer, uint32_t offset);
uint64_t ParseU64(const uint8_t * const buffer, uint32_t offset);


float ParseF32(const uint8_t * const buffer, uint32_t offset);

void WriteI16_BigEndian(int16_t value, uint8_t *buffer, uint32_t offset);
int16_t ParseI16_BigEndian(const uint8_t *const buffer, uint32_t offset);

void WriteU16_BigEndian(uint16_t value, uint8_t *buffer, uint32_t offset);
void WriteU32_BigEndian(uint32_t value, uint8_t *buffer, size_t offset);

uint16_t ParseU16_BigEndian(const uint8_t *const buffer, size_t offset);
uint32_t ParseU32_BigEndian(const uint8_t *const buffer, size_t offset);