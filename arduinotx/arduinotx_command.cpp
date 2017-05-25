/* arduinotx_command.ino - Command mode processing
** Serial Monitor: Autoscroll, Carriage return, 115200 baud
** 25-10-2012
** 30-10-2012 Serial 115200 Bauds
** 15-12-2012 parse_switch()
** 25-12-2012 print ppm
** 30-12-2012 CommitChanges_bool, print version
** 11-01-2013 replaced NCHANNELS by CHANNELS, external alloc for Serialized_str[], fixed buff overflow in serialEvent() and process_command_line()
** 13-01-2013 set back Serial 9600 Bauds because 115200 was causing issues when pasting multiple lines
** 24-02-2013 validate_value(), parse_last_digit()
** 05-03-2013 process_command_line() added CommitChanges_bool after CMD_MODEL
** 30-04-2013 CMD_MODEL echoes "MODEL=n", command line serial input must be '\n' terminated (instead of '\r'); 
** new command ECHO UPLOAD for txupload utility ; \n or \r are accepted as EOL char in command input (but not both)
** 03-05-2013 Input() ignores comments
** 15-05-2013 removed ECHO OFF from DUMP output, allow 0 input for N1M,N2M in validate_value()
** 05-06-2013 validate_value() arrays in PROGMEM
** 09-06-2013 aPrintf(), string consts in PROGMEM
** 15-06-2013 ModelVar*, validate_value(), DUMP MODEL
** 16-06-2013 InitCommand()
** 18-06-2013 Input() escape cancels user input
** 04-07-2013 comments start by '#' instead of ';'
** 18-08-2013 validate_value() case 9
** 25-08-2013 AllVarNames_str[], AllVarTests_byt[] Gvn_BAT
** 14-05-2014 NextDataset()
** 29-05-2014 new command PRINT VOLT
** 30-05-2014 lowered serial speed to 2400 bps to prevent Arduino's serial buffer overflow when uploading with txupload;
*/

/* Copyright (C) 2014 Richard Goutorbe.  All right reserved.
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
Contact information: http://www.reseau.org/arduinorc/index.php?n=Main.Contact
*/

#include "arduinotx_command.h"
#include "arduinotx_eeprom.h"
#include "arduinotx_lib.h"

#define CMDECHO_PROMPT  0x4
#define CMDECHO_REPLY  0x2
#define CMDECHO_INPUT  0x1

// test for EOL char in command input
#define isEOL(c) ((c) == '\n' || (c) == '\r')

/* 
** Variables allocated in arduinotx.ino
*/
// EEPROM manager
extern ArduinotxEeprom Eeprom_obj;
// Tx manager
extern ArduinoTx ArduinoTx_obj;
// These 2 global variables are used to request the PPM signal values from ISR(TIMER1_COMPA_vect)
extern volatile byte RequestPpmCopy_bool;
extern volatile unsigned int PpmCopy_int[]; // pulse widths (microseconds)


void ArduinotxCmd::InitCommand() {
	serialInit(2400);
	
	Echo_byt= CMDECHO_PROMPT | CMDECHO_REPLY | CMDECHO_INPUT;
	strcpy_P(Cmdline_str, PSTR("ECHO COMMAND MODE")); process_command_line(Cmdline_str);
	strcpy_P(Cmdline_str, PSTR("PRINT VERSION")); process_command_line(Cmdline_str);
	if (Eeprom_obj.CheckEEProm() > 0) {
		strcpy_P(Cmdline_str, PSTR("CHECK")); process_command_line(Cmdline_str);
	}
	else
		aPrintfln(PSTR("EEPROM error"));
#ifdef BATCHECK_ENABLED	
	strcpy_P(Cmdline_str, PSTR("PRINT VOLT")); process_command_line(Cmdline_str);
#endif
	serial_prompt();
}

void ArduinotxCmd::EndCommand() {
	Serial.end();
}

void ArduinotxCmd::serial_prompt() {
	aPrintf(PSTR("%d> "), Eeprom_obj.GetVar(0, "CDS"));
}

