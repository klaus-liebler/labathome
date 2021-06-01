#pragma once

#include <inttypes.h>
#include <errorcodes.hh>

enum class Direction{
  DIRECT,
  REVERSE
};
enum class Mode{
  OFF, //output is not written
  OPENLOOP, //output equals input
  CLOSEDLOOP, //output is set according to PID algorithm
};

class PIDController
{

  public:
    PIDController(
      double* input,
      double* output,
      double* setpoint,
      double outputMin,
      double outputMax,
      Mode mode,
      Direction direction,
      uint32_t cycleTimeMs
      );
    ErrorCode Compute(uint32_t nowMs);
    ErrorCode SetMode(Mode mode, uint32_t nowMs);
    ErrorCode SetKpTnTv(double kp, double tn_seconds, double tv_seconds);
    ErrorCode Reset();
   
										  


  private:
    void Initialize();

    double *input;              // * Pointers to the Input, Output, and Setpoint variables
    double *output;             //   This creates a hard link between the variables and the 
    double *setpoint;           //   PID, freeing the user from having to constantly tell us
                                  //   what these values are.  with pointers we'll just know.

    double kp;
    double ki;
    double kd;

    double kpAsSetByUser=0.0;
    double tnAsSetByUser=0.0;
    double tvAsSetByUser=0.0;

    double outputMin, outputMax;

    Mode mode;
    Direction direction;	  
    uint32_t cycleTimeMs;
	  
    uint32_t lastTimeMs;
    double Y_I;
    double last_Y_P;
};


