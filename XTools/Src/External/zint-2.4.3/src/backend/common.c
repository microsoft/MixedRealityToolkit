/* common.c - Contains functions needed for a number of barcodes */

/*
    libzint - the open source barcode library
    Copyright (C) 2008 Robin Stuart <robin@zint.org.uk>

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright 
       notice, this list of conditions and the following disclaimer.  
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.  
    3. Neither the name of the project nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission. 

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
    SUCH DAMAGE.
*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

int ustrlen(const unsigned char data[]) {
	/* Local replacement for strlen() with unsigned char strings */
	int i;
	for (i=0;data[i];i++);

	return i;
}

void ustrcpy(unsigned char target[],const unsigned char source[]) {
	/* Local replacement for strcpy() with unsigned char strings */
	int i, len;

	len = ustrlen(source);
	for(i = 0; i < len; i++) {
		target[i] = source[i];
	}
	target[i] = '\0';
}

void concat(char dest[],const char source[])
{ /* Concatinates dest[] with the contents of source[], copying /0 as well */
	unsigned int i, j, n;

	j = strlen(dest);
	n = strlen(source);
	for(i = 0; i <= n; i++) {
		dest[i + j] = source[i]; }
}

void uconcat(unsigned char dest[], const unsigned char source[])
{ /* Concatinates dest[] with the contents of source[], copying /0 as well */
	unsigned int i, j;

	j = ustrlen(dest);
	for(i = 0; i <= ustrlen(source); i++) {
		dest[i + j] = source[i]; }
}


int ctoi(char source)
{ /* Converts a character 0-9 to its equivalent integer value */
	if((source >= '0') && (source <= '9'))
		return (source - '0');
	return(source - 'A' + 10);
}

char itoc(int source)
{ /* Converts an integer value to its hexadecimal character */
	if ((source >= 0) && (source <= 9)) {
		return ('0' + source); }
	else {
		return ('A' + (source - 10)); }
}

void to_upper(unsigned char source[])
{ /* Converts lower case characters to upper case in a string source[] */
	unsigned int i, src_len = ustrlen(source);

	for (i = 0; i < src_len; i++) {
		if ((source[i] >= 'a') && (source[i] <= 'z')) {
			source [i] = (source[i] - 'a') + 'A'; }
	}
}

int is_sane(char test_string[], unsigned char source[], int length)
{ /* Verifies that a string only uses valid characters */
	unsigned int i, j, latch;
	unsigned int lt = strlen(test_string);

	for(i = 0; i < length; i++) {
		latch = FALSE;
		for(j = 0; j < lt; j++) {
			if (source[i] == test_string[j]) { 
				latch = TRUE; 
				break;
			} 
		}
		if (!(latch)) { 
			return ZINT_ERROR_INVALID_DATA; 
		}
	}

	return 0;
}

int posn(char set_string[], char data)
{ /* Returns the position of data in set_string */
	unsigned int i, n = strlen(set_string);

	for(i = 0; i < n; i++) {
		if (data == set_string[i]) { return i; } }
	return 0;
}

void lookup(char set_string[],const char *table[], char data, char dest[])
{ /* Replaces huge switch statements for looking up in tables */
	unsigned int i, n = strlen(set_string);

	for(i = 0; i < n; i++) {
		if (data == set_string[i]) { concat(dest, table[i]); } }
}

int module_is_set(struct zint_symbol *symbol, int y_coord, int x_coord)
{
	return (symbol->encoded_data[y_coord][x_coord / 7] >> (x_coord % 7)) & 1;
#if 0	
	switch(x_sub) {
		case 0: if((symbol->encoded_data[y_coord][x_char] & 0x01) != 0) { result = 1; } break;
		case 1: if((symbol->encoded_data[y_coord][x_char] & 0x02) != 0) { result = 1; } break;
		case 2: if((symbol->encoded_data[y_coord][x_char] & 0x04) != 0) { result = 1; } break;
		case 3: if((symbol->encoded_data[y_coord][x_char] & 0x08) != 0) { result = 1; } break;
		case 4: if((symbol->encoded_data[y_coord][x_char] & 0x10) != 0) { result = 1; } break;
		case 5: if((symbol->encoded_data[y_coord][x_char] & 0x20) != 0) { result = 1; } break;
		case 6: if((symbol->encoded_data[y_coord][x_char] & 0x40) != 0) { result = 1; } break;
	}
	
	return result;
#endif
}

