#pragma once
#include <common.hh>
#include <errorcodes.hh>
#include <driver/i2c.h>
#include <i2c.hh>


class I2CSensor{
    enum class STATE{
        INITIAL,
        FOUND,
        INITIALIZED,
        TRIGGERED,
        RETRIGGERED,
        READOUT,
        ERROR_NOT_FOUND,
        ERROR_COMMUNICATION,

    };
    private:
        int64_t nextAction{INT64_MAX};
        I2CSensor::STATE state{STATE::INITIAL};
    protected:
        i2c_port_t i2c_num;
        uint8_t address_7bit;
        I2CSensor(i2c_port_t i2c_num, uint8_t address_7bit):i2c_num(i2c_num), address_7bit(address_7bit){}
        virtual esp_err_t Trigger(int64_t& waitTillReadout)=0;
        virtual esp_err_t Readout(int64_t& waitTillNExtTrigger)=0;
        virtual esp_err_t Initialize(int64_t& waitTillFirstTrigger)=0;
        void ReInit(){
            this->state=STATE::FOUND;
        }
    public:

    virtual bool HasValidData(){
        return state == STATE::READOUT || state == STATE::RETRIGGERED;
    }

    ErrorCode Loop(int64_t currentMs){
        esp_err_t e;
        int64_t wait{0};
        switch (state)
        {
        case STATE::INITIAL:
            if(ESP_OK!= I2C::IsAvailable(i2c_num, address_7bit)){
                state = STATE::ERROR_NOT_FOUND;
                return ErrorCode::DEVICE_NOT_RESPONDING;
            }
            state=STATE::FOUND;
            break;
        case STATE::FOUND:
            e=Initialize(wait);
            if(e!=ESP_OK){
                state = STATE::ERROR_COMMUNICATION;
                return ErrorCode::DEVICE_NOT_RESPONDING;
            }
            nextAction=currentMs+wait;
            state=STATE::INITIALIZED;
            break;
        case STATE::INITIALIZED:
        case STATE::READOUT:
            if(currentMs<nextAction) return ErrorCode::OK;
            e=Trigger(wait);
            if(e!=ESP_OK){
                state = STATE::ERROR_COMMUNICATION;
                return ErrorCode::DEVICE_NOT_RESPONDING;
            }
            state=state==STATE::INITIALIZED?STATE::TRIGGERED:STATE::RETRIGGERED;
            break;
        case STATE::TRIGGERED:
            if(currentMs<nextAction) return ErrorCode::OK;
            e=Readout(wait);
            if(e!=ESP_OK){
                state = STATE::ERROR_COMMUNICATION;
                return ErrorCode::DEVICE_NOT_RESPONDING;
            }
            state=STATE::READOUT;
            break;   
        default:
            break;
        }
        return ErrorCode::OK;

    }
};
