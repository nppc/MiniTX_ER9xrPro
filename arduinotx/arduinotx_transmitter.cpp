/* arduinotx_transmitter.ino - Tx manager
** 01-05-2013 class ArduinoTx
** 04-05-2013 read_potentiometer()
** 16-05-2013 ReadControl()
** 22-06-2013 ComputeChannelPulse() fixed dual rates
** 25-06-2013 CurrentDataset_byt, TxAlarm_int, get_selected_dataset()
** 27-06-2013 get_selected_dataset()
** 13-08-2013 Init() configures PPM_PIN and LED_PIN as outputs and all other digital pins as inputs with pull-up
** 17-08-2013 Init() fixed, check_throttle() improved
** 18-08-2013 check_battery()
** 23-08-2013 Init()
** 24-08-2013 ComputeChannelPulse() fixed underflow in subtrim computation, changed end points computation
** 26-08-2013 changed Morse codes flashed on the Led
** 31-08-2013 check_battery() ignore invalid samples
** 14-05-2014 get_selected_dataset(), debounce_modelswitch()
** 17-05-2014 process_model_switch_rotating(), debounce_rotating_switch()
** 26-05-2014 dual rate and exponential may apply to the throttle channel too
** 28-05-2014 full exponential curve may be applied to throttle channel for gas engines
** 29-05-2014 ReadBattery()
** 30-05-2014 changed Endpoints semantics: now EPL,EPH: [0,100] defines end point position in % from the center
** 01-06-2014 fixed cond compil of ReadBattery()
**
Copyright (C) 2014 Richard Goutorbe.  All right reserved.
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
Contact information: http://www.reseau.org/arduinorc/index.php?n=Main.Contact
*/


#include "arduinotx_transmitter.h"
#include "arduinotx_led.h"
#include "arduinotx_command.h"
#ifdef BUZZER_ENABLED
#include "arduinotx_buzz.h"
#endif
#include "arduinotx_lib.h"

// PPM signal -----------------------------------------------------------------

const byte ArduinoTx::PPM_PIN = 10;				// PPM output pin, hard-wired to ISR Timer 1 on ATMega 328

#if CHANNELS <= 6
const unsigned int ArduinoTx::PPM_PERIOD = 20000;	// microseconds; send PPM sequence every 20ms for 6 channels
const unsigned int ArduinoTx::PPM_LOW = 400;		// microseconds; fixed channel sync pulse width in the PPM signal
#else
const unsigned int ArduinoTx::PPM_PERIOD = 22000;	// microseconds; send PPM sequence every 22ms if more than 6 channels
const unsigned int ArduinoTx::PPM_LOW = 300;		// microseconds; fixed channel sync pulse width in the PPM signal
#endif

// Morse codes flashed on the Led --------------------------------------------
// they must be defined in ArduinotxLed::LedMorseCodes_str[] and BuzzerMorseCodes_str[]
const char ArduinoTx::LEDCHAR_INIT = '0';				// ----- undefined, never displayed
const char ArduinoTx::LEDCHAR_COMMAND = 'C';			// -.-. command mode
const char ArduinoTx::LEDCHAR_ALARM_EEPROM = 'P';		// .--. settings failed to load from EEPROM
const char ArduinoTx::LEDCHAR_ALARM_THROTTLE = 'T';	// - throttle security check failed
const char ArduinoTx::LEDCHAR_ALARM_BATTERY = 'B';		// -... low battery voltage

/* 
** Variables allocated in arduinotx.ino  --------------------------------------------
*/

// Command mode interpreter
extern ArduinotxCmd Command_obj;
// EEPROM manager
extern ArduinotxEeprom Eeprom_obj;
// Led manager
extern ArduinotxLed Led_obj;
#ifdef BUZZER_ENABLED
// Buzzer manager
extern ArduinotxBuzz Buzzer_obj;
#endif
/*
** Public interface ------------------------------------------------------------
*/

