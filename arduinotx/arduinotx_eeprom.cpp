/* arduinotx_eeprom.ino - Persist user data in EEPROM
** 25-07-2012
** 08-08-2012 VERLIB 6, added global var TSC
** 15-09-2012 calibration
** 29-10-2012 VERLIB 8
** 12-01-2013 replaced NCHANNELS by CHANNELS, serialize_variable(), Serialize()
** 24-02-2013 VERLIB 9, int_to_short() constraint
** 02-03-2013 VERLIB 10, CheckEEProm()
** 08-03-2013 VERLIB 11 Throttle channel defined for each model, changed many bytes to ints
** 01-05-2013 changed default values of PWL, PWH in ChanVarDefault_int[]
** 03-05-2013 Mixers
** 09-06-2013 moved GlobalVarNames_str[] to PROGMEM
** 11-06-2013 moved GlobalVar*[] to PROGMEM
** 13-06-2013 moved MixerVar*[] ChanVar*[] to PROGMEM
** 14-06-2013 CHAN_EXP
** 15-06-2013 ModelVar*, SetVar()
** 16-06-2013 InitEEProm(), Serialize()
** 04-07-2013 comments start by '#' instead of ';'
*/

/* Copyright (C) 2014 Richard Goutorbe.  All right reserved.
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
Contact information: http://www.reseau.org/arduinorc/index.php?n=Main.Contact
*/

#include "arduinotx_transmitter.h"
#include "arduinotx_eeprom.h"
#include "arduinotx_lib.h"

// magic number of this library, tells if the EEProm has been initialized by ArduinotxEeprom::InitEEProm()
#define IDLIB 55
// version of this library, used to test if the EEProm contains data from an older version
#define VERLIB 15

/* 
EEPROM layout for 6 channels

------------------------ Dataset 0 -----------------------
0000 - 0039	Global Variables, "LIB" must be at offset 0, "VER" at offset 1

------------------------ Dataset 1 -----------------------
0040 - 0048	Model Variables (9 bytes)
0049 - 0056	Mixers Variables ( 2 x 4 bytes)
0057 - 0128	Channels Variables (6 x 12 bytes)
	
------------------------ Dataset 2 -----------------------
0129 - 0137	Model Variables (9 bytes)
0138 - 0145	Mixers Variables( 2 x 4 bytes)
0146 - 0217	Channels Variables (6 x 12 bytes)
...

EEPROM usage = GLOBAL_BYTES + ( NDATASETS * (BYTES_PER_MODEL + (NMIXERS * BYTES_PER_MIXER) + (CHANNELS * BYTES_PER_CHANNEL)) )
	6 channels: 9 datasets: 841 bytes	40+ 9 * (9 + (2*4) + (6*12))
	7 channels: 9 datasets: 949 bytes	40 + 9 * (9 + (2*4) + (7*12))
	8 channels: 8 datasets: 944 bytes	40 + 8 * (9 + (2*4) + (8*12))
	9 channels: 7 datasets: 915 bytes	40 + 7 * (9 + (2*4) + (9*12))
*/

/*
** Global variables -----------------------------------------------------------
*/

// These variables contain general settings for the whole application, they are not related to any channel
// Storage: they are stored at the beginning of the EEProm

// The API considers that global vars belong to dataset #0

