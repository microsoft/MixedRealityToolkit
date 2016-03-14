/* rss.c - Handles Reduced Space Symbology (GS1 DataBar) */

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

/* The functions "combins" and "getRSSwidths" are copyright BSI and are
   released with permission under the following terms:
   
   "Copyright subsists in all BSI publications. BSI also holds the copyright, in the
   UK, of the international standardisation bodies. Except as
   permitted under the Copyright, Designs and Patents Act 1988 no extract may be
   reproduced, stored in a retrieval system or transmitted in any form or by any
   means - electronic, photocopying, recording or otherwise - without prior written
   permission from BSI.
   
   "This does not preclude the free use, in the course of implementing the standard,
   of necessary details such as symbols, and size, type or grade designations. If these
   details are to be used for any other purpose than implementation then the prior
   written permission of BSI must be obtained."
   
   The date of publication for these functions is 30 November 2006
*/

/* Includes numerous bugfixes thanks to Pablo Orduña @ the PIRAmIDE project */
	
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _MSC_VER
#include <malloc.h> 
#endif
#include "common.h"
#include "large.h"
#include "rss.h"
#include "gs1.h"

/**********************************************************************
* combins(n,r): returns the number of Combinations of r selected from n:
*   Combinations = n! / ((n - r)! * r!)
**********************************************************************/
int combins(int n, int r) {
	int i, j;
	int maxDenom, minDenom;
	int val;
	
	if (n-r > r) {
		minDenom = r;
		maxDenom = n-r;
	}
	else {
		minDenom = n-r;
		maxDenom = r;
	}
	val = 1;
	j = 1;
	for (i = n; i > maxDenom; i--) {
		val *= i;
		if (j <= minDenom) {
			val /= j;
			j++;
		}
	}
	for (; j <= minDenom; j++) {
		val /= j;
	}
	return(val);
}

/**********************************************************************
* getRSSwidths
* routine to generate widths for RSS elements for a given value.#
*
* Calling arguments:
* val = required value
* n = number of modules
* elements = elements in a set (RSS-14 & Expanded = 4; RSS Limited = 7)
* maxWidth = maximum module width of an element
* noNarrow = 0 will skip patterns without a one module wide element
*
* Return:
* static int widths[] = element widths
**********************************************************************/
void getRSSwidths(int val, int n, int elements, int maxWidth, int noNarrow)
{
	int bar;
	int elmWidth;
	int mxwElement;
	int subVal, lessVal;
	int narrowMask = 0;
	for (bar = 0; bar < elements-1; bar++)
	{
		for(elmWidth = 1, narrowMask |= (1<<bar);
				  ;
				  elmWidth++, narrowMask &= ~(1<<bar))
		{
			/* get all combinations */
			subVal = combins(n-elmWidth-1, elements-bar-2);
			/* less combinations with no single-module element */
			if ((!noNarrow) && (!narrowMask) &&
						   (n-elmWidth-(elements-bar-1) >= elements-bar-1))
			{
				subVal -= combins(n-elmWidth-(elements-bar), elements-bar-2);
			}
			/* less combinations with elements > maxVal */
			if (elements-bar-1 > 1)
			{
				lessVal = 0;
				for (mxwElement = n-elmWidth-(elements-bar-2);
								 mxwElement > maxWidth;
								 mxwElement--)
				{
					lessVal += combins(n-elmWidth-mxwElement-1, elements-bar-3);
				}
				subVal -= lessVal * (elements-1-bar);
			}
			else if (n-elmWidth > maxWidth)
			{
				subVal--;
			}
			val -= subVal;
			if (val < 0) break;
		}
		val += subVal;
		n -= elmWidth;
		widths[bar] = elmWidth;
	}
	widths[bar] = n;
	return;
}

