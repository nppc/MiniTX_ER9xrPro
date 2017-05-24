/* arduinotx_led.ino - Led manager
** 05-03-2013
** 05-06-2013  SetCode() array in PROGMEM
** 14-08-2013 renaming, SetCode() uses getProgmemStrpos()
** 16-08-2013 fixed dcl LedCharset_str[]
*/

/* Copyright (C) 2014 Richard Goutorbe.  All right reserved.
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
Contact information: http://www.reseau.org/arduinorc/index.php?n=Main.Contact
*/

#include "arduinotx_led.h"
#include "arduinotx_lib.h"

// Characters that could be flashed on the led or played on the buzzer
const char LedCharset_str[] PROGMEM = "BCPT0123456789";

// Morse codes corresponding to LedCharset_str[], in the same order
// |A.-|B-...|C-.-.|D-..|E.|F..-.|G--.|H....|I..|J.---|K-.-|L.-..|M--|N-.|O---|P.--.|Q--.-|R.-.|S...|T-|U..-|V...-|W.--|X-..-|Y-.--|Z--..|0-----|1.----|2..---|3...--|4....-|5.....|6-....|7--...|8---..|9----.|..-.-.-|?..--..|!-.-.--|,--..--|@-.--.-.|
const char LedMorse_B[] PROGMEM = "-..."; const char LedMorse_C[] PROGMEM = "-.-."; const char LedMorse_P[] PROGMEM = ".--."; const char LedMorse_T[] PROGMEM = "-"; 
const char LedMorse_0[] PROGMEM = "-----"; const char LedMorse_1[] PROGMEM = ".----"; const char LedMorse_2[] PROGMEM = "..---"; const char LedMorse_3[] PROGMEM = "...--"; const char LedMorse_4[] PROGMEM = "....-";
const char LedMorse_5[] PROGMEM = "....."; const char LedMorse_6[] PROGMEM = "-...."; const char LedMorse_7[] PROGMEM = "--..."; const char LedMorse_8[] PROGMEM = "---.."; const char LedMorse_9[] PROGMEM = "----.";
PGM_P const LedMorseCodes_str[] PROGMEM = {
	LedMorse_B, LedMorse_C, LedMorse_P, LedMorse_T,
	LedMorse_0, LedMorse_1, LedMorse_2, LedMorse_3, LedMorse_4, LedMorse_5, LedMorse_6, LedMorse_7, LedMorse_8, LedMorse_9,
	NULL
};

/*
** Public -----------------------------------------------------------------
*/

ArduinotxLed::ArduinotxLed(byte pin_byt) {
	Current_char = '\0';
	OutPin_byt = pin_byt;
	for (byte idx_byt = 0; idx_byt < 11; idx_byt++)
		Pulses_byt[idx_byt] = 10;
	pinMode(pin_byt, OUTPUT);
}

// Set the character to output
// retrieve the morse sequence corresponding to given character
// and store it in the Pulses_byt[] array
// returns 0=ok, 1=char not found
byte ArduinotxLed::SetCode(const char c_char) {
	byte retval_byt = 0;
	if (c_char != Current_char) {
		int idx_int = getProgmemStrpos(LedCharset_str, c_char);
		if (idx_int >= 0) {
			// found
			char morse_str[7];
			getProgmemStrArrayValue(morse_str, LedMorseCodes_str, idx_int, 7);
			//~ aPrintfln(PSTR("%c 0x%02x idx=%d morse=%s"), c_char, c_char, idx_int, morse_str);
			// convert morse to delays
			byte idx_byt = 0;
			char *pos_ptr=morse_str;
			while (*pos_ptr) {
				Pulses_byt[idx_byt++] = *pos_ptr == '.' ? 20:60; // dot=200ms, dash=600ms
				Pulses_byt[idx_byt++] = 20; // inter-element gap between the dots and dashes within a character=200 ms
				++pos_ptr;
			}
			Pulses_byt[idx_byt - 1] = 100; // gap between letters=1000ms
			Pulses_byt[idx_byt] = 0;
			Current_char = c_char;
			Refresh(1); // reset
		}
		else {
			//~ aPrintfln(PSTR("SetCode(%c) failed"), c_char);
			retval_byt = 1;
		}
	}
	return retval_byt;
}

//~ // Refresh led, public call: no arg, call this method every few milliseconds to refresh the Led according to current OutPin_byt[] timings
//~ // Refresh led, private call: arg reset_byt 1=reset internal state, for private calls only
void ArduinotxLed::Refresh(byte reset_byt) {
	static byte Pulse_idx_byt = 0;
	static unsigned long Begin_int = 0;
	
	if (reset_byt) {
		Pulse_idx_byt = 0;
		Begin_int = millis();
		digitalWrite(OutPin_byt, *Pulses_byt ? HIGH:LOW);
	}
			
	byte pulse_byt = *(Pulses_byt + Pulse_idx_byt);
	if (pulse_byt) {
		unsigned long now_int = millis();
		if (now_int >= Begin_int + (10L * pulse_byt) || now_int < Begin_int) {
			++Pulse_idx_byt;
			Begin_int = now_int;
			digitalWrite(OutPin_byt, Pulse_idx_byt % 2 == 0 ? HIGH:LOW);
		}
	}
	else
		Pulse_idx_byt = 0; // restart from 1st pulse
}
