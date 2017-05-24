/* arduinotx_command.h
** 16-09-2012
** 15-12-2012 parse_switch()
** 24-02-2013 validate_value(), parse_last_digit()
** 30-04-2013 class ArduinotxCmd
** 02-05-2013 is_blank()
** 05-06-2013 VARTEST[]
** 09-06-2013 AllCommands_str[], AllVarNames_str, AllVarTests_byt[]
** 15-06-2013 ModelVar*
*/

/* Copyright (C) 2014 Richard Goutorbe.  All right reserved.
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
Contact information: http://www.reseau.org/arduinorc/index.php?n=Main.Contact
*/

#ifndef arduinotx_command_h
#define arduinotx_command_h
#include <Arduino.h>
#include "arduinotx_transmitter.h"

#define CMDLINESIZE 32

class ArduinotxCmd {
	private:
		typedef enum CmdTokens {
			CMD_UNDEFINED,
			CMD_CHECK,
			CMD_INIT,
			CMD_ECHO,
			CMD_MODEL,
			CMD_DUMP,
			CMD_PRINT,
			CMD_QMARK // "?" synonym for "PRINT"
		} CmdToken;
		
		byte Echo_byt; // b2=echo command prompt, b1=echo replies, b0=echo input characters

		//PGM_P const AllCommands_str[] PROGMEM; // Names of all commands
		//PGM_P AllVarNames_str[] PROGMEM; // Names of all variables that could be tested by validate_value()
		static const byte AllVarTests_byt[] PROGMEM; // Test number corresponding to variable name
		char Cmdline_str[CMDLINESIZE + 1];
		
		void serial_prompt();
		byte validate_value(const char *var_str, int value_int);
		CmdToken parse_command_line(const char *line_str, char *out_word1_str, char *out_separator_chr, char *out_word2_str);
		void process_command_line(char *line_str);
		int parse_potentiometer(char *word_str);
		int parse_switch(char *word_str);
		byte parse_last_digit(const char *radix_str, const char *word_str);
		void print_command_error(const char *text_str);
		void print_command_error_P(PGM_P text_str);
	
	public:
		void InitCommand();
		void EndCommand();
		void Input();
#if MODEL_SWITCH_BEHAVIOUR == MODEL_SWITCH_STEPPING
		void NextDataset();
#elif MODEL_SWITCH_BEHAVIOUR == MODEL_SWITCH_ROTATING
		void SelectDataset(byte dataset_int);
#endif
};
#endif