ArduinoTx::ArduinoTx() {
	// MODE_SWITCH	RunMode
	//	opened		RUNMODE_TRANSMISSION
	//	closed		RUNMODE_COMMAND
	RunMode_int = RUNMODE_INIT;

	CurrentDataset_byt = 0; // Dataset (model number) currently loaded in RAM
	TxAlarm_int = ALARM_NONE; // current alarm state
	DualRate_bool = true; // true=dual rate ON (NPPC will be never false)
	ThrottleCut_bool = false; // true=throttle disabled, false=throttle enabled
	EngineEnabled_bool = false; // will be updated by Refresh()
	SettingsLoaded_bool = false; // set by Init() at startup
	CommitChanges_bool = false; // set by process_command_line() after changing a variable, reset by loop()
}

void ArduinoTx::Init() {

	// Configure all unused digital pins as inputs with pull-up
	// because floating pins would increase current consumption
	for (byte idx_byt=2; idx_byt <= 13; idx_byt++) {
		switch (idx_byt) {
#ifdef BUZZER_ENABLED
			case BUZZER_PIN:
#endif
			case MULTIPROTOCOL_POWER_TX_PIN:
			case MULTIPROTOCOL_POWER_MODULE1_PIN:
			case MULTIPROTOCOL_POWER_MODULE2_PIN:
			case MULTIPROTOCOL_CONTROL1_PIN:
			case MULTIPROTOCOL_CONTROL2_PIN:
			case LED_PIN:
			case PPM_PIN:
				pinMode(idx_byt, OUTPUT);
				break;
			case THROTTLECUT_SWITCH_PIN: // NPPC. we do not need pullup here, because we have pulldown resistor connected on this pin. 
				pinMode(idx_byt, INPUT);
				break;
			default:
				pinMode(idx_byt, INPUT_PULLUP);
				break;
		}
	}
	
	// Configure all unused analog pins as inputs with pull-up
#ifndef BATCHECK_ENABLED // BATCHECK_PIN is A7
	pinMode(BATCHECK_PIN, INPUT_PULLUP);
#endif
#if NPOTS <= 6	
	pinMode(A6, INPUT_PULLUP);
#endif
#if NPOTS <= 5	
	pinMode(A5, INPUT_PULLUP);
#endif
#if NPOTS <= 4	
	pinMode(A4, INPUT_PULLUP);
#endif
#if NPOTS <= 3	
	pinMode(A3, INPUT_PULLUP);
#endif
#if NPOTS <= 2	
	pinMode(A2, INPUT_PULLUP);
#endif
#if NPOTS <= 1	
	pinMode(A1, INPUT_PULLUP);
#endif
#if NPOTS == 0	
	pinMode(A0, INPUT_PULLUP);
#endif
	
	if (Eeprom_obj.CheckEEProm() > 0) {
		load_settings();
		SettingsLoaded_bool = true;
		TxAlarm_int = ALARM_NONE;
	}
	else {
		SettingsLoaded_bool = false;
		TxAlarm_int = ALARM_EEPROM;  // clear by Reset of Arduino board
	}

	// configure Timer1 for PPM generation
	TCCR1A = B00110001;	// COM1B1, COM1B0, WGM10 set to 1 (8-bit register)
	TCCR1B = B00010010;	// WGM13 & CS11 set to 1 (8-bit register)
	TCCR1C = B00000000;
	TIMSK1 = B00000010;	// All interrupts are individually masked with the Timer Interrupt Mask Register TIMSK1
	TIFR1  = B00000010;	// Int on compare B
	OCR1A = PPM_PERIOD;	// PPM frequency (double buffered Output Compare 16-bit Register)
	OCR1B = PPM_LOW;		// (double buffered Output Compare 16-bit Register), hard-wired to PPM_PIN 
}	

// called by process_command_line() after changing a variable, reset by Refresh() 
void ArduinoTx::CommitChanges() {
	CommitChanges_bool = true;
}