int rss14(struct zint_symbol *symbol, unsigned char source[], int src_len)
{ /* GS1 DataBar-14 */
	int error_number = 0, i, j, mask;
	short int accum[112], left_reg[112], right_reg[112], x_reg[112], y_reg[112];
	int data_character[4], data_group[4], v_odd[4], v_even[4];
	int data_widths[8][4], checksum, c_left, c_right, total_widths[46], writer;
	char latch, hrt[15], temp[32];
	int check_digit, count, separator_row;
	
	separator_row = 0;

	if(src_len > 13) {
		strcpy(symbol->errtxt, "Input too long");
		return ZINT_ERROR_TOO_LONG;
	}
	error_number = is_sane(NEON, source, src_len);
	if(error_number == ZINT_ERROR_INVALID_DATA) {
		strcpy(symbol->errtxt, "Invalid characters in data");
		return error_number;
	}
	
	/* make some room for a separator row for composite symbols */
	switch(symbol->symbology) {
		case BARCODE_RSS14_CC:
		case BARCODE_RSS14STACK_CC:
		case BARCODE_RSS14_OMNI_CC:
			separator_row = symbol->rows;
			symbol->row_height[separator_row] = 1;
			symbol->rows += 1;
			break;
	}
	
	for(i = 0; i < 112; i++) {
		accum[i] = 0;
		x_reg[i] = 0;
		y_reg[i] = 0;
	}
	
	for(i = 0; i < 4; i++) {
		data_character[i] = 0;
		data_group[i] = 0;
	}
	
	binary_load(accum, (char*)source, src_len);
	strcpy(temp, "10000000000000");
	if(symbol->option_1 == 2) {
		/* Add symbol linkage flag */
		binary_load(y_reg, temp, strlen(temp));
		binary_add(accum, y_reg);
		for(i = 0; i < 112; i++) {
			y_reg[i] = 0;
		}
	}
	
	/* Calculate left and right pair values */
	strcpy(temp, "4537077");
	binary_load(x_reg, temp, strlen(temp));
	
	for(i = 0; i < 24; i++) {
		shiftup(x_reg);
	}
	
	for(i = 24; i >= 0; i--) {
		y_reg[i] = islarger(accum, x_reg);
		if(y_reg[i] == 1) {
			binary_subtract(accum, x_reg);
		}
		shiftdown(x_reg);
	}
	
	for(i = 0; i < 112; i++) {
		left_reg[i] = y_reg[i];
		right_reg[i] = accum[i];
	}
	
	/* Calculate four data characters */
	strcpy(temp,  "1597");
	binary_load(x_reg, temp, strlen(temp));
	for(i = 0; i < 112; i++) {
		accum[i] = left_reg[i];
	}
	
	for(i = 0; i < 24; i++) {
		shiftup(x_reg);
	}
	
	for(i = 24; i >= 0; i--) {
		y_reg[i] = islarger(accum, x_reg);
		if(y_reg[i] == 1) {
			binary_subtract(accum, x_reg);
		}
		shiftdown(x_reg);
	}
	
	data_character[0] = 0;
	data_character[1] = 0;
	mask = 0x2000;
	for(i = 13; i >= 0; i--) {
		if(y_reg[i] == 1) {
			data_character[0] += mask;
		}
		if(accum[i] == 1) {
			data_character[1] += mask;
		}
		mask = mask >> 1;
	}
	strcpy(temp,  "1597");
	binary_load(x_reg, temp, strlen(temp));
	for(i = 0; i < 112; i++) {
		accum[i] = right_reg[i];
	}
	
	for(i = 0; i < 24; i++) {
		shiftup(x_reg);
	}
	
	for(i = 24; i >= 0; i--) {
		y_reg[i] = islarger(accum, x_reg);
		if(y_reg[i] == 1) {
			binary_subtract(accum, x_reg);
		}
		shiftdown(x_reg);
	}
	
	data_character[2] = 0;
	data_character[3] = 0;
	mask = 0x2000;
	for(i = 13; i >= 0; i--) {
		if(y_reg[i] == 1) {
			data_character[2] += mask;
		}
		if(accum[i] == 1) {
			data_character[3] += mask;
		}
		mask = mask >> 1;
	}
	
	/* Calculate odd and even subset values */
	
	if((data_character[0] >= 0) && (data_character[0] <= 160)) { data_group[0] = 0; }
	if((data_character[0] >= 161) && (data_character[0] <= 960)) { data_group[0] = 1; }
	if((data_character[0] >= 961) && (data_character[0] <= 2014)) { data_group[0] = 2; }
	if((data_character[0] >= 2015) && (data_character[0] <= 2714)) { data_group[0] = 3; }
	if((data_character[0] >= 2715) && (data_character[0] <= 2840)) { data_group[0] = 4; }
	if((data_character[1] >= 0) && (data_character[1] <= 335)) { data_group[1] = 5; }
	if((data_character[1] >= 336) && (data_character[1] <= 1035)) { data_group[1] = 6; }
	if((data_character[1] >= 1036) && (data_character[1] <= 1515)) { data_group[1] = 7; }
	if((data_character[1] >= 1516) && (data_character[1] <= 1596)) { data_group[1] = 8; }
	if((data_character[3] >= 0) && (data_character[3] <= 335)) { data_group[3] = 5; }
	if((data_character[3] >= 336) && (data_character[3] <= 1035)) { data_group[3] = 6; }
	if((data_character[3] >= 1036) && (data_character[3] <= 1515)) { data_group[3] = 7; }
	if((data_character[3] >= 1516) && (data_character[3] <= 1596)) { data_group[3] = 8; }
	if((data_character[2] >= 0) && (data_character[2] <= 160)) { data_group[2] = 0; }
	if((data_character[2] >= 161) && (data_character[2] <= 960)) { data_group[2] = 1; }
	if((data_character[2] >= 961) && (data_character[2] <= 2014)) { data_group[2] = 2; }
	if((data_character[2] >= 2015) && (data_character[2] <= 2714)) { data_group[2] = 3; }
	if((data_character[2] >= 2715) && (data_character[2] <= 2840)) { data_group[2] = 4; }
	
	v_odd[0] = (data_character[0] - g_sum_table[data_group[0]]) / t_table[data_group[0]];
	v_even[0] = (data_character[0] - g_sum_table[data_group[0]]) % t_table[data_group[0]];
	v_odd[1] = (data_character[1] - g_sum_table[data_group[1]]) % t_table[data_group[1]];
	v_even[1] = (data_character[1] - g_sum_table[data_group[1]]) / t_table[data_group[1]];
	v_odd[3] = (data_character[3] - g_sum_table[data_group[3]]) % t_table[data_group[3]];
	v_even[3] = (data_character[3] - g_sum_table[data_group[3]]) / t_table[data_group[3]];
	v_odd[2] = (data_character[2] - g_sum_table[data_group[2]]) / t_table[data_group[2]];
	v_even[2] = (data_character[2] - g_sum_table[data_group[2]]) % t_table[data_group[2]];

	
	/* Use RSS subset width algorithm */
	for(i = 0; i < 4; i++) {
		if((i == 0)||(i == 2)) {
			getRSSwidths(v_odd[i], modules_odd[data_group[i]], 4, widest_odd[data_group[i]], 1);
			data_widths[0][i] = widths[0];
			data_widths[2][i] = widths[1];
			data_widths[4][i] = widths[2];
			data_widths[6][i] = widths[3];
			getRSSwidths(v_even[i], modules_even[data_group[i]], 4, widest_even[data_group[i]], 0);
			data_widths[1][i] = widths[0];
			data_widths[3][i] = widths[1];
			data_widths[5][i] = widths[2];
			data_widths[7][i] = widths[3];
		} else {
			getRSSwidths(v_odd[i], modules_odd[data_group[i]], 4, widest_odd[data_group[i]], 0);
			data_widths[0][i] = widths[0];
			data_widths[2][i] = widths[1];
			data_widths[4][i] = widths[2];
			data_widths[6][i] = widths[3];
			getRSSwidths(v_even[i], modules_even[data_group[i]], 4, widest_even[data_group[i]], 1);
			data_widths[1][i] = widths[0];
			data_widths[3][i] = widths[1];
			data_widths[5][i] = widths[2];
			data_widths[7][i] = widths[3];
		}
	}
	
	
	checksum = 0;
	/* Calculate the checksum */
	for(i = 0; i < 8; i++) {
		checksum += checksum_weight[i] * data_widths[i][0];
		checksum += checksum_weight[i+8] * data_widths[i][1];
		checksum += checksum_weight[i+16] * data_widths[i][2];
		checksum += checksum_weight[i+24] * data_widths[i][3];
	}
	checksum %= 79;
	
	/* Calculate the two check characters */
	if(checksum >= 8) { checksum++; }
	if(checksum >= 72) { checksum++; }
	c_left = checksum / 9;
	c_right = checksum % 9;
	
	/* Put element widths together */
	total_widths[0] = 1;
	total_widths[1] = 1;
	total_widths[44] = 1;
	total_widths[45] = 1;
	for(i = 0; i < 8; i++) {
		total_widths[i + 2] = data_widths[i][0];
		total_widths[i + 15] = data_widths[7 - i][1];
		total_widths[i + 23] = data_widths[i][3];
		total_widths[i + 36] = data_widths[7 - i][2];
	}
	for(i = 0; i < 5; i++) {
		total_widths[i + 10] = finder_pattern[i + (5 * c_left)];
		total_widths[i + 31] = finder_pattern[(4 - i) + (5 * c_right)];
	}
	
	/* Put this data into the symbol */
	if((symbol->symbology == BARCODE_RSS14) || (symbol->symbology == BARCODE_RSS14_CC)) {
		writer = 0;
		latch = '0';
		for(i = 0; i < 46; i++) {
			for(j = 0; j < total_widths[i]; j++) {
				if(latch == '1') { set_module(symbol, symbol->rows, writer); }
				writer++;
			}
			if(latch == '1') {
				latch = '0';
			} else {
				latch = '1';
			}
		}
		if(symbol->width < writer) { symbol->width = writer; }
		if(symbol->symbology == BARCODE_RSS14_CC) {
			/* separator pattern for composite symbol */
			for(i = 4; i < 92; i++) {
				if(!(module_is_set(symbol, separator_row + 1, i))) {
					set_module(symbol, separator_row, i); }
			}
			latch = '1';
			for(i = 16; i < 32; i++) {
				if(!(module_is_set(symbol, separator_row + 1, i))) {
					if(latch == '1') {
						set_module(symbol, separator_row, i);
						latch = '0';
					} else {
						unset_module(symbol, separator_row, i);
						latch = '1';
					}
				} else {
					unset_module(symbol, separator_row, i);
					latch = '1';
				}
			}
			latch = '1';
			for(i = 63; i < 78; i++) {
				if(!(module_is_set(symbol, separator_row + 1, i))) {
					if(latch == '1') {
						set_module(symbol, separator_row, i);
						latch = '0';
					} else {
						unset_module(symbol, separator_row, i);
						latch = '1';
					}
				} else {
					unset_module(symbol, separator_row, i);
					latch = '1';
				}
			}
		}
		symbol->rows = symbol->rows + 1;
		
		count = 0;
		check_digit = 0;
		
		/* Calculate check digit from Annex A and place human readable text */
		ustrcpy(symbol->text, (unsigned char*)"(01)");
		for(i = 0; i < 14; i++) {
			hrt[i] = '0';
		}
		for(i = 0; i < src_len; i++) {
			hrt[12 - i] = source[src_len - i - 1];
		}
		hrt[14] = '\0';
	
		for (i = 0; i < 13; i++) {
			count += ctoi(hrt[i]);

			if (!(i & 1)) {
				count += 2 * (ctoi(hrt[i]));
			}
		}

		check_digit = 10 - (count%10);
		if (check_digit == 10) { check_digit = 0; }
		hrt[13] = itoc(check_digit);
	
		uconcat(symbol->text, (unsigned char*)hrt);
	}
	
	if((symbol->symbology == BARCODE_RSS14STACK) || (symbol->symbology == BARCODE_RSS14STACK_CC)) {
		/* top row */
		writer = 0;
		latch = '0';
		for(i = 0; i < 23; i++) {
			for(j = 0; j < total_widths[i]; j++) {
				if(latch == '1') {
					set_module(symbol, symbol->rows, writer);
				} else {
					unset_module(symbol, symbol->rows, writer);
				}
				writer++;
			}
			if(latch == '1') {
				latch = '0';
			} else {
				latch = '1';
			}
		}
		set_module(symbol, symbol->rows, writer);
		unset_module(symbol, symbol->rows, writer + 1);
		symbol->row_height[symbol->rows] = 5;
		/* bottom row */
		symbol->rows = symbol->rows + 2;
		set_module(symbol, symbol->rows, 0);
		unset_module(symbol, symbol->rows, 1);
		writer = 0;
		latch = '1';
		for(i = 23; i < 46; i++) {
			for(j = 0; j < total_widths[i]; j++) {
				if(latch == '1') {
					set_module(symbol, symbol->rows, writer + 2);
				} else {
					unset_module(symbol, symbol->rows, writer + 2);
				}
				writer++;
			}
			if(latch == '1') {
				latch = '0';
			} else {
				latch = '1';
			}
		}
		symbol->row_height[symbol->rows] = 7;
		/* separator pattern */
		for(i = 4; i < 46; i++) {
			if(module_is_set(symbol, symbol->rows - 2, i) == module_is_set(symbol, symbol->rows, i)) {
				if(!(module_is_set(symbol, symbol->rows - 2, i))) {
					set_module(symbol, symbol->rows - 1, i);
				}
			} else {
				if(!(module_is_set(symbol, symbol->rows - 1, i - 1))) {
					set_module(symbol, symbol->rows - 1, i);
				}
			}
		}
		symbol->row_height[symbol->rows - 1] = 1;
		if(symbol->symbology == BARCODE_RSS14STACK_CC) {
			/* separator pattern for composite symbol */
			for(i = 4; i < 46; i++) {
				if(!(module_is_set(symbol, separator_row + 1, i))) {
					set_module(symbol, separator_row, i);
				}
			}
			latch = '1';
			for(i = 16; i < 32; i++) {
				if(!(module_is_set(symbol, separator_row + 1, i))) {
					if(latch == '1') {
						set_module(symbol, separator_row, i);
						latch = '0';
					} else {
						unset_module(symbol, separator_row, i);
						latch = '1';
					}
				} else {
					unset_module(symbol, separator_row, i);
					latch = '1';
				}
			}
		}
		symbol->rows = symbol->rows + 1;
		if(symbol->width < 50) { symbol->width = 50; }
	}

	if((symbol->symbology == BARCODE_RSS14STACK_OMNI) || (symbol->symbology == BARCODE_RSS14_OMNI_CC)) {
		/* top row */
		writer = 0;
		latch = '0';
		for(i = 0; i < 23; i++) {
			for(j = 0; j < total_widths[i]; j++) {
				if(latch == '1') { set_module(symbol, symbol->rows, writer); } else { unset_module(symbol, symbol->rows, writer); }
				writer++;
			}
			latch = (latch == '1' ? '0' : '1');
		}
		set_module(symbol, symbol->rows, writer);
		unset_module(symbol, symbol->rows, writer + 1);
		/* bottom row */
		symbol->rows = symbol->rows + 4;
		set_module(symbol, symbol->rows, 0);
		unset_module(symbol, symbol->rows, 1);
		writer = 0;
		latch = '1';
		for(i = 23; i < 46; i++) {
			for(j = 0; j < total_widths[i]; j++) {
				if(latch == '1') { set_module(symbol, symbol->rows, writer + 2); } else { unset_module(symbol, symbol->rows, writer + 2); }
				writer++;
			}
			if(latch == '1') {
				latch = '0';
			} else {
				latch = '1';
			}
		}
		/* middle separator */
		for(i = 5; i < 46; i += 2) {
			set_module(symbol, symbol->rows - 2, i);
		}
		symbol->row_height[symbol->rows - 2] = 1;
		/* top separator */
		for(i = 4; i < 46; i++) {
			if(!(module_is_set(symbol, symbol->rows - 4, i))) {
				set_module(symbol, symbol->rows - 3, i);
			}
		}
		latch = '1';
		for(i = 17; i < 33; i++) {
			if(!(module_is_set(symbol, symbol->rows - 4, i))) {
				if(latch == '1') {
					set_module(symbol, symbol->rows - 3, i);
					latch = '0';
				} else {
					unset_module(symbol, symbol->rows - 3, i);
					latch = '1';
				}
			} else {
				unset_module(symbol, symbol->rows - 3, i);
				latch = '1';
			}
		}
		symbol->row_height[symbol->rows - 3] = 1;
		/* bottom separator */
		for(i = 4; i < 46; i++) {
			if(!(module_is_set(symbol, symbol->rows, i))) {
				set_module(symbol, symbol->rows - 1, i);
			}
		}
		latch = '1';
		for(i = 16; i < 32; i++) {
			if(!(module_is_set(symbol, symbol->rows, i))) {
				if(latch == '1') {
					set_module(symbol, symbol->rows - 1, i);
					latch = '0';
				} else {
					unset_module(symbol, symbol->rows - 1, i);
					latch = '1';
				}
			} else {
				unset_module(symbol, symbol->rows - 1, i);
				latch = '1';
			}
		}
		symbol->row_height[symbol->rows - 1] = 1;
		if(symbol->width < 50) { symbol->width = 50; }
		if(symbol->symbology == BARCODE_RSS14_OMNI_CC) {
			/* separator pattern for composite symbol */
			for(i = 4; i < 46; i++) {
				if(!(module_is_set(symbol, separator_row + 1, i))) {
					set_module(symbol, separator_row, i);
				}
			}
			latch = '1';
			for(i = 16; i < 32; i++) {
				if(!(module_is_set(symbol, separator_row + 1, i))) {
					if(latch == '1') {
						set_module(symbol, separator_row, i);
						latch = '0';
					} else {
						unset_module(symbol, separator_row, i);
						latch = '1';
					}
				} else {
					unset_module(symbol, separator_row, i);
					latch = '1';
				}
			}
		}
		symbol->rows = symbol->rows + 1;
	}

	
	return error_number;
}

