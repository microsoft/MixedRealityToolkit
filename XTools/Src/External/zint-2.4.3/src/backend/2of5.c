/* 2of5.c - Handles Code 2 of 5 barcodes */

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
#ifdef _MSC_VER
#include <malloc.h>
#endif

static const char *C25MatrixTable[10] = {"113311", "311131", "131131", "331111", "113131", "313111",
	"133111", "111331", "311311", "131311"};

static const char *C25IndustTable[10] = {"1111313111", "3111111131", "1131111131", "3131111111", "1111311131",
	"3111311111", "1131311111", "1111113131", "3111113111", "1131113111"};

static const char *C25InterTable[10] = {"11331", "31113", "13113", "33111", "11313", "31311", "13311", "11133",
	"31131", "13131"};

static char check_digit(unsigned int count)
{
	return itoc((10 - (count % 10)) % 10);
}

int matrix_two_of_five(struct zint_symbol *symbol, unsigned char source[], int length)
{ /* Code 2 of 5 Standard (Code 2 of 5 Matrix) */
	
	int i, error_number;
	char dest[512]; /* 6 + 80 * 6 + 6 + 1 ~ 512*/

	error_number = 0;

	if(length > 80) {
		strcpy(symbol->errtxt, "Input too long");
		return ZINT_ERROR_TOO_LONG;
	}
	error_number = is_sane(NEON, source, length);
	if(error_number == ZINT_ERROR_INVALID_DATA) {
		strcpy(symbol->errtxt, "Invalid characters in data");
		return error_number;
	}

	/* start character */
	strcpy(dest, "411111");

	for(i = 0; i < length; i++) {
		lookup(NEON, C25MatrixTable, source[i], dest);
	}

	/* Stop character */
	concat (dest, "41111");

	expand(symbol, dest);
	ustrcpy(symbol->text, source);
	return error_number;
}

int industrial_two_of_five(struct zint_symbol *symbol, unsigned char source[], int length)
{ /* Code 2 of 5 Industrial */

	int i, error_number;
	char dest[512]; /* 6 + 40 * 10 + 6 + 1 */

	error_number = 0;

	if(length > 45) {
		strcpy(symbol->errtxt, "Input too long");
		return ZINT_ERROR_TOO_LONG;
	}
	error_number = is_sane(NEON, source, length);
	if(error_number == ZINT_ERROR_INVALID_DATA) {
		strcpy(symbol->errtxt, "Invalid character in data");
		return error_number;
	}

	/* start character */
	strcpy(dest, "313111");

	for(i = 0; i < length; i++) {
		lookup(NEON, C25IndustTable, source[i], dest);
	}

	/* Stop character */
	concat (dest, "31113");

	expand(symbol, dest);
	ustrcpy(symbol->text, source);
	return error_number;
}

int iata_two_of_five(struct zint_symbol *symbol, unsigned char source[], int length)
{ /* Code 2 of 5 IATA */
	int i, error_number;
	char dest[512]; /* 4 + 45 * 10 + 3 + 1 */

	error_number = 0;

	if(length > 45) {
		strcpy(symbol->errtxt, "Input too long");
		return ZINT_ERROR_TOO_LONG;
	}
	error_number = is_sane(NEON, source, length);
	if(error_number == ZINT_ERROR_INVALID_DATA) {
		strcpy(symbol->errtxt, "Invalid characters in data");
		return error_number;
	}

	/* start */
	strcpy(dest, "1111");
	
	for(i = 0; i < length; i++) {
		lookup(NEON, C25IndustTable, source[i], dest);
	}

	/* stop */
	concat (dest, "311");

	expand(symbol, dest);
	ustrcpy(symbol->text, source);
	return error_number;
}

int logic_two_of_five(struct zint_symbol *symbol, unsigned char source[], int length)
{ /* Code 2 of 5 Data Logic */
	
	int i, error_number;
	char dest[512]; /* 4 + 80 * 6 + 3 + 1 */

	error_number = 0;

	if(length > 80) {
		strcpy(symbol->errtxt, "Input too long");
		return ZINT_ERROR_TOO_LONG;
	}
	error_number = is_sane(NEON, source, length);
	if(error_number == ZINT_ERROR_INVALID_DATA) {
		strcpy(symbol->errtxt, "Invalid characters in data");
		return error_number;
	}

	/* start character */
	strcpy(dest, "1111");

	for(i = 0; i < length; i++) {
		lookup(NEON, C25MatrixTable, source[i], dest);
	}

	/* Stop character */
	concat (dest, "311");

	expand(symbol, dest);
	ustrcpy(symbol->text, source);
	return error_number;
}

