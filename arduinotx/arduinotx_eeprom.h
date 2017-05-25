/* arduinotx_eeprom.h - Persist user data in EEProm
** 27-07-2012
** 08-08-2012 global var TSC
** 15-09-2012 calibration
** 15-10-2012 ICT_OFF, ICT_SLAVE
** 29-10-2012 del ICT_SLAVE
** 11-01-2013 replaced NCHANNELS by CHANNELS
** 12-01-2013 del SERIALIZEDSIZE, changed serialize_variable(), Serialize(), NDATASETS=9
** 24-02-2013 VERLIB 9, int_to_short() constraint
** 08-03-2013 VERLIB 11 removed GLOBAL_THR, added GLOBAL_TH1
** 03-05-2013 Mixers
** 09-06-2013 moved GlobalVarNames_str[] to PROGMEM
** 11-06-2013 moved GlobalVar*[] to PROGMEM
** 13-06-2013 moved MixerVar*[] ChanVar*[] to PROGMEM
** 14-06-2013 CHAN_EXP, NDATASETS depends on CHANNELS 
** 15-06-2013 ModelVar*, MAXSTRLEN
** 18-08-2013 GLOBAL_BAT
*/

/* Copyright (C) 2014 Richard Goutorbe.  All right reserved.
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
Contact information: http://www.reseau.org/arduinorc/index.php?n=Main.Contact
*/

#ifndef arduinotx_eeprom_h
#define arduinotx_eeprom_h
#include <Arduino.h>
#include <EEPROM.h>

// number of data sets (models) stored in EEProm
#if CHANNELS == 9
	#define NDATASETS 6
#elif CHANNELS == 8
	#define NDATASETS 7
#else
	#define NDATASETS 8
#endif

// maximum length of var name
#define MAXVARNAME 3

// maximum length of a string stored in EEPROM (terminating \0 excluded)
// at least 6 to fit string representation of int's in serialize_variable()
#define MAXSTRLEN 8

// number of global variables (number of items in GlobalVarNames_str[])
#define GLOBAL_VARS 22

// number of variables of each model (number of items in ModelVarNames_str[])
#define VARS_PER_MODEL 3

// number of mixers defined in each dataset
// if you need more mixers you can simply change this value
#define NMIXERS 2

// number of variables of each mixer (number of items in MixerVarNames_str[])
#define VARS_PER_MIXER 4

// number of variables of each channel (number of items in ChanVarNames_str[])
#define VARS_PER_CHANNEL 10

// comments start by '#'
#define COMMENT_TOKEN '#'

// symbolic names defined for the global variables and their index in array GlobalVarNames_str[]
#define GLOBAL_LIB 0
#define GLOBAL_VER 1
#define GLOBAL_CDS 2
#define GLOBAL_ADS 3
#define GLOBAL_TSC 4
#define GLOBAL_BAT 5
#define GLOBAL_KL1 6
#define GLOBAL_KH1 14

// symbolic names defined for the model variables and their index in array ModelVarNames_str[]
#define MOD_NAM 0
#define MOD_THC 1
#define MOD_PRT 2

// symbolic names defined for the mixer variables and their index in array MixerVarNames_str[]
#define MIX_N1M 0
#define MIX_P1M 1
#define MIX_N2M 2
#define MIX_P2M 3

// symbolic names defined for the channel variables and their index in array ChanVarNames_str[]
#define CHAN_ICT 0
#define CHAN_ICN 1
#define CHAN_REV 2
#define CHAN_DUA 3
#define CHAN_EXP 4
#define CHAN_PWL 5
#define CHAN_PWH 6
#define CHAN_EPL 7
#define CHAN_EPH 8
#define CHAN_SUB 9

// symbolic values of channel variable ICT
#define ICT_OFF 0
#define ICT_ANALOG 1
#define ICT_DIGITAL 2
#define ICT_MIXER 3

// symbolic names defined for the calibration variables
#define CAL_LOW 0
#define CAL_HIGH 1