// Update ArduinoTx state, called by loop() state every TXREFRESH_PERIOD ms
void ArduinoTx::Refresh() {
	
#if MODEL_SWITCH_BEHAVIOUR == MODEL_SWITCH_STEPPING
	// select next dataset if the model switch is moved
	process_model_switch_stepping();
#elif MODEL_SWITCH_BEHAVIOUR == MODEL_SWITCH_ROTATING
	// select next dataset if the model switch is moved
	process_model_switch_rotating();
#endif
	
	// set CurrentDataset_byt according to the model switch
	byte current_dataset_byt = get_selected_dataset(); 
	if (current_dataset_byt != CurrentDataset_byt) {
		// Load settings of newly selected model
		load_settings(); // updates CurrentDataset_byt
	}
	
	// Read other transmitter special switches
	//DualRate_bool = digitalRead(DUALRATE_SWITCH_PIN); // NPPC never use dual rates functionality
	ThrottleCut_bool = !digitalRead(THROTTLECUT_SWITCH_PIN); // chaned by NPPC (we have pulldown resitor here. changing this behaviour to have double arming)
	
	// set RunMode according to switches 
	RunMode_int = refresh_runmode();
	
	// reload settings if they were updated while in command mode
	if (RunMode_int == RUNMODE_COMMAND && CommitChanges_bool) {
		load_settings();
		CommitChanges_bool = false;
	}
	
	if (!EngineEnabled_bool) {
		// re-enable throttle if the corresponding control has been reset to its lowest value
		EngineEnabled_bool = check_throttle() == 1;
	}

#ifdef BATCHECK_ENABLED
	const unsigned long BATCHECK_PERIOD = 5000 ; // ms
	static unsigned long Last_BatCheck_int = 0;
	unsigned long now_int = millis(); // timer overflows after 50 days
	if (now_int >= Last_BatCheck_int + BATCHECK_PERIOD || now_int < Last_BatCheck_int) {
		check_battery();
		Last_BatCheck_int = now_int;
	}
#endif
	// update the morse character displayed by the Led
	refresh_led_code();
	
}

// Read the input control corresponding to given channel, using the array of assignement of potentiometers and switches
// chan_byt : 0-based, channel number - 1
// Return value: calibrated value [0, 1023]
// It takes about 100 microseconds to read an analog input
unsigned int ArduinoTx::ReadControl(byte chan_byt) {
	unsigned int retval_int = 0;
	byte ctrl_type_byt = get_channel_var(chan_byt, CHAN_ICT);
	byte ctrl_number_byt = get_channel_var(chan_byt, CHAN_ICN);
	switch (ctrl_type_byt) {
		case ICT_ANALOG: 
			if (ctrl_number_byt > 0 && ctrl_number_byt <= NPOTS)
				retval_int = read_potentiometer(ctrl_number_byt);
			break;
		case ICT_DIGITAL:
			if (ctrl_number_byt > 0 && ctrl_number_byt <= NSWITCHES)
				retval_int = digitalRead(get_switch_pin(ctrl_number_byt)) == HIGH ? 1023: 0; // switch
			break;
		case ICT_MIXER: {
			long value_lng = 0L;
			byte pot_number_byt = 0;
			--ctrl_number_byt; // get_mixer_var() expects a 0-based mixer index
			// mixer input 1
			pot_number_byt = get_mixer_var(ctrl_number_byt, MIX_N1M);
			if (pot_number_byt > 0 && pot_number_byt <= NPOTS)
				value_lng = (read_potentiometer(pot_number_byt) - 512L) * get_mixer_var(ctrl_number_byt, MIX_P1M);
			// mixer input 2
			pot_number_byt = get_mixer_var(ctrl_number_byt, MIX_N2M);
			if (pot_number_byt > 0 && pot_number_byt <= NPOTS)
				value_lng += (read_potentiometer(pot_number_byt) - 512L) * get_mixer_var(ctrl_number_byt, MIX_P2M);
			// resulting value
			retval_int = constrain(512L + (value_lng / 100L), 0, 1023);
			}
			break;
		case ICT_OFF:
			retval_int = 0; // do not read actual value and always return 0
			break;
	}
	return retval_int;
}