/* Acquire character(s) received from the serial link
** This method is called by SerialEvent()
*/
void ArduinotxCmd::Input() {
	static byte idx_byt = 0;
	const byte ESC = 27; // Escape key
	
	if (idx_byt == 0)
		memset(Cmdline_str, '\0', CMDLINESIZE + 1);

	while (Serial.available()) {
		// get the new byte
		byte read_byt = Serial.read();
		
		// echo input
		if (Echo_byt & CMDECHO_INPUT) {
			if (read_byt >= ' ')
				putchar(read_byt);
			else if (read_byt == ESC) {
				// escape: send a \r \n EOL sequence 
				puts("\r");
			}
			else if (! isEOL(read_byt))
				aPrintf(PSTR("0x%02x"), read_byt);
		}
		
		// parse input
		if (isEOL(read_byt)) {
			if (Echo_byt & CMDECHO_INPUT)
				puts("\r"); // send a \r \n EOL sequence
		
			if (idx_byt)  { // else ignore empty commands
				Cmdline_str[idx_byt] = '\0';
				char *comment_chr = strchr(Cmdline_str, COMMENT_TOKEN); // strip comments
				if (comment_chr)
					*comment_chr = '\0';
				if (! Isblank(Cmdline_str))
					process_command_line(Trimwhitespace(Cmdline_str));
				idx_byt = 0;
			}

			if (Echo_byt & CMDECHO_PROMPT)
				serial_prompt();
		}
		else if (read_byt == ESC) {
			// escape: cancel user input
			idx_byt = 0;
			if (Echo_byt & CMDECHO_PROMPT)
				serial_prompt();
		}
		else {
			// store this char in the Cmdline_str buffer
			if (idx_byt < CMDLINESIZE) // else ignore extra char until EOL
				Cmdline_str[idx_byt++] = toupper(read_byt);
		}
	}
}

// Increment the Current dataset number, simulating a "MODEL x" command line
// this method is called by ArduinoTx::get_selected_dataset() when MODEL_SWITCH_STEPPING has been selected
#if MODEL_SWITCH_BEHAVIOUR == MODEL_SWITCH_STEPPING
void ArduinotxCmd::NextDataset() {
	byte ds_byt = Eeprom_obj.GetVar(0, "CDS");
	if (ds_byt == NDATASETS)
		ds_byt = 0;
	sprintf(Cmdline_str, "MODEL %d", ++ds_byt);
	process_command_line(Cmdline_str);
}
// Change the Current dataset number, simulating a "MODEL x" command line
// dataset_int in the range [1, NDATASETS]
// this method is called by ArduinoTx::get_selected_dataset() when MODEL_SWITCH_ROTATING has been selected
#elif MODEL_SWITCH_BEHAVIOUR == MODEL_SWITCH_ROTATING
void ArduinotxCmd::SelectDataset(byte dataset_int) {
	if (dataset_int > 0 && dataset_int <= NDATASETS) {
		sprintf(Cmdline_str, "MODEL %d", dataset_int);
		process_command_line(Cmdline_str);
	}
}
#endif


const char Cmd_CHECK[] PROGMEM = "CHECK"; const char Cmd_INIT[] PROGMEM = "INIT"; 
const char Cmd_ECHO[] PROGMEM = "ECHO"; const char Cmd_MODEL[] PROGMEM = "MODEL"; 
const char Cmd_DUMP[] PROGMEM = "DUMP"; const char Cmd_PRINT[] PROGMEM = "PRINT"; 
const char Cmd_QUMARK[] PROGMEM = "?"; 
// Names of all commands in same order as enum CmdTokens
PGM_P const AllCommands_str[] PROGMEM = {
	Cmd_CHECK, Cmd_INIT, Cmd_ECHO, Cmd_MODEL, Cmd_DUMP, Cmd_PRINT, Cmd_QUMARK,
	NULL
};

