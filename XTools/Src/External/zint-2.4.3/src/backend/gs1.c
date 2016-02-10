/* gs1.c - Verifies GS1 data */

/*
    libzint - the open source barcode library
    Copyright (C) 2009 Robin Stuart <robin@zint.org.uk>

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
#include <stdlib.h>
#include <stdio.h>
#ifdef _MSC_VER
#include <malloc.h> 
#endif
#include "common.h"
#include "gs1.h"

/* This code does some checks on the integrity of GS1 data. It is not intended
   to be bulletproof, nor does it report very accurately what problem was found
   or where, but should prevent some of the more common encoding errors */

void itostr(char ai_string[], int ai_value)
{
	int thou, hund, ten, unit;
	char temp[2];

	strcpy(ai_string, "(");
	thou = ai_value / 1000;
	hund = (ai_value - (1000 * thou)) / 100;
	ten = (ai_value - ((1000 * thou) + (100 * hund))) / 10;
	unit = ai_value - ((1000 * thou) + (100 * hund) + (10 * ten));

	temp[1] = '\0';
	if(ai_value >= 1000) { temp[0] = itoc(thou); concat(ai_string, temp); }
	if(ai_value >= 100) { temp[0] = itoc(hund); concat(ai_string, temp); }
	temp[0] = itoc(ten);
	concat(ai_string, temp);
	temp[0] = itoc(unit);
	concat(ai_string, temp);
	concat(ai_string, ")");
}