// Compute the channel pulse corresponding to given analog value
// chan_byt : 0-based, channel number - 1
// ana_value_int : read from input: [0,1023]
// Return value: PWM pulse width, microseconds
// Warning: calling Serial.print() within this method will probably hang the program, a safer way is to display global var DebugValue_int in loop() :
//~ extern unsigned int DebugValue_int;
unsigned int ArduinoTx::ComputeChannelPulse(byte chan_byt, unsigned int ana_value_int) {
	unsigned int retval_int = 0;
	unsigned int value_int = ana_value_int;
	
	byte throttle_channel_byt = get_model_var(MOD_THC) - 1; // 0-based throttle chan number
	if (ThrottleCut_bool || !EngineEnabled_bool)
		if (chan_byt == throttle_channel_byt)
			value_int = 0; // cut throttle
	
	// Dual rate and Exponential
	if (DualRate_bool) {
		// apply exponential
		byte expo_byt = get_channel_var(chan_byt, CHAN_EXP); // 0=none, 25=medium, 50=strong 100=too much
		if (expo_byt != 0) {
			// apply full exponential curve to the throttle channel (contributed by jbjb)
			float expoval_flt = expo_byt / 10.0;
			if (chan_byt == throttle_channel_byt) {
				float value_flt = value_int / 1023.0; // map to [0, +1] range
				value_flt = value_flt * exp(abs(expoval_flt * value_flt)) / exp(expoval_flt);
				value_int = (unsigned int)(1023 * value_flt); // map to [0, 1023] range
			}
			else {
				// apply centered symetrical curve to other channels
				float value_flt = 2.0 * ((value_int / 1023.0) - 0.5); // map to [-1, +1] range
				value_flt = value_flt * exp(abs(expoval_flt * value_flt)) / exp(expoval_flt);
				value_int = 512 + (unsigned int)(511.5 * value_flt); // map to [0, 1023] range
			}
		}
		else {
			// apply dual rate if no exponential for this channel
			unsigned int offset_int = get_channel_var(chan_byt, CHAN_DUA);
			if (offset_int != 100) {
				offset_int = offset_int << 9; // multiply by 512, max value = 100*512 = 51200
				value_int = (unsigned int)(map(value_int, 0, 1023, 51200 - offset_int, 51100 + offset_int) / 100);
			}
		}
	}
	
	// apply subtrim
	int trim_int = get_channel_var(chan_byt, CHAN_SUB);
	if (trim_int) {
		// approximate 1024/100 = 10.24 ~ 10
		value_int += 10 * trim_int;
		if (value_int > 32767)
			value_int = 0L; // underflow
		else if (value_int > 1023)
			value_int = 1023;
	}
	
	//~ // debug: display value_int in loop()
	//~ if (chan_byt == elevator_channel_byt)
		//~ DebugValue_int = value_int;
	
	// apply end points
	// EPL,EPH: [0,100] end point position in % from the center, 
	// examples: 10=10% from the center, 90=90% from the center (10% from the maximum throw), 100=maximum throw (no endpoint)
	//
#if ENDPOINTS_ALGORITHM == ENDPOINTS_LIMITED
	// the control stick has 2 dead-angles corresponding to each endpoint. Moving the stick
	// beyond this angle will have no effect on the PPM signal.
	unsigned int endpoint_int = (unsigned int)(5.11 * (100 - get_channel_var(chan_byt, CHAN_EPL))); // EPL=80: 5.11 * 20 = 102.2
	if (value_int < endpoint_int)
		value_int = endpoint_int;
	else {
		endpoint_int = 511 + (unsigned int)(5.12 * get_channel_var(chan_byt, CHAN_EPH)); // EPH=80: 511 + (5.12 * 80) = 920.6
		if (value_int > endpoint_int)
				value_int = endpoint_int;
	}
#else
	// ENDPOINTS_ALGORITHM == ENDPOINTS_BILINEAR
	// the control stick has no dead-angles: moving it from min to max will output a PPM signal 
	// within the endpoints interval. However, the variation rate of the signal in the lower half of
	// the interval will not be the same as in the higher half if CHAN_EPL != CHAN_EPH.
	// This may be acceptable or not.
	unsigned int endpoint_int = 0;
	if (value_int < 512) {
		endpoint_int = (unsigned int)(5.11 * (100 - get_channel_var(chan_byt, CHAN_EPL)));
		value_int = (unsigned int)map(value_int, 0, 511, endpoint_int, 511);
	}
	else {
		endpoint_int = 512 + (5.12 * get_channel_var(chan_byt, CHAN_EPH));
		if (endpoint_int == 1024)
			endpoint_int = 1023;
		value_int = (unsigned int)map(value_int, 512, 1023, 512, endpoint_int);
	}
#endif

	// apply reverse
	unsigned int low_int = get_channel_var(chan_byt, CHAN_PWL); // Minimal pulse width for this channel, in microseconds
	unsigned int high_int = get_channel_var(chan_byt, CHAN_PWH); // Maximal pulse width for this channel, in microseconds
	if (get_channel_var(chan_byt, CHAN_REV)) {
		unsigned int tmp_int = low_int;
		low_int = high_int;
		high_int = tmp_int;
	}
	
	// Map analog inputs to PPM rates
	retval_int =  map(value_int, 0, 1023, low_int, high_int);
	
	return retval_int;
}