// Parse the command line
// valid command line formats:
// word1
// word1 separator word2		where separator = ' ' or '='
// Return value: the token corresponding to out_word1_str, or CMD_UNDEFINED on error
ArduinotxCmd::CmdToken ArduinotxCmd::parse_command_line(const char *line_str, char *out_word1_str, char *out_separator_chr, char *out_word2_str) {
	CmdToken retval_int = CMD_UNDEFINED;
	*out_word1_str = '\0';
	*out_separator_chr = '\0';
	*out_word2_str = '\0';
	
	char *sep_chr = strchr(line_str, ' ');
	if (sep_chr == NULL) {
		sep_chr = strchr(line_str, '=');
		if (sep_chr == NULL) {
			strcpy(out_word1_str, line_str); // word1
		}
	}
	if (sep_chr != NULL) {
		*out_separator_chr = *sep_chr;
		*sep_chr = '\0';
		strcpy(out_word1_str, line_str);
		strcpy(out_word2_str, sep_chr+1); // word1 separator word2		where separator is ' ' or '='
		*sep_chr = *out_separator_chr;
	}
	
	// seek index of out_word1_str in AllCommands_str[]
	int idx_int = findProgmemStrArrayIndex(AllCommands_str, out_word1_str);
	if (idx_int >= 0)
		retval_int = (CmdToken)(idx_int + 1); // add +1 if found
	if (retval_int == CMD_QMARK)
		retval_int = CMD_PRINT;
	return retval_int;
}


// Names of all variables that could be tested by validate_value()
PGM_P const AllVarNames_str[] PROGMEM= {
	Gvn_TSC, Gvn_CDS, Gvn_ADS, Gvn_BAT, Gvn_THC, Gvn_PRT, Gvn_N1M, Gvn_P1M, Gvn_N2M, Gvn_P2M, Gvn_ICT,
	Gvn_ICN, Gvn_REV, Gvn_DUA, Gvn_EXP, Gvn_PWL, Gvn_PWH, Gvn_EPL, Gvn_EPH, Gvn_SUB,
	Gvn_KL1, Gvn_KL2, Gvn_KL3, Gvn_KL4, Gvn_KL5, Gvn_KL6, Gvn_KL7, Gvn_KL8, 
	Gvn_KH1, Gvn_KH2, Gvn_KH3, Gvn_KH4, Gvn_KH5, Gvn_KH6, Gvn_KH7, Gvn_KH8, 
	NULL
};

// Test number corresponding to variable name in AllVarNames_str[]
// AllVarTests_byt[] must be declared at class level because pgm_read_byte() will return wrong value if AllVarTests_byt[] is declared inside validate_value()
const byte ArduinotxCmd::AllVarTests_byt[] PROGMEM = {
	9,6,6,8,7,1,4,2,4,2,3,
	4,0,1,1,5,5,1,1,2,
	8,8,8,8,8,8,8,8,
	8,8,8,8,8,8,8,8
};
	
