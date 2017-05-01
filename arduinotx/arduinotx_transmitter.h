/* arduinotx_transmitter.h - Tx manager
** 01-05-2013 class ArduinoTx
** 04-05-2013 read_potentiometer()
** 12-05-2013 deleted 2 unused constants
** 15-06-2013 DatasetModel_int[]
** 25-06-2013 CurrentDataset_byt, TxAlarm_int, get_selected_dataset()
** 13-08-2013 LED_PIN
** 14-08-2013 BUZZER_PIN
** 18-08-2013 BATCHECK_ENABLED, BATCHECK_PIN
** 08-09-2013 v1.5.0 branche /arduinorc/branches/moreswitches
** 14-05-2014 MODEL_SWITCH_BEHAVIOUR
** 28-05-2014 ENDPOINTS_ALGORITHM
*/

/* Copyright (C) 2014 Richard Goutorbe.  All right reserved.
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
Contact information: http://www.reseau.org/arduinorc/index.php?n=Main.Contact
*/

#ifndef arduinotx_transmitter_h
#define arduinotx_transmitter_h

#define SOFTWARE_VERSION PSTR("1.5.5")

#include "arduinotx_config.h"

// ----------------------------------------------------------------------------------------
// Misc macros
// ----------------------------------------------------------------------------------------

// Access the value of given global variable
// notice: symbolic names GLOBAL_* are defined in arduinotx_eeprom.h for the variables indexes
#define get_global_var(idxvar) (Global_int[idxvar])

// Access the value of given model variable
// notice: symbolic names MOD_* are defined in arduinotx_eeprom.h for the variables indexes
#define get_model_var(idxvar) (DatasetModel_int[idxvar])

// Access the value of given variable in given mixer
// notice: symbolic names MIX_* are defined in arduinotx_eeprom.h for the variables indexes
// idxmixer : 0-based, mixer number - 1
#define get_mixer_var(idxmixer, idxvar) (DatasetMixers_int[idxmixer][idxvar])

// Access the value of given variable in given channel
// notice: symbolic names CHAN_* are defined in arduinotx_eeprom.h for the variables indexes
// idxchan : 0-based, channel number - 1
#define get_channel_var(idxchan, idxvar) (DatasetChannels_int[idxchan][idxvar])

// Assignement of potentiometers and switches
// icn=control (potentiometer or switch) number defined in channel var ICN
// value=corresponding Arduino's pin number:
// 	Potentiometer 1 is connected to A0, pot 2 to A1, ... pot 8 to A7
// 	Switch 1 is connected to D2, switch 2 to D3, ... switch 6 to D7
// 	You can rewrite these macros if you want different pin assignements
#define get_pot_pin(icn) (icn - 1)
#define get_switch_pin(icn) (icn + 1)

// Access the calibration value for given potentiometer
// icn=control (potentiometer) number defined in channel var ICN, 1-based
// calvar calibration variable (CAL_LOW, CAL_HIGH)
// return value: lowest/highest value returned by the potentiometer corresponding to given channel [0, 1023]
#define get_calibration_var(icn, calvar) (get_global_var(((calvar) == CAL_LOW ? GLOBAL_KL1 : GLOBAL_KH1)+((icn)-1)))

class ArduinoTx {
	private:
		typedef enum RunModes {
			RUNMODE_INIT,
			RUNMODE_TRANSMISSION,	// send PPM to receiver
			RUNMODE_COMMAND		// change configuration settings through Serial link
		} RunMode;
		
		typedef enum Alarms {
			ALARM_NONE,
			ALARM_EEPROM,	// -.-. settings failed to load from EEPROM
			ALARM_THROTTLE,	// -.. throttle security check failed
			ALARM_BATTERY	// -... low battery voltage
		} Alarm;
		
		// Transmitter state ----------------------------------------------------------

		// MODE_SWITCH	RunMode
		//	opened		RUNMODE_TRANSMISSION
		//	closed		RUNMODE_COMMAND
		RunMode RunMode_int;
		
		byte CurrentDataset_byt; // Dataset (model number) currently loaded in RAM
		Alarm TxAlarm_int; // current alarm state
		byte DualRate_bool; // true=dual rate ON
		byte ThrottleCut_bool ; // true=throttle cut off, false=throttle enabled
		byte EngineEnabled_bool; // set by check_throttle() at startup
		byte SettingsLoaded_bool; // set by load_settings() at startup
		byte CommitChanges_bool; // set by CommitChanges(), reset by Refresh()
	
		// Local copy of the values of the global variables
		int Global_int[GLOBAL_VARS];

		// Local copy of the values of the variables of the current dataset
		int DatasetModel_int[VARS_PER_MODEL];
		int DatasetMixers_int[NMIXERS][VARS_PER_MIXER];
		int DatasetChannels_int[CHANNELS][VARS_PER_CHANNEL];

		// Morse codes flashed on the Led ----------------------------------------------------------
		
		static const char LEDCHAR_INIT;
		static const char LEDCHAR_COMMAND;
		static const char LEDCHAR_ALARM_EEPROM;
		static const char LEDCHAR_ALARM_THROTTLE;
		static const char LEDCHAR_ALARM_BATTERY;
		
		byte get_selected_dataset();
		RunMode refresh_runmode();
		void refresh_led_code();
		void send_ppm();
		void load_settings();
		byte check_throttle();
#ifdef BATCHECK_ENABLED
		byte check_battery();
#endif
		unsigned int read_potentiometer(byte pot_number_byt);
#if MODEL_SWITCH_BEHAVIOUR == MODEL_SWITCH_STEPPING		
		void process_model_switch_stepping();
		byte debounce_modelswitch();
#elif MODEL_SWITCH_BEHAVIOUR == MODEL_SWITCH_ROTATING
		void process_model_switch_rotating();
		byte debounce_rotating_switch();
#endif
	public:
		// PPM signal

		static const byte PPM_PIN;
		static const unsigned int PPM_PERIOD;
		static const unsigned int PPM_LOW;

		ArduinoTx();
		void Init();
		void Refresh();
		void CommitChanges();
		unsigned int ReadControl(byte chan_byt);
		unsigned int ComputeChannelPulse(byte chan_byt, unsigned int ana_value_int);
#ifdef BATCHECK_ENABLED
		unsigned int ReadBattery();
#endif
};		
#endif