void set_module(struct zint_symbol *symbol, int y_coord, int x_coord)
{
	symbol->encoded_data[y_coord][x_coord / 7] |= 1 << (x_coord % 7);
#if 0
	int x_char, x_sub;
	

	x_char = x_coord / 7;
	x_sub = x_coord % 7;
	
	switch(x_sub) {
		case 0: symbol->encoded_data[y_coord][x_char] += 0x01; break;
		case 1: symbol->encoded_data[y_coord][x_char] += 0x02; break;
		case 2: symbol->encoded_data[y_coord][x_char] += 0x04; break;
		case 3: symbol->encoded_data[y_coord][x_char] += 0x08; break;
		case 4: symbol->encoded_data[y_coord][x_char] += 0x10; break;
		case 5: symbol->encoded_data[y_coord][x_char] += 0x20; break;
		case 6: symbol->encoded_data[y_coord][x_char] += 0x40; break;
	} /* The last binary digit is reserved for colour barcodes */
#endif
}

void unset_module(struct zint_symbol *symbol, int y_coord, int x_coord)
{
	symbol->encoded_data[y_coord][x_coord / 7] &= ~(1 << (x_coord % 7));
#if 0
	int x_char, x_sub;
	
	x_char = x_coord / 7;
	x_sub = x_coord % 7;
	
	switch(x_sub) {
		case 0: symbol->encoded_data[y_coord][x_char] -= 0x01; break;
		case 1: symbol->encoded_data[y_coord][x_char] -= 0x02; break;
		case 2: symbol->encoded_data[y_coord][x_char] -= 0x04; break;
		case 3: symbol->encoded_data[y_coord][x_char] -= 0x08; break;
		case 4: symbol->encoded_data[y_coord][x_char] -= 0x10; break;
		case 5: symbol->encoded_data[y_coord][x_char] -= 0x20; break;
		case 6: symbol->encoded_data[y_coord][x_char] -= 0x40; break;
	} /* The last binary digit is reserved for colour barcodes */
#endif
}

void expand(struct zint_symbol *symbol, char data[])
{ /* Expands from a width pattern to a bit pattern */

	unsigned int reader, n = strlen(data);
	int writer, i;
	char latch;

	writer = 0;
	latch = '1';

	for(reader = 0; reader < n; reader++) {
		for(i = 0; i < ctoi(data[reader]); i++) {
			if(latch == '1') { set_module(symbol, symbol->rows, writer); }
			writer++;
		}

		latch = (latch == '1' ? '0' : '1');
	}

	if(symbol->symbology != BARCODE_PHARMA) {
		if(writer > symbol->width) {
			symbol->width = writer;
		}
	} else {
		/* Pharmacode One ends with a space - adjust for this */
		if(writer > symbol->width + 2) {
			symbol->width = writer - 2;
		}
	}
	symbol->rows = symbol->rows + 1;
}

int is_stackable(int symbology) {
	/* Indicates which symbologies can have row binding */
	if(symbology < BARCODE_PDF417) { return 1; }
	if(symbology == BARCODE_CODE128B) { return 1; }
	if(symbology == BARCODE_ISBNX) { return 1; }
	if(symbology == BARCODE_EAN14) { return 1; }
	if(symbology == BARCODE_NVE18) { return 1; }
	if(symbology == BARCODE_KOREAPOST) { return 1; }
	if(symbology == BARCODE_PLESSEY) { return 1; }
	if(symbology == BARCODE_TELEPEN_NUM) { return 1; }
	if(symbology == BARCODE_ITF14) { return 1; }
	if(symbology == BARCODE_CODE32) { return 1; }

	return 0;
}