int gs1_verify(struct zint_symbol *symbol, unsigned char source[], const unsigned int src_len, char reduced[])
{
	int i, j, last_ai, ai_latch;
	char ai_string[6];
	int bracket_level, max_bracket_level, ai_length, max_ai_length, min_ai_length;
	int ai_value[100], ai_location[100], ai_count, data_location[100], data_length[100];
	int error_latch;

	/* Detect extended ASCII characters */
	for(i = 0; i < src_len; i++) {
		if(source[i] >=128) {
			strcpy(symbol->errtxt, "Extended ASCII characters are not supported by GS1");
			return ZINT_ERROR_INVALID_DATA;
		}
		if(source[i] < 32) {
			strcpy(symbol->errtxt, "Control characters are not supported by GS1");
			return ZINT_ERROR_INVALID_DATA;
		}
	}

	if(source[0] != '[') {
		strcpy(symbol->errtxt, "Data does not start with an AI");
		return ZINT_ERROR_INVALID_DATA;
	}

	/* Check the position of the brackets */
	bracket_level = 0;
	max_bracket_level = 0;
	ai_length = 0;
	max_ai_length = 0;
	min_ai_length = 5;
	j = 0;
	ai_latch = 0;
	for(i = 0; i < src_len; i++) {
		ai_length += j;
		if(((j == 1) && (source[i] != ']')) && ((source[i] < '0') || (source[i] > '9'))) { ai_latch = 1; }
		if(source[i] == '[') { bracket_level++; j = 1; }
		if(source[i] == ']') {
			bracket_level--;
			if(ai_length < min_ai_length) { min_ai_length = ai_length; }
			j = 0;
			ai_length = 0; 
		}
		if(bracket_level > max_bracket_level) { max_bracket_level = bracket_level; }
		if(ai_length > max_ai_length) { max_ai_length = ai_length; }
	}
	min_ai_length--;

	if(bracket_level != 0) {
		/* Not all brackets are closed */
		strcpy(symbol->errtxt, "Malformed AI in input data (brackets don\'t match)");
		return ZINT_ERROR_INVALID_DATA;
	}

	if(max_bracket_level > 1) {
		/* Nested brackets */
		strcpy(symbol->errtxt, "Found nested brackets in input data");
		return ZINT_ERROR_INVALID_DATA;
	}

	if(max_ai_length > 4) {
		/* AI is too long */
		strcpy(symbol->errtxt, "Invalid AI in input data (AI too long)");
		return ZINT_ERROR_INVALID_DATA;
	}

	if(min_ai_length <= 1) {
		/* AI is too short */
		strcpy(symbol->errtxt, "Invalid AI in input data (AI too short)");
		return ZINT_ERROR_INVALID_DATA;
	}

	if(ai_latch == 1) {
		/* Non-numeric data in AI */
		strcpy(symbol->errtxt, "Invalid AI in input data (non-numeric characters in AI)");
		return ZINT_ERROR_INVALID_DATA;
	}

	ai_count = 0;
	for(i = 1; i < src_len; i++) {
		if(source[i - 1] == '[') {
			ai_location[ai_count] = i;
			j = 0;
			do {
				ai_string[j] = source[i + j];
				j++;
			} while (ai_string[j - 1] != ']');
			ai_string[j - 1] = '\0';
			ai_value[ai_count] = atoi(ai_string);
			ai_count++;
		}
	}

	for(i = 0; i < ai_count; i++) {
		data_location[i] = ai_location[i] + 3;
		if(ai_value[i] >= 100) { data_location[i]++; }
		if(ai_value[i] >= 1000) { data_location[i]++; }
		data_length[i] = 0;
		do {
			data_length[i]++;
		} while ((source[data_location[i] + data_length[i] - 1] != '[') && (source[data_location[i] + data_length[i] - 1] != '\0'));
		data_length[i]--;
	}

	for(i = 0; i < ai_count; i++) {
		if(data_length[i] == 0) {
			/* No data for given AI */
			strcpy(symbol->errtxt, "Empty data field in input data");
			return ZINT_ERROR_INVALID_DATA;
		}
	}

	error_latch = 0;
	strcpy(ai_string, "");
	for(i = 0; i < ai_count; i++) {
		switch (ai_value[i]) {
			case 0: if(data_length[i] != 18) { error_latch = 1; } break;
			case 1:
			case 2:
			case 3: if(data_length[i] != 14) { error_latch = 1; } break;
			case 4: if(data_length[i] != 16) { error_latch = 1; } break;
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
			case 16:
			case 17:
			case 18:
			case 19: if(data_length[i] != 6) { error_latch = 1; } break;
			case 20: if(data_length[i] != 2) { error_latch = 1; } break;
			case 23:
			case 24:
			case 25:
			case 39:
			case 40:
			case 41:
			case 42:
			case 70:
			case 80:
			case 81: error_latch = 2; break;
		}
		if(
			((ai_value[i] >= 100) && (ai_value[i] <= 179))
			|| ((ai_value[i] >= 1000) && (ai_value[i] <= 1799))
			|| ((ai_value[i] >= 200) && (ai_value[i] <= 229))
			|| ((ai_value[i] >= 2000) && (ai_value[i] <= 2299))
			|| ((ai_value[i] >= 300) && (ai_value[i] <= 309))
			|| ((ai_value[i] >= 3000) && (ai_value[i] <= 3099))
			|| ((ai_value[i] >= 31) && (ai_value[i] <= 36))
			|| ((ai_value[i] >= 310) && (ai_value[i] <= 369))
		) {
			error_latch = 2;
		}
		if((ai_value[i] >= 3100) && (ai_value[i] <= 3699)) {
			if(data_length[i] != 6) {
				error_latch = 1;
			}
		}
		if(
			((ai_value[i] >= 370) && (ai_value[i] <= 379))
			|| ((ai_value[i] >= 3700) && (ai_value[i] <= 3799))
		) { 
			error_latch = 2; 
		}
		if((ai_value[i] >= 410) && (ai_value[i] <= 415)) {
			if(data_length[i] != 13) { 
				error_latch = 1; 
			} 
		}
		if(
			((ai_value[i] >= 4100) && (ai_value[i] <= 4199))
			|| ((ai_value[i] >= 700) && (ai_value[i] <= 703))
			|| ((ai_value[i] >= 800) && (ai_value[i] <= 810))
			|| ((ai_value[i] >= 900) && (ai_value[i] <= 999))
			|| ((ai_value[i] >= 9000) && (ai_value[i] <= 9999))
		) {
			error_latch = 2; 
		}
		if((error_latch < 4) && (error_latch > 0)) {
			/* error has just been detected: capture AI */
			itostr(ai_string, ai_value[i]);
			error_latch += 4;
		}
	}

	if(error_latch == 5) {
		strcpy(symbol->errtxt, "Invalid data length for AI ");
		concat(symbol->errtxt, ai_string);
		return ZINT_ERROR_INVALID_DATA;
	}

	if(error_latch == 6) {
		strcpy(symbol->errtxt, "Invalid AI value ");
		concat(symbol->errtxt, ai_string);
		return ZINT_ERROR_INVALID_DATA;
	}

	/* Resolve AI data - put resulting string in 'reduced' */
	j = 0;
	last_ai = 0;
	ai_latch = 1;
	for(i = 0; i < src_len; i++) {
		if((source[i] != '[') && (source[i] != ']')) {
			reduced[j++] = source[i];
		}
		if(source[i] == '[') {
			/* Start of an AI string */
			if(ai_latch == 0) {
				reduced[j++] = '[';
			}
			ai_string[0] = source[i + 1];
			ai_string[1] = source[i + 2];
			ai_string[2] = '\0';
			last_ai = atoi(ai_string);
			ai_latch = 0;
			/* The following values from "GS-1 General Specification version 8.0 issue 2, May 2008"
			figure 5.4.8.2.1 - 1 "Element Strings with Pre-Defined Length Using Application Identifiers" */
			if(
				((last_ai >= 0) && (last_ai <= 4))
				|| ((last_ai >= 11) && (last_ai <= 20))
				|| (last_ai == 23) /* legacy support - see 5.3.8.2.2 */
				|| ((last_ai >= 31) && (last_ai <= 36))
				|| (last_ai == 41)
			) {
				ai_latch = 1; 
			}
		}
		/* The ']' character is simply dropped from the input */
	}
	reduced[j] = '\0';

	/* the character '[' in the reduced string refers to the FNC1 character */
	return 0;
}

int ugs1_verify(struct zint_symbol *symbol, unsigned char source[], const unsigned int src_len, unsigned char reduced[])
{
	/* Only to keep the compiler happy */
#ifndef _MSC_VER
	char temp[src_len + 5];
#else
        char* temp = (char*)_alloca(src_len + 5);
#endif
	int error_number;

	error_number = gs1_verify(symbol, source, src_len, temp);
	if(error_number != 0) { return error_number; }

	if (strlen(temp) < src_len + 5) {
		ustrcpy(reduced, (unsigned char*)temp);
		return 0;
	}
	strcpy(symbol->errtxt, "ugs1_verify overflow");
	return ZINT_ERROR_INVALID_DATA;
}
