#pragma once

#include <cstring>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_timer.h"
#include "esp_log.h"

#include "lsm6ds3.hh"
#include "kalman.hh"

#define RESTRICT_PITCH // Comment out to restrict roll to ±90deg instead
#define RAD_TO_DEG (180.0 / M_PI)
#define DEG_TO_RAD 0.0174533

namespace imu_kalmanXY
{
    class M
    {
    private:
        Kalman kalmanX; // Create the Kalman instances
        Kalman kalmanY;
        int64_t lastTime{0};

        double roll, pitch;          // Roll and pitch are calculated using the accelerometer
        double kalAngleX, kalAngleY; // Calculated angle using a Kalman filter
    public:
        void getRollPitch(const float *acc, double *roll, double *pitch)
        {
            // atan2 outputs the value of - to	(radians) - see http://en.wikipedia.org/wiki/Atan2
            // It is then converted from radians to degrees
#ifdef RESTRICT_PITCH // Eq. 25 and 26
            *roll = atan2(acc[1], acc[2]) * RAD_TO_DEG;
            *pitch = atan(-acc[0] / sqrt(acc[1] * acc[1] + acc[2] * acc[2])) * RAD_TO_DEG;
#else // Eq. 28 and 29
            *roll = atan(accY / sqrt(accX * accX + accZ * accZ)) * RAD_TO_DEG;
            *pitch = atan2(-accX, accZ) * RAD_TO_DEG;
#endif
        }

        void processSetup(lsm6ds3::M *imu)
        {

            const float *acc = imu->GetAccXYZ();
            const float *gyro = imu->GetGyroXYZ();
            getRollPitch(accXYZ, &roll, &pitch);
            kalAngleX = roll;
            kalAngleY = pitch;
            kalmanX.setAngle(roll); // Set starting angle
            kalmanY.setAngle(pitch);
            lastTime = esp_timer_get_time();
        }

        void processLoop(lsm6ds3::M *imu)
        {
            const float *acc = imu->GetAccXYZ();
            const float *gyro = imu->GetGyroXYZ();
            getRollPitch(accXYZ, &roll, &pitch);
            int64_t now = esp_timer_get_time();

            double dt = (double)(now - lastTime) / 1000000; // Calculate delta time
            lastTime = now;

            /* Roll and pitch estimation */
            double gyroXrate = gyro[0];
            double gyroYrate = gyro[1];

            // This fixes the transition problem when the accelerometer angle jumps between -180 and 180 degrees
            if ((roll < -90 && kalAngleX > 90) || (roll > 90 && kalAngleX < -90))
            {
                kalmanX.setAngle(roll);
                kalAngleX = roll;
            }
            else
                kalAngleX = kalmanX.getAngle(roll, gyroXrate, dt); // Calculate the angle using a Kalman filter

            if (abs(kalAngleX) > 90)
                gyroYrate = -gyroYrate; // Invert rate, so it fits the restriced accelerometer reading
            kalAngleY = kalmanY.getAngle(pitch, gyroYrate, dt);
        }
    }
}