int is_extendable(int symbology) {
	/* Indicates which symbols can have addon */
	if(symbology == BARCODE_EANX) { return 1; }
	if(symbology == BARCODE_UPCA) { return 1; }
	if(symbology == BARCODE_UPCE) { return 1; }
	if(symbology == BARCODE_ISBNX) { return 1; }
	if(symbology == BARCODE_UPCA_CC) { return 1; }
	if(symbology == BARCODE_UPCE_CC) { return 1; }
	if(symbology == BARCODE_EANX_CC) { return 1; }

	return 0;
}

int roundup(float input)
{
	float remainder;
	int integer_part;

	integer_part = (int)input;
	remainder = input - integer_part;

	if(remainder > 0.1) {
		integer_part++;
	}

	return integer_part;
}

int istwodigits(unsigned char source[], int position)
{
	if((source[position] >= '0') && (source[position] <= '9')) {
		if((source[position + 1] >= '0') && (source[position + 1] <= '9')) {
			return 1;
		}
	}

	return 0;
}

float froundup(float input)
{
	float fraction, output = 0.0;

	fraction = input - (int)input;
	if(fraction > 0.01) { output = (input - fraction) + 1.0; } else { output = input; }

	return output;
}

int latin1_process(struct zint_symbol *symbol, unsigned char source[], unsigned char preprocessed[], int *length)
{
	int j, i, next;
	
	/* Convert Unicode to Latin-1 for those symbologies which only support Latin-1 */
	j = 0;
	i = 0;
	do {
		next = -1;
		if(source[i] < 128) {
			preprocessed[j] = source[i];
			j++;
			next = i + 1;
		} else {
			if(source[i] == 0xC2) {
				preprocessed[j] = source[i + 1];
				j++;
				next = i + 2;
			}
			if(source[i] == 0xC3) {
				preprocessed[j] = source[i + 1] + 64;
				j++;
				next = i + 2;
			}
		}
		if(next == -1) {
			strcpy(symbol->errtxt, "error: Invalid character in input string (only Latin-1 characters supported)");
			return ZINT_ERROR_INVALID_DATA;
		}
		i = next;
	} while(i < *length);
	preprocessed[j] = '\0';
	*length = j;

	return 0;
}

int utf8toutf16(struct zint_symbol *symbol, unsigned char source[], int vals[], int *length)
{
	int bpos, jpos, error_number;
	int next;

	bpos = 0;
	jpos = 0;
	error_number = 0;
	next = 0;

	do {
		if(source[bpos] <= 0x7f) {
			/* 1 byte mode (7-bit ASCII) */
			vals[jpos] = source[bpos];
			next = bpos + 1;
			jpos++;
		} else {
			if((source[bpos] >= 0x80) && (source[bpos] <= 0xbf)) {
				strcpy(symbol->errtxt, "Corrupt Unicode data");
				return ZINT_ERROR_INVALID_DATA;
			}
			if((source[bpos] >= 0xc0) && (source[bpos] <= 0xc1)) {
				strcpy(symbol->errtxt, "Overlong encoding not supported");
				return ZINT_ERROR_INVALID_DATA;
			}

			if((source[bpos] >= 0xc2) && (source[bpos] <= 0xdf)) {
				/* 2 byte mode */
				vals[jpos] = ((source[bpos] & 0x1f) << 6) + (source[bpos + 1] & 0x3f);
				next = bpos + 2;
				jpos++;
			} else
			if((source[bpos] >= 0xe0) && (source[bpos] <= 0xef)) {
				/* 3 byte mode */
				vals[jpos] = ((source[bpos] & 0x0f) << 12) + ((source[bpos + 1] & 0x3f) << 6) + (source[bpos + 2] & 0x3f);
				next = bpos + 3;
				jpos ++;
			} else
			if(source[bpos] >= 0xf0) {
				strcpy(symbol->errtxt, "Unicode sequences of more than 3 bytes not supported");
				return ZINT_ERROR_INVALID_DATA;
			}
		}

		bpos = next;

	} while(bpos < *length);
	*length = jpos;

	return error_number;
}

