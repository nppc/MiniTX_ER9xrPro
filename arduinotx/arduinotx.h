/* arduinotx.h - Open source RC transmitter software for the Arduino 
** 05-11-2012
** 02-03-2013 deleted TEST_MODE
*/

/* Copyright (C) 2014 Richard Goutorbe.  All right reserved.
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
Contact information: http://www.reseau.org/arduinorc/index.php?n=Main.Contact
*/

#ifndef arduinotx_h
#define arduinotx_h

enum RunMode {
	RUNMODE_INIT,
	RUNMODE_TRANSMISSION,	// send PPM to receiver
	RUNMODE_COMMAND		// change configuration settings through Serial link
};

byte load_settings();
RunMode update_RunMode();
byte check_throttle();
unsigned int read_control(byte chan_byt);
unsigned int compute_channel_pulse(byte chan_byt, unsigned int ana_value_int, byte enable_calibration_bool);
void send_ppm();
char get_led_code(RunMode runmode_int);
void set_led_code(const char c_char);
#endif