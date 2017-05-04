/* arduinotx.ino - Open source RC transmitter software for the Arduino 
** 27-07-2012
** 08-08-2012 send_ppm() fixed
** 20-09-2012 calibration
** 15-10-2012 ReadControl(), ICT_OFF, ICT_SLAVE
** 25-10-2012 generate PPM signal with Timer1 ISR
** 28-10-2012 get_test_value()
** 29-10-2012 del ICT_SLAVE
** 30-10-2012 renaming
** 01-11-2012 generate PPM with ISR
** 04-11-2012 flash() led, test mode optional
** 25-12-2012 ISR(TIMER1_COMPA_vect) added support for print ppm
** 30-12-2012 version 1.1.0: SOFTWARE_VERSION, CommitChanges_bool
** 05-01-2013 version 1.1.1: ComputeChannelPulse() dual rate
** 12-01-2013 version 1.1.2 (12548 bytes): replaced NCHANNELS by CHANNELS
** 13-01-2013 version 1.1.2a (12620 bytes): set back Serial 9600 Bauds because 115200 was causing issues when pasting multiple lines
** 23-02-2013 version 1.1.4 ComputeChannelPulse() integer math, deleted arg enable_calibration_bool from ComputeChannelPulse()
** 24-02-2013 input validation in command mode
** 02-03-2013 version 1.1.4a renamed TEST_SWITCH_PIN as MODEL_SWITCH_PIN, always defined, deleted TEST_MODE
** 03-03-2013 version 1.1.4a moved led management to new class ArduinotxLed
** 05-03-2013 version 1.1.4c fixed all compilation warnings, ArduinotxLed displays morse characters
** 08-03-2013 v1.1.4d Throttle channel defined for each model
** 09-03-2013 10:41:00 v1.1.4e disabled PPM during setup() 
** 30-04-2013  v1.1.4g command line serial input may be '\n' or '\r' terminated; ECHO UPLOAD for txupload utility
** 01-05-2013 v1.2.0 class ArduinoTx
** 02-05-2013 v1.2.1  ignore comments in commands
** 03-05-2013 v1.2.2 Mixers
** 04-05-2013 v1.2.2a
** 12-05-2013 v1.2.3 deleted 2 unused constants in arduinotx_transmitter.h
** 16-05-2013 v1.2.4 Command mode: removed ECHO OFF from DUMP output, allow 0 input for N1M,N2M in validate_value(), ArduinoTx::ReadControl()
** 06-06-2013 v1.2.5 moved arrays to PROGMEM, Exponential
** 29-06-2013 txupload: added optional arg to resume upload at line number 
** 04-07-2013 v1.3.0 Comments start by '#' instead of ';'
** 12-08-2013 v1.3.1 arduinotx_lib
** 13-08-2013 v1.3.2 LED_PIN, ArduinoTx::Init()
** 14-08-2013 v1.3.2  ArduinotxLed, ArduinotxBuzz, getProgmemStrpos(), getProgmemStrchr()
** 15-08-2013 v1.3.2 ArduinotxBuzz PlayCount_byt
** 17-08-2013 v1.3.2 ArduinotxBuzz, ArduinotxLed OK, Init() fixed, check_throttle() avarages throttle pot readings
** 18-08-2013 v1.3.2 buzzes 3 times current mode
** 18-08-2013 v1.4.0 ArduinoTx::check_battery(), GLOBAL_BAT
** 20-08-2013 v1.4.0 TXREFRESH_PERIOD = 50, ArduinoTx::Refresh() calls check_battery() every 5s
** 23-08-2013 v1.4.0  ArduinoTx::Init() fixed pinmode initialization
** 24-08-2013 v1.4.0  ComputeChannelPulse() fixed underflow in subtrim computation, this bug existed in v1.3 for negative values of CHAN_SUB< -10%
** 25-08-2013 v1.4.0  var BAT ok in cmd mode
** 26-08-2013 v1.4.0  changed Morse codes flashed on the Led
** 28-08-2013 v1.4.0 Battery check
** 31-08-2013 v1.4.0 Battery check ok
** 01-09-2013 v1.4.0 BAT default value  = 740	Flash=19298	Ram=683

** 08-09-2013 v1.5.0 branche /arduinorc/branches/moreswitches
** 14-05-2014 implemented MODEL_SWITCH_STEPPING
** 16-05-2014 v1.5.1 implemented MODEL_SWITCH_ROTATING; MODEL_SWITCH_BEHAVIOUR defaults to MODEL_SWITCH_STEPPING
** 26-05-2014 v1.5.2 full curve exponential may apply to the throttle channel
** 28-05-2014 v1.5.3 new bilinear endpoints algorithm; moved all user-customizable settings to arduinotx_config.h
** 29-05-2014 v1.5.4 new PRINT VOLT command displays battery voltage; EPL, EPH : End point percentage [51-100]
** 30-05-2014 v1.5.5 changed Endpoints semantics: now EPL,EPH: [0,100] defines end point position in % from the center ; 
	lowered command mode's serial speed to 2400 bps to prevent Arduino's serial buffer overflow when uploading with txupload; 
** 01-06-2014 v1.5.5 fixed cond compil of ReadBattery()
	avr-size: Flash=19808 Ram=686

Notice: avr-size reports memory usage for Arduino Nano ATmega328 with CHANNELS=6, BUZZER_ENABLED, BATCHECK_ENABLED, MODEL_SWITCH_BEHAVIOUR=MODEL_SWITCH_STEPPING, ENDPOINTS_ALGORITHM=ENDPOINTS_BILINEAR

Copyright (C) 2014 Richard Goutorbe.  All right reserved.
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
Contact information: http://www.reseau.org/arduinorc/index.php?n=Main.Contact
*/

