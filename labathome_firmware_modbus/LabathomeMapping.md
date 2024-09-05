# General Modbus Information

## Register

| REGISTERNUMBER | REGISTERADRESS HEX | TYP | NAME | TYP |
|:---:|:---:|:---:|:---:|:---:|
|1-9999| 0000 to 270E| Read / Write | Discrete Output Coils| DO|
|10001-19999| 0000 to 270E| Read | Discrete Input Contacts| DI|
|30001-39999| 0000 to 270E| Write | Analog Input Registers| AI|
|40001-49999| 0000 to 270E| Read / Write | Analog Output / Holding Registers| AO|

## Function Codes

| FUNCTIONCODES | FUNCTION | TYPE | ACCESTYPE |
|:---:|:---:|:---:|:---:|
| 01 (0x01) | Read DO, Read Discrete Output Coil | Bool | Read |
| 02 (0x02) | Read DI, Read Discrete Input Contact | Bool | Read |
| 03 (0x03) | Read AO, Read Analog Output Holding Register | 16 Bit | Read |
| 04 (0x04) | Read AI, Read Analog Input Register | 16 Bit | Read |
| 05 (0x05) | Write Discrete Output Coil | Bool | Write |
| 06 (0x06) | Write Single Analog Output / Holding Registers | 16 Bit | Write |
| 15 (0x0F) | Write Multiple Discrete Output Coils | Bool | Write Multiple |
| 16 (0x10) | Write Multiple Analog Outputs / Holding Registers | 16 Bit | Write Multiple |

## Modbus Mapping

These are the addresses and the formats at Lab@Home by Klaus Liebler.  
Adresses are **0-Based**.

### Discrete Output Coils (FC5 / FC15)

|Register (0-Based) [Offset]| Function|
|:---:|:---:|
|0|Relay K3|

### Discrete Input Contacts (FC2)

|Register (0-Based) [Offset]| Function|
|:---:|:---:|
|0|Green Button|
|1|Red Button|
|2|Yellow / Encoder Button |
|3|Movement Sensor|

### Holding Registers (FC3 / FC6)

|Register (0-Based) [Offset]| Function|
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

### Input Registers (FC 4)

|Register (0-Based) [Offset]| Function|
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
