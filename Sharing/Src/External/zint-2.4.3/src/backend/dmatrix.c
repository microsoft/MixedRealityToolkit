/* dmatrix.c Handles Data Matrix ECC 200 symbols */

/*
    libzint - the open source barcode library
    Copyright (C) 2009 Robin Stuart <robin@zint.org.uk>
    
    developed from and including some functions from:
	IEC16022 bar code generation
	Adrian Kennard, Andrews & Arnold Ltd
	with help from Cliff Hones on the RS coding
 
	(c) 2004 Adrian Kennard, Andrews & Arnold Ltd
	(c) 2006 Stefan Schmidt <stefan@datenfreihafen.org>

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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#ifdef _MSC_VER
#include <malloc.h> 
#endif
#include "reedsol.h"
#include "common.h"
#include "dmatrix.h"

// Annex M placement alorithm low level
static void ecc200placementbit(int *array, int NR, int NC, int r, int c, int p, char b)
{
	if (r < 0) {
		r += NR;
		c += 4 - ((NR + 4) % 8);
	}
	if (c < 0) {
		c += NC;
		r += 4 - ((NC + 4) % 8);
	}
	array[r * NC + c] = (p << 3) + b;
}

static void ecc200placementblock(int *array, int NR, int NC, int r,
				 int c, int p)
{
	ecc200placementbit(array, NR, NC, r - 2, c - 2, p, 7);
	ecc200placementbit(array, NR, NC, r - 2, c - 1, p, 6);
	ecc200placementbit(array, NR, NC, r - 1, c - 2, p, 5);
	ecc200placementbit(array, NR, NC, r - 1, c - 1, p, 4);
	ecc200placementbit(array, NR, NC, r - 1, c - 0, p, 3);
	ecc200placementbit(array, NR, NC, r - 0, c - 2, p, 2);
	ecc200placementbit(array, NR, NC, r - 0, c - 1, p, 1);
	ecc200placementbit(array, NR, NC, r - 0, c - 0, p, 0);
}

static void ecc200placementcornerA(int *array, int NR, int NC, int p)
{
	ecc200placementbit(array, NR, NC, NR - 1, 0, p, 7);
	ecc200placementbit(array, NR, NC, NR - 1, 1, p, 6);
	ecc200placementbit(array, NR, NC, NR - 1, 2, p, 5);
	ecc200placementbit(array, NR, NC, 0, NC - 2, p, 4);
	ecc200placementbit(array, NR, NC, 0, NC - 1, p, 3);
	ecc200placementbit(array, NR, NC, 1, NC - 1, p, 2);
	ecc200placementbit(array, NR, NC, 2, NC - 1, p, 1);
	ecc200placementbit(array, NR, NC, 3, NC - 1, p, 0);
}

static void ecc200placementcornerB(int *array, int NR, int NC, int p)
{
	ecc200placementbit(array, NR, NC, NR - 3, 0, p, 7);
	ecc200placementbit(array, NR, NC, NR - 2, 0, p, 6);
	ecc200placementbit(array, NR, NC, NR - 1, 0, p, 5);
	ecc200placementbit(array, NR, NC, 0, NC - 4, p, 4);
	ecc200placementbit(array, NR, NC, 0, NC - 3, p, 3);
	ecc200placementbit(array, NR, NC, 0, NC - 2, p, 2);
	ecc200placementbit(array, NR, NC, 0, NC - 1, p, 1);
	ecc200placementbit(array, NR, NC, 1, NC - 1, p, 0);
}

static void ecc200placementcornerC(int *array, int NR, int NC, int p)
{
	ecc200placementbit(array, NR, NC, NR - 3, 0, p, 7);
	ecc200placementbit(array, NR, NC, NR - 2, 0, p, 6);
	ecc200placementbit(array, NR, NC, NR - 1, 0, p, 5);
	ecc200placementbit(array, NR, NC, 0, NC - 2, p, 4);
	ecc200placementbit(array, NR, NC, 0, NC - 1, p, 3);
	ecc200placementbit(array, NR, NC, 1, NC - 1, p, 2);
	ecc200placementbit(array, NR, NC, 2, NC - 1, p, 1);
	ecc200placementbit(array, NR, NC, 3, NC - 1, p, 0);
}

static void ecc200placementcornerD(int *array, int NR, int NC, int p)
{
	ecc200placementbit(array, NR, NC, NR - 1, 0, p, 7);
	ecc200placementbit(array, NR, NC, NR - 1, NC - 1, p, 6);
	ecc200placementbit(array, NR, NC, 0, NC - 3, p, 5);
	ecc200placementbit(array, NR, NC, 0, NC - 2, p, 4);
	ecc200placementbit(array, NR, NC, 0, NC - 1, p, 3);
	ecc200placementbit(array, NR, NC, 1, NC - 3, p, 2);
	ecc200placementbit(array, NR, NC, 1, NC - 2, p, 1);
	ecc200placementbit(array, NR, NC, 1, NC - 1, p, 0);
}

// Annex M placement alorithm main function
static void ecc200placement(int *array, int NR, int NC)
{
	int r, c, p;
	// invalidate
	for (r = 0; r < NR; r++)
		for (c = 0; c < NC; c++)
			array[r * NC + c] = 0;
	// start
	p = 1;
	r = 4;
	c = 0;
	do {
		// check corner
		if (r == NR && !c)
			ecc200placementcornerA(array, NR, NC, p++);
		if (r == NR - 2 && !c && NC % 4)
			ecc200placementcornerB(array, NR, NC, p++);
		if (r == NR - 2 && !c && (NC % 8) == 4)
			ecc200placementcornerC(array, NR, NC, p++);
		if (r == NR + 4 && c == 2 && !(NC % 8))
			ecc200placementcornerD(array, NR, NC, p++);
		// up/right
		do {
			if (r < NR && c >= 0 && !array[r * NC + c])
				ecc200placementblock(array, NR, NC, r, c, p++);
			r -= 2;
			c += 2;
		}
		while (r >= 0 && c < NC);
		r++;
		c += 3;
		// down/left
		do {
			if (r >= 0 && c < NC && !array[r * NC + c])
				ecc200placementblock(array, NR, NC, r, c, p++);
			r += 2;
			c -= 2;
		}
		while (r < NR && c >= 0);
		r += 3;
		c++;
	}
	while (r < NR || c < NC);
	// unfilled corner
	if (!array[NR * NC - 1])
		array[NR * NC - 1] = array[NR * NC - NC - 2] = 1;
}

// calculate and append ecc code, and if necessary interleave
static void ecc200(unsigned char *binary, int bytes, int datablock, int rsblock, int skew)
{
	int blocks = (bytes + 2) / datablock, b;
	int n, p;
	rs_init_gf(0x12d);
	rs_init_code(rsblock, 1);
	for (b = 0; b < blocks; b++) {
		unsigned char buf[256], ecc[256];
		p = 0;
		for (n = b; n < bytes; n += blocks)
			buf[p++] = binary[n];
		rs_encode(p, buf, ecc);
		p = rsblock - 1;	// comes back reversed
		for (n = b; n < rsblock * blocks; n += blocks) {
			if (skew) {
				/* Rotate ecc data to make 144x144 size symbols acceptable */
				/* See http://groups.google.com/group/postscriptbarcode/msg/5ae8fda7757477da */
				if(b < 8) {
					binary[bytes + n + 2] = ecc[p--];
				} else {
					binary[bytes + n - 8] = ecc[p--];
				}
			} else {
				binary[bytes + n] = ecc[p--];
			}
		}
	}
	rs_free();
}

