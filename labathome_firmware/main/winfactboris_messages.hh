#pragma once
#include <stdint.h>









//Wir schicken an BORIS immer alle Inputs zurück und Boris soll auswählen. Inputs, die wir nicht kennen werden
// - bei bools, die in einem u8 codiert werden, als 0xFF gesetzt
// - bei ints entscheidet ein davor gesetzter BorisIsValid, ob der Wert gültig ist (0x01) oder nicht (alle anderen Werte, insbesondere auch 0x00 und 0xFF)
// - bei floats als NaN gesetzt
// Boris schickt mir immer alle Outputs. Wenn Boris einen Output nicht aktiv setzen möchte, schickt es bei bools ein 0xFF und bei floats ein NaN


//wenn später Outputs/Inputs dazu kommen, dann wird die MessageId angepasst. Zunächst gehen wir vom aktuellen Stand der Hardware aus
typedef float BorisFloat;
typedef uint8_t BorisBool;
typedef uint8_t BorisIsValid;
typedef int32_t BorisInt;
typedef uint32_t BorisMessageType;
typedef uint32_t BorisColor;

constexpr BorisMessageType MESSAGE_TYPE_CONFIG{1};
constexpr BorisMessageType MESSAGE_TYPE_OUTPUTDATA{2};
constexpr BorisMessageType MESSAGE_TYPE_INPUTDATA{3};
constexpr BorisIsValid VALID{0x01};


struct MessageConfig{
    BorisMessageType MessageType;
    BorisInt mode_HEATER_OR_LED_POWER;
    BorisInt mode_ROT_LDR_ANALOGIN;
    BorisInt mode_MOVEMENT_OR_FAN1SENSE;
    BorisInt mode_FAN1_DRIVE_OR_SERVO1;
};

//HAL kennt ein Prozessabbild "Input" ein "Prozessabbild Output". Die HAL muss dieses 
struct MessageInputData{
    BorisMessageType MessageType;//4byte
    BorisBool ButtonRed; //ab hier 6*1byte=6byte
    BorisBool ButtonYellow;
    BorisBool ButtonGreen;
    BorisBool MovementSensor;
    BorisIsValid IncrementalEncoderIsValid;
    BorisIsValid SoundIsValid;
    //hier 2byte paading
    BorisInt IncrementalEncoderDetents; //ab hier 14*4=56byte
    BorisInt SoundValue;
    BorisFloat UsbSupplyVoltageVolts;
    BorisFloat AmbientBrightnessLux;
    BorisFloat HeaterTemperatureDegCel;
    BorisFloat AirTemperatureDegCel;
    BorisFloat AirPressurePa;
    BorisFloat AirRelHumidityPercent; //0...100
    BorisFloat AirCo2PPM;
    BorisFloat AirQualityPercent; //0...100
    BorisFloat AirSpeedMeterPerSecond;
    BorisFloat WifiSignalStrengthDB;
    BorisFloat AnalogInputVolt;
    BorisFloat Fan1RotationsRpM;
};

struct MessageOutputData
{
    BorisMessageType MessageType;
    BorisBool RelayK3;
    BorisIsValid SoundIsValid;
    BorisInt SoundValue;
    BorisFloat UsbSupplyVoltageVolts;
    BorisFloat DutyHeaterPercent;//0...100
    BorisFloat DutyPowerLedPercent;//0...100
    BorisFloat DutyFan1Percent;//0...100
    BorisFloat DutyFan2Percent;//0...100
    BorisFloat AngleServo1Degress;//0..180
    BorisFloat AngleServo2Degress;//0..180
    BorisFloat AnalogOutputVolts;
    BorisColor LED0;
    BorisColor LED1;
    BorisColor LED2;
    BorisColor LED3;
    
};

