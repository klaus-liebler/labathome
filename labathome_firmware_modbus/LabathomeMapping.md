# Modbus Mapping

These are the addresses and the formats at LabAtHome
Adresses are **0-Based**.

## Discrete Output Coils

|Registrer (0-Based)| Function|
|:---:|:---:|
|0|Relay K3|

## Discrete Input Contacts

|Registrer (0-Based)| Function|
|:---:|:---:|
|0|Green Button|
|1|Red Button|
|2|Yellow / Encoder Button |
|3|Movement Sensor|

## Holding Registers

|Registrer (0-Based)| Function|
|:---:|:---:|
| 0| Not connected|
| 1| Servo 0, Position in Degrees 0...180|
| 2| Servo 1, Position in Degrees 0...180|
| 3| Servo 2, Position in Degrees 0...180|
| 4| Servo 3, Position in Degrees 0...180|
| 5| Fan 0, Power in Percent 0...100|
| 6| Fan 1, Power in Percent 0...100|
| 7| Heater, Power in Percent 0...100|
| 8| White Power LED, Power in Percent 0...100|
| 9| RGB LED 0, Color in RGB565|
|10| LED 1|
|11| LED 2|
|12| LED 3|
|13| Relay State (Alternative to Coil 0), 0 means off, all other values on|
|14| Play Sound, 0 means silence; try other values up to 9|

## Input Registers

|Registrer (0-Based)| Function|
|:---:|:---:|
|0| CO2 [PPM]|
|1| Air Pressure [hPa]|
|2| Ambient Brightness [?]|
|3| Analog Input [mV] //CHANNEL_ANALOGIN_OR_ROTB I34|
|4| Button Green [0 or 1]|
|5| Button Red [0 or 1]|
|6| Button Yellow/Encoder [0 or 1]|
|7| Fan 1 RpM|
|8| Heater Temperature [°C * 100] (Temperatur des "Dreibeiners")|
|9| Encoder Detents|
|10| Movement Sensor [0 or 1]|
|11| Distance Sensor [millimeters]|
|12| Analog Voltage on Pin 35 //CHANNEL_MOVEMENT_OR_FAN1SENSE I35|
|13| placeholder|
|14| placeholder|
|15| Relative Humidity AHT21 [%]|
|16| Temperature AHT21 [°C * 100]|
|17| Relative Humidity BME280 [%]|
|18| Temperature BME280 [°C * 100]|
|100| Firmware Version|