int isx12(unsigned char source)
{
	if(source == 13) { return 1; }
	if(source == 42) { return 1; }
	if(source == 62) { return 1; }
	if(source == 32) { return 1; }
	if((source >= '0') && (source <= '9')) { return 1; }
	if((source >= 'A') && (source <= 'Z')) { return 1; }

	return 0;
}

void dminsert(char binary_string[], int posn, char newbit)
{ /* Insert a character into the middle of a string at position posn */
	int i, end;

	end = strlen(binary_string);
	for(i = end; i > posn; i--) {
		binary_string[i] = binary_string[i - 1];
	}
	binary_string[posn] = newbit;
}

void insert_value(unsigned char binary_stream[], int posn, int streamlen, char newbit)
{
	int i;

	for(i = streamlen; i > posn; i--) {
		binary_stream[i] = binary_stream[i - 1];
	}
	binary_stream[posn] = newbit;
}

int look_ahead_test(unsigned char source[], int sourcelen, int position, int current_mode, int gs1)
{
	/* A custom version of the 'look ahead test' from Annex P */
	/* This version is deliberately very reluctant to end a data stream with EDIFACT encoding */

	float ascii_count, c40_count, text_count, x12_count, edf_count, b256_count, best_count;
	int sp, done, best_scheme;
	char reduced_char;

	/* step (j) */
	if(current_mode == DM_ASCII) {
		ascii_count = 0.0;
		c40_count = 1.0;
		text_count = 1.0;
		x12_count = 1.0;
		edf_count = 1.0;
		b256_count = 1.25;
	} else {
		ascii_count = 1.0;
		c40_count = 2.0;
		text_count = 2.0;
		x12_count = 2.0;
		edf_count = 2.0;
		b256_count = 2.25;
	}

	switch(current_mode) {
		case DM_C40: c40_count = 0.0; break;
		case DM_TEXT: text_count = 0.0; break;
		case DM_X12: x12_count = 0.0; break;
		case DM_EDIFACT: edf_count = 0.0; break;
		case DM_BASE256: b256_count = 0.0; break;
	}

	for(sp = position; (sp < sourcelen) && (sp <= (position + 8)); sp++) {

		if(source[sp] <= 127) { reduced_char = source[sp]; } else { reduced_char = source[sp] - 127; }

		if((source[sp] >= '0') && (source[sp] <= '9')) { ascii_count += 0.5; } else { ascii_count += 1.0; }
		if(source[sp] > 127) { ascii_count += 1.0; }

		done = 0;
		if(reduced_char == ' ') { c40_count += (2.0 / 3.0); done = 1; }
		if((reduced_char >= '0') && (reduced_char <= '9')) { c40_count += (2.0 / 3.0); done = 1; }
		if((reduced_char >= 'A') && (reduced_char <= 'Z')) { c40_count += (2.0 / 3.0); done = 1; }
		if(source[sp] > 127) { c40_count += (4.0 / 3.0); }
		if(done == 0) { c40_count += (4.0 / 3.0); }

		done = 0;
		if(reduced_char == ' ') { text_count += (2.0 / 3.0); done = 1; }
		if((reduced_char >= '0') && (reduced_char <= '9')) { text_count += (2.0 / 3.0); done = 1; }
		if((reduced_char >= 'a') && (reduced_char <= 'z')) { text_count += (2.0 / 3.0); done = 1; }
		if(source[sp] > 127) { text_count += (4.0 / 3.0); }
		if(done == 0) { text_count += (4.0 / 3.0); }

		if(isx12(source[sp])) { x12_count += (2.0 / 3.0); } else { x12_count += 4.0; }

		/* step (p) */
		done = 0;
		if((source[sp] >= ' ') && (source[sp] <= '^')) { edf_count += (3.0 / 4.0); } else { edf_count += 6.0; }
		if(gs1 && (source[sp] == '[')) { edf_count += 6.0; }
		if(sp >= (sourcelen - 5)) { edf_count += 6.0; } /* MMmmm fudge! */

		/* step (q) */
		if(gs1 && (source[sp] == '[')) { b256_count += 4.0; } else { b256_count += 1.0; }

		/* printf("%c lat a%.2f c%.2f t%.2f x%.2f e%.2f b%.2f\n", source[sp], ascii_count, c40_count, text_count, x12_count, edf_count, b256_count); */

	}

	best_count = ascii_count;
	best_scheme = DM_ASCII;

	if(b256_count <= best_count) {
		best_count = b256_count;
		best_scheme = DM_BASE256;
	}

	if(edf_count <= best_count) {
		best_count = edf_count;
		best_scheme = DM_EDIFACT;
	}

	if(text_count <= best_count) {
		best_count = text_count;
		best_scheme = DM_TEXT;
	}

	if(x12_count <= best_count) {
		best_count = x12_count;
		best_scheme = DM_X12;
	}

	if(c40_count <= best_count) {
		best_count = c40_count;
		best_scheme = DM_C40;
	}

	return best_scheme;
}

