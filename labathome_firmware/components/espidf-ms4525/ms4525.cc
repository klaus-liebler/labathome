#include "include/ms4525.hh"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "math.h"
#include <i2c.hh>


MS4525DO::MS4525DO(i2c_port_t i2c_port, MS4523_Adress address):i2c_port(i2c_port), address((uint8_t)address)
{
}

 
esp_err_t MS4525DO::Init()
{
    return I2C::IsAvailable(this->i2c_port, this->address);
}
 

 MS4525_Status MS4525DO::GetStatus()
 {
     return _status;
 }
 
esp_err_t MS4525DO::Read()
{
    uint8_t Press_H;
    uint8_t Press_L;
    uint8_t Temp_H;
    uint8_t Temp_L;
    uint8_t data[4];
    esp_err_t ret = I2C::Read(this->i2c_port, this->address, data, 4);
    if(ret!=ESP_OK) return ret;
    Press_H = data[0];
    Press_L = data[1];
    Temp_H = data[2];
    Temp_L = data[3];
    _status = (MS4525_Status)((Press_H >> 6) & 0x03);
    Press_H = Press_H & 0x3f;
    P_dat = (((uint16_t)Press_H) << 8) | Press_L;
    Temp_L = (Temp_L >> 5);
    T_dat = (((uint16_t)Temp_H) << 3) | Temp_L;
    return ret;
}
 
float MS4525DO::GetPSI(){             // returns the PSI of last measurement
    // convert and store PSI
    float psi=(static_cast<float>(static_cast<int16_t>(P_dat)-MS4525ZeroCounts))/static_cast<float>(MS4525Span)*static_cast<float>(MS4525FullScaleRange);
    // apply PSI calibration data
    //   psi = psi + 0.007f;
    
    /* Below code is Pixhawk code which doesnt seem to work correctly */
    // Calculate differential pressure. As its centered around 8000
    // and can go positive or negative
    /*
    const float P_min = -1.0f;
    const float P_max = 1.0f;
    const float PSI_to_Pa = 6894.757f;
    */ 
    /*
      this equation is an inversion of the equation in the
      pressure transfer function figure on page 4 of the datasheet
      We negate the result so that positive differential pressures
      are generated when the bottom port is used as the static
      port on the pitot and top port is used as the dynamic port
     */
     /*
    float diff_press_PSI = -((T_dat - 0.1f * 16383) * (P_max - P_min) / (0.8f * 16383) + P_min);
    float diff_press_pa_raw = diff_press_PSI * PSI_to_Pa;
    */
    return psi;
}             
  
float MS4525DO::GetTemperature(void){     // returns temperature of last measurement
    float temperature= (static_cast<float>(static_cast<int16_t>(T_dat)));
    temperature = (temperature / 10);   // now in deg F
    temperature = ((temperature -32) / 1.8f);   // now in deg C
    return temperature;
}
 
float MS4525DO::GetAirSpeedMetersPerSecond(void){        // calculates and returns the airspeed
    float psi = GetPSI();
    /* Velocity calculation from a pitot tube explanation */
    /* +/- 1PSI, approximately 100 m/s */
    float rho = 1.225; // density of air 
    // velocity = squareroot( (2*differential) / rho )
    float velocity;
    if (psi<0) {
        velocity = -sqrt(-(2*psi) / rho);
    }else{
        velocity = sqrt((2*psi) / rho);
        }
    velocity = velocity*10;
    
    return velocity;
}