// names of global variables (vars that are not related to a channel)
// LIB	magic number of this library, tells if the EEProm has been initialized by ArduinotxEeprom::InitEEProm()
// VER	version of the library which has initialized the EEProm
// CDS	current dataset [1,NDATASETS] ; this is the active, user-selected dataset in the transmitter when the MODEL_SWITCH is opened
// ADS	alternate dataset [1,NDATASETS], used when the MODEL_SWITCH is closed
// TSC	throttle cutoff value used by the throttle security check function, [0, 511], default = 50 (1024 * 5% ~ 50)
// BAT	minimum battery voltage required for Tx operation, ALARM_BATTERY is triggered when voltage gets lower. 
//		The voltage is measured at the center of a resistor bridge, so the value  [0, 1023] is an index relative to 50% of the actual battery voltage on a 5 volts scale
//		The precision of the measurement is affected by the tolerance of the resistors and you may have to calibrate it with a multimeter.
//		Since a 7805 powers the Tx, the minimal voltage required is 7 volts, leading to the minimum value for BAT = 1023 * (7 / 2) / 5 = 716
//		If the transmitter is powered by a 2S LiPo then 7 volts corresponds also to a 80% discharge, which is the maximum recommended discharge
//		You may choose to increase the value of  BAT to be warned before the minimal voltage is reached: add 21 per 100mV, eg: BAT = 716+21 = 737 for 7.1 volts
//		This is why the default value for BAT is 740.
// KL1...KL8	potentiometer #1 to #8 calibration: the lowest value returned by the analog input connected to this pot [0, 1023]
// KH1...KH8	potentiometer #1 to #8 calibration: the highest value returned by the analog input connected to this pot [0, 1023]
//
// Allocate Global variables names in PROGMEM
const char Gvn_LIB[] PROGMEM = "LIB"; const char Gvn_VER[] PROGMEM = "VER"; const char Gvn_CDS[] PROGMEM = "CDS";
const char Gvn_ADS[] PROGMEM = "ADS"; const char Gvn_TSC[] PROGMEM = "TSC"; const char Gvn_BAT[] PROGMEM = "BAT"; 
const char Gvn_KL1[] PROGMEM = "KL1"; const char Gvn_KL2[] PROGMEM = "KL2"; const char Gvn_KL3[] PROGMEM = "KL3"; const char Gvn_KL4[] PROGMEM = "KL4"; 
const char Gvn_KL5[] PROGMEM = "KL5"; const char Gvn_KL6[] PROGMEM = "KL6"; const char Gvn_KL7[] PROGMEM = "KL7"; const char Gvn_KL8[] PROGMEM = "KL8"; 
const char Gvn_KH1[] PROGMEM = "KH1"; const char Gvn_KH2[] PROGMEM = "KH2"; const char Gvn_KH3[] PROGMEM = "KH3"; const char Gvn_KH4[] PROGMEM = "KH4"; 
const char Gvn_KH5[] PROGMEM = "KH5"; const char Gvn_KH6[] PROGMEM = "KH6"; const char Gvn_KH7[] PROGMEM = "KH7"; const char Gvn_KH8[] PROGMEM = "KH8"; 
//
PGM_P ArduinotxEeprom::GlobalVarNames_str[] PROGMEM = {
	Gvn_LIB, Gvn_VER, Gvn_CDS, Gvn_ADS, Gvn_TSC, Gvn_BAT,
	Gvn_KL1, Gvn_KL2, Gvn_KL3, Gvn_KL4, Gvn_KL5, Gvn_KL6, Gvn_KL7, Gvn_KL8, 
	Gvn_KH1, Gvn_KH2, Gvn_KH3, Gvn_KH4, Gvn_KH5, Gvn_KH6, Gvn_KH7, Gvn_KH8, 
	NULL
};

// type of values of the global variables:
// a)rray of chars, b)yte, i)nt, s)hort : a short is a signed byte
const byte ArduinotxEeprom::GlobalVarType_byt[] PROGMEM = {'b','b','b','b','i','i',
	'i','i','i','i','i','i','i','i','i','i','i','i','i','i','i','i'
};

// size of values of the global variables
const byte ArduinotxEeprom::GlobalVarSize_byt[] PROGMEM = {1,1,1,1,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
};

// default values of the global variables used by InitEEProm()
const int ArduinotxEeprom::GlobalVarDefault_int[] PROGMEM = {IDLIB, VERLIB, 1, 1, 50, 740,
	0, 0, 0, 0, 0, 0, 0, 0,
	1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023
};

// total size of the values stored in the global variables (sum of GlobalVarSize_byt[])
#define GLOBAL_BYTES 40

// see also GLOBAL_VARS in arduinotx_eeprom.h

/*
** Model variables -----------------------------------------------------------
*/
// These variables contain per-model settings
// NAM	model name, 8 chars, no spaces
// THC	channel number used for throttle control, 0=no throttle channel
//
// Allocate Mixer variables base names in PROGMEM
const char Gvn_NAM[] PROGMEM = "NAM"; const char Gvn_THC[] PROGMEM = "THC"; 
//
PGM_P ArduinotxEeprom::ModelVarNames_str[] PROGMEM = {
	Gvn_NAM, Gvn_THC,
	NULL
};

// type of values of the model variables:
// a)rray of chars, b)yte, i)nt, s)hort : a short is a signed byte
const byte ArduinotxEeprom::ModelVarType_byt[] PROGMEM = {'a','b'};

// size of values of the model variables
const byte ArduinotxEeprom::ModelVarSize_byt[] PROGMEM = {8,1};

// default values of the model variables used by InitEEProm()
// NAM	a question mark
// THC	chan 3
const int ArduinotxEeprom::ModelVarDefault_int[] PROGMEM = {'?', 3};