int dm200encode(struct zint_symbol *symbol, unsigned char source[], unsigned char target[], int *last_mode, int length)
{
	/* Encodes data using ASCII, C40, Text, X12, EDIFACT or Base 256 modes as appropriate */
	/* Supports encoding FNC1 in supporting systems */

	int sp, tp, i, gs1;
	int current_mode, next_mode;
	int inputlen = length;
	int c40_buffer[6], c40_p;
	int text_buffer[6], text_p;
	int x12_buffer[6], x12_p;
	int edifact_buffer[8], edifact_p;
	int debug = 0;
#ifndef _MSC_VER
        char binary[2 * inputlen];
#else
        char* binary = (char*)_alloca(2 * inputlen);
#endif

	sp = 0;
	tp = 0;
	memset(c40_buffer, 0, 6);
	c40_p = 0;
	memset(text_buffer, 0, 6);
	text_p = 0;
	memset(x12_buffer, 0, 6);
	x12_p = 0;
	memset(edifact_buffer, 0, 8);
	edifact_p = 0;
	strcpy(binary, "");

	/* step (a) */
	current_mode = DM_ASCII;
	next_mode = DM_ASCII;

	if(symbol->input_mode == GS1_MODE) { gs1 = 1; } else { gs1 = 0; }

	if(gs1) {
		target[tp] = 232; tp++;
		concat(binary, " ");
		if(debug) printf("FN1 ");
	} /* FNC1 */

	if(symbol->output_options & READER_INIT) {
		if(gs1) {
			strcpy(symbol->errtxt, "Cannot encode in GS1 mode and Reader Initialisation at the same time");
			return ZINT_ERROR_INVALID_OPTION;
		} else {
			target[tp] = 234; tp++; /* Reader Programming */
			concat(binary, " ");
			if(debug) printf("RP ");
		}
	}

	while (sp < inputlen) {

		current_mode = next_mode;

		/* step (b) - ASCII encodation */
		if(current_mode == DM_ASCII) {
			next_mode = DM_ASCII;

			if(istwodigits(source, sp) && ((sp + 1) != inputlen)) {
				target[tp] = (10 * ctoi(source[sp])) + ctoi(source[sp + 1]) + 130;
				if(debug) printf("N%d ", target[tp] - 130);
				tp++; concat(binary, " ");
				sp += 2;
			} else {
				next_mode = look_ahead_test(source, inputlen, sp, current_mode, gs1);

				if(next_mode != DM_ASCII) {
					switch(next_mode) {
						case DM_C40: target[tp] = 230; tp++; concat(binary, " ");
							if(debug) printf("C40 "); break;
						case DM_TEXT: target[tp] = 239; tp++; concat(binary, " ");
							if(debug) printf("TEX "); break;
						case DM_X12: target[tp] = 238; tp++; concat(binary, " ");
							if(debug) printf("X12 "); break;
						case DM_EDIFACT: target[tp] = 240; tp++; concat(binary, " ");
							if(debug) printf("EDI "); break;
						case DM_BASE256: target[tp] = 231; tp++; concat(binary, " ");
							if(debug) printf("BAS "); break;
					}
				} else {
					if(source[sp] > 127) {
						target[tp] = 235; /* FNC4 */
						if(debug) printf("FN4 ");
						tp++;
						target[tp] = (source[sp] - 128) + 1;
						if(debug) printf("A%02X ", target[tp] - 1);
						tp++; concat(binary, "  ");
					} else {
						if(gs1 && (source[sp] == '[')) {
							target[tp] = 232; /* FNC1 */
							if(debug) printf("FN1 ");
						} else {
							target[tp] = source[sp] + 1;
							if(debug) printf("A%02X ", target[tp] - 1);
						}
						tp++; 
						concat(binary, " ");
					}
					sp++;
				}
			}

		}

		/* step (c) C40 encodation */
		if(current_mode == DM_C40) {
			int shift_set, value;

			next_mode = DM_C40;
			if(c40_p == 0) {
				next_mode = look_ahead_test(source, inputlen, sp, current_mode, gs1);
			}

			if(next_mode != DM_C40) {
				target[tp] = 254; tp++; concat(binary, " ");/* Unlatch */
				next_mode = DM_ASCII;
				if (debug) printf("ASC ");
			} else {
				if(source[sp] > 127) {
					c40_buffer[c40_p] = 1; c40_p++;
					c40_buffer[c40_p] = 30; c40_p++; /* Upper Shift */
					shift_set = c40_shift[source[sp] - 128];
					value = c40_value[source[sp] - 128];
				} else {
					shift_set = c40_shift[source[sp]];
					value = c40_value[source[sp]];
				}

				if(gs1 && (source[sp] == '[')) {
					shift_set = 2;
					value = 27; /* FNC1 */
				}

				if(shift_set != 0) {
					c40_buffer[c40_p] = shift_set - 1; c40_p++;
				}
				c40_buffer[c40_p] = value; c40_p++;

				if(c40_p >= 3) {
					int iv;

					iv = (1600 * c40_buffer[0]) + (40 * c40_buffer[1]) + (c40_buffer[2]) + 1;
					target[tp] = iv / 256; tp++;
					target[tp] = iv % 256; tp++;
					concat(binary, "  ");
					if (debug) printf("[%d %d %d] ", c40_buffer[0], c40_buffer[1], c40_buffer[2]);

					c40_buffer[0] = c40_buffer[3];
					c40_buffer[1] = c40_buffer[4];
					c40_buffer[2] = c40_buffer[5];
					c40_buffer[3] = 0;
					c40_buffer[4] = 0;
					c40_buffer[5] = 0;
					c40_p -= 3;
				}
				sp++;
			}
		}

		/* step (d) Text encodation */
		if(current_mode == DM_TEXT) {
			int shift_set, value;

			next_mode = DM_TEXT;
			if(text_p == 0) {
				next_mode = look_ahead_test(source, inputlen, sp, current_mode, gs1);
			}

			if(next_mode != DM_TEXT) {
				target[tp] = 254; tp++; concat(binary, " ");/* Unlatch */
				next_mode = DM_ASCII;
				if (debug) printf("ASC ");
			} else {
				if(source[sp] > 127) {
					text_buffer[text_p] = 1; text_p++;
					text_buffer[text_p] = 30; text_p++; /* Upper Shift */
					shift_set = text_shift[source[sp] - 128];
					value = text_value[source[sp] - 128];
				} else {
					shift_set = text_shift[source[sp]];
					value = text_value[source[sp]];
				}

				if(gs1 && (source[sp] == '[')) {
					shift_set = 2;
					value = 27; /* FNC1 */
				}

				if(shift_set != 0) {
					text_buffer[text_p] = shift_set - 1; text_p++;
				}
				text_buffer[text_p] = value; text_p++;

				if(text_p >= 3) {
					int iv;

					iv = (1600 * text_buffer[0]) + (40 * text_buffer[1]) + (text_buffer[2]) + 1;
					target[tp] = iv / 256; tp++;
					target[tp] = iv % 256; tp++;
					concat(binary, "  ");
					if (debug) printf("[%d %d %d] ", text_buffer[0], text_buffer[1], text_buffer[2]);

					text_buffer[0] = text_buffer[3];
					text_buffer[1] = text_buffer[4];
					text_buffer[2] = text_buffer[5];
					text_buffer[3] = 0;
					text_buffer[4] = 0;
					text_buffer[5] = 0;
					text_p -= 3;
				}
				sp++;
			}
		}

		/* step (e) X12 encodation */
		if(current_mode == DM_X12) {
			int value = 0;

			next_mode = DM_X12;
			if(text_p == 0) {
				next_mode = look_ahead_test(source, inputlen, sp, current_mode, gs1);
			}

			if(next_mode != DM_X12) {
				target[tp] = 254; tp++; concat(binary, " ");/* Unlatch */
				next_mode = DM_ASCII;
				if (debug) printf("ASC ");
			} else {
				if(source[sp] == 13) { value = 0; }
				if(source[sp] == '*') { value = 1; }
				if(source[sp] == '>') { value = 2; }
				if(source[sp] == ' ') { value = 3; }
				if((source[sp] >= '0') && (source[sp] <= '9')) { value = (source[sp] - '0') + 4; }
				if((source[sp] >= 'A') && (source[sp] <= 'Z')) { value = (source[sp] - 'A') + 14; }

				x12_buffer[x12_p] = value; x12_p++;

				if(x12_p >= 3) {
					int iv;

					iv = (1600 * x12_buffer[0]) + (40 * x12_buffer[1]) + (x12_buffer[2]) + 1;
					target[tp] = iv / 256; tp++;
					target[tp] = iv % 256; tp++;
					concat(binary, "  ");
					if (debug) printf("[%d %d %d] ", x12_buffer[0], x12_buffer[1], x12_buffer[2]);

					x12_buffer[0] = x12_buffer[3];
					x12_buffer[1] = x12_buffer[4];
					x12_buffer[2] = x12_buffer[5];
					x12_buffer[3] = 0;
					x12_buffer[4] = 0;
					x12_buffer[5] = 0;
					x12_p -= 3;
				}
				sp++;
			}
		}

		/* step (f) EDIFACT encodation */
		if(current_mode == DM_EDIFACT) {
			int value = 0;

			next_mode = DM_EDIFACT;
			if(edifact_p == 3) {
				next_mode = look_ahead_test(source, inputlen, sp, current_mode, gs1);
			}

			if(next_mode != DM_EDIFACT) {
				edifact_buffer[edifact_p] = 31; edifact_p++;
				next_mode = DM_ASCII;
			} else {
				if((source[sp] >= '@') && (source[sp] <= '^')) { value = source[sp] - '@'; }
				if((source[sp] >= ' ') && (source[sp] <= '?')) { value = source[sp]; }

				edifact_buffer[edifact_p] = value; edifact_p++;
				sp++;
			}

			if(edifact_p >= 4) {
				target[tp] = (edifact_buffer[0] << 2) + ((edifact_buffer[1] & 0x30) >> 4); tp++;
				target[tp] = ((edifact_buffer[1] & 0x0f) << 4) + ((edifact_buffer[2] & 0x3c) >> 2); tp++;
				target[tp] = ((edifact_buffer[2] & 0x03) << 6) + edifact_buffer[3]; tp++;
				concat(binary, "   ");
				if (debug) printf("[%d %d %d %d] ", edifact_buffer[0], edifact_buffer[1], edifact_buffer[2], edifact_buffer[3]);

				edifact_buffer[0] = edifact_buffer[4];
				edifact_buffer[1] = edifact_buffer[5];
				edifact_buffer[2] = edifact_buffer[6];
				edifact_buffer[3] = edifact_buffer[7];
				edifact_buffer[4] = 0;
				edifact_buffer[5] = 0;
				edifact_buffer[6] = 0;
				edifact_buffer[7] = 0;
				edifact_p -= 4;
			}
		}

		/* step (g) Base 256 encodation */
		if(current_mode == DM_BASE256) {
			next_mode = look_ahead_test(source, inputlen, sp, current_mode, gs1);

			if(next_mode == DM_BASE256) {
				target[tp] = source[sp];
				if(debug) printf("B%02X ", target[tp]);
				tp++;
				sp++;
				concat(binary, "b");
			} else {
				next_mode = DM_ASCII;
				if(debug) printf("ASC ");
			}
		}

		if(tp > 1558) {
			return 0;
		}

	} /* while */

	/* Empty buffers */
	if(c40_p == 2) {
		target[tp] = 254; tp++; /* unlatch */
		target[tp] = source[inputlen - 2] + 1; tp++;
		target[tp] = source[inputlen - 1] + 1; tp++;
		concat(binary, "   ");
		if(debug) printf("ASC A%02X A%02X ", target[tp - 2] - 1, target[tp - 1] - 1);
		current_mode = DM_ASCII;
	}
	if(c40_p == 1) {
		// don't unlatch before sending a single remaining ASCII character.
		target[tp] = source[inputlen - 1] + 1; tp++;
		concat(binary, "  ");
		if(debug) printf("ASC A%02X ", target[tp - 1] - 1);
		current_mode = DM_ASCII;
	}

	if(text_p == 2) {
		target[tp] = 254; tp++; /* unlatch */
		target[tp] = source[inputlen - 2] + 1; tp++;
		target[tp] = source[inputlen - 1] + 1; tp++;
		concat(binary, "   ");
		if(debug) printf("ASC A%02X A%02X ", target[tp - 2] - 1, target[tp - 1] - 1);
		current_mode = DM_ASCII;
	}
	if(text_p == 1) {
		target[tp] = 254; tp++; /* text encodation requires unlatch */
		target[tp] = source[inputlen - 1] + 1; tp++;
		concat(binary, "  ");
		if(debug) printf("ASC A%02X ", target[tp - 1] - 1);
		current_mode = DM_ASCII;
	}

	if(x12_p == 2) {
		target[tp] = 254; tp++; /* unlatch */
		target[tp] = source[inputlen - 2] + 1; tp++;
		target[tp] = source[inputlen - 1] + 1; tp++;
		concat(binary, "   ");
		if(debug) printf("ASC A%02X A%02X ", target[tp - 2] - 1, target[tp - 1] - 1);
		current_mode = DM_ASCII;
	}
	if(x12_p == 1) {
		// don't unlatch before sending a single remaining ASCII character.
		target[tp] = source[inputlen - 1] + 1; tp++;
		concat(binary, "  ");
		if(debug) printf("ASC A%02X ", target[tp - 1] - 1);
		current_mode = DM_ASCII;
	}

	/* Add length and randomising algorithm to b256 */
	i = 0;
	while (i < tp) {
		if(binary[i] == 'b') {
			if((i == 0) || ((i != 0) && (binary[i - 1] != 'b'))) {
				/* start of binary data */
				int binary_count; /* length of b256 data */

				for(binary_count = 0; binary[binary_count + i] == 'b'; binary_count++);

				if(binary_count <= 249) {
					dminsert(binary, i, 'b');
					insert_value(target, i, tp, binary_count); tp++;
				} else {
					dminsert(binary, i, 'b');
					dminsert(binary, i + 1, 'b');
					insert_value(target, i, tp, (binary_count / 250) + 249); tp++;
					insert_value(target, i + 1, tp, binary_count % 250); tp++;
				}
			}
		}
		i++;
	}

	for(i = 0; i < tp; i++) {
		if(binary[i] == 'b') {
			int prn, temp;

			prn = ((149 * (i + 1)) % 255) + 1;
			temp = target[i] + prn;
			if (temp <= 255) { target[i] = temp; } else { target[i] = temp - 256; }
		}
	}

	if(debug) {
		printf("\n\n");
		for(i = 0; i < tp; i++){
			printf("%02X ", target[i]);
		}
		printf("\n");
	}

	*(last_mode) = current_mode;
	return tp;
}

