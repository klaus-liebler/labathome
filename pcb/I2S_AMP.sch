EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 2 2
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L MAX98357AETE:MAX98357AETE+T U?
U 1 1 5FDDD145
P 4200 3100
F 0 "U?" H 4850 3365 50  0000 C CNN
F 1 "MAX98357AETE+T" H 4850 3274 50  0000 C CNN
F 2 "smopla:MAX98357AETE" H 4200 3500 50  0001 L CNN
F 3 "http://datasheets.maximintegrated.com/en/ds/MAX98357A-MAX98357B.pdf" H 4200 3600 50  0001 L CNN
F 4 "IC" H 4200 3700 50  0001 L CNN "category"
F 5 "Amplifier IC 2-Channel (Stereo) Class D 16-TQFN (3x3)" H 4200 3800 50  0001 L CNN "digikey description"
F 6 "MAX98357AETE+T-ND" H 4200 3900 50  0001 L CNN "digikey part number"
F 7 "yes" H 4200 4000 50  0001 L CNN "lead free"
F 8 "0f54a4b5bdb80fd2" H 4200 4100 50  0001 L CNN "library id"
F 9 "Maxim Electronics" H 4200 4200 50  0001 L CNN "manufacturer"
F 10 "700-MAX98357AETE+T" H 4200 4300 50  0001 L CNN "mouser part number"
F 11 "TQFN16" H 4200 4400 50  0001 L CNN "package"
F 12 "yes" H 4200 4500 50  0001 L CNN "rohs"
F 13 "+85°C" H 4200 4600 50  0001 L CNN "temperature range high"
F 14 "+40°C" H 4200 4700 50  0001 L CNN "temperature range low"
F 15 "" H 4200 4800 50  0001 L CNN "voltage"
	1    4200 3100
	1    0    0    -1  
$EndComp
$Comp
L Device:C C?
U 1 1 5FDEB3BC
P 3500 3250
F 0 "C?" H 3615 3296 50  0000 L CNN
F 1 "u1" H 3615 3205 50  0000 L CNN
F 2 "Capacitor_SMD:C_1206_3216Metric_Pad1.33x1.80mm_HandSolder" H 3538 3100 50  0001 C CNN
F 3 "~" H 3500 3250 50  0001 C CNN
	1    3500 3250
	1    0    0    -1  
$EndComp
$Comp
L Device:C C?
U 1 1 5FDEB954
P 3200 3250
F 0 "C?" H 3315 3296 50  0000 L CNN
F 1 "10u" H 3315 3205 50  0000 L CNN
F 2 "Capacitor_SMD:C_1206_3216Metric_Pad1.33x1.80mm_HandSolder" H 3238 3100 50  0001 C CNN
F 3 "~" H 3200 3250 50  0001 C CNN
	1    3200 3250
	1    0    0    -1  
$EndComp
Wire Wire Line
	4200 3100 4200 3200
Wire Wire Line
	4200 3100 3500 3100
Connection ~ 4200 3100
Wire Wire Line
	3500 3100 3200 3100
Connection ~ 3500 3100
Wire Wire Line
	5500 3900 5500 4000
Connection ~ 5500 4000
Wire Wire Line
	5500 4000 5500 4100
Connection ~ 5500 4100
Wire Wire Line
	5500 4100 5500 4200
$Comp
L power:GND #PWR?
U 1 1 5FDEC024
P 5500 4200
F 0 "#PWR?" H 5500 3950 50  0001 C CNN
F 1 "GND" H 5505 4027 50  0000 C CNN
F 2 "" H 5500 4200 50  0001 C CNN
F 3 "" H 5500 4200 50  0001 C CNN
	1    5500 4200
	1    0    0    -1  
$EndComp
Connection ~ 5500 4200
$Comp
L Device:R R?
U 1 1 5FDEC758
P 4050 4000
F 0 "R?" V 3950 4000 50  0000 C CNN
F 1 "1M" V 4050 4000 50  0000 C CNN
F 2 "Resistor_SMD:R_1206_3216Metric_Pad1.30x1.75mm_HandSolder" V 3980 4000 50  0001 C CNN
F 3 "~" H 4050 4000 50  0001 C CNN
	1    4050 4000
	0    1    1    0   
$EndComp
$Comp
L power:+3V3 #PWR?
U 1 1 5FDED190
P 3200 3100
F 0 "#PWR?" H 3200 2950 50  0001 C CNN
F 1 "+3V3" H 3215 3273 50  0000 C CNN
F 2 "" H 3200 3100 50  0001 C CNN
F 3 "" H 3200 3100 50  0001 C CNN
	1    3200 3100
	1    0    0    -1  
$EndComp
Connection ~ 3200 3100
$Comp
L power:+3V3 #PWR?
U 1 1 5FDED6DC
P 3900 4000
F 0 "#PWR?" H 3900 3850 50  0001 C CNN
F 1 "+3V3" V 3915 4128 50  0000 L CNN
F 2 "" H 3900 4000 50  0001 C CNN
F 3 "" H 3900 4000 50  0001 C CNN
	1    3900 4000
	0    -1   -1   0   
$EndComp
Text GLabel 4200 3400 0    50   Input ~ 0
I2S_DATA
Text GLabel 4200 3800 0    50   Input ~ 0
I2S_LRCLK
Text GLabel 4200 4200 0    50   Input ~ 0
I2S_BCLK
$Comp
L Connector_Generic:Conn_01x02 J?
U 1 1 5FDEE67D
P 5700 3100
F 0 "J?" H 5780 3092 50  0000 L CNN
F 1 "SPK" H 5780 3001 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical" H 5700 3100 50  0001 C CNN
F 3 "~" H 5700 3100 50  0001 C CNN
	1    5700 3100
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5FDEF180
P 3350 3400
F 0 "#PWR?" H 3350 3150 50  0001 C CNN
F 1 "GND" H 3355 3227 50  0000 C CNN
F 2 "" H 3350 3400 50  0001 C CNN
F 3 "" H 3350 3400 50  0001 C CNN
	1    3350 3400
	1    0    0    -1  
$EndComp
Wire Wire Line
	3200 3400 3350 3400
Connection ~ 3350 3400
Wire Wire Line
	3350 3400 3500 3400
NoConn ~ 5500 3700
NoConn ~ 5500 3600
NoConn ~ 5500 3500
NoConn ~ 5500 3400
$Comp
L Jumper:SolderJumper_3_Open JP?
U 1 1 5FDEFD06
P 2850 3300
F 0 "JP?" V 2650 3050 50  0000 L CNN
F 1 "+12/9/6dB" V 2750 2750 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical" H 2850 3300 50  0001 C CNN
F 3 "~" H 2850 3300 50  0001 C CNN
	1    2850 3300
	0    1    1    0   
$EndComp
Wire Wire Line
	3200 3100 2850 3100
Wire Wire Line
	3200 3400 3200 3500
Wire Wire Line
	3200 3500 2850 3500
Connection ~ 3200 3400
Wire Wire Line
	4200 3600 4200 3650
Wire Wire Line
	4200 3650 2700 3650
Wire Wire Line
	2700 3650 2700 3300
$EndSCHEMATC
