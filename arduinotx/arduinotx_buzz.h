/* arduinotx_buzz.h - Buzzer manager
** 05-03-2013
** 14-08-2013 renaming
** 15-08-2013 PlayCount_byt
** 27-08-2013 HalfPeriod_int
*/

/* Copyright (C) 2014 Richard Goutorbe.  All right reserved.
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
Contact information: http://www.reseau.org/arduinorc/index.php?n=Main.Contact
*/

#ifndef arduinotx_buzz_h
#define arduinotx_buzz_h
#include <Arduino.h>

#define BUZZER_REPEAT 255

class ArduinotxBuzz {
	private:
		char Current_char; // current character being played
		byte OutPin_byt;
		byte Pulses_byt[11]; // duration of each pulse, in multiples of 10 milliseconds; 1st pulse is HIGH, sequence is terminated by 0
		byte PlayCount_byt; // Current_char will be played Count_byt times, 0=silent, BUZZER_REPEAT=infinite (will play until Stop() is called)
		unsigned int HalfPeriod_int; // Period of the sound wave / 2, in microseconds
	
	public:
		ArduinotxBuzz(byte pin_byt);
		byte SetCode(const char c_char, byte count_byt = 1, unsigned int frequency_int = 0);
		void Play(byte count_byt = 1);
		void Stop();
		void Refresh(byte reset_byt = 0);
};
#endif