int rsslimited(struct zint_symbol *symbol, unsigned char source[], int src_len)
{ /* GS1 DataBar Limited */
	int error_number = 0, i, mask;
	short int accum[112], left_reg[112], right_reg[112], x_reg[112], y_reg[112];
	int left_group, right_group, left_odd, left_even, right_odd, right_even;
	int left_character, right_character, left_widths[14], right_widths[14];
	int checksum, check_elements[14], total_widths[46], writer, j, check_digit, count;
	char latch, hrt[15], temp[32];
	int separator_row;

	separator_row = 0;
	
	if(src_len > 13) {
		strcpy(symbol->errtxt, "Input too long");
		return ZINT_ERROR_TOO_LONG;
	}
	error_number = is_sane(NEON, source, src_len);
	if(error_number == ZINT_ERROR_INVALID_DATA) {
		strcpy(symbol->errtxt, "Invalid characters in data");
		return error_number;
	}
	if(src_len == 13) {
		if((source[0] != '0') && (source[0] != '1')) {
			strcpy(symbol->errtxt, "Input out of range");
			return ZINT_ERROR_INVALID_DATA;
		}
	}
	
	/* make some room for a separator row for composite symbols */
	if(symbol->symbology == BARCODE_RSS_LTD_CC) {
		separator_row = symbol->rows;
		symbol->row_height[separator_row] = 1;
		symbol->rows += 1;
	}

	for(i = 0; i < 112; i++) {
		accum[i] = 0;
		x_reg[i] = 0;
		y_reg[i] = 0;
	}
	
	binary_load(accum, (char*)source, src_len);
	if(symbol->option_1 == 2) {
		/* Add symbol linkage flag */
		strcpy(temp, "2015133531096");
		binary_load(y_reg, temp, strlen(temp));
		binary_add(accum, y_reg);
		for(i = 0; i < 112; i++) {
			y_reg[i] = 0;
		}
	}
	
	/* Calculate left and right pair values */
	strcpy(temp, "2013571");
	binary_load(x_reg, temp, strlen(temp));
	
	for(i = 0; i < 24; i++) {
		shiftup(x_reg);
	}
	
	for(i = 24; i >= 0; i--) {
		y_reg[i] = islarger(accum, x_reg);
		if(y_reg[i] == 1) {
			binary_subtract(accum, x_reg);
		}
		shiftdown(x_reg);
	}
	
	for(i = 0; i < 112; i++) {
		left_reg[i] = y_reg[i];
		right_reg[i] = accum[i];
	}
	
	left_group = 0;
	strcpy(temp, "183063");
	binary_load(accum, temp, strlen(temp));
	if(islarger(left_reg, accum)) { left_group = 1; }
	strcpy(temp, "820063");
	binary_load(accum, temp, strlen(temp));
	if(islarger(left_reg, accum)) { left_group = 2; }
	strcpy(temp, "1000775");
	binary_load(accum, temp, strlen(temp));
	if(islarger(left_reg, accum)) { left_group = 3; }
	strcpy(temp, "1491020");
	binary_load(accum, temp, strlen(temp));
	if(islarger(left_reg, accum)) { left_group = 4; }
	strcpy(temp, "1979844");
	binary_load(accum, temp, strlen(temp));
	if(islarger(left_reg, accum)) { left_group = 5; }
	strcpy(temp, "1996938");
	binary_load(accum, temp, strlen(temp));
	if(islarger(left_reg, accum)) { left_group = 6; }
	right_group = 0;
	strcpy(temp, "183063");
	binary_load(accum, temp, strlen(temp));
	if(islarger(right_reg, accum)) { right_group = 1; }
	strcpy(temp, "820063");
	binary_load(accum, temp, strlen(temp));
	if(islarger(right_reg, accum)) { right_group = 2; }
	strcpy(temp, "1000775");
	binary_load(accum, temp, strlen(temp));
	if(islarger(right_reg, accum)) { right_group = 3; }
	strcpy(temp, "1491020");
	binary_load(accum, temp, strlen(temp));
	if(islarger(right_reg, accum)) { right_group = 4; }
	strcpy(temp, "1979844");
	binary_load(accum, temp, strlen(temp));
	if(islarger(right_reg, accum)) { right_group = 5; }
	strcpy(temp, "1996938");
	binary_load(accum, temp, strlen(temp));
	if(islarger(right_reg, accum)) { right_group = 6; }
	
	switch(left_group) {
		case 1: strcpy(temp, "183064");
			binary_load(accum, temp, strlen(temp));
			binary_subtract(left_reg, accum);
			break;
		case 2: strcpy(temp, "820064");
			binary_load(accum, temp, strlen(temp));
			binary_subtract(left_reg, accum);
			break;
		case 3: strcpy(temp, "1000776");
			binary_load(accum, temp, strlen(temp));
			binary_subtract(left_reg, accum);
			break;
		case 4: strcpy(temp, "1491021");
			binary_load(accum, temp, strlen(temp));
			binary_subtract(left_reg, accum);
			break;
		case 5: strcpy(temp, "1979845");
			binary_load(accum, temp, strlen(temp));
			binary_subtract(left_reg, accum);
			break;
		case 6: strcpy(temp, "1996939");
			binary_load(accum, temp, strlen(temp));
			binary_subtract(left_reg, accum);
			break;
	}
	
	switch(right_group) {
		case 1: strcpy(temp, "183064");
			binary_load(accum, temp, strlen(temp));
			binary_subtract(right_reg, accum);
			break;
		case 2: strcpy(temp, "820064");
			binary_load(accum, temp, strlen(temp));
			binary_subtract(right_reg, accum);
			break;
		case 3: strcpy(temp, "1000776");
			binary_load(accum, temp, strlen(temp));
			binary_subtract(right_reg, accum);
			break;
		case 4: strcpy(temp, "1491021");
			binary_load(accum, temp, strlen(temp));
			binary_subtract(right_reg, accum);
			break;
		case 5: strcpy(temp, "1979845");
			binary_load(accum, temp, strlen(temp));
			binary_subtract(right_reg, accum);
			break;
		case 6: strcpy(temp, "1996939");
			binary_load(accum, temp, strlen(temp));
			binary_subtract(right_reg, accum);
			break;
	}

	left_character = 0;
	right_character = 0;
	mask = 0x800000;
	for(i = 23; i >= 0; i--) {
		if(left_reg[i] == 1) {
			left_character += mask;
		}
		if(right_reg[i] == 1) {
			right_character += mask;
		}
		mask = mask >> 1;
	}
	
	left_odd = left_character / t_even_ltd[left_group];
	left_even = left_character % t_even_ltd[left_group];
	right_odd = right_character / t_even_ltd[right_group];
	right_even = right_character % t_even_ltd[right_group];
	
	getRSSwidths(left_odd, modules_odd_ltd[left_group], 7, widest_odd_ltd[left_group], 1);
	left_widths[0] = widths[0];
	left_widths[2] = widths[1];
	left_widths[4] = widths[2];
	left_widths[6] = widths[3];
	left_widths[8] = widths[4];
	left_widths[10] = widths[5];
	left_widths[12] = widths[6];
	getRSSwidths(left_even, modules_even_ltd[left_group], 7, widest_even_ltd[left_group], 0);
	left_widths[1] = widths[0];
	left_widths[3] = widths[1];
	left_widths[5] = widths[2];
	left_widths[7] = widths[3];
	left_widths[9] = widths[4];
	left_widths[11] = widths[5];
	left_widths[13] = widths[6];
	getRSSwidths(right_odd, modules_odd_ltd[right_group], 7, widest_odd_ltd[right_group], 1);
	right_widths[0] = widths[0];
	right_widths[2] = widths[1];
	right_widths[4] = widths[2];
	right_widths[6] = widths[3];
	right_widths[8] = widths[4];
	right_widths[10] = widths[5];
	right_widths[12] = widths[6];
	getRSSwidths(right_even, modules_even_ltd[right_group], 7, widest_even_ltd[right_group], 0);
	right_widths[1] = widths[0];
	right_widths[3] = widths[1];
	right_widths[5] = widths[2];
	right_widths[7] = widths[3];
	right_widths[9] = widths[4];
	right_widths[11] = widths[5];
	right_widths[13] = widths[6];
	
	checksum = 0;
	/* Calculate the checksum */
	for(i = 0; i < 14; i++) {
		checksum += checksum_weight_ltd[i] * left_widths[i];
		checksum += checksum_weight_ltd[i + 14] * right_widths[i];
	}
	checksum %= 89;
	
	for(i = 0; i < 14; i++) {
		check_elements[i] = finder_pattern_ltd[i + (checksum * 14)];
	}
	
	total_widths[0] = 1;
	total_widths[1] = 1;
	total_widths[44] = 1;
	total_widths[45] = 1;
	for(i = 0; i < 14; i++) {
		total_widths[i + 2] = left_widths[i];
		total_widths[i + 16] = check_elements[i];
		total_widths[i + 30] = right_widths[i];
	}
	
	writer = 0;
	latch = '0';
	for(i = 0; i < 46; i++) {
		for(j = 0; j < total_widths[i]; j++) {
			if(latch == '1') { set_module(symbol, symbol->rows, writer); } else { unset_module(symbol, symbol->rows, writer); }
			writer++;
		}
		latch = (latch == '1' ? '0' : '1');
	}
	if(symbol->width < writer) { symbol->width = writer; }
	symbol->rows = symbol->rows + 1;
	
	/* add separator pattern if composite symbol */
	if(symbol->symbology == BARCODE_RSS_LTD_CC) {
		for(i = 4; i < 70; i++) {
				if(!(module_is_set(symbol, separator_row + 1, i))) {
					set_module(symbol, separator_row, i);
				}
			}
	}

	/* Calculate check digit from Annex A and place human readable text */
	
	check_digit = 0;
	count = 0;
	
	ustrcpy(symbol->text, (unsigned char*)"(01)");
	for(i = 0; i < 14; i++) {
		hrt[i] = '0';
	}
	for(i = 0; i < src_len; i++) {
		hrt[12 - i] = source[src_len - i - 1];
	}
	
	for (i = 0; i < 13; i++) {
		count += ctoi(hrt[i]);

		if (!(i & 1)) {
			count += 2 * (ctoi(hrt[i]));
		}
	}

	check_digit = 10 - (count%10);
	if (check_digit == 10) { check_digit = 0; }
	
	hrt[13] = itoc(check_digit);
	hrt[14] = '\0';
	
	uconcat(symbol->text, (unsigned char*)hrt);
	
	return error_number;
}

