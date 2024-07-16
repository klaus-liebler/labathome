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
typedef int BorisInt;
typedef uint32_t BorisMessageType;
typedef uint32_t BorisColor;

constexpr BorisMessageType MESSAGE_TYPE_CONFIG{1};
constexpr BorisMessageType MESSAGE_TYPE_OUTPUTDATA_LABATHOME{2};
constexpr BorisMessageType MESSAGE_TYPE_INPUTDATA_LABATHOME{3};
constexpr BorisMessageType MESSAGE_TYPE_OUTPUTDATA_PTNCHEN{4};
constexpr BorisMessageType MESSAGE_TYPE_INPUTDATA_PTNCHEN{5};
constexpr BorisIsValid VALID{0x01};


struct MessageConfig{
    BorisMessageType MessageType;//4byte integer; Wert=1
    BorisInt mode_HEATER_OR_LED_POWER;//4byte integer, Wert siehe entsprechende Enumeration, Schiebeschalter wählt zwischen Heizwiderstand / PowerLEDs aus
    BorisInt mode_ROT_LDR_ANALOGIN;//4byte integer, Wert siehe entsprechende Enumeration, Schiebeschalter und Softwarekonfiguration eines Pins wählt zwischen RotaryEncoder / Analoger Spannungseingang / Lichtabhängiger Widerstand / Analoger Spannungseingang UND Lichtabhängiger Widerstand 
    BorisInt mode_MOVEMENT_OR_FAN1SENSE;//4byte integer, Wert siehe entsprechende Enumeration, Schiebeschalter wählt zwischen Bewegungssensor / Tacho-Eingang Lüfter 1 aus
    BorisInt mode_FAN1_DRIVE_OR_SERVO1;//4byte integer, Wert siehe entsprechende Enumeration, Über Softwarekonfiguration wird festgelegt, ob Servo1 oder FAN1 angesteuert werden soll
};

//HAL kennt ein Prozessabbild "Input" ein "Prozessabbild Output". Die HAL muss dieses 
struct MessageInputDataLabAtHome{
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

struct MessageOutputDataLabAtHome
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

struct MessageInputDataPtnchen{
    BorisMessageType MessageType;//4byte
    BorisBool Button; //4byte
    BorisFloat INPUT;//4
    BorisFloat PTN1;//4
    BorisFloat PTN2;//4
    BorisFloat PTN3;//4
};

struct MessageOutputDataPtnchen
{
    BorisMessageType MessageType;
    BorisFloat AnalogOutputVolts;
    BorisColor LED0;
    BorisColor LED1;
    BorisColor LED2;
    BorisColor LED3; 
};