// Test if given numerical value is valid for given variable
// Return value: 0=valid, 1=out of range
byte ArduinotxCmd::validate_value(const char *var_str, int value_int) {
	byte retval_byt = 1;
	char var3_str[4]; var3_str[0] = var_str[0]; var3_str[1] = var_str[1]; var3_str[2] = var_str[2]; var3_str[3] = '\0';
	// seek index of var3_str in AllVarNames_str[]
	int idx_int = findProgmemStrArrayIndex(AllVarNames_str, var3_str);
	if (idx_int >= 0) {
		switch (pgm_read_byte(&AllVarTests_byt[idx_int])) {
			case 0:
				// REV : Reverse, 1=servo direction is reversed for this channel, 0=servo direction is not reversed.
				if (value_int == 0 || value_int == 1)
					retval_byt = 0;
				break;
			case 1:
				// DUA : Dual rate applied to this channel when the Dual Rate switch is ON. Percentage [0-100].
				// EXP : Exponential percentage applied to the input value, [0-100] 
				// EPL, EPH : End point percentage [0-100], EPL=low end point, EPH=high end point, both default=100.
				if (value_int >= 0 && value_int <= 100)
					retval_byt = 0;
				break;
			case 2:
				// SUB : Subtrim percentage [-100, +100], default=0.
				// P1M, P2M: mixer 1 & 2's percent mix [-100, +100]
				if (value_int >= -100 && value_int <= 100)
					retval_byt = 0;
				break;
			case 3:
				// ICT : Input control type: 1=potentiometer, 2=switch, 3=mixer, 0=off (do not read this control)
				if (value_int >= 0 && value_int <= 3)
					retval_byt = 0;
				break;
			case 4:
				// ICN : Input control number: potentiometer number or switch number
				// potentiometer number belongs to the interval [1,8]
				// switch number belongs to the interval [1,6]
				// N1M, N2M : mixer 1 & 2's input1 potentiometer number [1,8] or 0=none
				if (value_int >= 0 && value_int <= 8)
					retval_byt = 0;
				break;
			case 5:
				// PWL, PWH : Minimal/Maximal pulse width for this channel, in microseconds
				if (value_int >= (int)ArduinoTx::PPM_LOW && value_int <= 10 * (int)ArduinoTx::PPM_LOW)
					retval_byt = 0;
				break;
			case 6:
				// CDS : current dataset ; this is the model selected by the MODEL command
				// ADS : alternate dataset ; this is the model selected when the MODEL_SWITCH is closed
				if (value_int >= 1 && value_int <= NDATASETS)
					retval_byt = 0;
				break;
			case 7:
				// THC : throttle channel (controlled by the throttle cut switch) ; default = 3, no throttle channel = 0
				if (value_int >= 0 && value_int <= CHANNELS)
					retval_byt = 0;
				break;
			case 8:
				//BAT : minimum battery voltage required for Tx operation, ALARM_BATTERY is triggered when voltage gets lower. 
				//KL1-8 : lowest analog value read on potentiometer #1, when the stick (and the hardware trim, if any) is at its lowest position. KL1...KL8 correspond to potentiometers 1 to 9. Value in the range [0-1023]
				//KH1-8 : highest analog value read on potentiometer #1, when the stick (and the hardware trim, if any) is at its highest position. KH1...KH8 correspond to potentiometers 1 to 9. Value in the range [0-1023]
				if (value_int >= 0 && value_int <= 1023)
					retval_byt = 0;
				break;
			case 9:
				// TSC : throttle cutoff value used by the throttle security check, [0-511]
				if (value_int >= 0 && value_int <= 511)
					retval_byt = 0;
				break;
		}
	}
	return retval_byt;
}
	
