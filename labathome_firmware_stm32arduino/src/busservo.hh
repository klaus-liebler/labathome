#pragma once
#include <algorithm>
#include <common.hh>
#include <HardwareSerial.h>
#include <gpioG4.hh> // Include the header file that defines GPIO

enum class Instruction
{
    PING = 0x01,
    READ = 0x02,
    WRITE = 0x03,
    REG_WRITE = 0x04,
    ACTION = 0x05,
    FACTORY_RESET = 0x06,
    REBOOT = 0x08,
    CLEAR = 0x10,
    STATUS = 0x55,
    SYNC_WRITE = 0x83,
    BULK_WRITE = 0x84,
};

namespace ST3215_MEMORY {

    //memory table definition
    //-------EPROM(read only)--------
    constexpr int MODEL_L {3};
    constexpr int MODEL_H {4};

    //-------EPROM(read & write)--------
    constexpr int ID {5};
    constexpr int BAUD_RATE {6};
    constexpr int MIN_ANGLE_LIMIT_L {9};
    constexpr int MIN_ANGLE_LIMIT_H {10};
    constexpr int MAX_ANGLE_LIMIT_L {11};
    constexpr int MAX_ANGLE_LIMIT_H {12};
    constexpr int CW_DEAD {26};
    constexpr int CCW_DEAD {27};
    constexpr int OFS_L {31};
    constexpr int OFS_H {32};
    constexpr int MODE {33};

    //-------SRAM(read & write)--------
    constexpr int TORQUE_ENABLE {40};
    constexpr int ACC {41};
    constexpr int GOAL_POSITION_L {42};
    constexpr int GOAL_POSITION_H {43};
    constexpr int GOAL_TIME_L {44};
    constexpr int GOAL_TIME_H {45};
    constexpr int GOAL_SPEED_L {46};
    constexpr int GOAL_SPEED_H {47};
    constexpr int TORQUE_LIMIT_L {48};
    constexpr int TORQUE_LIMIT_H {49};
    constexpr int LOCK {55};

    //-------SRAM(read only)--------
    constexpr int CURRENT_POSITION_L {56};
    constexpr int CURRENT_POSITION_H {57};
    constexpr int CURRENT_SPEED_L {58};
    constexpr int CURRENT_SPEED_H {59};
    constexpr int CURRENT_LOAD_L {60};
    constexpr int CURRENT_LOAD_H {61};
    constexpr int CURRENT_VOLTAGE {62};
    constexpr int CURRENT_TEMPERATURE {63};
    constexpr int MOVING {66};
    constexpr int PRESENT_CURRENT_L {69};
    constexpr int PRESENT_CURRENT_H {70};

} // namespace SMS_STS

constexpr uint32_t BAUD_RATES[]{1000000, 500000, 250000, 128000, 115200, 76800, 57600, 38400};

template<bool consumeLoopback, GPIO::Pin TX_PIN>
class ServoBus
{
public:
    ServoBus(HardwareSerial *serial) : serial(serial)
    {
        GPIO::SetPinPullMode(TX_PIN, GPIO::PullMode::PullUp);
    }

    int Ping(u8 servoId)
    {
        sendAndReceive(servoId, Instruction::PING, nullptr, 0, nullptr, 0);
        return error;
    }

    int WritePosEx(uint8_t servoId, int16_t Position, uint16_t Speed, uint8_t ACC = 0)
    {
        if (Position < 0)
        {
            Position = -Position;
            Position |= (1 << 15);
        }
        u8 params[8];
        WriteU8(ST3215_MEMORY::ACC, params, 0);
        WriteU8(ACC, params, 1);
        WriteU16(Position, params, 2);
        WriteU16(0, params, 4);
        WriteU16(Speed, params, 6);
        sendAndReceive(servoId, Instruction::WRITE, params, sizeof(params), nullptr,0);
        return error;
    }

    int ReadVoltage(u8 servoId)
    {
        u8 data;
        ReadData(servoId, ST3215_MEMORY::CURRENT_VOLTAGE, &data, 1);
        return data;
    }

    int ReadData(u8 servoId, u8 memAddr, u8 *data, u8 dataLength)
    {
        u8 params[]{memAddr, dataLength};
        sendAndReceive(servoId, Instruction::READ, params, sizeof(params), data, dataLength);
        return error;
    }

    int WriteData(u8 servoId, u8 memAddr, u8 *data, size_t dataLength)
    {
        u8 params[dataLength + 2];
        params[0] = memAddr;
        params[1] = dataLength;
        for (int i = 0; i < dataLength; i++)
        {
            params[2 + i] = data[i];
        }
        sendAndReceive(servoId, Instruction::WRITE, params, sizeof(params), nullptr, 0);
        return error;
    }