/*
** Private Implementation ------------------------------------------------------------
*/

// load settings values from EEPROM
// updates CurrentDataset_byt
void ArduinoTx::load_settings() {
	Eeprom_obj.GetGlobal(Global_int); // load values of GLOBAL_CDS and GLOBAL_ADS
	CurrentDataset_byt = get_selected_dataset(); // Dataset (model number) currently loaded in RAM
	Eeprom_obj.GetDataset(CurrentDataset_byt, DatasetModel_int, DatasetMixers_int, DatasetChannels_int);
}

// set RunMode according to switches settings
// MODE_SWITCH	RunMode
//	opened		RUNMODE_TRANSMISSION
//	closed		RUNMODE_COMMAND
// Return value: current run mode
ArduinoTx::RunMode ArduinoTx::refresh_runmode() {
	RunMode retval_int = RUNMODE_INIT;
	static RunMode Last_runmode_int = RUNMODE_INIT;
	
	// read the mode switch
	byte mode_switch_bool = digitalRead(MODE_SWITCH_PIN);
	
	if (!SettingsLoaded_bool) {
		// Invalid settings have been detected in EEPROM
		// and the Alarm Led is lit.
		// Ignore actual Mode switch position and force Command mode.
		// User should enter command INIT to properly initialize all settings,
		// then user must reset the Arduino (press the reset switch or cycle the power) 
		mode_switch_bool = LOW;
	}
	
	if (mode_switch_bool)
		retval_int = RUNMODE_TRANSMISSION;
	else
		retval_int = RUNMODE_COMMAND;
	
	if (retval_int != Last_runmode_int) {
		if (retval_int == RUNMODE_COMMAND) {
			// entering command mode
			// open serial and print command prompt
			Command_obj.InitCommand();
		}
		else {
			if (Last_runmode_int == RUNMODE_COMMAND) {
				// leaving command mode: close serial
				Command_obj.EndCommand();
			}
		}
		Last_runmode_int = retval_int;
	}
	return retval_int;
}

// Throttle security check
// updates TxAlarm_int
// return values:
/// 1 if throttle is < GLOBAL_TSC or if no throttle channel
// 0 if throttle is >= GLOBAL_TSC and sets ALARM_THROTTLE
byte ArduinoTx::check_throttle() {
	byte retval_byt = 1;
	static unsigned int Average_int = 2 * get_global_var(GLOBAL_TSC); // average of last 8 analog readings
	static byte Count_byt = 0;
	int sample_int = 0;
	byte throttle_chan_byt = get_model_var(MOD_THC);
	if (throttle_chan_byt) {
		do {
			sample_int = ReadControl(throttle_chan_byt - 1);
			Average_int = (7 * Average_int + sample_int) >> 3; // >>3 divides by 8
			if (Count_byt == 30)
					break;
				else
					Count_byt++;
		} while (1);
		
		retval_byt = Average_int < (unsigned int)get_global_var(GLOBAL_TSC) ? 1:0;
		if (retval_byt == 0)
			TxAlarm_int = ALARM_THROTTLE; // Throttle security check has top priority: overwrite all other alarms
		else if (TxAlarm_int == ALARM_THROTTLE)
			TxAlarm_int = ALARM_NONE;
	}
	//~ if (retval_byt == 0)
		//~ aprintfln("check_throttle() pot=%d av=%u returns %d", sample_int, Average_int, retval_byt);
	return retval_byt;
}