// Process command
void ArduinotxCmd::process_command_line(char *line_str) {
	int value_int = 0;
	// parse the line
	char word1_str[CMDLINESIZE + 1];
	char separator_chr;
	char word2_str[CMDLINESIZE + 1];
	CmdToken token_int = parse_command_line(line_str, word1_str, &separator_chr, word2_str);
	
	//~ // Debug
	//~ aPrintfln(PSTR("Line \"%s\""), line_str);
	//~ aPrintfln(PSTR("    word1_str = \"%s\""), word1_str);
	//~ aPrintfln(PSTR("    separator_chr = \"%c\""), separator_chr);
	//~ aPrintfln(PSTR("    word2_str = \"%s\""), word2_str);
	
	byte valid_bool = false;
	
	// execute the command
	switch (token_int) {
		case CMD_CHECK:
			value_int = Eeprom_obj.CheckEEProm();
			if (value_int > 0)
				aPrintfln(PSTR("EEPROM ok, %d bytes"), value_int);
			else
				print_command_error_P(PSTR("EEPROM"));
		break;
			
		case CMD_INIT:
			Eeprom_obj.InitEEProm();
			value_int = Eeprom_obj.CheckEEProm();
			if (value_int > 0)
				aPrintfln(PSTR("EEPROM initialized, %d bytes"), value_int);
			else
				print_command_error_P(PSTR("EEPROM initialization"));
		break;

		case CMD_ECHO:
			if (strcmp(word2_str, "OFF") == 0) 
				Echo_byt = 0;
			else if (strcmp(word2_str, "ON") == 0)
				Echo_byt = CMDECHO_PROMPT | CMDECHO_REPLY | CMDECHO_INPUT;
			else if (strcmp(word2_str, "UPLOAD") == 0) 
				Echo_byt = CMDECHO_PROMPT | CMDECHO_REPLY;
			if (Echo_byt & CMDECHO_REPLY)
				aprintfln(word2_str);
		break;
			
		case CMD_MODEL: {
			// persist model number in the global vars dataset 0
			// new value will be echoed in the prompt
			value_int = atoi(word2_str);
			if (Eeprom_obj.SetVar(0, "CDS", value_int) == 0) {
				if (Echo_byt & CMDECHO_REPLY)
					aPrintfln(PSTR("MODEL=%d"), value_int);
				ArduinoTx_obj.CommitChanges(); 
			}
			else
				print_command_error(word2_str);
		}
		break;
		
		// dump GLOBAL	will dump global variables
		// dump			will dump all model vars, mixer vars and channels of current model
		// dump MODEL	will dump all model vars of current model
		// dump MIXERS	will dump all mixer vars of current model
		// dump channel	will dump the specified channel of current model
		case CMD_DUMP: { 
			byte current_dataset_byt = Eeprom_obj.GetVar(0, "CDS");
			byte dataset_byt = current_dataset_byt;
			byte channel_byt = 0;
			if (*word2_str == '\0')
				valid_bool = true; // dump all mixers and all channels of current model
			else {
				if (strcmp(word2_str, "GLOBAL") == 0) {
					dataset_byt = 0;
					valid_bool = true;
				}
				else if (strcmp(word2_str, "MODEL") == 0) {
					channel_byt = CHANNELS+1;
					valid_bool = true;
				}
				else if (strcmp(word2_str, "MIXERS") == 0) {
					channel_byt = CHANNELS+2;
					valid_bool = true;
				}
				else {
					// dump the specified channel of current model
					channel_byt = atoi(word2_str);
					valid_bool = (channel_byt > 0 && channel_byt <= CHANNELS);
				}
			}
			if (valid_bool) {
				if (dataset_byt > 0)
					aPrintfln(PSTR("MODEL %d"), current_dataset_byt);
				Eeprom_obj.Serialize(dataset_byt, channel_byt);
			}
			else 
				print_command_error(word2_str);
		}
		break;

		case CMD_PRINT: { // print varname|pot#|sw#|ppm|ver
			byte current_dataset_byt = Eeprom_obj.GetVar(0, "CDS");
			byte dataset_byt = 255; 
			if (Eeprom_obj.IsVar(current_dataset_byt, word2_str)) {
				dataset_byt = current_dataset_byt; // model, mixer or channel dataset
			}
			else if (Eeprom_obj.IsVar(0, word2_str))
				dataset_byt = 0; // global dataset
			byte printed_bool = false;
			if (dataset_byt < 255) {
				char type_char = Eeprom_obj.GetType(dataset_byt, word2_str);
				if (type_char == 'a') {
					Eeprom_obj.GetVar(dataset_byt, word2_str, line_str);
					aPrintfln(PSTR("%s=%s"), word2_str, line_str);
					printed_bool = true;
				}
				else
					value_int = Eeprom_obj.GetVar(dataset_byt, word2_str);
				valid_bool = true;
			}
			else if (strcmp(word2_str, "PPM") == 0) {
				RequestPpmCopy_bool = true; // query PPM values from ISR(TIMER1_COMPA_vect)
				while (RequestPpmCopy_bool) ; // wait until ISR() updates PpmCopy_int[]
				for (byte chan_byt = 0; chan_byt < CHANNELS; chan_byt++)
					aPrintfln(PSTR("CH%d=%d"), chan_byt+1, PpmCopy_int[chan_byt]);
				printed_bool = true;
			}
			else if (strcmp(word2_str, "VERSION") == 0) {
				aPrintfln(PSTR("VERSION=%S"), SOFTWARE_VERSION);
				printed_bool = true;
			}
#ifdef BATCHECK_ENABLED			
			else if (strcmp(word2_str, "VOLT") == 0) {
				float volt_flt = ArduinoTx_obj.ReadBattery() / 102.3; // 102.3 = 1V
				aPrintfln(PSTR("VOLT=%d.%d"), int(volt_flt), int(10 * (volt_flt - int(volt_flt))));
				printed_bool = true;
			}
#endif
			//~ else  if (strcmp(word2_str, "DEBUG") == 0) {
				//~ for (int idx_int = 0; idx_int<GLOBAL_VARS; idx_int++) {
					//~ aPrintfln(PSTR("idx %d type=%c"), idx_int, getProgmemByteArrayValue(GlobalVarTypeP_byt, idx_int));
				//~ }
				//~ printed_bool = true;
			//~ }
			else {
				byte number_byt = parse_last_digit("CAL", word2_str); // calibrated input value for given channel
				if (number_byt > 0) {
					value_int = ArduinoTx_obj.ReadControl(number_byt-1); 	
					valid_bool = true;
				}
				else {
					number_byt = parse_last_digit("POT", word2_str); // raw value for given potentiometer
					if (number_byt > 0) {
						value_int = analogRead(get_pot_pin(number_byt)); 	
						valid_bool = true;
					}
					else {
						number_byt = parse_last_digit("SW", word2_str); // raw value for given switch
						if (number_byt > 0) {
							value_int = digitalRead(get_switch_pin(number_byt)); 	
							valid_bool = true;
						}
					}
				}
			}
			
			if (! printed_bool) {
				if (valid_bool)
					aPrintfln(PSTR("%s=%d"), word2_str, value_int);
				else
					print_command_error(word2_str);  // invalid variable
			}
		}
		break;

		default: {
			// varname=value
			if (separator_chr == '=') { 
				byte current_dataset_byt = Eeprom_obj.GetVar(0, "CDS"); 
				byte dataset_byt = 255; 
				if (Eeprom_obj.IsVar(current_dataset_byt, word1_str))
					dataset_byt = current_dataset_byt; // model, mixer or channel dataset
				else	if (Eeprom_obj.IsVar(0, word1_str))
					dataset_byt = 0; // global dataset
				
				if (dataset_byt < 255) {
					// test var type
					char var_type = Eeprom_obj.GetType(dataset_byt, word1_str);
					if (var_type == 'a') {
						// set string value
						if  (Eeprom_obj.SetVar(dataset_byt, word1_str, 0, word2_str) == 0) {
							if (Echo_byt & CMDECHO_REPLY) {
								Eeprom_obj.GetVar(dataset_byt, word1_str, word2_str);
								aPrintfln(PSTR("%s=\"%s\""), word1_str, word2_str);
							}
							ArduinoTx_obj.CommitChanges();
						}
						else
							print_command_error(word1_str); // SetVar() failed
					}
					else {
						// set numerical value
						value_int = atoi(word2_str);
						if (validate_value(word1_str, value_int) == 0) {
							if  (Eeprom_obj.SetVar(dataset_byt, word1_str, value_int) == 0) {
								if (Echo_byt & CMDECHO_REPLY)
									aPrintfln(PSTR("%s=%d"), word1_str, Eeprom_obj.GetVar(dataset_byt, word1_str));
								ArduinoTx_obj.CommitChanges(); 
							}
							else
								print_command_error(word1_str); // SetVar() failed
						}
						else
							print_command_error(word2_str); // value out of range
					}
				}
				else
					print_command_error(word1_str); // invalid variable
			}
			else if (strlen(line_str) != 0) // else ignore empty commands
				print_command_error(line_str);
		}
		break;
	}
}

// Parse number in last char of given word
// radix_str: the reference string
// word_str: the string match against the radix
// Return value: if word matches "radix[1-9]" then returns the number else returns 0
byte ArduinotxCmd::parse_last_digit(const char *radix_str, const char *word_str) {
	byte retval_byt = 0;
	byte length_byt = strlen(radix_str);
	if (strncmp(word_str, radix_str, length_byt) == 0) {
		char number_char = word_str[length_byt];
		if (number_char >= '1' && number_char <= '9')
			retval_byt = number_char - '0';
	}
	return retval_byt;
}

// print error message
void ArduinotxCmd::print_command_error(const char *text_str) {
	aPrintfln(PSTR("%s: error"), text_str);
}

// print error message from PROGMEM
void ArduinotxCmd::print_command_error_P(PGM_P text_str) {
	aPrintfln(PSTR("%S: error"), text_str);
}