    int WriteRegister(u8 servoId, u8 memAddr, u8 *data, size_t dataLength)
    {
        u8 params[dataLength + 2];
        params[0] = memAddr;
        params[1] = dataLength;
        for (int i = 0; i < dataLength; i++)
        {
            params[2 + i] = data[i];
        }
        sendAndReceive(servoId, Instruction::REG_WRITE, params, sizeof(params), nullptr, 0);
        return error;
    }

    int Action(u8 servoId)
    {
        sendAndReceive(servoId, Instruction::ACTION, nullptr, 0, nullptr, 0);
        return error;
    }

    int Reset(u8 servoId)
    {
        sendAndReceive(servoId, Instruction::FACTORY_RESET, nullptr, 0, nullptr, 0);
        return error;
    }

private:
    int error=0;

    void sendAndReceive(u8 servoId, Instruction instruction, const u8 *txParams, size_t txParamsLength, u8 *rxParams, size_t rxParamsLength)
    {
        serial->flush();
        
        u8 txBuf[5];
        txBuf[0] = 0xff;
        txBuf[1] = 0xff;
        txBuf[2] = servoId;
        txBuf[3] = txParamsLength + 2;
        txBuf[4] = (uint8_t)instruction;
        while (!(serial->getHandle()->Instance->ISR & USART_ISR_TC));
        GPIO::SetPinMode(TX_PIN, GPIO::Mode::AlternateFunction);
        serial->write(txBuf, sizeof(txBuf));
        u8 CheckSum = txBuf[2] + txBuf[3] + txBuf[4];
        if (txParams && txParamsLength>0)
        {
            for (int i = 0; i < txParamsLength; i++)
            {
                CheckSum += txParams[i];
            }
            serial->write(txParams, txParamsLength);
        }
        serial->write(~CheckSum);
        while (!(serial->getHandle()->Instance->ISR & USART_ISR_TC));
        GPIO::SetPinMode(TX_PIN, GPIO::Mode::Input);
        serial->flush();
        uint8_t rxBuf[std::max(6+txParamsLength, 6+rxParamsLength)];
        if(consumeLoopback){
            serial->readBytes(rxBuf, sizeof(rxBuf));
        }
 
        if(serial->readBytes(rxBuf, 6+rxParamsLength)!=6+rxParamsLength){
            log_error("Failed to read response");
            error=-1;
            return;
        }
        if(rxBuf[0]!=0xff || rxBuf[1]!=0xff || rxBuf[2]!=servoId || rxBuf[3]!=(rxParamsLength+2) || rxBuf[4]!=0){
            log_error("Invalid response 0xff 0xff %d %d %d", rxBuf[2], rxBuf[3], rxBuf[4]);
            error=-2;
            return;
        }
        CheckSum = rxBuf[2] + rxBuf[3] + rxBuf[4];
        if (rxParams && rxParamsLength>0)
        {
            for (int i = 0; i < rxParamsLength; i++)
            {
                CheckSum += rxBuf[5+i];
            }
            
        }
        if((uint8_t)~CheckSum!=rxBuf[5+rxParamsLength]){
            log_error("Invalid checksum 0xff 0xff %d %d %d %d %d", rxBuf[2], rxBuf[3], rxBuf[4], rxBuf[5+rxParamsLength], (uint8_t)~CheckSum);
            error=-3;
            return;
        }
        if (rxParams && rxParamsLength>0)
        {
            for (int i = 0; i < rxParamsLength; i++)
            {
                rxParams[i]=rxBuf[5+i];
            }   
        }
        error=rxBuf[4];
        return;
    }

    void writeBuf(u8 ID, u8 MemAddr, u8 *nDat, u8 nLen, Instruction inst)
    {
        u8 msgLen = 2;
        u8 bBuf[6];
        u8 CheckSum = 0;
        bBuf[0] = 0xff;
        bBuf[1] = 0xff;
        bBuf[2] = ID;
        bBuf[4] = (uint8_t)inst;
        if (nDat)
        {
            msgLen += nLen + 1;
            bBuf[3] = msgLen;
            bBuf[5] = MemAddr;
            serial->write(bBuf, 6);
        }
        else
        {
            bBuf[3] = msgLen;
            serial->write(bBuf, 5);
        }
        CheckSum = ID + msgLen + (uint8_t)inst + MemAddr;
        u8 i = 0;
        if (nDat)
        {
            for (i = 0; i < nLen; i++)
            {
                CheckSum += nDat[i];
            }
            serial->write(nDat, nLen);
        }
        serial->write(~CheckSum);
    }
private:
    HardwareSerial *serial;
};