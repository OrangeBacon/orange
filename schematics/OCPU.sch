EESchema Schematic File Version 4
EELAYER 29 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 8
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Wire Bus Line
	1850 1600 2500 1600
Text Label 1850 1600 2    50   ~ 0
ALULeft
Wire Bus Line
	2650 2200 2500 2200
Wire Bus Line
	2500 2200 2500 1600
Connection ~ 2500 1600
Wire Bus Line
	1850 1750 2400 1750
Text Label 1850 1750 2    50   ~ 0
ALURight
Wire Bus Line
	2650 2350 2400 2350
Wire Bus Line
	2400 2350 2400 1750
Connection ~ 2400 1750
Wire Bus Line
	1900 3500 2500 3500
Text Label 1900 3500 2    50   ~ 0
Data
Text Label 1900 3600 2    50   ~ 0
Address
Wire Bus Line
	3800 2200 3900 2200
Wire Bus Line
	3900 2200 3900 3500
Connection ~ 3900 3500
$Sheet
S 2650 2050 1100 1000
U 5D0F9E5E
F0 "ALU" 50
F1 "ALU.sch" 50
F2 "InputA" I L 2650 2200 50 
F3 "InputB" I L 2650 2350 50 
F4 "FunctionSelect" I L 2650 2500 50 
F5 "Out" O R 3750 2200 50 
F6 "8BitMode" I L 2650 2650 50 
F7 "Zero" O R 3750 2350 50 
F8 "Overflow" O R 3750 2500 50 
F9 "Carry" O R 3750 2650 50 
F10 "Sign" O R 3750 2800 50 
F11 "Compare" O R 3750 2950 50 
F12 "OutputEnable" I L 2650 2800 50 
$EndSheet
$Sheet
S 2650 4500 1050 600 
U 5D0FE34E
F0 "InstructionRegister" 50
F1 "InstructionRegister.sch" 50
F2 "Instruction" I L 2650 4650 50 
F3 "Opcode" O L 2650 4900 50 
$EndSheet
Wire Bus Line
	2650 4650 2500 4650
Wire Bus Line
	2500 4650 2500 3500
Connection ~ 2500 3500
Wire Bus Line
	2500 3500 3900 3500
$Sheet
S 2750 5900 1050 800 
U 5D0FE6BF
F0 "InstructionDecoder" 50
F1 "InstructionDecoder.sch" 50
F2 "Phase" I R 3800 6350 50 
F3 "OpCode" I L 2750 6200 50 
F4 "Flags" I L 2750 6350 50 
F5 "Ctrl1" O R 3800 6050 50 
F6 "Ctrl2" O R 3800 6200 50 
$EndSheet
$Sheet
S 5100 4550 1000 500 
U 5D0FE705
F0 "FlagsRegister" 50
F1 "FlagsRegister.sch" 50
$EndSheet
$Sheet
S 4900 2200 1000 700 
U 5D0FE731
F0 "Registers" 50
F1 "Registers.sch" 50
F2 "Left" O R 5900 2300 50 
F3 "Right" O R 5900 2450 50 
F4 "Data" B L 4900 2300 50 
F5 "Address" O R 5900 2600 50 
F6 "Register1" I L 4900 2450 50 
F7 "Register2" I L 4900 2600 50 
F8 "Register3" I L 4900 2750 50 
$EndSheet
$Sheet
S 7150 2200 1550 950 
U 5D0FE757
F0 "MMU" 50
F1 "MMU.sch" 50
$EndSheet
Wire Bus Line
	2400 1750 6150 1750
Wire Bus Line
	3900 3500 4750 3500
Wire Bus Line
	1900 3600 6050 3600
Wire Bus Line
	2500 1600 6050 1600
Wire Bus Line
	4750 2300 4750 3500
Wire Bus Line
	4750 2300 4900 2300
Connection ~ 4750 3500
Wire Bus Line
	4750 3500 10350 3500
Wire Bus Line
	5900 2300 6050 2300
Wire Bus Line
	6050 2300 6050 1600
Connection ~ 6050 1600
Wire Bus Line
	6050 1600 10400 1600
Wire Bus Line
	5900 2450 6150 2450
Wire Bus Line
	6150 2450 6150 1750
Connection ~ 6150 1750
Wire Bus Line
	6150 1750 10350 1750
Wire Bus Line
	5900 2600 6050 2600
Wire Bus Line
	6050 2600 6050 3600
Connection ~ 6050 3600
Wire Bus Line
	6050 3600 10350 3600
$Sheet
S 5200 6050 1000 650 
U 5D0FE655
F0 "PhaseCounter" 50
F1 "PhaseCounter.sch" 50
F2 "Phase" O L 5200 6350 50 
$EndSheet
Wire Wire Line
	3800 6350 5200 6350
Wire Bus Line
	2650 4900 2500 4900
Wire Bus Line
	2500 4900 2500 6200
Wire Bus Line
	2500 6200 2750 6200
$EndSCHEMATC
