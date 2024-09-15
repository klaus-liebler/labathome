#pragma once

#include <cstdio>
#include <cstdint>
#include <i2c_sensor.hh>
namespace lsm6ds3
{

    constexpr uint8_t ADDRESS{0x6A};

    constexpr uint8_t WHO_AM_I_REG{0X0F};
    constexpr uint8_t CTRL1_XL{0X10};
    constexpr uint8_t CTRL2_G{0X11};

    constexpr uint8_t STATUS_REG{0X1E};

    constexpr uint8_t CTRL6_C{0X15};
    constexpr uint8_t CTRL7_G{0X16};
    constexpr uint8_t CTRL8_XL{0X17};

    constexpr uint8_t OUTX_L_G{0X22};
    constexpr uint8_t OUTX_H_G{0X23};
    constexpr uint8_t OUTY_L_G{0X24};
    constexpr uint8_t OUTY_H_G{0X25};
    constexpr uint8_t OUTZ_L_G{0X26};
    constexpr uint8_t OUTZ_H_G{0X27};

    constexpr uint8_t OUTX_L_XL{0X28};
    constexpr uint8_t OUTX_H_XL{0X29};
    constexpr uint8_t OUTY_L_XL{0X2A};
    constexpr uint8_t OUTY_H_XL{0X2B};
    constexpr uint8_t OUTZ_L_XL{0X2C};
    constexpr uint8_t OUTZ_H_XL{0X2D};

    class M : public I2CSensor
    {
    private:
        float acc_xyz[3];
        float gyro_xyz[3];
    public:
        M(i2c_master_bus_handle_t bus_handle):I2CSensor(bus_handle, ADDRESS){}
        ErrorCode Trigger(int64_t &waitTillReadout) override{
            waitTillReadout=20;
            return ErrorCode::OK
        }
        ErrorCode Readout(int64_t &waitTillNExtTrigger) override{
            // Results are in g (earth gravity).
            int16_t data[6];
            RETURN_ON_ERRORCODE(ReadRegs8(OUTX_L_G, (uint8_t *)data, sizeof(data)));

            // Results are in degrees/second.
            gyro_xyz[0] = data[0] / 131.0;
            gyro_xyz[1] = data[1] / 131.0;
            gyro_xyz[2] = data[2] / 131.0;
            acc_xyz[0] = data[3] / 8192.0;
            acc_xyz[1] = data[4] / 8192.0;
            acc_xyz[2] = data[5] / 8192.0;
            waitTillNExtTrigger=0;
            return ErrorCode::OK;
        }
        // Precondition: dev_handle exists!
        ErrorCode Initialize(int64_t &waitTillFirstTrigger) override
        {
            waitTillFirstTrigger=0;
            uint8_t buf[8];
            RETURN_ON_ERRORCODE(ReadRegs8(WHO_AM_I_REG, buf, 1));
            if (*buf != 0x6C && *buf != 0x69)
            {
                return ErrorCode::UNKNOWN_HARDWARE_ID;
            }
            // Set the Accelerometer control register to work at 104 Hz, 4 g,and in bypass mode and enable ODR/4
            // low pass filter (check figure9 of LSM6DS3's datasheet)
            RETURN_ON_ERRORCODE(this->WriteReg8(CTRL1_XL, 0x4A));

            // set the gyroscope control register to work at 104 Hz, 250 dps and in bypass mode
            RETURN_ON_ERRORCODE(this->WriteReg8(CTRL2_G, 0x40));

            // set gyroscope power mode to high performance and bandwidth to 16 MHz
            RETURN_ON_ERRORCODE(this->WriteReg8(CTRL7_G, 0x00));

            // Set the ODR config register to ODR/4
            return this->WriteReg8(CTRL8_XL, 0x09);
        }

        float accelerationSampleRate() { return 104.0F; }
        
        bool accelerationAvailable()
        {
            uint8_t buf{0};
            ReadRegs8(STATUS_REG, &buf, 1);
            return buf & 0x01 == 1;
        }
        
        const float* GetAccXYZ(){
            return this->acc_xyz;
        }
        
        const float* GetGyroXYZ(){
            return this->gyro_xyz;
        }
        
        float gyroscopeSampleRate()
        {
            return 104.0F;
        }
        
        bool gyroscopeAvailable()
        {
            uint8_t buf{0};
            ReadRegs8(STATUS_REG, &buf, 1);
            return buf & 0x02 == 1;
        }
    }
}