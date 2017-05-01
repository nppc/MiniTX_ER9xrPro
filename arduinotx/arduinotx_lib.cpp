/* arduinotx_lib.c - Library
** 15-06-2013
** 2013-08-14 getProgmemStrchr(), getProgmemStrpos()
** 2013-08-16 del getProgmemStrchr(), getProgmemStrpos() fixed

Copyright (C) 2014 Richard Goutorbe.  All right reserved.
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
Contact information: http://www.reseau.org/arduinorc/index.php?n=Main.Contact
*/

#include "arduinotx_lib.h"

/*
** printf() to Serial --------------------------------------------------------
**
** Before using these functions, you must call serialInit() in setup() to redirect stdout to Serial
*/

// Open Serial port and redirect stdout to Serial
// Return value: 0=Ok, >0=error
byte serialInit(long bauds_lng) {
	byte retval_byt = 0;
	Serial.begin(bauds_lng);
	FILE *stream_ptr =  fdevopen(serialWrite, NULL); // see aPrintf() in arduinotx_lib.cpp
	if (stream_ptr) {
		stdout = stream_ptr;
		stderr = stream_ptr;
		//aPrintfln(PSTR("serialInit(%ld)"), bauds_lng);
	}
	else {
		Serial.println("fdevopen failed");
		retval_byt = 232;
	}
	return retval_byt;
}

// Send given variable list of args over the serial link using a printf-like format string and append \r\n
void aprintfln(const char *fmt_str, ... ) {
	va_list args;
	va_start (args, fmt_str );
	vprintf(fmt_str, args);
	va_end (args);
	puts("\r");
}

// Send given variable list of args over the serial link using a PSTR printf-like format string and append \r\n
void aPrintfln(const char *fmt_pstr, ... ) {
	va_list args;
	va_start (args, fmt_pstr );
	vfprintf_P(stdout, fmt_pstr, args);
	va_end (args);
	puts("\r");
}


// Send given variable list of args over the serial link using a printf-like format string
void aprintf(const char *fmt_str, ... ) {
	va_list args;
	va_start (args, fmt_str );
	vprintf(fmt_str, args);
	va_end (args);
}

// Send given variable list of args over the serial link using a PSTR printf-like format string
void aPrintf(const char *fmt_str, ... ) {
	va_list args;
	va_start (args, fmt_str );
	vfprintf_P(stdout, fmt_str, args);
	va_end (args);
}

int serialWrite(char c, FILE *f) {
    Serial.write(c);
    return 0;
}

/*
** PROGMEM-related --------------------------------------------------------
*/

// Return the position of a character in a PROGMEM string, or -1 if not found
// Warning: the string must be declared as an array of chars, NOT as a pointer to a char
// example: const char pgm_str[] PROGMEM = "CDEIT0123456789";
int getProgmemStrpos(PGM_P pgm_str, const char c_chr) {
	int retval_int = -1;
	for (int idx_int = 0; idx_int < 255; idx_int++) {
		char s_chr = pgm_read_byte(pgm_str + idx_int);
		if (s_chr) {
			if (s_chr == c_chr) {
				retval_int = idx_int;
				break;
			}
		}
		else
			break; // not found
	}
	return retval_int;
}

// Copy an item of a PROGMEM string array into a RAM buffer
// out_buffer_str : the target buffer, user-allocated
// array_str : the PROGMEM string array
// idx_int : index of the item of array_str to copy into out_buffer_str
// buffersize_int : size of out_buffer_str, at most buffersize_int-1 chars will be copied and a \0 will be appended
char *getProgmemStrArrayValue(char *out_buffer_str, PGM_P *array_str, int idx_int, size_t buffersize_int) {
	char *retval_str = out_buffer_str;
	PGM_P item_ptr = (PGM_P)pgm_read_word(array_str + idx_int);
	if (item_ptr) {
		strncpy_P(out_buffer_str, item_ptr, buffersize_int);
		out_buffer_str[buffersize_int-1] = '\0';
	}
	else {
		*out_buffer_str = '\0';
		retval_str = NULL;
	}
	return retval_str;
}

// Return the index of given item in the given PROGMEM string array or -1 if item not found
// array_str : the PROGMEM string array
// item_str : the string value to search for in array_str
// nitems_int : optional, number of items in array_str;
// if nitems_int is not specified then the last item of the array must be a NULL pointer else this function will probably crash your program
int findProgmemStrArrayIndex(PGM_P *array_str, const char *value_str, int nitems_int) {
	int retval_int = -1;
	int idx_int = 0;
	PGM_P item_ptr = NULL;
	do {
		item_ptr = (PGM_P)pgm_read_word(array_str + idx_int);
		if (item_ptr) {
			if (strcmp_P(value_str, item_ptr) == 0)
				retval_int = idx_int;
			else
				idx_int++;
		}
	} while (idx_int < nitems_int && item_ptr && retval_int == -1);
	return retval_int;
}

// print a PROGMEM string array
// array_str : the PROGMEM string array
// nitems_int : optional, maximum number of items to print;
// if nitems_int is not specified then the last item of the array must be a NULL pointer else this function will probably crash your program
void printProgmemStrArray(PGM_P *array_str, int nitems_int) {
	char line_str[10];
	int idx_int = 0;
	char *value_str = NULL;
	do {
		value_str = getProgmemStrArrayValue(line_str, array_str, idx_int, 10);
		if (value_str) {
			aprintf("#%d: \"%s\"\n", idx_int, value_str);
			idx_int++;
		}
	} while (value_str && idx_int < nitems_int);
}

/*
** Strings --------------------------------------------------------
*/
	
// Test if given null-terminated string is empty or contains only white-space characters
// return value: 1=string is empty or blank, 0=string contains non-blank characters
byte Isblank(const char *line_str) {
	byte retval_byt = 1;
	const char *c_ptr = line_str;
	while (*c_ptr && retval_byt) {
		if (isspace(*c_ptr))
			c_ptr++;
		else
			retval_byt = 0;
	}
	return retval_byt;
}

char *Trimwhitespace(char *out_line_str) {
	char *retval_str = out_line_str;
	while (isspace(*retval_str))
		retval_str++;
	if (*retval_str) {
		char *last_chr = retval_str + strlen(retval_str) - 1;
		while (last_chr > retval_str && isspace(*last_chr))
			last_chr--;
		*(last_chr+1) = 0;
	}
	return retval_str;
}


// Format given time in seconds in given user-allocated buffer (11 chars)
// Returns hhhh:mm:ss
char *TimeString(unsigned long seconds_lng, char *out_buffer_str) {
	unsigned long m = seconds_lng / 60;
	unsigned long h = seconds_lng / 3600;
	m = m - (h * 60);
	sprintf(out_buffer_str, "%02ld:%02ld:%02ld", h, m, seconds_lng - (m * 60) - (h * 3600));
	return out_buffer_str;
}

byte ishexdigit(char a_chr) {
	return (a_chr >= '0' && a_chr <= '9') || (a_chr >= 'A' && a_chr <= 'F') || (a_chr >= 'a' && a_chr <= 'f');;
}

// convert hex_str to long int
// stops conversion when a non-hex char is found
unsigned long hex2dec(char *hex_str, byte length_byt) {
	unsigned long retval_lng = 0;
	for (byte idx_byt=0; idx_byt<length_byt; idx_byt++) {
		char x_chr = toupper(hex_str[idx_byt]);
		if (ishexdigit(x_chr))
			retval_lng += (x_chr - ( x_chr <= '9' ? '0':'7')) * (1<<(4*(length_byt-1-idx_byt)));
		else
			break;
	}
	return retval_lng;
}