// Read the state of the Model switch and return the corresponding dataset number
byte ArduinoTx::get_selected_dataset() {
	byte retval_byt = 0;
#if MODEL_SWITCH_BEHAVIOUR == MODEL_SWITCH_SIMPLE
	retval_byt = get_global_var(digitalRead(MODEL_SWITCH_PIN) ? GLOBAL_CDS:GLOBAL_ADS);
#elif MODEL_SWITCH_BEHAVIOUR == MODEL_SWITCH_STEPPING
	retval_byt = get_global_var(GLOBAL_CDS);
#elif MODEL_SWITCH_BEHAVIOUR == MODEL_SWITCH_ROTATING
	retval_byt = get_global_var(GLOBAL_CDS);
#endif
	return retval_byt;
}

#if MODEL_SWITCH_BEHAVIOUR == MODEL_SWITCH_STEPPING
// select next dataset if the model switch is moved
// when MODEL_SWITCH_STEPPING has been selected, the active dataset can be changed in Command Mode only 
void ArduinoTx::process_model_switch_stepping() {
	if (RunMode_int == RUNMODE_COMMAND) {
		static byte Last_State_bool = HIGH;
		byte state_bool = debounce_modelswitch();
		if (state_bool != Last_State_bool) {
			if (state_bool == LOW) {
				// opened -> closed transition : select next model
				Command_obj.NextDataset();
			}
			Last_State_bool = state_bool;
		}
	}
}
// Return the debounced state of the Model switch
byte ArduinoTx::debounce_modelswitch() {
	static byte Retval_bool = LOW;
	const byte COUNT = 4; // update the debounced value every COUNT calls (this method is called every TXREFRESH_PERIOD ms)
	static byte Count_byt = COUNT - 1; // force read the switch state on 1st call
	if (++Count_byt == COUNT) {
		Retval_bool = digitalRead(MODEL_SWITCH_PIN);
		Count_byt = 0;
	}
	return Retval_bool;
}

#elif MODEL_SWITCH_BEHAVIOUR == MODEL_SWITCH_ROTATING
// select next dataset if the model switch is moved
// when MODEL_SWITCH_STEPPING has been selected, the active dataset can be changed in Command Mode only 
void ArduinoTx::process_model_switch_rotating() {
	if (RunMode_int == RUNMODE_COMMAND) {
		static byte Last_position_int = 255;
		byte position_int = debounce_rotating_switch();
		if (position_int != Last_position_int) {
			Command_obj.SelectDataset(position_int+1);
			Last_position_int = position_int;
		}
	}
}
// Return the debounced position of the rotating switch
// return value: [0, MODEL_ROTATING_SWITCH_STEPS]
byte ArduinoTx::debounce_rotating_switch() {
	static byte Retval_byt = 0;

	// read current position
	byte position_int = 0;
	const unsigned int step_int = 1024 / MODEL_ROTATING_SWITCH_STEPS;
	const unsigned int offset_int = step_int / 2;
	unsigned int limit_int = offset_int;
	unsigned int sample_int = 0; //analogRead(MODEL_ROTATING_SWITCH_PIN) & 0xFFF8; // filter noise: ignore least significant 3 bits (NPPC not in use)
	while (sample_int > limit_int) {
		position_int++;
		limit_int += step_int;
	}

	// validate position if did not change for COUNT calls
	const byte COUNT = 4; // update the debounced value every COUNT calls (this method is called every TXREFRESH_PERIOD ms)
	static byte Count_byt = 0;
	static byte Last_position_int = 0;
	if (position_int == Last_position_int) {
		if (++Count_byt == COUNT) {
			Retval_byt = position_int;
			Count_byt = 0;
		}
	}
	else
		Count_byt = 0;
	Last_position_int = position_int;
	
	return Retval_byt;
}
#endif