// total size of the values stored in each model  (sum of ModelVarSize_byt[])
#define BYTES_PER_MODEL 9

// see also VARS_PER_MODEL and symbolic names defined for the variables indexes in arduinotx_eeprom.h


/*
** Mixer variables -----------------------------------------------------------
*/
// These variables contain per-model settings, they define 2 programmable mixers: Mixer1 and Mixer2
// Mixer variables, where 'x' is a mixer number [1, 2]
// N1Mx	input1 potentiometer number [1,8] or 0=none
// P1Mx 	Percent mix applied to N1Mx [-100, +100]
// N2Mx	input2 potentiometer number [1,8] or 0=none
// P2Mx 	Percent mix applied to N2Mx [-100, +100]
//
// Allocate Mixer variables base names in PROGMEM
const char Gvn_N1M[] PROGMEM = "N1M"; const char Gvn_P1M[] PROGMEM = "P1M"; const char Gvn_N2M[] PROGMEM = "N2M"; const char Gvn_P2M[] PROGMEM = "P2M"; 
//
PGM_P ArduinotxEeprom::MixerVarNames_str[] PROGMEM = {
	Gvn_N1M, Gvn_P1M, Gvn_N2M, Gvn_P2M,
	NULL
};
	
// type of values of the mixer variables:
// a)rray of chars, b)yte, i)nt, s)hort : a short is a signed byte
const byte ArduinotxEeprom::MixerVarType_byt[] PROGMEM = {'b','s','b','s'};

// size of values of the variables in each channel
const byte ArduinotxEeprom::MixerVarSize_byt[] PROGMEM = {1,1,1,1};

// default values of the variables of each mixer, used by InitEEProm()
const int ArduinotxEeprom::MixerVarDefault_int[] PROGMEM = {0, 100, 0, 100};

// total size of the values stored in each mixer  (sum of MixerVarSize_byt[])
#define BYTES_PER_MIXER 4

// see also VARS_PER_MIXER and symbolic names defined for the variables indexes in arduinotx_eeprom.h


/*
** Channel variables -----------------------------------------------------------
*/

// These variables contain per-channel settings
// Storage: they are stored after the Model variables

// Names of variables in each channel, where x is a channel number [1,9]
// ICTx	input control type: 0=none (slave channel), 1=potentiometer, 2=switch, 3=mixer; see symbolic values of channel variable ICT in arduinotx_eeprom.h
// ICNx	input control number 0=none, potentiometer number, switch number, mixer number ; potentiometers:[1,8], switch:[1,5], mixer:[1,2]
// REVx	1=reversed, 0=normal
// DUAx	dual rate reduction percentage applied to the end point values, [0, 100] 
// EXPx	exponential percentage applied to the input value, [0, 100] 
// PWLx	minimum pulse width acceptable by the servo, microsec
// PWHx	maximum pulse width acceptable by the servo, microsec
// EPLx	end point low value, >=MINPW, constrains the minimum pulse width sent to the servo, [0, 100] 
// EPHx	end point high value, <=MAXPW, constrains the maximum pulse width sent to the servo, [0, 100] 
// SUBx	subtrim centering offset, [-100, 100]
//
// Allocate Channel variables base names in PROGMEM
const char Gvn_ICT[] PROGMEM = "ICT"; const char Gvn_ICN[] PROGMEM = "ICN"; const char Gvn_REV[] PROGMEM = "REV"; 
const char Gvn_DUA[] PROGMEM = "DUA"; const char Gvn_EXP[] PROGMEM = "EXP"; const char Gvn_PWL[] PROGMEM = "PWL"; const char Gvn_PWH[] PROGMEM = "PWH"; 
const char Gvn_EPL[] PROGMEM = "EPL"; const char Gvn_EPH[] PROGMEM = "EPH"; const char Gvn_SUB[] PROGMEM = "SUB";
//
PGM_P ArduinotxEeprom::ChanVarNames_str[] PROGMEM = {
	Gvn_ICT, Gvn_ICN, Gvn_REV, Gvn_DUA, Gvn_EXP, Gvn_PWL, Gvn_PWH, Gvn_EPL, Gvn_EPH, Gvn_SUB,
	NULL
};

// type of values of the variables in each channel:
// a)rray of chars, b)yte, i)nt, s)hort : a short is a signed byte
const byte ArduinotxEeprom::ChanVarType_byt[] PROGMEM = {'b','b','b','b','b','i','i','b','b','s'};