/*
** Resources -----------------------------------------------
*/

#include <EEPROM.h>
#include "arduinotx_lib.h"
#include "arduinotx_eeprom.h"
#include "arduinotx_led.h"
#include "arduinotx_command.h"
#include "arduinotx_transmitter.h"
#ifdef BUZZER_ENABLED
#include "arduinotx_buzz.h"
#endif

/*
** Global declarations -----------------------------------------------
*/

// Tx manager
ArduinoTx ArduinoTx_obj;

// Command mode interpreter
ArduinotxCmd Command_obj;

// EEPROM manager
ArduinotxEeprom Eeprom_obj;

// Led manager
ArduinotxLed Led_obj(LED_PIN);

#ifdef BUZZER_ENABLED
// Buzzer manager
ArduinotxBuzz Buzzer_obj(BUZZER_PIN);
#endif

// These 2 global variables are used to request the PPM signal values from ISR(TIMER1_COMPA_vect)
volatile byte RequestPpmCopy_bool = false;
volatile unsigned int PpmCopy_int[CHANNELS]; // pulse widths (microseconds)

/*
** Arduino specific functions -----------------------------------------------------------------
*/

ModelVarNames_str[] PROGMEM = {
	"FRSKYX",
	"FRSKYD",
	"BAYANG",
	"ERROR "};

void setup() {
	// hold Multiprotocol Arduino in RESET state while setting data
	digitalWrite(MULTIPROTOCOL_RESET_PIN,LOW);

	noInterrupts(); // No PPM signal until initialization complete
	ArduinoTx_obj.Init();
	interrupts();

	// check, do we have protocol change button pressed?
	byte tmp = getLastEepromValue();
	if (digitalRead(MODEL_SWITCH_PIN)==LOW){
		// change protocol
		tmp++;
		if (tmp>3){tmp=1;}
		storeLastEepromValue(tmp);
		// TODO: in the future show message that protocol is changed
		// wait untill button released
		while (digitalRead(MODEL_SWITCH_PIN)==LOW){}
		delay(200); //software debounce
	}
	
	// now lets set Multiprotocol encoder
	char var_str[7];
	switch (tmp) {
		case B00000001:	// Protocol 1 - FrSkyX
			digitalWrite(MULTIPROTOCOL_CONTROL1_PIN,LOW);
			digitalWrite(MULTIPROTOCOL_CONTROL1_PIN,HIGH);
			getProgmemStrArrayValue(var_str, ModelVarNames_str, 0, 7);
			break;
		case B00000010:	// Protocol 2 - FrSkyD
			digitalWrite(MULTIPROTOCOL_CONTROL1_PIN,HIGH);
			digitalWrite(MULTIPROTOCOL_CONTROL1_PIN,LOW);
			getProgmemStrArrayValue(var_str, ModelVarNames_str, 1, 7);
			break;
		case B00000011:	// Protocol 3 - Eachine H8
			digitalWrite(MULTIPROTOCOL_CONTROL1_PIN,LOW);
			digitalWrite(MULTIPROTOCOL_CONTROL1_PIN,LOW);
			getProgmemStrArrayValue(var_str, ModelVarNames_str, 2, 7);
			break;
		default:
			digitalWrite(MULTIPROTOCOL_CONTROL1_PIN,HIGH);
			digitalWrite(MULTIPROTOCOL_CONTROL1_PIN,HIGH);
			getProgmemStrArrayValue(var_str, ModelVarNames_str, 3, 7);
	}
	// boot Multiprotocol Arduino
	digitalWrite(MULTIPROTOCOL_RESET_PIN,HIGH);
	// TODO: show protocol name and wait 2 seconds var_str[7]
	

}