// Make Global variables names visible to other modules
extern const char Gvn_LIB[] PROGMEM, Gvn_VER[] PROGMEM, Gvn_CDS[] PROGMEM, Gvn_ADS[] PROGMEM, Gvn_TSC[] PROGMEM, Gvn_BAT[] PROGMEM, 
	Gvn_KL1[] PROGMEM, Gvn_KL2[] PROGMEM, Gvn_KL3[] PROGMEM, Gvn_KL4[] PROGMEM, Gvn_KL5[] PROGMEM, Gvn_KL6[] PROGMEM, Gvn_KL7[] PROGMEM, Gvn_KL8[] PROGMEM, 
	Gvn_KH1[] PROGMEM, Gvn_KH2[] PROGMEM, Gvn_KH3[] PROGMEM, Gvn_KH4[] PROGMEM, Gvn_KH5[] PROGMEM, Gvn_KH6[] PROGMEM, Gvn_KH7[] PROGMEM, Gvn_KH8[] PROGMEM;

// Make Model variables names visible to other modules
extern const char 	Gvn_NAM[] PROGMEM, Gvn_THC[] PROGMEM, Gvn_PRT[] PROGMEM;

// Make Mixer variables names visible to other modules
extern const char 	Gvn_N1M[] PROGMEM, Gvn_P1M[] PROGMEM, Gvn_N2M[] PROGMEM, Gvn_P2M[] PROGMEM;

// Make Channel variables names visible to other modules
extern const char Gvn_ICT[] PROGMEM,	Gvn_ICN[] PROGMEM, Gvn_REV[] PROGMEM, Gvn_DUA[] PROGMEM, Gvn_EXP[] PROGMEM,
	Gvn_PWL[] PROGMEM, Gvn_PWH[] PROGMEM, Gvn_EPL[] PROGMEM, Gvn_EPH[] PROGMEM, Gvn_SUB[] PROGMEM,
	Gvn_N1M[] PROGMEM, Gvn_P1M[] PROGMEM, Gvn_N2M[] PROGMEM, Gvn_P2M[] PROGMEM;
	

class ArduinotxEeprom {
	private:
		//PGM_P GlobalVarNames_str[] PROGMEM;
		static const byte GlobalVarSize_byt[] PROGMEM;
		static const byte GlobalVarType_byt[] PROGMEM;
		static const int GlobalVarDefault_int[] PROGMEM;
		
		//PGM_P ModelVarNames_str[] PROGMEM;
		static const byte ModelVarSize_byt[] PROGMEM;
		static const byte ModelVarType_byt[] PROGMEM;
		static const int ModelVarDefault_int[] PROGMEM;
	
		//PGM_P MixerVarNames_str[] PROGMEM;
		static const byte MixerVarSize_byt[] PROGMEM;
		static const byte MixerVarType_byt[] PROGMEM;
		static const int MixerVarDefault_int[] PROGMEM;
		
		//const char *ChanVarNames_str[] PROGMEM;
		static const byte ChanVarSize_byt[] PROGMEM;
		static const byte ChanVarType_byt[] PROGMEM;
		static const int ChanVarDefault_int[] PROGMEM;
	
		int get_var_offset(byte dataset_byt, const char *var_str, byte *out_size_int, char *out_type_chr);
		void format_variable_name(const char *var_str, byte channel_byt, char *out_name_str);
		void serialize_variable(byte dataset_byt, const char *var_str, byte channel_byt);
		int short_to_int(int value_byt);
		byte int_to_short(int value_int);
	
	public:
		void InitEEProm();
		int CheckEEProm();
		byte IsVar(byte dataset_int, const char *var_str);
		char GetType(byte dataset_byt, const char *var_str);
		int GetVar(byte dataset_byt, const char *var_str);
		int GetVar(byte dataset_byt, const char *var_str, char *out_value_str);
		byte SetVar(byte dataset_byt,char const *var_str, int value_int, const char *value_str = NULL);
		void GetGlobal(int out_global_int[]);
		byte GetDataset(byte dataset_byt, int out_model_int[], int out_mixers_byt[][VARS_PER_MIXER], int out_channels_byt[][VARS_PER_CHANNEL]);
		byte Serialize(byte dataset_byt, byte channel_byt);
};
#endif
