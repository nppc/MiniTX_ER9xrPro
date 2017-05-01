/* arduinotx_buzz.ino - Buzzer manager
** 14-08-2013 created from arduinotx_led
** 15-08-2013 PlayCount_byt
** 16-08-2013 fixed dcl BuzzerCharset_str[]
*/

/* Copyright (C) 2014 Richard Goutorbe.  All right reserved.
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
Contact information: http://www.reseau.org/arduinorc/index.php?n=Main.Contact
*/

#include "arduinotx_buzz.h"
#include "arduinotx_lib.h"


// Characters that could be flashed on the led or played on the buzzer
const char BuzzerCharset_str[] PROGMEM = "BCPT0123456789";

// Morse codes corresponding to BuzzerCharset_str[], in the same order
// |A.-|B-...|C-.-.|D-..|E.|F..-.|G--.|H....|I..|J.---|K-.-|L.-..|M--|N-.|O---|P.--.|Q--.-|R.-.|S...|T-|U..-|V...-|W.--|X-..-|Y-.--|Z--..|0-----|1.----|2..---|3...--|4....-|5.....|6-....|7--...|8---..|9----.|..-.-.-|?..--..|!-.-.--|,--..--|@-.--.-.|
const char BuzzerMorse_B[] PROGMEM = "-..."; const char BuzzerMorse_C[] PROGMEM = "-.-."; const char BuzzerMorse_P[] PROGMEM = ".--."; const char BuzzerMorse_T[] PROGMEM = "-"; 
const char BuzzerMorse_0[] PROGMEM = "-----"; const char BuzzerMorse_1[] PROGMEM = ".----"; const char BuzzerMorse_2[] PROGMEM = "..---"; const char BuzzerMorse_3[] PROGMEM = "...--"; const char BuzzerMorse_4[] PROGMEM = "....-";
const char BuzzerMorse_5[] PROGMEM = "....."; const char BuzzerMorse_6[] PROGMEM = "-...."; const char BuzzerMorse_7[] PROGMEM = "--..."; const char BuzzerMorse_8[] PROGMEM = "---.."; const char BuzzerMorse_9[] PROGMEM = "----.";
PGM_P BuzzerMorseCodes_str[] PROGMEM = {
	BuzzerMorse_B, BuzzerMorse_C, BuzzerMorse_P, BuzzerMorse_T,
	BuzzerMorse_0, BuzzerMorse_1, BuzzerMorse_2, BuzzerMorse_3, BuzzerMorse_4, BuzzerMorse_5, BuzzerMorse_6, BuzzerMorse_7, BuzzerMorse_8, BuzzerMorse_9,
	NULL
};

/*
** Public -----------------------------------------------------------------
*/

ArduinotxBuzz::ArduinotxBuzz(byte pin_byt) {
	HalfPeriod_int = 250; // default frequency = 500 Hz
	Current_char = '\0';
	OutPin_byt = pin_byt;
	for (byte idx_byt = 0; idx_byt < 11; idx_byt++)
		Pulses_byt[idx_byt] = 10;
	pinMode(pin_byt, OUTPUT);
	PlayCount_byt = 0;
}

// Set the character to output
// retrieve the morse sequence corresponding to given character
// and store it in the Pulses_byt[] array
// count_byt : optional, c_char will be played count_byt times (default=1), BUZZER_REPEAT=infinite (will play until Stop() is called)
// frequency_int : optional, freq of the sound wave in Hz, 0=use current frequency  (default=500 Hz), [15,5000] use given frequency, out of range: ignored
// returns 0=ok, 1=char not found
byte ArduinotxBuzz::SetCode(const char c_char, byte count_byt, unsigned int frequency_int) {
	byte retval_byt = 0;
	const int morse_int = 10; // increase this number to slow down
	if (c_char != Current_char) {
		int idx_int = getProgmemStrpos(BuzzerCharset_str, c_char);
		if (idx_int >= 0) {
			// found
			char morse_str[7];
			getProgmemStrArrayValue(morse_str, BuzzerMorseCodes_str, idx_int, 7);
			// convert morse to delays
			byte idx_byt = 0;
			char *pos_ptr=morse_str;
			while (*pos_ptr) {
				Pulses_byt[idx_byt++] = *pos_ptr == '.' ? morse_int:3*morse_int; // dot=100ms, dash=300ms
				Pulses_byt[idx_byt++] = morse_int; // inter-element gap between the dots and dashes within a character=100 ms
				++pos_ptr;
			}
			Pulses_byt[idx_byt - 1] = 5*morse_int; // gap between letters=500ms
			Pulses_byt[idx_byt] = 0;
			Current_char = c_char;
			if (frequency_int > 15 && frequency_int <= 5000) // else use current frequency
				HalfPeriod_int = 500000 / frequency_int;
			Refresh(1); // reset
			Play(count_byt);
		}
		else
			retval_byt = 1;
	}
	return retval_byt;
}

// Play contents of Pulses_byt[]
// count_byt : optional, Current_char will be played count_byt times (default=1), BUZZER_REPEAT=infinite (will play until Stop() is called)
void ArduinotxBuzz::Play(byte count_byt) {
	PlayCount_byt = count_byt;
}

// Stay silent until Enable() is called
void ArduinotxBuzz::Stop() {
	PlayCount_byt = 0;
	Refresh(1);
}

// Refresh buzzer, public call: no arg, call this method every few milliseconds to refresh the buzzer according to current OutPin_byt[] timings
// Refresh buzzer, private call: arg reset_byt 1=reset internal state, for private calls only
void ArduinotxBuzz::Refresh(byte reset_byt) {
	static byte Pulse_idx_byt = 0xff;
	static byte Pulse_byt = 0; 
	static unsigned long Begin_int = 0;
	static unsigned long Period_int = 0;
	static byte Buzz_state_byt = 0;  // 1=playing audible pulse, 0=playing silent pulse
	static byte Wave_state_byt = 0;
	
	if (reset_byt) {
		Pulse_idx_byt = 0xff;
		Pulse_byt = 0;
		Begin_int = 0;
		Period_int = 0;
		Buzz_state_byt = 0;
		Wave_state_byt = 0;
		digitalWrite(OutPin_byt, 0); // silent
	}
	else if (PlayCount_byt) {
			
		unsigned long now_int = micros();
		if (now_int >= Begin_int + (10000L * Pulse_byt) || now_int < Begin_int) {
			++Pulse_idx_byt;
			Pulse_byt = *(Pulses_byt + Pulse_idx_byt); 
			Begin_int = now_int;
			Buzz_state_byt = (Pulse_idx_byt % 2 == 0); // buzz on even intervals
			Wave_state_byt = 0; // set sound wave LOW
			digitalWrite(OutPin_byt, 0); // silent
			//~ aprintf("\npulse %d=%d: ", Pulse_idx_byt, Pulse_byt);
		}
		
		if (Pulse_byt) {
			if (Buzz_state_byt) {
				if (now_int > Period_int + HalfPeriod_int || now_int < Period_int ) {
					// toggle sound wave
					Wave_state_byt = !Wave_state_byt;
					Period_int = now_int;
					digitalWrite(OutPin_byt, Wave_state_byt);
					//~ aprintf(Wave_state_byt ? "+":"-");
				}
			}
		}
		else {
			Pulse_idx_byt = 0xff; // restart from 1st pulse
			if (PlayCount_byt != BUZZER_REPEAT) {
				--PlayCount_byt;
				//~ aprintfln("");
			}
		}
	}
	else if (Wave_state_byt) {
		Wave_state_byt = 0;
		digitalWrite(OutPin_byt, 0); // silent
	}
}