// size of values of the variables in each channel
const byte ArduinotxEeprom::ChanVarSize_byt[] PROGMEM = {1,1,1,1,1,2,2,1,1,1};

// Default values of the variables of each channel, used by InitEEProm()
// Pulse width: these values correspond to the Hextronic HXT500 servo and will accomodate most other servos:
// 	PWL: pulse length for 0 degrees in microseconds: 720uS
// 	PWH: pulse length for 180 degrees in microseconds: 2200uS default for 6 channels, limited to 1700uS for 7-9 channels
#if CHANNELS <= 6
const int ArduinotxEeprom::ChanVarDefault_int[] PROGMEM = {1, 0, 0, 100, 0, 720, 2200, 100, 100, 0};
#else
const int ArduinotxEeprom::ChanVarDefault_int[] PROGMEM = {1, 0, 0, 100, 0, 720, 1700, 100, 100, 0};
#endif

// total size of the values stored in each channel  (sum of ChanVarSize_byt[])
#define BYTES_PER_CHANNEL 12

// see also VARS_PER_CHANNEL and symbolic names defined for the variables indexes in arduinotx_eeprom.h

#define BYTES_PER_DATASET	(BYTES_PER_MODEL + (NMIXERS * BYTES_PER_MIXER) + (CHANNELS * BYTES_PER_CHANNEL))

/*
** Public interface
*/

// Set default values of all variables
void ArduinotxEeprom::InitEEProm() {
	char var_str[MAXVARNAME + 1];
	char name_str[MAXSTRLEN + 1]; // at least MAXVARNAME + 2, but we also use this buffer to format the model name string value
	
	// for each global variable
	for (byte idx_byt = 0; idx_byt < GLOBAL_VARS; idx_byt++) {
		getProgmemStrArrayValue(var_str, GlobalVarNames_str, idx_byt, MAXVARNAME + 1);
		SetVar(0, var_str, getProgmemIntArrayValue(GlobalVarDefault_int, idx_byt));
		//aPrintfln(PSTR("%s default=%d"), var_str, getProgmemIntArrayValue(GlobalVarDefault_int, idx_byt));
	}

	// for each data set
	for (byte ds_byt = 1; ds_byt <= NDATASETS; ds_byt++) {
		
		// model variables
		for (byte idx_byt = 0; idx_byt < VARS_PER_MODEL; idx_byt++) {
			getProgmemStrArrayValue(var_str, ModelVarNames_str, idx_byt, MAXVARNAME + 1);
			SetVar(ds_byt, var_str, getProgmemIntArrayValue(ModelVarDefault_int, idx_byt));
		}
		// set the model name
		sprintf(name_str, "MODEL%d", ds_byt);
		SetVar(ds_byt, "NAM", 0, name_str);
		
		// for each mixer
		for (byte mixer_byt = 1; mixer_byt <= NMIXERS; mixer_byt++) {
			// for each mixer variable
			for (byte idx_byt = 0; idx_byt < VARS_PER_MIXER; idx_byt++) {
				getProgmemStrArrayValue(var_str, MixerVarNames_str, idx_byt, MAXVARNAME + 1);
				format_variable_name(var_str, mixer_byt, name_str);
				SetVar(ds_byt, name_str, getProgmemIntArrayValue(MixerVarDefault_int, idx_byt));
			}
		}
		
		// for each channel
		for (byte chan_byt = 1; chan_byt <= CHANNELS; chan_byt++) {
			// for each channel variable
			for (byte idx_byt = 0; idx_byt < VARS_PER_CHANNEL; idx_byt++) {
				int value_int = getProgmemIntArrayValue(ChanVarDefault_int, idx_byt);
				getProgmemStrArrayValue(var_str, ChanVarNames_str, idx_byt, MAXVARNAME + 1);
				if (strcmp(var_str, "ICN") == 0)
					value_int = chan_byt ; // default: potentiometer having same number as channel
				format_variable_name(var_str, chan_byt, name_str);
				SetVar(ds_byt, name_str, value_int);
			}
		}
	}
}

// check if the EEProm contains valid data
// return value: >0=EEPROM is Ok and returns total size of data stored in EEPROM; -1 EEProm is not initialized
int ArduinotxEeprom::CheckEEProm() {
	int retval_int = -1;
	if (EEPROM.read(0) == IDLIB && EEPROM.read(1) == VERLIB)
		retval_int = GLOBAL_BYTES + (NDATASETS * BYTES_PER_DATASET);
	return retval_int;
}

