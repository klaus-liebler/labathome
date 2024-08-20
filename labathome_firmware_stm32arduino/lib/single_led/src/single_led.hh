#pragma once
#include <Arduino.h>
#include <cstdio>
#include <cstdint>
#include <cstring>

#include <array>
#define TAG "SINGLE_LED"

typedef int64_t time_t;

namespace SINGLE_LED
{
    class AnimationPattern
    {
    public:
        virtual void Reset(time_t now) = 0;
        virtual bool Animate(time_t now) = 0;
    };

    class BlinkPattern : public AnimationPattern
    {
    private:
        time_t nextChange{0};
        bool state{false};
        time_t timeOn;
        time_t timeOff;

    public:
        void Reset(time_t now) override
        {
            nextChange = now+timeOn;
            state = true;
        }
        bool Animate(time_t now) override
        {
            if(now < nextChange) return state;
            if (state)
            {
                state = false;
                nextChange = now+timeOff;
            }
            else
            {
                state = true;
                nextChange = now+timeOn;
            }
            return state;
        }
        BlinkPattern(time_t timeOn, time_t timeOff) : timeOn(timeOn), timeOff(timeOff) {}
    };

    class :public AnimationPattern{
        void Reset(time_t now){}
        bool Animate(time_t now){return false;}
    }CONST_OFF;

    class :public AnimationPattern{
        void Reset(time_t now){}
        bool Animate(time_t now){return true;}
    }CONST_ON;


    class M
    {
    private:
    	
        uint32_t gpio{NUM_DIGITAL_PINS};
        bool invert{false};
        AnimationPattern* pattern;
        time_t timeToAutoOff=INT64_MAX;//time is absolute!
        AnimationPattern* standbyPattern;
    public:
        M(uint32_t  gpio, bool invert=false, AnimationPattern* standbyPattern=&CONST_OFF):gpio(gpio), invert(invert), standbyPattern(standbyPattern) {}

        void AnimatePixel(time_t now, AnimationPattern *pattern, time_t timeToAutoOff=0)//time is relative, "0" means: no auto off
        {
            if(pattern==nullptr) this->pattern=standbyPattern;
            
            if(timeToAutoOff==0){
                this->timeToAutoOff=INT64_MAX;
            }else{
                this->timeToAutoOff=now+timeToAutoOff;
            }
            pattern->Reset(now);
            this->pattern = pattern;
        }

        void Loop(time_t now)
        {
            if(now>=timeToAutoOff){
                this->pattern=standbyPattern;
            } 
            bool on = this->pattern->Animate(now);
            
            digitalWrite(gpio, (on^invert)?GPIO_PIN_SET:GPIO_PIN_RESET);
            
        }

        void Begin(time_t now, AnimationPattern *pattern=&CONST_OFF, time_t timeToAutoOff=0)
        {
            this->AnimatePixel(now, pattern, timeToAutoOff);
        }
    };
}
#undef TAG