/* arduinotx_lib.h - Library
** 2013-08-11 aPrintfln()
** 2013-08-14 getProgmemStrchr(), getProgmemStrpos()
** 2013-08-16 del getProgmemStrchr()
*/

/* Copyright (C) 2014 Richard Goutorbe.  All right reserved.
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
Contact information: http://www.reseau.org/arduinorc/index.php?n=Main.Contact
*/

#ifndef arduinotx_lib_h
#define arduinotx_lib_h
#include <Arduino.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdarg.h>

#define getProgmemByteArrayValue(array, idx) (pgm_read_byte((array) + (idx)))
#define getProgmemIntArrayValue(array, idx) (pgm_read_word((array) + (idx)))

byte serialInit(long bauds_lng = 9600L);
void aprintfln(const char *fmt_str, ... );
void aPrintfln(const char *fmt_pstr, ... );
void aprintf(const char *fmt_str, ... );
void aPrintf(const char *fmt_str, ... );
int serialWrite(char c, FILE *f);
int getProgmemStrpos(PGM_P pgm_str, const char c_chr);
char *getProgmemStrArrayValue(char *out_buffer_str, PGM_P *array_str, int idx_int, size_t buffersize_int);
int findProgmemStrArrayIndex(PGM_P *array_str, const char *value_str, int nitems_int = 32767);
void printProgmemStrArray(PGM_P *array_str, int nitems_int = 32767);
byte Isblank(const char *line_str);
char *Trimwhitespace(char *out_line_str);
char *TimeString(unsigned long seconds_lng, char *out_buffer_str);
byte ishexdigit(char a_chr);
unsigned long hex2dec(char *hex_str, byte length_byt);
#endif