// test if given variable exists
// return value: 1=ok, 0=var not found
byte ArduinotxEeprom::IsVar(byte dataset_byt, const char *var_str) {
	byte retval_byt = 0;
	byte size_byt = 0;
	char type_chr = ' ';
	int offset_int = get_var_offset(dataset_byt, var_str, &size_byt, &type_chr);
	if (offset_int >= 0)
		retval_byt = 1;
	return retval_byt;
}

// Retrieve the type of given variable:
// Return value: a)rray of chars, b)yte, i)nt, s)hort : a short is a signed byte, '?' = variable not found
char ArduinotxEeprom::GetType(byte dataset_byt, const char *var_str) {
	char retval_chr;
	byte size_byt = 0;
	int offset_int = get_var_offset(dataset_byt, var_str, &size_byt, &retval_chr);
	if (offset_int < 0)
		retval_chr = '?';
	return retval_chr;
}

// Retrieve the numerical value of given 'b', 's', 'i'-type variable
// dataset_int: 0=global variable, or dataset number [1, NDATASETS]
// var_str: name of global or model variable, or VARn where n=channel number or mixer number
// Return value: the numerical value of the variable, or -1 on invalid name or invalid dataset or invalid variable type
int ArduinotxEeprom::GetVar(byte dataset_byt, const char *var_str) {
	int retval_int = -1;
	byte size_byt = 0;
	char type_chr = ' ';
	int offset_int = get_var_offset(dataset_byt, var_str, &size_byt, &type_chr);
	if (offset_int >= 0) {
		switch (type_chr) {
			case 'b':
				retval_int = EEPROM.read(offset_int);
				break;
			
			case 's':
				retval_int = EEPROM.read(offset_int);
				retval_int = short_to_int(retval_int);
				break;
			
			case 'i': {
				union bytes_int {
					byte value_byt[2];
					int value_int;
				} buffer_uni;
				for (byte idx_byt = 0; idx_byt < size_byt; idx_byt++) {
					buffer_uni.value_byt[idx_byt] = EEPROM.read(offset_int + idx_byt);
				}
				retval_int = buffer_uni.value_int;
				break;
			}
		}
	}
	return retval_int;
}

		
// Copy the string value of given 'a'-type variable into given buffer
// dataset_int: 0=global variable, or dataset number [1, NDATASETS]
// var_str: name of global or model variable, or VARn where n=channel number or mixer number
// out_value_str: user-allocated buffer where the string value will be copied; the terminating spaces will be stripped, a '\0' will be appended
// Return value: length of the string value (without the terminating spaces which have been stripped) , or -1 on invalid name or invalid dataset
int ArduinotxEeprom::GetVar(byte dataset_byt, const char *var_str, char *out_value_str) {
	int retval_int = -1;
	byte size_byt = 0;
	char type_chr = ' ';
	int offset_int = get_var_offset(dataset_byt, var_str, &size_byt, &type_chr);
	if (offset_int >= 0) {
		if (type_chr == 'a') {
			for (byte idx_byt = 0; idx_byt < size_byt; idx_byt++)
				*(out_value_str + idx_byt) = EEPROM.read(offset_int + idx_byt);
			*(out_value_str + size_byt) = '\0';
		}
		Trimwhitespace(out_value_str);
		retval_int = strlen(out_value_str);
	}
	return retval_int;
}

// Set the value of given variable, or 0 on invalid name or invalid dataset
// dataset_int: 0=global variable, or dataset number [1, NDATASETS]
// var_str: name of global or model variable, or VARn where n=channel number or mixer number
// value_int : set this value for 'b', 'i', 's' types
// value_str : set this value for 'a' type; if NULL then fill the array with value_int characters
// return value: 0=Ok, 1=error
byte ArduinotxEeprom::SetVar(byte dataset_byt, const char *var_str, int value_int, const char *value_str) {
	byte retval_byt = 0;
	byte size_byt = 0;
	char type_chr = ' ';
	int offset_int = get_var_offset(dataset_byt, var_str, &size_byt, &type_chr);
	if (offset_int >= 0) {
		switch (type_chr) {
			case 'a':
				if (value_str) {
					char next_chr;
					byte end_byt = 0;
					for (byte idx_byt = 0; idx_byt < size_byt; idx_byt++) {
						if (!end_byt) {
							next_chr = *(value_str+idx_byt);
							if (next_chr == '\0')
								end_byt = 1;
						}
						if (end_byt)
							next_chr = ' '; // append spaces if value_str is shorter than size_byt
						EEPROM.write(offset_int + idx_byt, next_chr);
					}
				}
				else {
					// fill the string with value_int characters
					for (byte idx_byt = 0; idx_byt < size_byt; idx_byt++) {
						EEPROM.write(offset_int + idx_byt, (char)value_int);
					}
				}
				break;
				
			case 'b':
				EEPROM.write(offset_int, value_int);
				break;
			
			case 'i': {
				union bytes_int {
					byte value_byt[2];
					int value_int;
				} buffer_uni;
				buffer_uni.value_int = value_int;
				for (byte idx_byt = 0; idx_byt < size_byt; idx_byt++) {
					EEPROM.write(offset_int + idx_byt, buffer_uni.value_byt[idx_byt]);
				}
				break;
			}
			
			case 's':
				EEPROM.write(offset_int,  int_to_short(value_int));
				break;
		}
	}
	else
		retval_byt = 1;
	return retval_byt;
}