/* serialEvent() occurs whenever a new data comes in the hardware serial RX. 
** This routine is run between each time loop() runs, so using delay inside loop()
** can delay response. Multiple bytes of data may be available.
*/
void serialEvent() {
	if (Serial.available())
		Command_obj.Input();
}

//~ volatile unsigned int DebugValue_int = 0; // debug

// Generate PPM sequence
// Warning: calling Serial.print() within this method will probably hang the program
ISR(TIMER1_COMPA_vect) {
	static byte Chan_idx_byt = CHANNELS;
	static unsigned int Chan_pulse_int[CHANNELS]; // pulse widths (microseconds)
	static unsigned int Sum_int = 0;
	
	if (Chan_idx_byt < CHANNELS) {
		unsigned int chanpulse_int = Chan_pulse_int[Chan_idx_byt++];
		// send current channel pulse
		OCR1A = chanpulse_int;
		Sum_int += chanpulse_int;
	}
	else {
		// send final sync pulse
		Chan_idx_byt = 0;
		OCR1A = ArduinoTx::PPM_PERIOD - Sum_int;
		Sum_int = 0;
		// Read input controls and update Chan_pulse_int[]
		// Execution time: [2308, 2324] microseconds < OCR1A
		for (byte chan_byt = 0; chan_byt < CHANNELS; chan_byt++) {
			unsigned int control_value_int = 0;
			control_value_int = ArduinoTx_obj.ReadControl(chan_byt);
			Chan_pulse_int[chan_byt] = ArduinoTx_obj.ComputeChannelPulse(chan_byt, control_value_int);
		}
		if (RequestPpmCopy_bool) {
			// copy the PPM sequence values into global array for the "print ppm" command
			for (byte chan_byt = 0; chan_byt < CHANNELS; chan_byt++)
				PpmCopy_int[chan_byt] = Chan_pulse_int[chan_byt] ;
			RequestPpmCopy_bool = false;
		}
	}
}

// The main loop is interrupted every PPM_PERIOD ms by ISR(TIMER1_COMPA_vect)
// we perform non time-critical operations in here
void loop() {
	// TXREFRESH_PERIOD defines the frequency at which Special Switches are read and corresponding transmitter state is updated
	const unsigned int TXREFRESH_PERIOD = 50L; // 50ms =20Hz, should be >= 20 ms
	static unsigned long Last_TxRefresh_int = 0L;
	unsigned long now_int = millis(); // timer overflows after 50 days
	if (now_int >= Last_TxRefresh_int + TXREFRESH_PERIOD || now_int < Last_TxRefresh_int) {
		ArduinoTx_obj.Refresh();
		Last_TxRefresh_int = now_int;
	}
	
	Led_obj.Refresh();
#ifdef BUZZER_ENABLED	
	Buzzer_obj.Refresh();
#endif

	//~ // print debug info every 0.5 second
	//~ static unsigned long Debug_time_lng = 0L;
	//~ if ( millis() > Debug_time_lng + 500L) {
		//~ Debug_time_lng = millis();
		
		//~ Serial.println(DebugValue_int);
	//~ }
}