int general_rules(char field[], char type[])
{ /* Attempts to apply encoding rules from secions 7.2.5.5.1 to 7.2.5.5.3
	of ISO/IEC 24724:2006 */
	int block[2][200], block_count, i, j, k;
	char current, next, last;

	block_count = 0;
	
	block[0][block_count] = 1;
	block[1][block_count] = type[0];
	
	for(i = 1; i < strlen(type); i++) {
		current = type[i];
		last = type[i - 1];
		
		if(current == last) {
			block[0][block_count] = block[0][block_count] + 1;
		} else {
			block_count++;
			block[0][block_count] = 1;
			block[1][block_count] = type[i];
		}
	}
	
	block_count++;
	
	for(i = 0; i < block_count; i++) {
	}
	
	for(i = 0; i < block_count; i++) {
		current = block[1][i];
		next = (block[1][i + 1] & 0xFF);
		
		if((current == ISOIEC) && (i != (block_count - 1))) {
			if((next == ANY_ENC) && (block[0][i + 1] >= 4)) {
				block[1][i + 1] = NUMERIC;
			}
			if((next == ANY_ENC) && (block[0][i + 1] < 4)) {
				block[1][i + 1] = ISOIEC;
			}
			if((next == ALPHA_OR_ISO) && (block[0][i + 1] >= 5)) {
				block[1][i + 1] = ALPHA;
			}
			if((next == ALPHA_OR_ISO) && (block[0][i + 1] < 5)) {
				block[1][i + 1] = ISOIEC;
			}
		}
		
		if(current == ALPHA_OR_ISO) {
			block[1][i] = ALPHA;
		}
		
		if((current == ALPHA) && (i != (block_count - 1))) {
			if((next == ANY_ENC) && (block[0][i + 1] >= 6)) {
				block[1][i + 1] = NUMERIC;
			}
			if((next == ANY_ENC) && (block[0][i + 1] < 6)) {
				if((i == block_count - 2) && (block[0][i + 1] >= 4)) {
					block[1][i + 1] = NUMERIC;
				} else {
					block[1][i + 1] = ALPHA;
				}
			}
		}
		
		if(current == ANY_ENC) {
			block[1][i] = NUMERIC;
		}
	}
	
	if(block_count > 1) {
		i = 1;
		while(i < block_count) {
			if(block[1][i - 1] == block[1][i]) {
				/* bring together */
				block[0][i - 1] = block[0][i - 1] + block[0][i];
				j = i + 1;
				
				/* decreace the list */
				while(j < block_count) {
					block[0][j - 1] = block[0][j];
					block[1][j - 1] = block[1][j];
					j++;
				}
				block_count--;
				i--;
			}
			i++;
		}
	}

	for(i = 0; i < block_count - 1; i++) {
		if((block[1][i] == NUMERIC) && (block[0][i] & 1)) {
			/* Odd size numeric block */
			block[0][i] = block[0][i] - 1;
			block[0][i + 1] = block[0][i + 1] + 1;
		}
	}
	
	j = 0;
	for(i = 0; i < block_count; i++) {
		for(k = 0; k < block[0][i]; k++) {
			type[j] = block[1][i];
			j++;
		}
	}
	
	if((block[1][block_count - 1] == NUMERIC) && (block[0][block_count - 1] & 1)) {
		/* If the last block is numeric and an odd size, further
		processing needs to be done outside this procedure */
		return 1;
	} else {
		return 0;
	}
}