// Load global variables into given array
void ArduinotxEeprom::GetGlobal(int out_global_int[]) {
	char name_str[MAXVARNAME + 1];
	// for each variable
	for (byte idx_byt = 0; idx_byt < GLOBAL_VARS; idx_byt++) {
		getProgmemStrArrayValue(name_str, GlobalVarNames_str, idx_byt, MAXVARNAME + 1);
		out_global_int[idx_byt] = GetVar(0, name_str);
	}
}

// Load given dataset data into given arrays
// return value: 0=ok, 1=invalid dataset
byte ArduinotxEeprom::GetDataset(byte dataset_byt, int out_model_int[], int out_mixers_int[][VARS_PER_MIXER], int out_channels_int[][VARS_PER_CHANNEL]) {
	byte retval_byt = 0;
	char var_str[MAXVARNAME + 1];
	char name_str[MAXVARNAME + 2];
	if (dataset_byt > 0 && dataset_byt <= NDATASETS) {
		
		// model variables
		for (byte idx_byt = 0; idx_byt < VARS_PER_MIXER; idx_byt++) {
			getProgmemStrArrayValue(var_str, ModelVarNames_str, idx_byt, MAXVARNAME + 1);
			out_model_int[idx_byt] = GetVar(dataset_byt, var_str); // GetVar() returns -1 for string variables
		}

		// for each mixer
		for (byte mixer_byt = 1; mixer_byt <= NMIXERS; mixer_byt++) {
			// for each variable
			for (byte idx_byt = 0; idx_byt < VARS_PER_MIXER; idx_byt++) {
				getProgmemStrArrayValue(var_str, MixerVarNames_str, idx_byt, MAXVARNAME + 1);
				format_variable_name(var_str, mixer_byt, name_str);
				out_mixers_int[mixer_byt - 1][idx_byt] = GetVar(dataset_byt, name_str);
			}
		}
		// for each channel
		for (byte chan_byt = 1; chan_byt <= CHANNELS; chan_byt++) {
			// for each variable
			for (byte idx_byt = 0; idx_byt < VARS_PER_CHANNEL; idx_byt++) {
				getProgmemStrArrayValue(var_str, ChanVarNames_str, idx_byt, MAXVARNAME + 1);
				format_variable_name(var_str, chan_byt, name_str);
				out_channels_int[chan_byt - 1][idx_byt] = GetVar(dataset_byt, name_str);
			}
		}
	}
	else
		retval_byt = 1; // invalid dataset
	return retval_byt;
}