// Update the morse character displayed by the Led
void ArduinoTx::refresh_led_code() {
	static char Current_ledcode_char = LEDCHAR_INIT;
	char ledcode_char = LEDCHAR_INIT;
	
	// alarm modes have priority over normal modes
	//aprintfln("refresh_led_code() TxAlarm_int=%d", TxAlarm_int);
	switch(TxAlarm_int) {
		// alarm modes
		case ALARM_EEPROM:
			ledcode_char = LEDCHAR_ALARM_EEPROM;
			break;
		
		case ALARM_THROTTLE:
			ledcode_char = LEDCHAR_ALARM_THROTTLE;
			break;
		
		case ALARM_BATTERY:
			ledcode_char = LEDCHAR_ALARM_BATTERY;
			break;
		
		case ALARM_NONE:
			// normal modes
			//aprintfln("refresh_led_code() RunMode_int=%d CurrentDataset_byt=%d", RunMode_int, CurrentDataset_byt);
			switch (RunMode_int) {
				case RUNMODE_COMMAND:
					ledcode_char = LEDCHAR_COMMAND;
					break;
				case RUNMODE_TRANSMISSION:
					ledcode_char = CurrentDataset_byt + '0';
					break;
				case RUNMODE_INIT: // suppress compiler warning
					break;
			}
			break;
	}
	if (ledcode_char != Current_ledcode_char) {
		//aprintfln("refresh_led_code() RunMode_int=%d TxAlarm_int=%d set code %c", RunMode_int, TxAlarm_int, ledcode_char);
		Led_obj.SetCode(ledcode_char);
#ifdef BUZZER_ENABLED
		if (TxAlarm_int != ALARM_NONE)
			Buzzer_obj.SetCode(ledcode_char, BUZZER_REPEAT, 400);
		else
			Buzzer_obj.SetCode(ledcode_char, 3, 800);
#endif
		Current_ledcode_char = ledcode_char;
	}
}

#ifdef BATCHECK_ENABLED
// Battery voltage check
// updates TxAlarm_int
// return values:
// 1 if voltage is > GLOBAL_BAT
// 0 if throttle is <= GLOBAL_BAT and sets ALARM_BATTERY
byte ArduinoTx::check_battery() {
	byte retval_byt = ReadBattery() > (unsigned int)get_global_var(GLOBAL_BAT) ? 1:0;
	if (retval_byt == 0) {
		if (TxAlarm_int == ALARM_NONE)
			TxAlarm_int = ALARM_BATTERY;
	}
	else if (TxAlarm_int == ALARM_BATTERY)
		TxAlarm_int = ALARM_NONE;
	return retval_byt;
}

// Read the battery voltage, averaged over several measurements
// Return value: [0, 1023] ; since we sample the voltage through a 50/50 resistor bridge we return 1023 for 10V, i.e.  102 for 1V
unsigned int ArduinoTx::ReadBattery() {
	unsigned int retval_int = 2 * get_global_var(GLOBAL_BAT); // average of last 8 analog readings
	byte count_byt = 0;
	int last_sample_int = 0;
	int sample_int = 0;
	do {
		sample_int = analogRead(BATCHECK_PIN);
		if (sample_int >=  last_sample_int >> 1) {
			retval_int = (7 * retval_int + sample_int) >> 3; // >>3 divides by 8
			last_sample_int = sample_int;
		}
		else {
			// False readings happen once in a while, may be due to analogRead() being interrupted by ISR1 ?
			//~ aprintfln("check_battery() ignore %d", sample_int);
			sample_int =  last_sample_int; // ignore invalid sample
		}
		count_byt++;
	} while (count_byt < 30);
	retval_int += BATVOLT_CORRECTION; // see arduinotx_config.h
	return retval_int;
}
#endif

// Return calibrated value of given potentiometer
unsigned int ArduinoTx::read_potentiometer(byte pot_number_byt) {
	unsigned int retval_int = 0;
	unsigned int chan_cal_int = get_calibration_var(pot_number_byt, CAL_LOW); // lowest value returned by the potentiometer corresponding to given channel
	unsigned int chan_cah_int = get_calibration_var(pot_number_byt, CAL_HIGH); // highest value returned by the potentiometer corresponding to given channel
	retval_int = analogRead(get_pot_pin(pot_number_byt));
	retval_int = constrain(retval_int, chan_cal_int, chan_cah_int);
	retval_int = map(retval_int, chan_cal_int, chan_cah_int, 0, 1023);
	return retval_int;
}