void add_tail(unsigned char target[], int tp, int tail_length, int last_mode)
{
	/* adds unlatch and pad bits */
	int i, prn, temp;

	switch(last_mode) {
		case DM_C40:
		case DM_TEXT:
		case DM_X12:
			target[tp] = 254; tp++; /* Unlatch */
			tail_length--;
	}

	for(i = tail_length; i > 0; i--) {
		if(i == tail_length) {
			target[tp] = 129; tp++; /* Pad */
		} else {
			prn = ((149 * (tp + 1)) % 253) + 1;
			temp = 129 + prn;
			if(temp <= 254) {
				target[tp] = temp; tp++;
			} else {
				target[tp] = temp - 254; tp++;
			}
		}
	}
}

int data_matrix_200(struct zint_symbol *symbol, unsigned char source[], int length)
{
	int           i, skew = 0;
	unsigned char binary[2200];
	int binlen;
	int symbolsize, optionsize, calcsize;
	int taillength, error_number = 0;
	int H, W, FH, FW, datablock, bytes, rsblock;
	int last_mode;
	unsigned char *grid = 0;

	binlen = dm200encode(symbol, source, binary, &last_mode, length);

	if(binlen == 0) {
		strcpy(symbol->errtxt, "Data too long to fit in symbol");
		return ZINT_ERROR_TOO_LONG;
	}

	if((symbol->option_2 >= 1) && (symbol->option_2 <= 30)) {
		optionsize = intsymbol[symbol->option_2 - 1];
	} else {
		optionsize = -1;
	}

	calcsize = 29;
	for(i = 29; i > -1; i--) {
		if(matrixbytes[i] >= binlen) {
			calcsize = i;
		}
	}

	if(symbol->option_3 == DM_SQUARE) {
		/* Force to use square symbol */
		switch(calcsize) {
			case 2:
			case 4:
			case 6:
			case 9:
			case 11:
			case 14:
				calcsize++;
		}
	}

	symbolsize = optionsize;
	if(calcsize > optionsize) {
		symbolsize = calcsize;
		if(optionsize != -1) {
			/* flag an error */
			error_number = WARN_INVALID_OPTION;
			strcpy(symbol->errtxt, "Data does not fit in selected symbol size");
		}
	}

	H = matrixH[symbolsize];
	W = matrixW[symbolsize];
	FH = matrixFH[symbolsize];
	FW = matrixFW[symbolsize];
	bytes = matrixbytes[symbolsize];
	datablock = matrixdatablock[symbolsize];
	rsblock = matrixrsblock[symbolsize];

	taillength = bytes - binlen;

	if(taillength != 0) {
		add_tail(binary, binlen, taillength, last_mode);
	}

	// ecc code
	if(symbolsize == 29) { skew = 1; }
	ecc200(binary, bytes, datablock, rsblock, skew);
	{			// placement
		int x, y, NC, NR, *places;
		NC = W - 2 * (W / FW);
		NR = H - 2 * (H / FH);
		places = (int*)malloc(NC * NR * sizeof(int));
		ecc200placement(places, NR, NC);
		grid = (unsigned char*)malloc(W * H);
		memset(grid, 0, W * H);
		for (y = 0; y < H; y += FH) {
			for (x = 0; x < W; x++)
				grid[y * W + x] = 1;
			for (x = 0; x < W; x += 2)
				grid[(y + FH - 1) * W + x] = 1;
		}
		for (x = 0; x < W; x += FW) {
			for (y = 0; y < H; y++)
				grid[y * W + x] = 1;
			for (y = 0; y < H; y += 2)
				grid[y * W + x + FW - 1] = 1;
		}
		for (y = 0; y < NR; y++) {
			for (x = 0; x < NC; x++) {
				int v = places[(NR - y - 1) * NC + x];
				//fprintf (stderr, "%4d", v);
				if (v == 1 || (v > 7 && (binary[(v >> 3) - 1] & (1 << (v & 7)))))
					grid[(1 + y + 2 * (y / (FH - 2))) * W + 1 + x + 2 * (x / (FW - 2))] = 1;
			}
			//fprintf (stderr, "\n");
		}
		for(y = H - 1; y >= 0; y--) {
			int x;
			for(x = 0; x < W; x++) {
				if(grid[W * y + x]) {
					set_module(symbol, (H - y) - 1, x);
				}
			}
			symbol->row_height[(H - y) - 1] = 1;
		}
		free(grid);
		free(places);
	}

	symbol->rows = H;
	symbol->width = W;

	return error_number;
}

int dmatrix(struct zint_symbol *symbol, unsigned char source[], int length)
{
	int error_number;

	if(symbol->option_1 <= 1) {
		/* ECC 200 */
		error_number = data_matrix_200(symbol, source, length);
	} else {
		/* ECC 000 - 140 */
		strcpy(symbol->errtxt, "Older Data Matrix standards are no longer supported");
		error_number = ZINT_ERROR_INVALID_OPTION;
	}

	return error_number;
}