// Print the serialized data of given dataset on the Serial port
// dataset_int: 0=global variables, or dataset number [1, NDATASETS]
// channel_int: channel_int is ignored if dataset_int==0
//	0 : model vars, all mixer vars and all channel vars in dataset, 
//	[1, CHANNELS] : this channel only
//	CHANNELS+1 : model vars only
//	CHANNELS+2 : mixer vars only
// return value: 0=Ok, 1=error
byte ArduinotxEeprom::Serialize(byte dataset_byt, byte channel_byt) {
	int retval_byt = 0;
	char var_str[MAXVARNAME + 1];
	if (dataset_byt == 0) {
		aPrintfln(PSTR("%c Global"), COMMENT_TOKEN);
		// for each global variable, skipping IDLIB (index 0) and VERLIB (index 1) that are read-only
		for (byte idx_byt = 2; idx_byt < GLOBAL_VARS; idx_byt++) {
			getProgmemStrArrayValue(var_str, GlobalVarNames_str, idx_byt, MAXVARNAME + 1);
			serialize_variable(dataset_byt, var_str, 0);
		}
	}
	else if (dataset_byt > 0 && dataset_byt <= NDATASETS) {
		if (channel_byt == 0 || channel_byt == CHANNELS+1) {
			// for each model variable
			for (byte idx_byt = 0; idx_byt < VARS_PER_MODEL; idx_byt++) {
				getProgmemStrArrayValue(var_str, ModelVarNames_str, idx_byt, MAXVARNAME + 1);
				serialize_variable(dataset_byt, var_str, 0);
			}
		}
		if (channel_byt == 0 || channel_byt == CHANNELS+2) {
			aPrintfln(PSTR("%c Mixers"), COMMENT_TOKEN);
			// for each mixer
			for (byte mixer_byt = 1; mixer_byt <= NMIXERS; mixer_byt++) {
				// for each variable
				for (byte idx_byt = 0; idx_byt < VARS_PER_MIXER; idx_byt++) {
					getProgmemStrArrayValue(var_str, MixerVarNames_str, idx_byt, MAXVARNAME + 1);
					serialize_variable(dataset_byt, var_str, mixer_byt);
				}
			}
		}
		// for each channel
		for (byte chan_byt = 1; chan_byt <= CHANNELS; chan_byt++) {
			if (channel_byt == 0 || chan_byt == channel_byt) {
				aPrintfln(PSTR("%c Channel %d"), COMMENT_TOKEN, chan_byt);
				// for each variable
				for (byte idx_byt = 0; idx_byt < VARS_PER_CHANNEL; idx_byt++) {
					getProgmemStrArrayValue(var_str, ChanVarNames_str, idx_byt, MAXVARNAME + 1);
					serialize_variable(dataset_byt, var_str, chan_byt);
				}
			}
		}
	}
	else
		retval_byt = 1; // invalid dataset
	return retval_byt;
}

/*
** Private implementation
*/

// convert given short (signed) value originally stored as a byte (unsigned), into a signed integer
// Arduino does not implement the "short" type. If we store a short value into a byte, we must use conversion functions to preserve the sign
// negative values[-128,-1] maped to [0,127]; 0 maped to 128; positive values [1,127] maped to [129, 255]
int ArduinotxEeprom::short_to_int(int value_byt) {
	return value_byt - 128;
}
byte ArduinotxEeprom::int_to_short(int value_int) {
	return constrain(value_int, -128, 127) + 128;
}

// Append given number (>= 1) to given variable name (name length must be exactly MAXVARNAME) 
// Return value: the variable name with the digit appended, length = MAXVARNAME + 1
void ArduinotxEeprom::format_variable_name(const char *var_str, byte number_byt, char *out_name_str) {
	strncpy(out_name_str, var_str, MAXVARNAME + 2);
	if (number_byt) {
		out_name_str[MAXVARNAME] = '0' + number_byt;
		out_name_str[MAXVARNAME + 1] = '\0';
	}
}

// Print the serialized data of given variable on the Serial port
void ArduinotxEeprom::serialize_variable(byte dataset_byt, const char *var_str, byte number_byt) {
	char name_str[MAXVARNAME + 2]; // 2 = 1 channel digit + 1 \0
	format_variable_name(var_str, number_byt, name_str);
	char type_chr = GetType(dataset_byt, name_str);
	if (type_chr == 'a') {
		char value_str[MAXSTRLEN + 1]; // 9, at least 7 to fit string representation of int's: 7 = 6 int value + 1 \0
		GetVar(dataset_byt, name_str, value_str);
		aPrintfln(PSTR("%s=%s"), name_str, value_str);
	}
	else
		aPrintfln(PSTR("%s=%d"), name_str, GetVar(dataset_byt, name_str));
}

