# Registers
## Read

|Register  |Width [byte]  |Description |
|---|---|---|
|| 1  | Status?!?  |
|| 1  | Bit0=BTN_RED, Bit1=BTN_YEL, Bit2=Movement Sensor, Bit3=Brushless_FAULT|
||2|Rotary Encoder (absolute value as uint_16 with overflow)|
||2|Brightness Sensor|
||2|USB PD Voltage as is|
||2|ADC_IN 1|
||2|ADC_IN 2|

## Write
|Register  |Width [byte]  |Description |
|---|---|---|
|| 1  | Address Pointer (default=0)  |
|| 1  | Bit0=Relay, Bit1=BrushlessReset|
||1|Servo 1 0-255|
||1|Servo 2 0-255|
||1|Servo 3 0-255|
||1|Fan 0-255|
||2|USB PD Voltage as should|
||2|DAC_OUT 1|
||2|DAC_OUT 2|
||1|Heater 0-255|
||1|Brightness of Power LED 0-255|
||1|3Phase-Driver-Mode (Brushless vs three separate half bridges etc.)|
||1|3Phase-Driver-Speed 1|
||1|3Phase-Driver-Speed 2|
||1|3Phase-Driver-Speed 3|