int interleaved_two_of_five(struct zint_symbol *symbol, unsigned char source[], int length)
{ /* Code 2 of 5 Interleaved */

	int i, j, k, error_number;
	char bars[7], spaces[7], mixed[14], dest[1000];
#ifndef _MSC_VER
	unsigned char temp[length + 2];
#else
	unsigned char* temp = (unsigned char *)_alloca((length + 2) * sizeof(unsigned char));
#endif

	error_number = 0;

	if(length > 89) {
		strcpy(symbol->errtxt, "Input too long");
		return ZINT_ERROR_TOO_LONG;
	}
	error_number = is_sane(NEON, source, length);
	if (error_number == ZINT_ERROR_INVALID_DATA) {
		strcpy(symbol->errtxt, "Invalid characters in data");
		return error_number;
	}

	ustrcpy(temp, (unsigned char *) "");
	/* Input must be an even number of characters for Interlaced 2 of 5 to work:
	   if an odd number of characters has been entered then add a leading zero */
	if (length & 1)
	{
		ustrcpy(temp, (unsigned char *) "0");
		length++;
	}
	uconcat(temp, source);

	/* start character */
	strcpy(dest, "1111");

	for(i = 0; i < length; i+=2 )
	{
		/* look up the bars and the spaces and put them in two strings */
		strcpy(bars, "");
		lookup(NEON, C25InterTable, temp[i], bars);
		strcpy(spaces, "");
		lookup(NEON, C25InterTable, temp[i + 1], spaces);

		/* then merge (interlace) the strings together */
		k = 0;
		for(j = 0; j <= 4; j++)
		{
			mixed[k] = bars[j]; k++;
			mixed[k] = spaces[j]; k++;
		}
		mixed[k] = '\0';
		concat (dest, mixed);
	}

	/* Stop character */
	concat (dest, "311");

	expand(symbol, dest);
	ustrcpy(symbol->text, temp);
	return error_number;

}

int itf14(struct zint_symbol *symbol, unsigned char source[], int length)
{
	int i, error_number, zeroes;
	unsigned int count;
	char localstr[16];

	error_number = 0;

	count = 0;

	if(length > 13) {
		strcpy(symbol->errtxt, "Input too long");
		return ZINT_ERROR_TOO_LONG;
	}

	error_number = is_sane(NEON, source, length);
	if(error_number == ZINT_ERROR_INVALID_DATA) {
		strcpy(symbol->errtxt, "Invalid character in data");
		return error_number;
	}

	/* Add leading zeros as required */
	zeroes = 13 - length;
	for(i = 0; i < zeroes; i++) {
		localstr[i] = '0';
	}
	strcpy(localstr + zeroes, (char *)source);

	/* Calculate the check digit - the same method used for EAN-13 */

	for (i = 12; i >= 0; i--) {
		count += ctoi(localstr[i]);

		if (!(i & 1)) {
			count += 2 * ctoi(localstr[i]);
		}
	}
	localstr[13] = check_digit(count);
	localstr[14] = '\0';
	error_number = interleaved_two_of_five(symbol, (unsigned char *)localstr, strlen(localstr));
	ustrcpy(symbol->text, (unsigned char*)localstr);
	return error_number;
}

int dpleit(struct zint_symbol *symbol, unsigned char source[], int length)
{ /* Deutshe Post Leitcode */
	int i, error_number;
	unsigned int count;
	char localstr[16];
	int zeroes;

	error_number = 0;
	count = 0;
	if(length > 13) {
		strcpy(symbol->errtxt, "Input wrong length");
		return ZINT_ERROR_TOO_LONG;
	}
	error_number = is_sane(NEON, source, length);
	if(error_number == ZINT_ERROR_INVALID_DATA) {
		strcpy(symbol->errtxt, "Invalid characters in data");
		return error_number;
	}

	zeroes = 13 - length;
	for(i = 0; i < zeroes; i++)
		localstr[i] = '0';
	strcpy(localstr + zeroes, (char *)source);
	
	for (i = 12; i >= 0; i--)
	{
		count += 4 * ctoi(localstr[i]);

		if (i & 1) {
			count += 5 * ctoi(localstr[i]);
		}
	}
	localstr[13] = check_digit(count);
	localstr[14] = '\0';
	error_number = interleaved_two_of_five(symbol, (unsigned char *)localstr, strlen(localstr));
	ustrcpy(symbol->text, (unsigned char*)localstr);
	return error_number;
}

int dpident(struct zint_symbol *symbol, unsigned char source[], int length)
{ /* Deutsche Post Identcode */
	int i, error_number, zeroes;
	unsigned int count;
	char localstr[16];

	count = 0;
	if(length > 11) {
		strcpy(symbol->errtxt, "Input wrong length");
		return ZINT_ERROR_TOO_LONG;
	}
	error_number = is_sane(NEON, source, length);
	if(error_number == ZINT_ERROR_INVALID_DATA) {
		strcpy(symbol->errtxt, "Invalid characters in data");
		return error_number;
	}

	zeroes = 11 - length;
	for(i = 0; i < zeroes; i++)
		localstr[i] = '0';
	strcpy(localstr + zeroes, (char *)source);
	
	for (i = 10; i >= 0; i--)
	{
		count += 4 * ctoi(localstr[i]);

		if (i & 1) {
			count += 5 * ctoi(localstr[i]);
		}
	}
	localstr[11] = check_digit(count);
	localstr[12] = '\0';
	error_number = interleaved_two_of_five(symbol, (unsigned char *)localstr, strlen(localstr));
	ustrcpy(symbol->text, (unsigned char*)localstr);
	return error_number;
}