// Return the offset in the EEProm and the size and type of given variable, or -1 on invalid name or invalid dataset
// dataset_byt : 0=global var, [1, 9]=model var, channel var or a mixer var
// see "EEPROM layout" comments at the top of this file
int ArduinotxEeprom::get_var_offset(byte dataset_byt, const char *var_str, byte *out_size_byt, char *out_type_chr) {
	int retval_int = -1;
	int varlen_int = strlen(var_str);
	int idx_int = 0;
	int ix_int = 0;
	
	if (dataset_byt == 0) {
		// global variable
		if (varlen_int > 0 && varlen_int <= MAXVARNAME) {
			// seek index of var in GlobalVarNames_str[]
			idx_int = findProgmemStrArrayIndex(GlobalVarNames_str, var_str);
			if (idx_int >= 0) {
				// compute the corresponding offset in the EEProm
				retval_int = 0;
				ix_int = 0;
				while (ix_int < idx_int) { // size of previous variables in current channel
					retval_int += getProgmemByteArrayValue(GlobalVarSize_byt, ix_int);
					ix_int++;
				}
				// return the value size
				*out_size_byt = getProgmemByteArrayValue(GlobalVarSize_byt, idx_int);
				// return the type
				*out_type_chr = getProgmemByteArrayValue(GlobalVarType_byt, idx_int);
			}
		}
	}
	else if (dataset_byt > 0 && dataset_byt <= NDATASETS) {
		// model, mixer or channel variable
		if (varlen_int > 0 && varlen_int <= MAXVARNAME + 1) {
			char name_str[MAXVARNAME + 2];
			strcpy(name_str, var_str);

			// if last char is a digit then var_str is a channel var or a mixer var
			// 	and this digit is a channel number or a mixer number
			// else var_str is a model var
			char last_char = var_str[varlen_int - 1];
			if (last_char >= '0' && last_char <= '9') {
				byte number_byt = last_char - '0';
			
				// overwrite the last character of the var name:
				// for a channel variable this will remove the channel number and keep the base name: eg "ICT1" -> "ICT"
				// for a mixer variable this will remove the mixer number and keep the base name: eg "N1M1" -> "N1M"
				name_str[varlen_int - 1] = '\0'; 
							
				// look for mixer variable
				idx_int = -1;
				if (number_byt >0 && number_byt <= NMIXERS) {
					// seek index of var in MixerVarNames_str[]
					idx_int = findProgmemStrArrayIndex(MixerVarNames_str, name_str);
					if (idx_int >= 0) {
						// compute the corresponding offset in the EEProm
						retval_int = GLOBAL_BYTES;
						retval_int += (dataset_byt - 1) * BYTES_PER_DATASET; // size of previous datasets
						retval_int += BYTES_PER_MODEL; // size of model variables in current dataset
						retval_int += (number_byt - 1) * BYTES_PER_MIXER; // size of previous mixers in current dataset
						ix_int = 0;
						while (ix_int < idx_int) {	// size of previous variables in current mixer
							retval_int += getProgmemByteArrayValue(MixerVarSize_byt, ix_int);
							ix_int++;
						}
						// return the value size
						*out_size_byt = getProgmemByteArrayValue(MixerVarSize_byt, idx_int);
						// return the type
						*out_type_chr = getProgmemByteArrayValue(MixerVarType_byt, idx_int);
					}
				}
				
				if (idx_int == -1) {
					// not found in mixers, look for channel variable
					if (number_byt >0 && number_byt <= CHANNELS) {
						// seek index of var in ChanVarNames_str[]
						idx_int = findProgmemStrArrayIndex(ChanVarNames_str, name_str);
						if (idx_int >= 0) {
							// compute the corresponding offset in the EEProm
							retval_int = GLOBAL_BYTES;
							retval_int += (dataset_byt - 1) * BYTES_PER_DATASET; // size of previous datasets
							retval_int += BYTES_PER_MODEL; // size of model variables in current dataset
							retval_int += NMIXERS * BYTES_PER_MIXER; // size of the mixers of current dataset
							retval_int += (number_byt - 1) * BYTES_PER_CHANNEL; // size of previous channels in current dataset
							ix_int = 0;
							while (ix_int < idx_int) {	// size of previous variables in current channel
								retval_int += getProgmemByteArrayValue(ChanVarSize_byt, ix_int);
								ix_int++;
							}
							// return the value size
							*out_size_byt = getProgmemByteArrayValue(ChanVarSize_byt, idx_int);
							// return the type
							*out_type_chr = getProgmemByteArrayValue(ChanVarType_byt, idx_int);
						}
					}
				}
			}
			else {
				// look for model variable
				// seek index of var in ModelVarNames_str[]
				idx_int = findProgmemStrArrayIndex(ModelVarNames_str, name_str);
				if (idx_int >= 0) {
					// compute the corresponding offset in the EEProm
					retval_int = GLOBAL_BYTES;
					retval_int += (dataset_byt - 1) * BYTES_PER_DATASET; // size of previous datasets
					ix_int = 0;
					while (ix_int < idx_int) {	// size of previous variables in current model
						retval_int += getProgmemByteArrayValue(ModelVarSize_byt, ix_int);
						ix_int++;
					}
					// return the value size
					*out_size_byt = getProgmemByteArrayValue(ModelVarSize_byt, idx_int);
					// return the type
					*out_type_chr = getProgmemByteArrayValue(ModelVarType_byt, idx_int);
				}
			}
		}
	}
	return retval_int;
}