int rss_binary_string(struct zint_symbol *symbol, char source[], char binary_string[])
{ /* Handles all data encodation from section 7.2.5 of ISO/IEC 24724 */
	int encoding_method, i, mask, j, read_posn, latch, debug = 0, last_mode = ISOIEC;
#ifndef _MSC_VER
	char general_field[strlen(source)], general_field_type[strlen(source)];
#else
	char* general_field = (char*)_alloca(strlen(source));
        char* general_field_type = (char*)_alloca(strlen(source));
#endif
	int remainder, d1, d2, value;
	char padstring[40];

	read_posn=0;
	value=0;

	/* Decide whether a compressed data field is required and if so what
	method to use - method 2 = no compressed data field */
	
	if((strlen(source) >= 16) && ((source[0] == '0') && (source[1] == '1'))) {
		/* (01) and other AIs */
		encoding_method = 1;
		if(debug) printf("Choosing Method 1\n");
	} else {
		/* any AIs */
		encoding_method = 2;
		if(debug) printf("Choosing Mehod 2\n");
	}
	
	if(((strlen(source) >= 20) && (encoding_method == 1)) && ((source[2] == '9') && (source[16] == '3'))) {
		/* Possibly encoding method > 2 */
		if(debug) printf("Checking for other methods\n");
		
		if((strlen(source) >= 26) && (source[17] == '1')) {
			/* Methods 3, 7, 9, 11 and 13 */
			
			if(source[18] == '0') {
				/* (01) and (310x) */
				char weight_str[7];
				float weight; /* In kilos */
			
				for(i = 0; i < 6; i++) {
					weight_str[i] = source[20 + i];
				}
				weight_str[6] = '\0';
				
				if (weight_str[0] == '0') { /* Maximum weight = 99999 */


					encoding_method = 7;
					
					if((source[19] == '3') && (strlen(source) == 26)) {
						/* (01) and (3103) */
						weight = atof(weight_str) / 1000.0;
						
						if(weight <= 32.767) { encoding_method = 3; }
					}
					
					if(strlen(source) == 34){
						if((source[26] == '1') && (source[27] == '1')) {
							/* (01), (310x) and (11) - metric weight and production date */
							encoding_method = 7;
						}
						
						if((source[26] == '1') && (source[27] == '3')) {
							/* (01), (310x) and (13) - metric weight and packaging date */
							encoding_method = 9;
						}
						
						if((source[26] == '1') && (source[27] == '5')) {
							/* (01), (310x) and (15) - metric weight and "best before" date */
							encoding_method = 11;
						}
						
						if((source[26] == '1') && (source[27] == '7')) {
							/* (01), (310x) and (17) - metric weight and expiration date */
							encoding_method = 13;
						}
					}
				}
			}
			if(debug) printf("Now using method %d\n", encoding_method);
		}
		
		if((strlen(source) >= 26) && (source[17] == '2')) {
			/* Methods 4, 8, 10, 12 and 14 */
			
			if(source[18] == '0') {
				/* (01) and (320x) */
				char weight_str[7];
				float weight; /* In pounds */
			
				for(i = 0; i < 6; i++) {
					weight_str[i] = source[20 + i];
				}
				weight_str[6] = '\0';
				
				if (weight_str[0] == '0') { /* Maximum weight = 99999 */

					encoding_method = 8;
					
					if(((source[19] == '2') || (source[19] == '3')) && (strlen(source) == 26)) {
						/* (01) and (3202)/(3203) */
						
						if(source[19] == '3') {
							weight = atof(weight_str) / 1000.0;
							if(weight <= 22.767) {
								encoding_method = 4;
							}
						} else {
							weight = atof(weight_str) / 100.0;
							if(weight <= 99.99) {
								encoding_method = 4;
							}
						}
						
					}
					
					if(strlen(source) == 34){
						if((source[26] == '1') && (source[27] == '1')) {
							/* (01), (320x) and (11) - English weight and production date */
							encoding_method = 8;
						}
						
						if((source[26] == '1') && (source[27] == '3')) {
							/* (01), (320x) and (13) - English weight and packaging date */
							encoding_method = 10;
						}
						
						if((source[26] == '1') && (source[27] == '5')) {
							/* (01), (320x) and (15) - English weight and "best before" date */
							encoding_method = 12;
						}
						
						if((source[26] == '1') && (source[27] == '7')) {
							/* (01), (320x) and (17) - English weight and expiration date */
							encoding_method = 14;
						}
					}
				}
			}
			if(debug) printf("Now using method %d\n", encoding_method);

		}
		
		if(source[17] == '9') {
			/* Methods 5 and 6 */
			if((source[18] == '2') && ((source[19] >= '0') && (source[19] <= '3'))) {
				/* (01) and (392x) */
				encoding_method = 5;
			}
			if((source[18] == '3') && ((source[19] >= '0') && (source[19] <= '3'))) {
				/* (01) and (393x) */
				encoding_method = 6;
			}
			if(debug) printf("Now using method %d\n", encoding_method);
		}
	}
	
	switch(encoding_method) { /* Encoding method - Table 10 */
		case 1: concat(binary_string, "1XX"); read_posn = 16; break;
		case 2: concat(binary_string, "00XX"); read_posn = 0; break;
		case 3: concat(binary_string, "0100"); read_posn = strlen(source); break;
		case 4: concat(binary_string, "0101"); read_posn = strlen(source); break;
		case 5: concat(binary_string, "01100XX"); read_posn = 20; break;
		case 6: concat(binary_string, "01101XX"); read_posn = 23; break;
		case 7: concat(binary_string, "0111000"); read_posn = strlen(source); break;
		case 8: concat(binary_string, "0111001"); read_posn = strlen(source); break;
		case 9: concat(binary_string, "0111010"); read_posn = strlen(source); break;
		case 10: concat(binary_string, "0111011"); read_posn = strlen(source); break;
		case 11: concat(binary_string, "0111100"); read_posn = strlen(source); break;
		case 12: concat(binary_string, "0111101"); read_posn = strlen(source); break;
		case 13: concat(binary_string, "0111110"); read_posn = strlen(source); break;
		case 14: concat(binary_string, "0111111"); read_posn = strlen(source); break;
	}
	if(debug) printf("Setting binary = %s\n", binary_string);
	
	/* Variable length symbol bit field is just given a place holder (XX)
	for the time being */
	
	/* Verify that the data to be placed in the compressed data field is all
	numeric data before carrying out compression */
	for(i = 0; i < read_posn; i++) {
		if((source[i] < '0') || (source[i] > '9')) {
			if((source[i] != '[') && (source[i] != ']')) {
				/* Something is wrong */
				strcpy(symbol->errtxt, "Invalid characters in input data");
				return ZINT_ERROR_INVALID_DATA;
			}
		}
	}
	
	/* Now encode the compressed data field */
	
	if(debug) printf("Proceeding to encode data\n");
	if(encoding_method == 1) {
		/* Encoding method field "1" - general item identification data */
		char group[4];
		int group_val;
		
		group[0] = source[2];
		group[1] = '\0';
		group_val = atoi(group);
		
		mask = 0x08;
		for(j = 0; j < 4; j++) {
			concat(binary_string, (group_val & mask) ? "1" : "0");
			mask = mask >> 1;
		}
		
		for(i = 1; i < 5; i++) {
			group[0] = source[(i * 3)];
			group[1] = source[(i * 3) + 1];
			group[2] = source[(i * 3) + 2];
			group[3] = '\0';
			group_val = atoi(group);
			
			mask = 0x200;
			for(j = 0; j < 10; j++) {
				concat(binary_string, (group_val & mask) ? "1" : "0");
				mask = mask >> 1;
			}
		}
		
		
	}
	
	
	if(encoding_method == 3) {
		/* Encoding method field "0100" - variable weight item
		(0,001 kilogram icrements) */
		char group[4];
		int group_val;
		char weight_str[7];
		
		for(i = 1; i < 5; i++) {
			group[0] = source[(i * 3)];
			group[1] = source[(i * 3) + 1];
			group[2] = source[(i * 3) + 2];
			group[3] = '\0';
			group_val = atoi(group);
			
			mask = 0x200;
			for(j = 0; j < 10; j++) {
				concat(binary_string, (group_val & mask) ? "1" : "0");
				mask = mask >> 1;
			}
		}
			
		for(i = 0; i < 6; i++) {
			weight_str[i] = source[20 + i];
		}
		weight_str[6] = '\0';
		group_val = atoi(weight_str);
		
		mask = 0x4000;
		for(j = 0; j < 15; j++) {
			concat(binary_string, (group_val & mask) ? "1" : "0");
			mask = mask >> 1;
		}
		
		
	}
	
	if(encoding_method == 4) {
		/* Encoding method field "0101" - variable weight item (0,01 or
		0,001 pound increment) */
		char group[4];
		int group_val;
		char weight_str[7];
		
		for(i = 1; i < 5; i++) {
			group[0] = source[(i * 3)];
			group[1] = source[(i * 3) + 1];
			group[2] = source[(i * 3) + 2];
			group[3] = '\0';
			group_val = atoi(group);
			
			mask = 0x200;
			for(j = 0; j < 10; j++) {
				concat(binary_string, (group_val & mask) ? "1" : "0");
				mask = mask >> 1;
			}
		}
			
		for(i = 0; i < 6; i++) {
			weight_str[i] = source[20 + i];
		}
		weight_str[6] = '\0';
		group_val = atoi(weight_str);

		if(source[19] == '3') {
			group_val = group_val + 10000;
		}
		
		mask = 0x4000;
		for(j = 0; j < 15; j++) {
			concat(binary_string, (group_val & mask) ? "1" : "0");
			mask = mask >> 1;
		}
		
	}
		
	
	if((encoding_method >= 7) && (encoding_method <= 14)) {
		/* Encoding method fields "0111000" through "0111111" - variable
		weight item plus date */
		char group[4];
		int group_val;
		char weight_str[8];
		char date_str[4];
		
		for(i = 1; i < 5; i++) {
			group[0] = source[(i * 3)];
			group[1] = source[(i * 3) + 1];
			group[2] = source[(i * 3) + 2];
			group[3] = '\0';
			group_val = atoi(group);
			
			mask = 0x200;
			for(j = 0; j < 10; j++) {
				concat(binary_string, (group_val & mask) ? "1" : "0");
				mask = mask >> 1;
			}
		}
		
		weight_str[0] = source[19];
		
		for(i = 0; i < 5; i++) {
			weight_str[i + 1] = source[21 + i];
		}
		weight_str[6] = '\0';
		group_val = atoi(weight_str);
		
		mask = 0x80000;
		for(j = 0; j < 20; j++) {
			concat(binary_string, (group_val & mask) ? "1" : "0");
			mask = mask >> 1;
		}
		
		if(strlen(source) == 34) {
			/* Date information is included */
			date_str[0] = source[28];
			date_str[1] = source[29];
			date_str[2] = '\0';
			group_val = atoi(date_str) * 384;
			
			date_str[0] = source[30];
			date_str[1] = source[31];
			group_val += (atoi(date_str) - 1) * 32;
			
			date_str[0] = source[32];
			date_str[1] = source[33];
			group_val += atoi(date_str);
		} else {
			group_val = 38400;
		}
		
		mask = 0x8000;
		for(j = 0; j < 16; j++) {
			concat(binary_string, (group_val & mask) ? "1" : "0");
			mask = mask >> 1;
		}
		
	}
	
	if(encoding_method == 5) {
		/* Encoding method field "01100" - variable measure item and price */
		char group[4];
		int group_val;
		
		for(i = 1; i < 5; i++) {
			group[0] = source[(i * 3)];
			group[1] = source[(i * 3) + 1];
			group[2] = source[(i * 3) + 2];
			group[3] = '\0';
			group_val = atoi(group);
			
			mask = 0x200;
			for(j = 0; j < 10; j++) {
				concat(binary_string, (group_val & mask) ? "1" : "0");
				mask = mask >> 1;
			}
		}
		
		switch(source[19]) {
			case '0': concat(binary_string, "00"); break;
			case '1': concat(binary_string, "01"); break;
			case '2': concat(binary_string, "10"); break;
			case '3': concat(binary_string, "11"); break;
		}
	}
	
	if(encoding_method == 6) {
		/* Encoding method "01101" - variable measure item and price with ISO 4217
		Currency Code */
		
		char group[4];
		int group_val;
		char currency_str[5];
		
		for(i = 1; i < 5; i++) {
			group[0] = source[(i * 3)];
			group[1] = source[(i * 3) + 1];
			group[2] = source[(i * 3) + 2];
			group[3] = '\0';
			group_val = atoi(group);
			
			mask = 0x200;
			for(j = 0; j < 10; j++) {
				concat(binary_string, (group_val & mask) ? "1" : "0");
				mask = mask >> 1;
			}
		}
		
		switch(source[19]) {
			case '0': concat(binary_string, "00"); break;
			case '1': concat(binary_string, "01"); break;
			case '2': concat(binary_string, "10"); break;
			case '3': concat(binary_string, "11"); break;
		}
		
		for(i = 0; i < 3; i++) {
			currency_str[i] = source[20 + i];
		}
		currency_str[3] = '\0';
		group_val = atoi(currency_str);
		
		mask = 0x200;
		for(j = 0; j < 10; j++) {
			concat(binary_string, (group_val & mask) ? "1" : "0");
			mask = mask >> 1;
		}
		
		
	}
	
	/* The compressed data field has been processed if appropriate - the
	rest of the data (if any) goes into a general-purpose data compaction field */
	
	j = 0;
	for(i = read_posn; i < strlen(source); i++) {
		general_field[j] = source[i];
		j++;
	}
	general_field[j] = '\0';
	if(debug) printf("General field data = %s\n", general_field);
	
	latch = 0;
	for(i = 0; i < strlen(general_field); i++) {
		/* Table 13 - ISO/IEC 646 encodation */
		if((general_field[i] < ' ') || (general_field[i] > 'z')) {
			general_field_type[i] = INVALID_CHAR; latch = 1;
		} else {
			general_field_type[i] = ISOIEC;
		}
		
		if(general_field[i] == '#') {
			general_field_type[i] = INVALID_CHAR; latch = 1;
		}
		if(general_field[i] == '$') {
			general_field_type[i] = INVALID_CHAR; latch = 1;
		}
		if(general_field[i] == '@') {
			general_field_type[i] = INVALID_CHAR; latch = 1;
		}
		if(general_field[i] == 92) {
			general_field_type[i] = INVALID_CHAR; latch = 1;
		}
		if(general_field[i] == '^') {
			general_field_type[i] = INVALID_CHAR; latch = 1;
		}
		if(general_field[i] == 96) {
			general_field_type[i] = INVALID_CHAR; latch = 1;
		}

		/* Table 12 - Alphanumeric encodation */
		if((general_field[i] >= 'A') && (general_field[i] <= 'Z')) {
			general_field_type[i] = ALPHA_OR_ISO;
		}
		if(general_field[i] == '*') {
			general_field_type[i] = ALPHA_OR_ISO;
		}
		if(general_field[i] == ',') {
			general_field_type[i] = ALPHA_OR_ISO;
		}
		if(general_field[i] == '-') {
			general_field_type[i] = ALPHA_OR_ISO;
		}
		if(general_field[i] == '.') {
			general_field_type[i] = ALPHA_OR_ISO;
		}
		if(general_field[i] == '/') {
			general_field_type[i] = ALPHA_OR_ISO;
		}

		/* Numeric encodation */
		if((general_field[i] >= '0') && (general_field[i] <= '9')) {
			general_field_type[i] = ANY_ENC;
		}
		if(general_field[i] == '[') {
			/* FNC1 can be encoded in any system */
			general_field_type[i] = ANY_ENC;
		}

	}
	
	general_field_type[strlen(general_field)] = '\0';
	if(debug) printf("General field type: %s\n", general_field_type);
	
	if(latch == 1) {
		/* Invalid characters in input data */
		strcpy(symbol->errtxt, "Invalid characters in input data");
		return ZINT_ERROR_INVALID_DATA;
	}
	
	for(i = 0; i < strlen(general_field); i++) {
		if((general_field_type[i] == ISOIEC) && (general_field[i + 1] == '[')) {
			general_field_type[i + 1] = ISOIEC;
		}
	}
	
	for(i = 0; i < strlen(general_field); i++) {
		if((general_field_type[i] == ALPHA_OR_ISO) && (general_field[i + 1] == '[')) {
			general_field_type[i + 1] = ALPHA_OR_ISO;
		}
	}
	
	latch = general_rules(general_field, general_field_type);
	if(debug) printf("General field type: %s\n", general_field_type);
	
	last_mode = NUMERIC;
	
	/* Set initial mode if not NUMERIC */
	if(general_field_type[0] == ALPHA) {
		concat(binary_string, "0000"); /* Alphanumeric latch */
		last_mode = ALPHA;
	}
	if(general_field_type[0] == ISOIEC) {
		concat(binary_string, "0000"); /* Alphanumeric latch */
		concat(binary_string, "00100"); /* ISO/IEC 646 latch */
		last_mode = ISOIEC;
	}
	
	i = 0;
	do {
		if(debug) printf("Processing character %d ", i);
		switch(general_field_type[i]) {
			case NUMERIC:
				if(debug) printf("as NUMERIC:");
				
				if(last_mode != NUMERIC) {
					concat(binary_string, "000"); /* Numeric latch */
					if(debug) printf("<NUMERIC LATCH>\n");
				}
				
				if(debug) printf("  %c%c > ", general_field[i], general_field[i + 1]);
				if(general_field[i] != '[') {
					d1 = ctoi(general_field[i]);
				} else {
					d1 = 10;
				}
				
				if(general_field[i + 1] != '[') {
					d2 = ctoi(general_field[i + 1]);
				} else {
					d2 = 10;
				}
				
				value = (11 * d1) + d2 + 8;
				
				mask = 0x40;
				for(j = 0; j < 7; j++) {
					concat(binary_string, (value & mask) ? "1" : "0");
					if (debug) {
						printf("%d", !!(value & mask));
					}
					mask = mask >> 1;
				}
				
				i += 2;
				if(debug) printf("\n");
				last_mode = NUMERIC;
				break;
			
			case ALPHA:
				if(debug) printf("as ALPHA\n");
				if(i != 0) {
					if(last_mode == NUMERIC) {
						concat(binary_string, "0000"); /* Alphanumeric latch */
					}
					if(last_mode == ISOIEC) {
						concat(binary_string, "00100"); /* Alphanumeric latch */
					}
				}
				
				if((general_field[i] >= '0') && (general_field[i] <= '9')) {
					
					value = general_field[i] - 43;
					
					mask = 0x10;
					for(j = 0; j < 5; j++) {
						concat(binary_string, (value & mask) ? "1" : "0");
						mask = mask >> 1;
					}
				}
				
				if((general_field[i] >= 'A') && (general_field[i] <= 'Z')) {
					
					value = general_field[i] - 33;
					
					mask = 0x20;
					for(j = 0; j < 6; j++) {
						concat(binary_string, (value & mask) ? "1" : "0");
						mask = mask >> 1;
					}
				}
				
				last_mode = ALPHA;
				if(general_field[i] == '[') { concat(binary_string, "01111"); last_mode = NUMERIC; } /* FNC1/Numeric latch */
				if(general_field[i] == '*') concat(binary_string, "111010"); /* asterisk */
				if(general_field[i] == ',') concat(binary_string, "111011"); /* comma */
				if(general_field[i] == '-') concat(binary_string, "111100"); /* minus or hyphen */
				if(general_field[i] == '.') concat(binary_string, "111101"); /* period or full stop */
				if(general_field[i] == '/') concat(binary_string, "111110"); /* slash or solidus */
				
				i++;
				break;
			
			case ISOIEC:
				if(debug) printf("as ISOIEC\n");
				if(i != 0) {
					if(last_mode == NUMERIC) {
						concat(binary_string, "0000"); /* Alphanumeric latch */
						concat(binary_string, "00100"); /* ISO/IEC 646 latch */
					}
					if(last_mode == ALPHA) {
						concat(binary_string, "00100"); /* ISO/IEC 646 latch */
					}
				}
				
				if((general_field[i] >= '0') && (general_field[i] <= '9')) {
					
					value = general_field[i] - 43;
					
					mask = 0x10;
					for(j = 0; j < 5; j++) {
						concat(binary_string, (value & mask) ? "1" : "0");
						mask = mask >> 1;
					}
				}
				
				if((general_field[i] >= 'A') && (general_field[i] <= 'Z')) {
					
					value = general_field[i] - 1;
					
					mask = 0x40;
					for(j = 0; j < 7; j++) {
						concat(binary_string, (value & mask) ? "1" : "0");
						mask = mask >> 1;
					}
				}
				
				if((general_field[i] >= 'a') && (general_field[i] <= 'z')) {
					
					value = general_field[i] - 7;
					
					mask = 0x40;
					for(j = 0; j < 7; j++) {
						concat(binary_string, (value & mask) ? "1" : "0");
						mask = mask >> 1;
					}
				}
				
				last_mode = ISOIEC;
				if(general_field[i] == '[') { concat(binary_string, "01111"); last_mode = NUMERIC; } /* FNC1/Numeric latch */
				if(general_field[i] == '!') concat(binary_string, "11101000"); /* exclamation mark */
				if(general_field[i] == 34) concat(binary_string, "11101001"); /* quotation mark */
				if(general_field[i] == 37) concat(binary_string, "11101010"); /* percent sign */
				if(general_field[i] == '&') concat(binary_string, "11101011"); /* ampersand */
				if(general_field[i] == 39) concat(binary_string, "11101100"); /* apostrophe */
				if(general_field[i] == '(') concat(binary_string, "11101101"); /* left parenthesis */
				if(general_field[i] == ')') concat(binary_string, "11101110"); /* right parenthesis */
				if(general_field[i] == '*') concat(binary_string, "11101111"); /* asterisk */
				if(general_field[i] == '+') concat(binary_string, "11110000"); /* plus sign */
				if(general_field[i] == ',') concat(binary_string, "11110001"); /* comma */
				if(general_field[i] == '-') concat(binary_string, "11110010"); /* minus or hyphen */
				if(general_field[i] == '.') concat(binary_string, "11110011"); /* period or full stop */
				if(general_field[i] == '/') concat(binary_string, "11110100"); /* slash or solidus */
				if(general_field[i] == ':') concat(binary_string, "11110101"); /* colon */
				if(general_field[i] == ';') concat(binary_string, "11110110"); /* semicolon */
				if(general_field[i] == '<') concat(binary_string, "11110111"); /* less-than sign */
				if(general_field[i] == '=') concat(binary_string, "11111000"); /* equals sign */
				if(general_field[i] == '>') concat(binary_string, "11111001"); /* greater-than sign */
				if(general_field[i] == '?') concat(binary_string, "11111010"); /* question mark */
				if(general_field[i] == '_') concat(binary_string, "11111011"); /* underline or low line */
				if(general_field[i] == ' ') concat(binary_string, "11111100"); /* space */
				
				i++;
				break;
		}
	} while (i + latch < strlen(general_field));
	if(debug) printf("Resultant binary = %s\n", binary_string);
	if(debug) printf("\tLength: %d\n", (int)strlen(binary_string));
	
	remainder = 12 - (strlen(binary_string) % 12);
	if(remainder == 12) { remainder = 0; }
	if(strlen(binary_string) < 36) { remainder = 36 - strlen(binary_string); }

	if(latch == 1) {
		/* There is still one more numeric digit to encode */
		if(debug) printf("Adding extra (odd) numeric digit\n");
		
		if(last_mode == NUMERIC) {
			if((remainder >= 4) && (remainder <= 6)) {
				value = ctoi(general_field[i]);
				value++;
				
				mask = 0x08;
				for(j = 0; j < 4; j++) {
					concat(binary_string, (value & mask) ? "1" : "0");
					mask = mask >> 1;
				}
			} else {
				d1 = ctoi(general_field[i]);
				d2 = 10;
					
				value = (11 * d1) + d2 + 8;
					
				mask = 0x40;
				for(j = 0; j < 7; j++) {
					concat(binary_string, (value & mask) ? "1" : "0");
					mask = mask >> 1;
				}
			}
		} else {
			value = general_field[i] - 43;
					
			mask = 0x10;
			for(j = 0; j < 5; j++) {
				concat(binary_string, (value & mask) ? "1" : "0");
				mask = mask >> 1;
			}
		}
		
		remainder = 12 - (strlen(binary_string) % 12);
		if(remainder == 12) { remainder = 0; }
		if(strlen(binary_string) < 36) { remainder = 36 - strlen(binary_string); }
		if(debug) printf("Resultant binary = %s\n", binary_string);
		if(debug) printf("\tLength: %d\n", (int)strlen(binary_string));
	}
	
	if(strlen(binary_string) > 252) {
		strcpy(symbol->errtxt, "Input too long");
		return ZINT_ERROR_TOO_LONG;
	}
	
	/* Now add padding to binary string (7.2.5.5.4) */
	i = remainder;
	if((strlen(general_field) != 0) && (last_mode == NUMERIC)) {
		strcpy(padstring, "0000");
		i -= 4;
	} else {
		strcpy(padstring, "");
	}
	for(;i > 0;i -= 5) {
		concat(padstring, "00100");
	}

	padstring[remainder] = '\0';
	concat(binary_string, padstring);
	
	/* Patch variable length symbol bit field */
	d1 = ((strlen(binary_string) / 12) + 1) & 1;
	if(strlen(binary_string) <= 156) { d2 = 0; } else { d2 = 1; }
	
	if(encoding_method == 1) {
		binary_string[2] = d1 ? '1' : '0';
		binary_string[3] = d2 ? '1' : '0';
	}
	if(encoding_method == 2) {
		binary_string[3] = d1 ? '1' : '0';
		binary_string[4] = d2 ? '1' : '0';
	}
	if((encoding_method == 5) || (encoding_method == 6)) {
		binary_string[6] = d1 ? '1' : '0';
		binary_string[7] = d2 ? '1' : '0';
	}
	if(debug) printf("Resultant binary = %s\n", binary_string);
	if(debug) printf("\tLength: %d\n", (int)strlen(binary_string));
	return 0;
}

