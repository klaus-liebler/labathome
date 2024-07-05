#include "common.hh"

bool GetBitInU8Buf(const uint8_t *buf, size_t offset, size_t bitIdx)
{
	uint8_t b = buf[offset + (bitIdx >> 3)];
	uint32_t bitpos = bitIdx & 0b111;
	return b & (1 << bitpos);
}

void WriteI8(int8_t value, uint8_t *buffer, uint32_t offset)
{
	uint8_t *ptr1 = (uint8_t *)&value;
	*(buffer + offset) = *ptr1;
}

void WriteI16(int16_t value, uint8_t *buffer, uint32_t offset)
{
	uint8_t *ptr1 = (uint8_t *)&value;
	uint8_t *ptr2 = ptr1 + 1;
	*(buffer + offset) = *ptr1;
	*(buffer + offset + 1) = *ptr2;
}

void WriteI32(int32_t value, uint8_t *buffer, uint32_t offset)
{
	buffer[0 + offset] = (value & 0xFF) >> 0;
	buffer[1 + offset] = (value & 0xFFFF) >> 8;
	buffer[2 + offset] = (value & 0xFFFFFF) >> 16;
	buffer[3 + offset] = (value & 0xFFFFFFFF) >> 24;
}

void WriteI64(int64_t value, uint8_t *buffer, uint32_t offset)
{
	buffer[0 + offset] = (value & 0xFF) >> 0;
	buffer[1 + offset] = (value & 0xFFFF) >> 8;
	buffer[2 + offset] = (value & 0xFFFFFF) >> 16;
	buffer[3 + offset] = (value & 0xFFFFFFFF) >> 24;
	buffer[4 + offset] = (value & 0xFFFFFFFFFF) >> 32;
	buffer[5 + offset] = (value & 0xFFFFFFFFFFFF) >> 40;
	buffer[6 + offset] = (value & 0xFFFFFFFFFFFFFF) >> 48;
	buffer[7 + offset] = (value & 0xFFFFFFFFFFFFFFFF) >> 56;
}

int16_t ParseI16(const uint8_t *const buffer, uint32_t offset)
{
	int16_t step;
	uint8_t *ptr1 = (uint8_t *)&step;
	uint8_t *ptr2 = ptr1 + 1;
	*ptr1 = *(buffer + offset);
	*ptr2 = *(buffer + offset + 1);
	return step;
}

int32_t ParseI32(const uint8_t *const buffer, uint32_t offset)
{
	int32_t value;
	uint8_t *ptr0 = (uint8_t *)&value;
	uint8_t *ptr1 = ptr0 + 1;
	uint8_t *ptr2 = ptr0 + 2;
	uint8_t *ptr3 = ptr0 + 3;
	*ptr0 = *(buffer + offset + 0);
	*ptr1 = *(buffer + offset + 1);
	*ptr2 = *(buffer + offset + 2);
	*ptr3 = *(buffer + offset + 3);
	return value;
}

void WriteU8(uint8_t value, uint8_t *buffer, uint32_t offset)
{
	*(buffer + offset) = value;
}

void WriteU16(uint16_t value, uint8_t *buffer, uint32_t offset)
{
	uint8_t *ptr1 = (uint8_t *)&value;
	uint8_t *ptr2 = ptr1 + 1;
	*(buffer + offset) = *ptr1;
	*(buffer + offset + 1) = *ptr2;
}

void WriteU32(uint32_t value, uint8_t *buffer, uint32_t offset)
{
	buffer[0 + offset] = (value & 0xFF) >> 0;
	buffer[1 + offset] = (value & 0xFFFF) >> 8;
	buffer[2 + offset] = (value & 0xFFFFFF) >> 16;
	buffer[3 + offset] = (value & 0xFFFFFFFF) >> 24;
}


uint8_t ParseU8(const uint8_t *const buffer, uint32_t offset){
	return buffer[offset];
}

uint16_t ParseU16(const uint8_t *const buffer, uint32_t offset)
{
	uint16_t step;
	uint8_t *ptr1 = (uint8_t *)&step;
	uint8_t *ptr2 = ptr1 + 1;
	*ptr1 = *(buffer + offset);
	*ptr2 = *(buffer + offset + 1);
	return step;
}

