#include <pidcontroller.hh>
#include "esp_log.h"
constexpr const char *TAG = "plcmanager";

PIDController::PIDController(
      double* input,
      double* output,
      double* setpoint,
      double outputMin,
      double outputMax,
      Mode mode,
      Direction direction,
      uint32_t cycleTimeMs
      ):input(input), output(output),setpoint(setpoint),outputMin(outputMin), outputMax(outputMax), mode(mode), direction(direction), cycleTimeMs(cycleTimeMs)
{}

ErrorCode PIDController::Compute(uint32_t nowMs)
{
   if(this->mode==Mode::OFF) return ErrorCode::TEMPORARYLY_NOT_AVAILABLE;
   uint32_t timeChangeMs = (nowMs - lastTimeMs);
   if(timeChangeMs<cycleTimeMs) return ErrorCode::OBJECT_NOT_CHANGED;
   
   /*Compute all the working error variables*/
   double input = *this->input;
  
   double Y_P = kp*(*this->setpoint - input);
   this->Y_I+= (ki * Y_P);
   double Y_D= kd*(Y_P-last_Y_P);

   //limit integrator
   this->Y_I = Y_I>outputMax?outputMax:Y_I<outputMin?outputMin:Y_I;

   double output = Y_P+Y_I+Y_D;
   
   //limit output & anti integrator windup
   if(output>outputMax){
      output=outputMax;
      this->Y_I-= (ki * Y_P);
   }else if(output<outputMin){
      output=outputMin;
      this->Y_I-= (ki * Y_P);
   }
   
   *this->output= output;

   last_Y_P = Y_P;
   lastTimeMs = nowMs;
   return ErrorCode::OK;
}

ErrorCode PIDController::Reset(){
   this->Y_I=0;
   return ErrorCode::OK;
}

ErrorCode PIDController::SetKpTnTv(double Kp, double Tn_s, double Tv_s)
{
   if(Kp==this->kpAsSetByUser && Tn_s==this->tnAsSetByUser && Tv_s==this->tvAsSetByUser) return ErrorCode::OBJECT_NOT_CHANGED;
   if(Kp<0 || Tn_s<=0 || Tv_s<=0) return ErrorCode::INVALID_ARGUMENT_VALUES;

   this->kpAsSetByUser=Kp;
   this->tnAsSetByUser=Tn_s;
   this->tvAsSetByUser=Tv_s;

   double cycleTimeInSeconds= (double)cycleTimeMs/(double)1000;
   kp = Kp;
   ki = (1/Tn_s)*cycleTimeInSeconds;
   kd = Tv_s/cycleTimeInSeconds;
   ESP_LOGI(TAG, "kp=%F ki=%F kd=%F", kp, ki, kd);
   return ErrorCode::OK;
}


ErrorCode PIDController::SetMode(Mode nextMode, uint32_t nowMs)
{
   if(this->mode==nextMode) return ErrorCode::OBJECT_NOT_CHANGED;
   if(nextMode==Mode::CLOSEDLOOP){
      this->Y_I=0;
      this->last_Y_P = kp*(*this->setpoint - *this->input);
      this->lastTimeMs=nowMs;
    }
    this->mode=nextMode;
    return ErrorCode::OK;
}