int rssexpanded(struct zint_symbol *symbol, unsigned char source[], int src_len)
{ /* GS1 DataBar Expanded */
	int i, j, k, l, data_chars, vs[21], group[21], v_odd[21], v_even[21];
	char substring[21][14], latch;
	int char_widths[21][8], checksum, check_widths[8], c_group;
	int check_char, c_odd, c_even, elements[235], pattern_width, reader, writer;
	int row, elements_in_sub, special_case_row, left_to_right;
	int codeblocks, sub_elements[235], stack_rows, current_row, current_block;
	int separator_row;
#ifndef _MSC_VER
	char reduced[src_len], binary_string[7 * src_len];
#else
        char* reduced = (char*)_alloca(src_len);
        char* binary_string = (char*)_alloca(7 * src_len);
#endif
	
	separator_row = 0;
	reader=0;

	if(symbol->input_mode != GS1_MODE) {
		/* GS1 data has not been verified yet */
		i = gs1_verify(symbol, source, src_len, reduced);
		if(i != 0) { return i; }
	}
	
	if((symbol->symbology == BARCODE_RSS_EXP_CC) || (symbol->symbology == BARCODE_RSS_EXPSTACK_CC)) {
		/* make space for a composite separator pattern */
		separator_row = symbol->rows;
		symbol->row_height[separator_row] = 1;
		symbol->rows += 1;
	}
	
	strcpy(binary_string, "");
	
	if(symbol->option_1 == 2) {
		concat(binary_string, "1");
	} else {
		concat(binary_string, "0");
	}
	
	i = rss_binary_string(symbol, reduced, binary_string);
	if(i != 0) {
		return i;
	}

	data_chars = strlen(binary_string) / 12;
	
	for(i = 0; i < data_chars; i++) {
		for(j = 0; j < 12; j++) {
			substring[i][j] = binary_string[(i * 12) + j];
		}
		substring[i][12] = '\0';
	}
	
	for(i = 0; i < data_chars; i++) {
		vs[i] = 0;
		if(substring[i][0] == '1') { vs[i] += 2048; }
		if(substring[i][1] == '1') { vs[i] += 1024; }
		if(substring[i][2] == '1') { vs[i] += 512; }
		if(substring[i][3] == '1') { vs[i] += 256; }
		if(substring[i][4] == '1') { vs[i] += 128; }
		if(substring[i][5] == '1') { vs[i] += 64; }
		if(substring[i][6] == '1') { vs[i] += 32; }
		if(substring[i][7] == '1') { vs[i] += 16; }
		if(substring[i][8] == '1') { vs[i] += 8; }
		if(substring[i][9] == '1') { vs[i] += 4; }
		if(substring[i][10] == '1') { vs[i] += 2; }
		if(substring[i][11] == '1') { vs[i] += 1; }
	}
	
	for(i = 0; i < data_chars; i++) {
		if(vs[i] <= 347) { group[i] = 1; }
		if((vs[i] >= 348) && (vs[i] <= 1387)) { group[i] = 2; }
		if((vs[i] >= 1388) && (vs[i] <= 2947)) { group[i] = 3; }
		if((vs[i] >= 2948) && (vs[i] <= 3987)) { group[i] = 4; }
		if(vs[i] >= 3988) { group[i] = 5; }
		v_odd[i] = (vs[i] - g_sum_exp[group[i] - 1]) / t_even_exp[group[i] - 1];
		v_even[i] = (vs[i] - g_sum_exp[group[i] - 1]) % t_even_exp[group[i] - 1];
		
		getRSSwidths(v_odd[i], modules_odd_exp[group[i] - 1], 4, widest_odd_exp[group[i] - 1], 0);
		char_widths[i][0] = widths[0];
		char_widths[i][2] = widths[1];
		char_widths[i][4] = widths[2];
		char_widths[i][6] = widths[3];
		getRSSwidths(v_even[i], modules_even_exp[group[i] - 1], 4, widest_even_exp[group[i] - 1], 1);
		char_widths[i][1] = widths[0];
		char_widths[i][3] = widths[1];
		char_widths[i][5] = widths[2];
		char_widths[i][7] = widths[3];
	}
	
	/* 7.2.6 Check character */
	/* The checksum value is equal to the mod 211 residue of the weighted sum of the widths of the
	   elements in the data characters. */
	checksum = 0;
	for(i = 0; i < data_chars; i++) {
		row = weight_rows[(((data_chars - 2) / 2) * 21) + i];
		for(j = 0; j < 8; j++) {
			checksum += (char_widths[i][j] * checksum_weight_exp[(row * 8) + j]);
			
		}
	}
	
	check_char = (211 * ((data_chars + 1) - 4)) + (checksum % 211);
	
	if(check_char <= 347) { c_group = 1; }
	if((check_char >= 348) && (check_char <= 1387)) { c_group = 2; }
	if((check_char >= 1388) && (check_char <= 2947)) { c_group = 3; }
	if((check_char >= 2948) && (check_char <= 3987)) { c_group = 4; }
	if(check_char >= 3988) { c_group = 5; }
	
	c_odd = (check_char - g_sum_exp[c_group - 1]) / t_even_exp[c_group - 1];
	c_even = (check_char - g_sum_exp[c_group - 1]) % t_even_exp[c_group - 1];
	
	getRSSwidths(c_odd, modules_odd_exp[c_group - 1], 4, widest_odd_exp[c_group - 1], 0);
	check_widths[0] = widths[0];
	check_widths[2] = widths[1];
	check_widths[4] = widths[2];
	check_widths[6] = widths[3];
	getRSSwidths(c_even, modules_even_exp[c_group - 1], 4, widest_even_exp[c_group - 1], 1);
	check_widths[1] = widths[0];
	check_widths[3] = widths[1];
	check_widths[5] = widths[2];
	check_widths[7] = widths[3];
	
	/* Initialise element array */
	pattern_width = ((((data_chars + 1) / 2) + ((data_chars + 1) & 1)) * 5) + ((data_chars + 1) * 8) + 4;
	for(i = 0; i < pattern_width; i++) {
		elements[i] = 0;
	}
	
	elements[0] = 1;
	elements[1] = 1;
	elements[pattern_width - 2] = 1;
	elements[pattern_width - 1] = 1;
	
	/* Put finder patterns in element array */
	for(i = 0; i < (((data_chars + 1) / 2) + ((data_chars + 1) & 1)); i++) {
		k = ((((((data_chars + 1) - 2) / 2) + ((data_chars + 1) & 1)) - 1) * 11) + i;
		for(j = 0; j < 5; j++) {
			elements[(21 * i) + j + 10] = finder_pattern_exp[((finder_sequence[k] - 1) * 5) + j];
		}
	}
	
	/* Put check character in element array */
	for(i = 0; i < 8; i++) {
		elements[i + 2] = check_widths[i];
	}
	
	/* Put forward reading data characters in element array */
	for(i = 1; i < data_chars; i += 2) {
		for(j = 0; j < 8; j++) {
			elements[(((i - 1) / 2) * 21) + 23 + j] = char_widths[i][j];
		}
	}
	
	/* Put reversed data characters in element array */
	for(i = 0; i < data_chars; i += 2) {
		for(j = 0; j < 8; j++) {
			elements[((i / 2) * 21) + 15 + j] = char_widths[i][7 - j];
		}
	}
	
	if((symbol->symbology == BARCODE_RSS_EXP) || (symbol->symbology == BARCODE_RSS_EXP_CC)) {
		/* Copy elements into symbol */
		writer = 0;
		latch = '0';
		for(i = 0; i < pattern_width; i++) {
			for(j = 0; j < elements[i]; j++) {
				if(latch == '1') { set_module(symbol, symbol->rows, writer); } else { unset_module(symbol, symbol->rows, writer); }
				writer++;
			}
			if(latch == '1') {
				latch = '0';
			} else {
				latch = '1';
			}
		}
		if(symbol->width < writer) { symbol->width = writer; }
		symbol->rows = symbol->rows + 1;
		if(symbol->symbology == BARCODE_RSS_EXP_CC) {
			for(j = 4; j < (symbol->width - 4); j++) {
				if(module_is_set(symbol, separator_row + 1, j)) {
					unset_module(symbol, separator_row, j);
				} else {
					set_module(symbol, separator_row, j);
				}
			}
			/* finder bar adjustment */
			for(j = 0; j < (writer / 49); j++) {
				k = (49 * j) + 18;
				for(i = 0; i < 15; i++) {
					if((!(module_is_set(symbol, separator_row + 1, i + k - 1))) &&
					(!(module_is_set(symbol, separator_row + 1, i + k))) &&
					module_is_set(symbol, separator_row, i + k - 1)) {
						unset_module(symbol, separator_row, i + k);
					}
				}
			}
		}
		
		/* Add human readable text */
		for(i = 0; i <= src_len; i++) {
			if((source[i] != '[') && (source[i] != ']')) {
				symbol->text[i] = source[i];
			} else {
				if(source[i] == '[') {
					symbol->text[i] = '(';
				}
				if(source[i] == ']') {
					symbol->text[i] = ')';
				}
			}
		}
		
	} else {
		/* RSS Expanded Stacked */
		
		codeblocks = (data_chars + 1) / 2;
		
		if((symbol->option_2 < 1) || (symbol->option_2 > 10)) {
			symbol->option_2 = 2;
		}
		if((symbol->option_1 == 2) && (symbol->option_2 == 1)) {
			/* "There shall be a minimum of four symbol characters in the
			first row of an RSS Expanded Stacked symbol when it is the linear
			component of an EAN.UCC Composite symbol." */
			symbol->option_2 = 2;
		}
		
		stack_rows = codeblocks / symbol->option_2;
		if(codeblocks % symbol->option_2 > 0) {
			stack_rows++;
		}
		
		current_block = 0;
		for(current_row = 1; current_row <= stack_rows; current_row++) {
			for(i = 0; i < 235; i++) {
				sub_elements[i] = 0;
			}
			special_case_row = 0;
			
			/* Row Start */
			sub_elements[0] = 1;
			sub_elements[1] = 1;
			elements_in_sub = 2;
			
			/* Row Data */
			reader = 0;
			do {
				if(((symbol->option_2 & 1) || (current_row & 1)) ||
					((current_row == stack_rows) && (codeblocks != (current_row * symbol->option_2)) &&
					(((current_row * symbol->option_2) - codeblocks) & 1))) {
					/* left to right */
					left_to_right = 1;
					i = 2 + (current_block * 21);
					for(j = 0; j < 21; j++) {
						sub_elements[j + (reader * 21) + 2] = elements[i + j];
						elements_in_sub++;
					}
				} else {
					/* right to left */
					left_to_right = 0;
					if((current_row * symbol->option_2) < codeblocks) {
						/* a full row */
						i = 2 + (((current_row * symbol->option_2) - reader - 1) * 21);
						for(j = 0; j < 21; j++) {
							sub_elements[(20 - j) + (reader * 21) + 2] = elements[i + j];
							elements_in_sub++;
						}
					} else {
						/* a partial row */
						k = ((current_row * symbol->option_2) - codeblocks);
						l = (current_row * symbol->option_2) - reader - 1;
						i = 2 + ((l - k) * 21);
						for(j = 0; j < 21; j++) {
							sub_elements[(20 - j) + (reader * 21) + 2] = elements[i + j];
							elements_in_sub++;
						}
					}
				}
				reader++;
				current_block++;
			} while ((reader < symbol->option_2) && (current_block < codeblocks));
			
			/* Row Stop */
			sub_elements[elements_in_sub] = 1;
			sub_elements[elements_in_sub + 1] = 1;
			elements_in_sub += 2;
		
			latch = current_row & 1 ? '0' : '1';
			
			if ((current_row == stack_rows) && (codeblocks != (current_row * symbol->option_2)) &&
				(((current_row * symbol->option_2) - codeblocks) & 1) ) {
				/* Special case bottom row */
				special_case_row = 1;
				sub_elements[0] = 2;
				latch = '0';
			}
			
			writer = 0;
			for(i = 0; i < elements_in_sub; i++) {
				for(j = 0; j < sub_elements[i]; j++) {
					if(latch == '1') { set_module(symbol, symbol->rows, writer); } else { unset_module(symbol, symbol->rows, writer); }
					writer++;
				}
				if(latch == '1') {
					latch = '0';
				} else {
					latch = '1';
				}
			}
			if(symbol->width < writer) { symbol->width = writer; }
			
			if(current_row != 1) {
				/* middle separator pattern (above current row) */
				for(j = 5; j < (49 * symbol->option_2); j += 2) {
					set_module(symbol, symbol->rows - 2, j);
				}
				symbol->row_height[symbol->rows - 2] = 1;
				/* bottom separator pattern (above current row) */
				for(j = 4; j < (writer - 4); j++) {
					if(module_is_set(symbol, symbol->rows, j)) {
						unset_module(symbol, symbol->rows - 1, j);
					} else {
						set_module(symbol, symbol->rows - 1, j);
					}
				}
				symbol->row_height[symbol->rows - 1] = 1;
				/* finder bar adjustment */
				for(j = 0; j < reader; j++) {
					k = (49 * j) + (special_case_row ? 19 : 18);
					if(left_to_right) {
						for(i = 0; i < 15; i++) {
							if((!(module_is_set(symbol, symbol->rows, i + k - 1))) &&
							(!(module_is_set(symbol, symbol->rows, i + k))) &&
							module_is_set(symbol, symbol->rows - 1, i + k - 1)) {
								unset_module(symbol, symbol->rows - 1, i + k);
							}
						}
					} else {
						for(i = 14; i >= 0; i--) {
							if((!(module_is_set(symbol, symbol->rows, i + k + 1))) &&
							(!(module_is_set(symbol, symbol->rows, i + k))) &&
							module_is_set(symbol, symbol->rows - 1, i + k + 1)) {
								unset_module(symbol, symbol->rows - 1, i + k);
							}
						}
					}
				}
			}
			
			if(current_row != stack_rows) {
				/* top separator pattern (below current row) */
				for(j = 4; j < (writer - 4); j++) {
					if(module_is_set(symbol, symbol->rows, j)) {
						unset_module(symbol, symbol->rows + 1, j);
					} else {
						set_module(symbol, symbol->rows + 1, j);
					}
				}
				symbol->row_height[symbol->rows + 1] = 1;
				/* finder bar adjustment */
				for(j = 0; j < reader; j++) {
					k = (49 * j) + 18;
					if(left_to_right) {
						for(i = 0; i < 15; i++) {
							if((!(module_is_set(symbol, symbol->rows, i + k - 1))) &&
							(!(module_is_set(symbol, symbol->rows, i + k))) &&
							module_is_set(symbol, symbol->rows + 1, i + k - 1)) {
								unset_module(symbol, symbol->rows + 1, i + k);
							}
						}
					} else{
						for(i = 14; i >= 0; i--) {
							if((!(module_is_set(symbol, symbol->rows, i + k + 1))) &&
							(!(module_is_set(symbol, symbol->rows, i + k))) &&
							module_is_set(symbol, symbol->rows + 1, i + k + 1)) {
								unset_module(symbol, symbol->rows + 1, i + k);
							}
						}
					}
				}
			}
			
			symbol->rows = symbol->rows + 4;
		}
		symbol->rows = symbol->rows - 3;
		if(symbol->symbology == BARCODE_RSS_EXPSTACK_CC) {
			for(j = 4; j < (symbol->width - 4); j++) {
				if(module_is_set(symbol, separator_row + 1, j)) {
					unset_module(symbol, separator_row, j);
				} else {
					set_module(symbol, separator_row, j);
				}
			}
			/* finder bar adjustment */
			for(j = 0; j < reader; j++) {
				k = (49 * j) + 18;
				for(i = 0; i < 15; i++) {
					if((!(module_is_set(symbol, separator_row + 1, i + k - 1))) &&
					(!(module_is_set(symbol, separator_row + 1, i + k))) &&
					module_is_set(symbol, separator_row, i + k - 1)) {
						unset_module(symbol, separator_row, i + k);
					}
				}
			}
		}
		
	}
	
	return 0;
}