uint32_t ParseU32(const uint8_t *const buffer, uint32_t offset)
{
	uint32_t value;
	uint8_t *ptr0 = (uint8_t *)&value;
	uint8_t *ptr1 = ptr0 + 1;
	uint8_t *ptr2 = ptr0 + 2;
	uint8_t *ptr3 = ptr0 + 3;
	*ptr0 = *(buffer + offset + 0);
	*ptr1 = *(buffer + offset + 1);
	*ptr2 = *(buffer + offset + 2);
	*ptr3 = *(buffer + offset + 3);
	return value;
}

uint64_t ParseU64(const uint8_t *const buffer, uint32_t offset)
{
	uint64_t step;
	uint8_t *ptr0 = (uint8_t *)&step;
	uint8_t *ptr1 = ptr0 + 1;
	uint8_t *ptr2 = ptr0 + 2;
	uint8_t *ptr3 = ptr0 + 3;
	uint8_t *ptr4 = ptr0 + 4;
	uint8_t *ptr5 = ptr0 + 5;
	uint8_t *ptr6 = ptr0 + 6;
	uint8_t *ptr7 = ptr0 + 7;
	*ptr0 = *(buffer + offset + 0);
	*ptr1 = *(buffer + offset + 1);
	*ptr2 = *(buffer + offset + 2);
	*ptr3 = *(buffer + offset + 3);
	*ptr4 = *(buffer + offset + 4);
	*ptr5 = *(buffer + offset + 5);
	*ptr6 = *(buffer + offset + 6);
	*ptr7 = *(buffer + offset + 7);
	return step;
}

float ParseF32(const uint8_t *const buffer, uint32_t offset)
{
	float value;
	uint8_t *ptr0 = (uint8_t *)&value;
	uint8_t *ptr1 = ptr0 + 1;
	uint8_t *ptr2 = ptr0 + 2;
	uint8_t *ptr3 = ptr0 + 3;
	*ptr0 = *(buffer + offset + 0);
	*ptr1 = *(buffer + offset + 1);
	*ptr2 = *(buffer + offset + 2);
	*ptr3 = *(buffer + offset + 3);
	return value;
}

void WriteI16_BigEndian(int16_t value, uint8_t *buffer, uint32_t offset)
{
	uint8_t *ptr1 = (uint8_t *)&value;
	uint8_t *ptr2 = ptr1 + 1;
	*(buffer + offset) = *ptr2;
	*(buffer + offset + 1) = *ptr1;
}

int16_t ParseI16_BigEndian(const uint8_t *const buffer, uint32_t offset)
{
	int16_t step;
	uint8_t *ptr1 = (uint8_t *)&step;
	uint8_t *ptr2 = ptr1 + 1;
	*ptr2 = *(buffer + offset);
	*ptr1 = *(buffer + offset + 1);
	return step;
}

void WriteU16_BigEndian(uint16_t value, uint8_t *buffer, size_t offset)
{
	uint8_t *ptr1 = (uint8_t *)&value;
	uint8_t *ptr2 = ptr1 + 1;
	*(buffer + offset) = *ptr2;
	*(buffer + offset + 1) = *ptr1;
}

void WriteU32_BigEndian(uint32_t value, uint8_t *buffer, size_t offset)
{
	uint8_t *ptr1 = (uint8_t *)&value;
	uint8_t *ptr2 = ptr1 + 1;
	uint8_t *ptr3 = ptr1 + 2;
	uint8_t *ptr4 = ptr1 + 3;
	*(buffer + offset) = *ptr4;
	*(buffer + offset + 1) = *ptr3;
	*(buffer + offset + 2) = *ptr2;
	*(buffer + offset + 3) = *ptr1;
}

uint16_t ParseU16_BigEndian(const uint8_t *const buffer, size_t offset)
{
	uint16_t step;
	uint8_t *ptr1 = (uint8_t *)&step;
	uint8_t *ptr2 = ptr1 + 1;
	*ptr2 = *(buffer + offset);
	*ptr1 = *(buffer + offset + 1);
	return step;
}

uint32_t ParseU32_BE(const uint8_t *const buffer, size_t offset)
{
	uint32_t step;
	uint8_t *ptr1 = (uint8_t *)&step;
	uint8_t *ptr2 = ptr1 + 1;
	uint8_t *ptr3 = ptr1 + 2;
	uint8_t *ptr4 = ptr1 + 3;
	*ptr4 = *(buffer + offset);
	*ptr3 = *(buffer + offset + 1);
	*ptr2 = *(buffer + offset + 2);
	*ptr1 = *(buffer + offset + 3);
	return step;
}




