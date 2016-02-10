/* qr.c Handles QR Code */

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
#ifdef _MSC_VER
#include <malloc.h>
#endif
#include "common.h"
#include <stdio.h>
#include "sjis.h"
#include "qr.h"
#include "reedsol.h"
#include <stdlib.h>     /* abs */

int in_alpha(int glyph) {
	/* Returns true if input glyph is in the Alphanumeric set */
	int retval = 0;
	char cglyph = (char) glyph;

	if((cglyph >= '0') && (cglyph <= '9')) {
		retval = 1;
	}
	if((cglyph >= 'A') && (cglyph <= 'Z')) {
		retval = 1;
	}
	switch (cglyph) {
		case ' ':
		case '$':
		case '%':
		case '*':
		case '+':
		case '-':
		case '.':
		case '/':
		case ':':
			retval = 1;
			break;
	}

	return retval;
}

void define_mode(char mode[], int jisdata[], int length, int gs1)
{
	/* Values placed into mode[] are: K = Kanji, B = Binary, A = Alphanumeric, N = Numeric */
	int i, mlen, j;

	for(i = 0; i < length; i++) {
		if(jisdata[i] > 0xff) {
			mode[i] = 'K';
		} else {
			mode[i] = 'B';
			if(in_alpha(jisdata[i])) { mode[i] = 'A'; }
			if(gs1 && (jisdata[i] == '[')) { mode[i] = 'A'; }
			if((jisdata[i] >= '0') && (jisdata[i] <= '9')) { mode[i] = 'N'; }
		}
	}

	/* If less than 6 numeric digits together then don't use numeric mode */
	for(i = 0; i < length; i++) {
		if(mode[i] == 'N') {
			if(((i != 0) && (mode[i - 1] != 'N')) || (i == 0)) {
				mlen = 0;
				while (((mlen + i) < length) && (mode[mlen + i] == 'N')) {
					mlen++;
				};
				if(mlen < 6) {
					for(j = 0; j < mlen; j++) {
						mode[i + j] = 'A';
					}
				}
			}
		}
	}

	/* If less than 4 alphanumeric characters together then don't use alphanumeric mode */
	for(i = 0; i < length; i++) {
		if(mode[i] == 'A') {
			if(((i != 0) && (mode[i - 1] != 'A')) || (i == 0)) {
				mlen = 0;
				while (((mlen + i) < length) && (mode[mlen + i] == 'A')) {
					mlen++;
				};
				if(mlen < 6) {
					for(j = 0; j < mlen; j++) {
						mode[i + j] = 'B';
					}
				}
			}
		}
	}
}

int estimate_binary_length(char mode[], int length, int gs1)
{
	/* Make an estimate (worst case scenario) of how long the binary string will be */
	int i, count = 0;
	char current = 0;
	int a_count = 0;
	int n_count = 0;

	if(gs1) { count += 4; }

	for(i = 0; i < length; i++) {
		if(mode[i] != current) {
			switch(mode[i]) {
				case 'K': count += 12 + 4; current = 'K'; break;
				case 'B': count += 16 + 4; current = 'B'; break;
				case 'A': count += 13 + 4; current = 'A'; a_count = 0; break;
				case 'N': count += 14 + 4; current = 'N'; n_count = 0; break;
			}
		}

		switch(mode[i]) {
		case 'K': count += 13; break;
		case 'B': count += 8; break;
		case 'A':
			a_count++;
			if((a_count & 1) == 0) {
				count += 5;        // 11 in total
				a_count = 0;
			}
			else
				count += 6;
			break;
		case 'N':
			n_count++;
			if((n_count % 3) == 0) {
				count += 3;        // 10 in total
				n_count = 0;
			}
			else if ((n_count & 1) == 0)
				count += 3;        // 7 in total
			else
				count += 4;
			break;
		}
	}

	return count;
}

static void qr_bscan(char *binary, int data, int h)
{
	for (; h; h>>=1) {
		concat(binary, data & h ? "1" : "0");
	}
}

void qr_binary(int datastream[], int version, int target_binlen, char mode[], int jisdata[], int length, int gs1, int est_binlen)
{
	/* Convert input data to a binary stream and add padding */
	int position = 0, debug = 0;
	int short_data_block_length, i, scheme = 1;
	char data_block, padbits;
	int current_binlen, current_bytes;
	int toggle, percent;
	
#ifndef _MSC_VER
	char binary[est_binlen + 12];
#else
	char* binary = (char *)_alloca(est_binlen + 12);
#endif
	strcpy(binary, "");

	if(gs1) {
		concat(binary, "0101"); /* FNC1 */
	}

	if(version <= 9) {
		scheme = 1;
	} else if((version >= 10) && (version <= 26)) {
		scheme = 2;
	} else if(version >= 27) {
		scheme = 3;
	}

	if(debug) {
		for(i = 0; i < length; i++) {
			printf("%c", mode[i]);
		}
		printf("\n");
	}

	percent = 0;

	do {
		data_block = mode[position];
		short_data_block_length = 0;
		do {
			short_data_block_length++;
		} while (((short_data_block_length + position) < length) && (mode[position + short_data_block_length] == data_block));

		switch(data_block) {
			case 'K':
				/* Kanji mode */
				/* Mode indicator */
				concat(binary, "1000");

				/* Character count indicator */
				qr_bscan(binary, short_data_block_length, 0x20 << (scheme*2)); /* scheme = 1..3 */
				
				if(debug) { printf("Kanji block (length %d)\n\t", short_data_block_length); }

				/* Character representation */
				for(i = 0; i < short_data_block_length; i++) {
					int jis = jisdata[position + i];
					int msb, lsb, prod;

					if(jis > 0x9fff) { jis -= 0xc140; }
					msb = (jis & 0xff00) >> 4;
					lsb = (jis & 0xff);
					prod = (msb * 0xc0) + lsb;
				
					qr_bscan(binary, prod, 0x1000);
					
					if(debug) { printf("0x%4X ", prod); }
				}

				if(debug) { printf("\n"); }

				break;
			case 'B':
				/* Byte mode */
				/* Mode indicator */
				concat(binary, "0100");

				/* Character count indicator */
				qr_bscan(binary, short_data_block_length, scheme > 1 ? 0x8000 : 0x80); /* scheme = 1..3 */
				
				if(debug) { printf("Byte block (length %d)\n\t", short_data_block_length); }

				/* Character representation */
				for(i = 0; i < short_data_block_length; i++) {
					int byte = jisdata[position + i];

					if(gs1 && (byte == '[')) {
						byte = 0x1d; /* FNC1 */
					}
					
					qr_bscan(binary, byte, 0x80);
					
					if(debug) { printf("0x%2X(%d) ", byte, byte); }
				}

				if(debug) { printf("\n"); }

				break;
			case 'A':
				/* Alphanumeric mode */
				/* Mode indicator */
				concat(binary, "0010");

				/* Character count indicator */
				qr_bscan(binary, short_data_block_length, 0x40 << (2 * scheme)); /* scheme = 1..3 */
				
				if(debug) { printf("Alpha block (length %d)\n\t", short_data_block_length); }

				/* Character representation */
				i = 0;
				while ( i < short_data_block_length ) {
					int count;
					int first = 0, second = 0, prod;

					if(percent == 0) {
						if(gs1 && (jisdata[position + i] == '%')) {
							first = posn(RHODIUM, '%');
							second = posn(RHODIUM, '%');
							count = 2;
							prod = (first * 45) + second;
							i++;
						} else {
							if(gs1 && (jisdata[position + i] == '[')) {
								first = posn(RHODIUM, '%'); /* FNC1 */
							} else {
								first = posn(RHODIUM, (char) jisdata[position + i]);
							}
							count = 1;
							i++;
							prod = first;

							if(mode[position + i] == 'A') {
								if(gs1 && (jisdata[position + i] == '%')) {
									second = posn(RHODIUM, '%');
									count = 2;
									prod = (first * 45) + second;
									percent = 1;
								} else {
									if(gs1 && (jisdata[position + i] == '[')) {
										second = posn(RHODIUM, '%'); /* FNC1 */
									} else {
										second = posn(RHODIUM, (char) jisdata[position + i]);
									}
									count = 2;
									i++;
									prod = (first * 45) + second;
								}
							}
						}
					} else {
						first = posn(RHODIUM, '%');
						count = 1;
						i++;
						prod = first;
						percent = 0;

						if(mode[position + i] == 'A') {
							if(gs1 && (jisdata[position + i] == '%')) {
								second = posn(RHODIUM, '%');
								count = 2;
								prod = (first * 45) + second;
								percent = 1;
							} else {
								if(gs1 && (jisdata[position + i] == '[')) {
									second = posn(RHODIUM, '%'); /* FNC1 */
								} else {
									second = posn(RHODIUM, (char) jisdata[position + i]);
								}
								count = 2;
								i++;
								prod = (first * 45) + second;
							}
						}
					}

					qr_bscan(binary, prod, count == 2 ? 0x400 : 0x20); /* count = 1..2 */
					
					if(debug) { printf("0x%4X ", prod); }
				};

				if(debug) { printf("\n"); }

				break;
			case 'N':
				/* Numeric mode */
				/* Mode indicator */
				concat(binary, "0001");

				/* Character count indicator */
				qr_bscan(binary, short_data_block_length, 0x80 << (2 * scheme)); /* scheme = 1..3 */
				
				if(debug) { printf("Number block (length %d)\n\t", short_data_block_length); }

								/* Character representation */
				i = 0;
				while ( i < short_data_block_length ) {
					int count;
					int first = 0, second = 0, third = 0, prod;

					first = posn(NEON, (char) jisdata[position + i]);
					count = 1;
					prod = first;

					if(mode[position + i + 1] == 'N') {
						second = posn(NEON, (char) jisdata[position + i + 1]);
						count = 2;
						prod = (prod * 10) + second;

						if(mode[position + i + 2] == 'N') {
							third = posn(NEON, (char) jisdata[position + i + 2]);
							count = 3;
							prod = (prod * 10) + third;
						}
					}
					
					qr_bscan(binary, prod, 1 << (3 * count)); /* count = 1..3 */
					
					if(debug) { printf("0x%4X (%d)", prod, prod); }

					i += count;
				};

				if(debug) { printf("\n"); }

				break;
		}

		position += short_data_block_length;
	} while (position < length) ;

	/* Terminator */
	concat(binary, "0000");

	current_binlen = strlen(binary);
	padbits = 8 - (current_binlen % 8);
	if(padbits == 8) { padbits = 0; }
	current_bytes = (current_binlen + padbits) / 8;

	/* Padding bits */
	for(i = 0; i < padbits; i++) {
		concat(binary, "0");
	}

	/* Put data into 8-bit codewords */
	for(i = 0; i < current_bytes; i++) {
		datastream[i] = 0x00;
		if(binary[i * 8] == '1') { datastream[i] += 0x80; }
		if(binary[i * 8 + 1] == '1') { datastream[i] += 0x40; }
		if(binary[i * 8 + 2] == '1') { datastream[i] += 0x20; }
		if(binary[i * 8 + 3] == '1') { datastream[i] += 0x10; }
		if(binary[i * 8 + 4] == '1') { datastream[i] += 0x08; }
		if(binary[i * 8 + 5] == '1') { datastream[i] += 0x04; }
		if(binary[i * 8 + 6] == '1') { datastream[i] += 0x02; }
		if(binary[i * 8 + 7] == '1') { datastream[i] += 0x01; }
	}

	/* Add pad codewords */
	toggle = 0;
	for(i = current_bytes; i < target_binlen; i++) {
		if(toggle == 0) {
			datastream[i] = 0xec;
			toggle = 1;
		} else {
			datastream[i] = 0x11;
			toggle = 0;
		}
	}

	if(debug) {
		printf("Resulting codewords:\n\t");
		for(i = 0; i < target_binlen; i++) {
			printf("0x%2X ", datastream[i]);
		}
		printf("\n");
	}
}

void add_ecc(int fullstream[], int datastream[], int version, int data_cw, int blocks)
{
	/* Split data into blocks, add error correction and then interleave the blocks and error correction data */
	int ecc_cw = qr_total_codewords[version - 1] - data_cw;
	int short_data_block_length = data_cw / blocks;
	int qty_long_blocks = data_cw % blocks;
	int qty_short_blocks = blocks - qty_long_blocks;
	int ecc_block_length = ecc_cw / blocks;
	int i, j, length_this_block, posn, debug = 0;
	
	
#ifndef _MSC_VER
	unsigned char data_block[short_data_block_length + 2];
	unsigned char ecc_block[ecc_block_length + 2];
	int interleaved_data[data_cw + 2];
	int interleaved_ecc[ecc_cw + 2];
#else
	unsigned char* data_block = (unsigned char *)_alloca(short_data_block_length + 2);
	unsigned char* ecc_block = (unsigned char *)_alloca(ecc_block_length + 2);
	int* interleaved_data = (int *)_alloca((data_cw + 2) * sizeof(int));
	int* interleaved_ecc = (int *)_alloca((ecc_cw + 2) * sizeof(int));
#endif

	posn = 0;

	for(i = 0; i < blocks; i++) {
		if(i < qty_short_blocks) { length_this_block = short_data_block_length; } else { length_this_block = short_data_block_length + 1; }

		for(j = 0; j < ecc_block_length; j++) {
			ecc_block[j] = 0;
		}

		for(j = 0; j < length_this_block; j++) {
			data_block[j] = (unsigned char) datastream[posn + j];
		}

		rs_init_gf(0x11d);
		rs_init_code(ecc_block_length, 0);
		rs_encode(length_this_block, data_block, ecc_block);
		rs_free();

		if(debug) {
			printf("Block %d: ", i + 1);
			for(j = 0; j < length_this_block; j++) {
				printf("%2X ", data_block[j]);
			}
			if(i < qty_short_blocks) {
				printf("   ");
			}
			printf(" // ");
			for(j = 0; j < ecc_block_length; j++) {
				printf("%2X ", ecc_block[ecc_block_length - j - 1]);
			}
			printf("\n");
		}

		for(j = 0; j < short_data_block_length; j++) {
			interleaved_data[(j * blocks) + i] = (int) data_block[j];
		}

		if(i >= qty_short_blocks){
			interleaved_data[(short_data_block_length * blocks) + (i - qty_short_blocks)] = (int) data_block[short_data_block_length];
		}

		for(j = 0; j < ecc_block_length; j++) {
			interleaved_ecc[(j * blocks) + i] = (int) ecc_block[ecc_block_length - j - 1];
		}

		posn += length_this_block;
	}

	for(j = 0; j < data_cw; j++) {
		fullstream[j] = interleaved_data[j];
	}
	for(j = 0; j < ecc_cw; j++) {
		fullstream[j + data_cw] = interleaved_ecc[j];
	}

	if(debug) {
		printf("\nData Stream: \n");
		for(j = 0; j < (data_cw + ecc_cw); j++) {
			printf("%2X ", fullstream[j]);
		}
		printf("\n");
	}
}

void place_finder(unsigned char grid[], int size, int x, int y)
{
	int xp, yp;

	int finder[] = {
		1, 1, 1, 1, 1, 1, 1,
		1, 0, 0, 0, 0, 0, 1,
		1, 0, 1, 1, 1, 0, 1,
		1, 0, 1, 1, 1, 0, 1,
		1, 0, 1, 1, 1, 0, 1,
		1, 0, 0, 0, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1
	};

	for(xp = 0; xp < 7; xp++) {
		for(yp = 0; yp < 7; yp++) {
			if (finder[xp + (7 * yp)] == 1) {
				grid[((yp + y) * size) + (xp + x)] = 0x11;
			} else {
				grid[((yp + y) * size) + (xp + x)] = 0x10;
			}
		}
	}
}

void place_align(unsigned char grid[], int size, int x, int y)
{
	int xp, yp;

	int alignment[] = {
		1, 1, 1, 1, 1,
		1, 0, 0, 0, 1,
		1, 0, 1, 0, 1,
		1, 0, 0, 0, 1,
		1, 1, 1, 1, 1
	};

	x -= 2;
	y -= 2; /* Input values represent centre of pattern */

	for(xp = 0; xp < 5; xp++) {
		for(yp = 0; yp < 5; yp++) {
			if (alignment[xp + (5 * yp)] == 1) {
				grid[((yp + y) * size) + (xp + x)] = 0x11;
			} else {
				grid[((yp + y) * size) + (xp + x)] = 0x10;
			}
		}
	}
}

void setup_grid(unsigned char* grid, int size, int version)
{
	int i, toggle = 1;
	int loopsize, x, y, xcoord, ycoord;

	/* Add timing patterns */
	for(i = 0; i < size; i++) {
		if(toggle == 1) {
			grid[(6 * size) + i] = 0x21;
			grid[(i * size) + 6] = 0x21;
			toggle = 0;
		} else {
			grid[(6 * size) + i] = 0x20;
			grid[(i * size) + 6] = 0x20;
			toggle = 1;
		}
	}

	/* Add finder patterns */
	place_finder(grid, size, 0, 0);
	place_finder(grid, size, 0, size - 7);
	place_finder(grid, size, size - 7, 0);

	/* Add separators */
	for(i = 0; i < 7; i++) {
		grid[(7 * size) + i] = 0x10;
		grid[(i * size) + 7] = 0x10;
		grid[(7 * size) + (size - 1 - i)] = 0x10;
		grid[(i * size) + (size - 8)] = 0x10;
		grid[((size - 8) * size) + i] = 0x10;
		grid[((size - 1 - i) * size) + 7] = 0x10;
	}
	grid[(7 * size) + 7] = 0x10;
	grid[(7 * size) + (size - 8)] = 0x10;
	grid[((size - 8) * size) + 7] = 0x10;

	/* Add alignment patterns */
	if(version != 1) {
		/* Version 1 does not have alignment patterns */

		loopsize = qr_align_loopsize[version - 1];
		for(x = 0; x < loopsize; x++) {
			for(y = 0; y < loopsize; y++) {
				xcoord = qr_table_e1[((version - 2) * 7) + x];
				ycoord = qr_table_e1[((version - 2) * 7) + y];

				if(!(grid[(ycoord * size) + xcoord] & 0x10)) {
					place_align(grid, size, xcoord, ycoord);
				}
			}
		}
	}

	/* Reserve space for format information */
	for(i = 0; i < 8; i++) {
		grid[(8 * size) + i] += 0x20;
		grid[(i * size) + 8] += 0x20;
		grid[(8 * size) + (size - 1 - i)] = 0x20;
		grid[((size - 1 - i) * size) + 8] = 0x20;
	}
	grid[(8 * size) + 8] += 20;
	grid[((size - 1 - 7) * size) + 8] = 0x21; /* Dark Module from Figure 25 */

	/* Reserve space for version information */
	if(version >= 7) {
		for(i = 0; i < 6; i++) {
			grid[((size - 9) * size) + i] = 0x20;
			grid[((size - 10) * size) + i] = 0x20;
			grid[((size - 11) * size) + i] = 0x20;
			grid[(i * size) + (size - 9)] = 0x20;
			grid[(i * size) + (size - 10)] = 0x20;
			grid[(i * size) + (size - 11)] = 0x20;
		}
	}
}

int cwbit(int* datastream, int i) {
	int word = i / 8;
	int bit = i % 8;
	int resultant = 0;
	
	switch(bit) {
		case 0: if(datastream[word] & 0x80) { resultant = 1; } else { resultant = 0; } break;
		case 1: if(datastream[word] & 0x40) { resultant = 1; } else { resultant = 0; } break;
		case 2: if(datastream[word] & 0x20) { resultant = 1; } else { resultant = 0; } break;
		case 3: if(datastream[word] & 0x10) { resultant = 1; } else { resultant = 0; } break;
		case 4: if(datastream[word] & 0x08) { resultant = 1; } else { resultant = 0; } break;
		case 5: if(datastream[word] & 0x04) { resultant = 1; } else { resultant = 0; } break;
		case 6: if(datastream[word] & 0x02) { resultant = 1; } else { resultant = 0; } break;
		case 7: if(datastream[word] & 0x01) { resultant = 1; } else { resultant = 0; } break;
	}
	
	return resultant;
}

void populate_grid(unsigned char* grid, int size, int* datastream, int cw)
{
	int direction = 1; /* up */
	int row = 0; /* right hand side */

	int i, n, x, y;

	n = cw * 8;
	y = size - 1;
	i = 0;
	do {
		x = (size - 2) - (row * 2);
		if(x < 6)
			x--; /* skip over vertical timing pattern */

		if(!(grid[(y * size) + (x + 1)] & 0xf0)) {
			if (cwbit(datastream, i)) {
				grid[(y * size) + (x + 1)] = 0x01;
			} else {
				grid[(y * size) + (x + 1)] = 0x00;
			}
			i++;
		}

		if(i < n) {
			if(!(grid[(y * size) + x] & 0xf0)) {
				if (cwbit(datastream, i)) {
					grid[(y * size) + x] = 0x01;
				} else {
					grid[(y * size) + x] = 0x00;
				}
				i++;
			}
		}

		if(direction) { y--; } else { y++; }
		if(y == -1) {
			/* reached the top */
			row++;
			y = 0;
			direction = 0;
		}
		if(y == size) {
			/* reached the bottom */
			row++;
			y = size - 1;
			direction = 1;
		}
	} while (i < n);
}

#ifdef ZINTLOG 
int append_log(char log)
{  
	FILE *file;
	
	file = fopen("zintlog.txt", "a+");   
	fprintf(file, "%c", log); 
	fclose(file);   
	return 0;
}

int write_log(char log[])
{  
	FILE *file;
	
	file = fopen("zintlog.txt", "a+");   
	fprintf(file, log); /*writes*/
	fprintf(file, "\r\n"); /*writes*/
	fclose(file);   
	return 0;
}
#endif

int evaluate(unsigned char *grid, int size, int pattern)
{ 
	int x, y, block;
	int result = 0;
	int result_b = 0;
	char state;
	int p;
	int dark_mods; 
	int percentage, k, k2;

#ifndef _MSC_VER
	char local[size * size];
#else
	char* local = (char *)_alloca((size * size) * sizeof(char));
#endif


#ifdef ZINTLOG 
	char str[15];
	write_log("");
	sprintf(str, "%d", pattern);
	write_log(str);
#endif

	// all eight bitmask variants have been encoded in the 8 bits of the bytes that make up the grid array. select them for evaluation according to the desired pattern.
	for(x = 0; x < size; x++) {
		for(y = 0; y < size; y++) {
			switch(pattern) {
				case 0: if (grid[(y * size) + x] & 0x01) { local[(y * size) + x] = '1'; } else { local[(y * size) + x] = '0'; } break;
				case 1: if (grid[(y * size) + x] & 0x02) { local[(y * size) + x] = '1'; } else { local[(y * size) + x] = '0'; } break;
				case 2: if (grid[(y * size) + x] & 0x04) { local[(y * size) + x] = '1'; } else { local[(y * size) + x] = '0'; } break;
				case 3: if (grid[(y * size) + x] & 0x08) { local[(y * size) + x] = '1'; } else { local[(y * size) + x] = '0'; } break;
				case 4: if (grid[(y * size) + x] & 0x10) { local[(y * size) + x] = '1'; } else { local[(y * size) + x] = '0'; } break;
				case 5: if (grid[(y * size) + x] & 0x20) { local[(y * size) + x] = '1'; } else { local[(y * size) + x] = '0'; } break;
				case 6: if (grid[(y * size) + x] & 0x40) { local[(y * size) + x] = '1'; } else { local[(y * size) + x] = '0'; } break;
				case 7: if (grid[(y * size) + x] & 0x80) { local[(y * size) + x] = '1'; } else { local[(y * size) + x] = '0'; } break;
			}
		}
	}

#ifdef ZINTLOG 	 
	//bitmask output
	for(y = 0; y < size; y++) { 
		strcpy (str, "");
		for(x = 0; x < size; x++) {  
			state =local[(y * size) + x];  
			append_log(state);
		} 
		write_log("");		
	}
	write_log("");
#endif

	/* Test 1: Adjacent modules in row/column in same colour */
	/* Vertical */ 
	for(x = 0; x < size; x++) {
		state = local[x];
		block = 0; 
		for(y = 0; y < size; y++) {
			if(local[(y * size) + x] == state) {
				block++;				
				if(block ==5)
					result += 3;
				
				if(block>5)				
					result +=1;
			} else {
				block=0;
			}
		} 
	}

	/* Horizontal */
	for(y = 0; y < size; y++) {
		state = local[y * size];
		block = 0;
		for(x = 0; x < size; x++) {
			if(local[(y * size) + x] == state) {
				block++;				
				if(block ==5)
					result += 3;
				
				if(block>5)				
					result +=1;
			} else {
				block=0;
			}
		} 
	}
	 
#ifdef ZINTLOG 
	/* output Test 1 */
	sprintf(str, "%d", result);
	result_b=result;
	write_log(str);
#endif
	
	/* Test 2 fd02131114 */
	for(x = 0; x < size-1; x++) {
		for(y = 0; y < (size - 7) -1; y++) {	
			// y + 1???		
			if((local[((y + 1) * size) + x] == '1') && 
				(local[((y + 1) * size) + x+1] == '1') &&
				(local[(((y + 1)+1) * size) + x] == '1') &&
				(local[(((y + 1)+1) * size) + x+1] == '1') 
				) { result += 3; }		 

			if((local[((y + 1) * size) + x] == '0') && 
				(local[((y + 1) * size) + x+1] == '0') &&
				(local[(((y + 1)+1) * size) + x] == '0') &&
				(local[(((y + 1)+1) * size) + x+1] == '0') 
				) { result += 3; }		
		}
	}

#ifdef ZINTLOG 
	/* output Test 2 */
	sprintf(str, "%d", result-result_b);
	result_b=result;
	write_log(str);
#endif

	/* Test 3: fd02131114 */ 
	/*pattern 10111010000 */ 
	/* Vertical */ 
	for(x = 0; x < size; x++) {
		for(y = 0; y < (size - 11); y++) {
			p = 0;
			if(local[(y * size) + x] == '1') { p += 1; }
			if(local[((y + 1) * size) + x] == '0') { p += 1; }
			if(local[((y + 2) * size) + x] == '1') { p += 1; }
			if(local[((y + 3) * size) + x] == '1') { p += 1; }
			if(local[((y + 4) * size) + x] == '1') { p += 1; }
			if(local[((y + 5) * size) + x] == '0') { p += 1; }
			if(local[((y + 6) * size) + x] == '1') { p += 1; }
			if(local[((y + 7) * size) + x] == '0') { p += 1; }
			if(local[((y + 8) * size) + x] == '0') { p += 1; }
			if(local[((y + 9) * size) + x] == '0') { p += 1; }
			if(local[((y + 10) * size) + x] == '0') { p += 1; } 		
			if(p == 11) {
				result += 40;
			}
		}
	}

	/* Horizontal */
	for(y = 0; y < size; y++) {
		for(x = 0; x < (size - 11); x++) {
			p = 0;
			if(local[(y * size) + x] == '1') { p += 1; }
			if(local[(y * size) + x + 1] == '0') { p += 1; }
			if(local[(y * size) + x + 2] == '1') { p += 1; }
			if(local[(y * size) + x + 3] == '1') { p += 1; }
			if(local[(y * size) + x + 4] == '1') { p += 1; }
			if(local[(y * size) + x + 5] == '0') { p += 1; }
			if(local[(y * size) + x + 6] == '1') { p += 1; }
			if(local[(y * size) + x + 7] == '0') { p += 1; }
			if(local[(y * size) + x + 8] == '0') { p += 1; }
			if(local[(y * size) + x + 9] == '0') { p += 1; }
			if(local[(y * size) + x + 10] == '0') { p += 1; }
			if(p == 11) {
				result += 40;
			}
		}
	}

	/*pattern 00001011101 */ 
	/* Vertical */ 
	for(x = 0; x < size; x++) {
		for(y = 0; y < (size - 11); y++) {
			p = 0;
			if(local[(y * size) + x] == '0') { p += 1; }
			if(local[((y + 1) * size) + x] == '0') { p += 1; }
			if(local[((y + 2) * size) + x] == '0') { p += 1; }
			if(local[((y + 3) * size) + x] == '0') { p += 1; }
			if(local[((y + 4) * size) + x] == '1') { p += 1; }
			if(local[((y + 5) * size) + x] == '0') { p += 1; }
			if(local[((y + 6) * size) + x] == '1') { p += 1; }
			if(local[((y + 7) * size) + x] == '1') { p += 1; }
			if(local[((y + 8) * size) + x] == '1') { p += 1; }
			if(local[((y + 9) * size) + x] == '0') { p += 1; }
			if(local[((y + 10) * size) + x] == '1') { p += 1; } 		
			if(p == 11) {
				result += 40;
			}
		}
	}

	/* Horizontal */
	for(y = 0; y < size; y++) {
		for(x = 0; x < (size - 11); x++) {
			p = 0;
			if(local[(y * size) + x] == '0') { p += 1; }
			if(local[(y * size) + x + 1] == '0') { p += 1; }
			if(local[(y * size) + x + 2] == '0') { p += 1; }
			if(local[(y * size) + x + 3] == '0') { p += 1; }
			if(local[(y * size) + x + 4] == '1') { p += 1; }
			if(local[(y * size) + x + 5] == '0') { p += 1; }
			if(local[(y * size) + x + 6] == '1') { p += 1; }
			if(local[(y * size) + x + 7] == '1') { p += 1; }
			if(local[(y * size) + x + 8] == '1') { p += 1; }
			if(local[(y * size) + x + 9] == '0') { p += 1; }
			if(local[(y * size) + x + 10] == '1') { p += 1; }
			if(p == 11) {
				result += 40;
			}
		}
	} 

#ifdef ZINTLOG 
	/* output Test 3 */
	sprintf(str, "%d", result-result_b);
	result_b=result;
	write_log(str);
#endif

	/* Test 4: Proportion of dark modules in entire symbol */
	dark_mods = 0; 
	for(x = 0; x < size; x++) {
		for(y = 0; y < size; y++) {
			if(local[(y * size) + x] == '1') {
				dark_mods++;
			} 
		}
	}
	percentage = 100 * (dark_mods / (size * size));   
	int m=0;
	for(x = 0; x < 100; x+=5) {
		if(x<percentage)
			m=x;
	}

		
	k=abs((m-50)/5);
	k2=abs((m+5-50)/5);

	int smallest=k;
	if(k2<smallest)
		smallest=k2;
	 
	result += 10 * smallest;

#ifdef ZINTLOG 
	/* output Test 4+summary */
	sprintf(str, "%d", result-result_b); 
	write_log(str); 
	write_log("==========");
	sprintf(str, "%d", result); 
	write_log(str);
#endif 

	return result;
}

void add_format_info_eval(unsigned char *eval, int size, int ecc_level, int pattern)
{
	/* Add format information to grid */

	int format = pattern;
	unsigned int seq;
	int i;

	switch(ecc_level) {
		case LEVEL_L: format += 0x08; break;
		case LEVEL_Q: format += 0x18; break;
		case LEVEL_H: format += 0x10; break;
	}

	seq = qr_annex_c[format];
	
	for(i = 0; i < 6; i++) {
		eval[(i * size) + 8] = (seq >> i) & 0x01 ? (0x01 >> pattern) : 0x00;
	}

	for(i = 0; i < 8; i++) {
		eval[(8 * size) + (size - i - 1)] = (seq >> i) & 0x01 ? (0x01 >> pattern) : 0x00;
	}

	for(i = 0; i < 6; i++) {
		eval[(8 * size) + (5 - i)] = (seq >> (i + 9)) & 0x01 ? (0x01 >> pattern) : 0x00;
	}

	for(i = 0; i < 7; i++) {
		eval[(((size - 7) + i) * size) + 8] = (seq >> (i + 8)) & 0x01 ? (0x01 >> pattern) : 0x00;
	}

	eval[(7 * size) + 8] = (seq >> 6) & 0x01 ? (0x01 >> pattern) : 0x00;
	eval[(8 * size) + 8] = (seq >> 7) & 0x01 ? (0x01 >> pattern) : 0x00;
	eval[(8 * size) + 7] = (seq >> 8) & 0x01 ? (0x01 >> pattern) : 0x00;
}

int apply_bitmask(unsigned char *grid, int size, int ecc_level)
{
	int x, y;
	unsigned char p;
	int pattern, penalty[8];
	int best_val, best_pattern;
	int bit;
	
#ifndef _MSC_VER
	unsigned char mask[size * size];
	unsigned char eval[size * size];
#else
	unsigned char* mask = (unsigned char *)_alloca((size * size) * sizeof(unsigned char));
	unsigned char* eval = (unsigned char *)_alloca((size * size) * sizeof(unsigned char));
#endif

	/* Perform data masking */
	for(x = 0; x < size; x++) {
		for(y = 0; y < size; y++) {
			mask[(y * size) + x] = 0x00;
			
			// all eight bitmask variants are encoded in the 8 bits of the bytes that make up the mask array. 
			if (!(grid[(y * size) + x] & 0xf0)) { // exclude areas not to be masked.
				if(((y + x) & 1) == 0) { mask[(y * size) + x] += 0x01; }
				if((y & 1) == 0) { mask[(y * size) + x] += 0x02; }
				if((x % 3) == 0) { mask[(y * size) + x] += 0x04; }
				if(((y + x) % 3) == 0) { mask[(y * size) + x] += 0x08; }
				if((((y / 2) + (x / 3)) & 1) == 0) { mask[(y * size) + x] += 0x10; }
				if((((y * x) & 1) + ((y * x) % 3)) == 0) { mask[(y * size) + x] += 0x20; }
				if(((((y * x) & 1) + ((y * x) % 3)) & 1) == 0) { mask[(y * size) + x] += 0x40; }
				if(((((y + x) & 1) + ((y * x) % 3)) & 1) == 0) { mask[(y * size) + x] += 0x80; }
			}
		}
	}

	// apply data masks to grid, result in eval
	for(x = 0; x < size; x++) {
		for(y = 0; y < size; y++) {
			if(grid[(y * size) + x] & 0x01) 
			{ p = 0xff; } 
			else { p = 0x00; }

			eval[(y * size) + x] = mask[(y * size) + x] ^ p;
		}
	}


	/* Evaluate result */
	for(pattern = 0; pattern < 8; pattern++) {
	
	    add_format_info_eval(eval, size, ecc_level, pattern);
	
		penalty[pattern] = evaluate(eval, size, pattern);
	}

	best_pattern = 0;
	best_val = penalty[0];
	for(pattern = 1; pattern < 8; pattern++) {
		if(penalty[pattern] < best_val) {
			best_pattern = pattern;
			best_val = penalty[pattern];
		}
	}

#ifdef ZINTLOG 
	char str[15];
	sprintf(str, "%d", best_val);
	write_log("choosed pattern:");
	write_log(str);
#endif

	/* Apply mask */
	for(x = 0; x < size; x++) {
		for(y = 0; y < size; y++) {
			bit = 0;
			switch(best_pattern) {
				case 0: if(mask[(y * size) + x] & 0x01) { bit = 1; } break;
				case 1: if(mask[(y * size) + x] & 0x02) { bit = 1; } break;
				case 2: if(mask[(y * size) + x] & 0x04) { bit = 1; } break;
				case 3: if(mask[(y * size) + x] & 0x08) { bit = 1; } break;
				case 4: if(mask[(y * size) + x] & 0x10) { bit = 1; } break;
				case 5: if(mask[(y * size) + x] & 0x20) { bit = 1; } break;
				case 6: if(mask[(y * size) + x] & 0x40) { bit = 1; } break;
				case 7: if(mask[(y * size) + x] & 0x80) { bit = 1; } break;
			}
			if(bit == 1) {
				if(grid[(y * size) + x] & 0x01) {
					grid[(y * size) + x] = 0x00;
				} else {
					grid[(y * size) + x] = 0x01;
				}
			}
		}
	}

	return best_pattern;
}

void add_format_info(unsigned char *grid, int size, int ecc_level, int pattern)
{
	/* Add format information to grid */

	int format = pattern;
	unsigned int seq;
	int i;

	switch(ecc_level) {
		case LEVEL_L: format += 0x08; break;
		case LEVEL_Q: format += 0x18; break;
		case LEVEL_H: format += 0x10; break;
	}

	seq = qr_annex_c[format];

	for(i = 0; i < 6; i++) {
		grid[(i * size) + 8] += (seq >> i) & 0x01;
	}

	for(i = 0; i < 8; i++) {
		grid[(8 * size) + (size - i - 1)] += (seq >> i) & 0x01;
	}

	for(i = 0; i < 6; i++) {
		grid[(8 * size) + (5 - i)] += (seq >> (i + 9)) & 0x01;
	}

	for(i = 0; i < 7; i++) {
		grid[(((size - 7) + i) * size) + 8] += (seq >> (i + 8)) & 0x01;
	}

	grid[(7 * size) + 8] += (seq >> 6) & 0x01;
	grid[(8 * size) + 8] += (seq >> 7) & 0x01;
	grid[(8 * size) + 7] += (seq >> 8) & 0x01;
}

void add_version_info(unsigned char *grid, int size, int version)
{
	/* Add version information */
	int i;

	long int version_data = qr_annex_d[version - 7];
	for(i = 0; i < 6; i++) {
		grid[((size - 11) * size) + i] += (version_data >> (i * 3)) & 0x41;
		grid[((size - 10) * size) + i] += (version_data >> ((i * 3) + 1)) & 0x41;
		grid[((size - 9) * size) + i] += (version_data >> ((i * 3) + 2)) & 0x41;
		grid[(i * size) + (size - 11)] += (version_data >> (i * 3)) & 0x41;
		grid[(i * size) + (size - 10)] += (version_data >> ((i * 3) + 1)) & 0x41;
		grid[(i * size) + (size - 9)] += (version_data >> ((i * 3) + 2)) & 0x41;
	}
}

int qr_code(struct zint_symbol *symbol, unsigned char source[], int length)
{
	int error_number, i, j, glyph, est_binlen;
	int ecc_level, autosize, version, max_cw, target_binlen, blocks, size;
	int bitmask, gs1;
	
#ifndef _MSC_VER
	int utfdata[length + 1];
	int jisdata[length + 1];
	char mode[length + 1];
#else
	int* utfdata = (int *)_alloca((length + 1) * sizeof(int));
	int* jisdata = (int *)_alloca((length + 1) * sizeof(int));
	char* mode = (char *)_alloca(length + 1);
#endif
	
	gs1 = (symbol->input_mode == GS1_MODE);

	switch(symbol->input_mode) {
		case DATA_MODE:
			for(i = 0; i < length; i++) {
				jisdata[i] = (int)source[i];
			}
			break;
		default:
			/* Convert Unicode input to Shift-JIS */
			error_number = utf8toutf16(symbol, source, utfdata, &length);
			if(error_number != 0) { return error_number; }
			
			for(i = 0; i < length; i++) {
				if(utfdata[i] <= 0xff) {
					jisdata[i] = utfdata[i];
				} else {
					j = 0;
					glyph = 0;
					do {
						if(sjis_lookup[j * 2] == utfdata[i]) {
							glyph = sjis_lookup[(j * 2) + 1];
						}
						j++;
					} while ((j < 6843) && (glyph == 0));
					if(glyph == 0) {
						strcpy(symbol->errtxt, "Invalid character in input data");
						return ZINT_ERROR_INVALID_DATA;
					}
					jisdata[i] = glyph;
				}
			}
			break;
	}

	define_mode(mode, jisdata, length, gs1);
	est_binlen = estimate_binary_length(mode, length, gs1);

	ecc_level = LEVEL_L;
	max_cw = 2956;
	if((symbol->option_1 >= 1) && (symbol->option_1 <= 4)) {
		switch (symbol->option_1) {
			case 1: ecc_level = LEVEL_L; max_cw = 2956; break;
			case 2: ecc_level = LEVEL_M; max_cw = 2334; break;
			case 3: ecc_level = LEVEL_Q; max_cw = 1666; break;
			case 4: ecc_level = LEVEL_H; max_cw = 1276; break;
		}
	}

	if(est_binlen > (8 * max_cw)) {
		strcpy(symbol->errtxt, "Input too long for selected error correction level");
		return ZINT_ERROR_TOO_LONG;
	}

	autosize = 40;
	for(i = 39; i >= 0; i--) {
		switch(ecc_level) {
			case LEVEL_L:
				if ((8 * qr_data_codewords_L[i]) >= est_binlen) {
					autosize = i + 1;
				}
				break;
			case LEVEL_M:
				if ((8 * qr_data_codewords_M[i]) >= est_binlen) {
					autosize = i + 1;
				}
				break;
			case LEVEL_Q:
				if ((8 * qr_data_codewords_Q[i]) >= est_binlen) {
					autosize = i + 1;
				}
				break;
			case LEVEL_H:
				if ((8 * qr_data_codewords_H[i]) >= est_binlen) {
					autosize = i + 1;
				}
				break;
		}
	}

	if((symbol->option_2 >= 1) && (symbol->option_2 <= 40)) {
		if (symbol->option_2 > autosize) {
			version = symbol->option_2;
		} else {
			version = autosize;
		}
	} else {
		version = autosize;
	}

	/* Ensure maxium error correction capacity */
	if(est_binlen <= qr_data_codewords_M[version - 1]) { ecc_level = LEVEL_M; }
	if(est_binlen <= qr_data_codewords_Q[version - 1]) { ecc_level = LEVEL_Q; }
	if(est_binlen <= qr_data_codewords_H[version - 1]) { ecc_level = LEVEL_H; }

	target_binlen = qr_data_codewords_L[version - 1]; blocks = qr_blocks_L[version - 1];
	switch(ecc_level) {
		case LEVEL_M: target_binlen = qr_data_codewords_M[version - 1]; blocks = qr_blocks_M[version - 1]; break;
		case LEVEL_Q: target_binlen = qr_data_codewords_Q[version - 1]; blocks = qr_blocks_Q[version - 1]; break;
		case LEVEL_H: target_binlen = qr_data_codewords_H[version - 1]; blocks = qr_blocks_H[version - 1]; break;
	}
	
#ifndef _MSC_VER
	int datastream[target_binlen + 1];
	int fullstream[qr_total_codewords[version - 1] + 1];
#else
	int* datastream = (int *)_alloca((target_binlen + 1) * sizeof(int));
	int* fullstream = (int *)_alloca((qr_total_codewords[version - 1] + 1) * sizeof(int));
#endif

	qr_binary(datastream, version, target_binlen, mode, jisdata, length, gs1, est_binlen);
	add_ecc(fullstream, datastream, version, target_binlen, blocks);

	size = qr_sizes[version - 1];
#ifndef _MSC_VER
	unsigned char grid[size * size];
#else
	unsigned char* grid = (unsigned char *)_alloca((size * size) * sizeof(unsigned char));
#endif
	
	for(i = 0; i < size; i++) {
		for(j = 0; j < size; j++) {
			grid[(i * size) + j] = 0;
		}
	}

	setup_grid(grid, size, version);
	populate_grid(grid, size, fullstream, qr_total_codewords[version - 1]);

	if(version >= 7) {
		add_version_info(grid, size, version);
	}
	
	bitmask = apply_bitmask(grid, size, ecc_level);

	add_format_info(grid, size, ecc_level, bitmask);
	
	

	symbol->width = size;
	symbol->rows = size;
	
	for(i = 0; i < size; i++) {
		for(j = 0; j < size; j++) {
			if(grid[(i * size) + j] & 0x01) {
				set_module(symbol, i, j);
			}
		}
		symbol->row_height[i] = 1;
	}

	return 0;
}

/* NOTE: From this point forward concerns Micro QR Code only */

int micro_qr_intermediate(char binary[], int jisdata[], char mode[], int length, int *kanji_used, int *alphanum_used, int *byte_used)
{
	/* Convert input data to an "intermediate stage" where data is binary encoded but
	   control information is not */
	int position = 0, debug = 0;
	int short_data_block_length, i;
	char data_block;
	char buffer[2];

	strcpy(binary, "");
	
	if(debug) { 
		for(i = 0; i < length; i++) {
			printf("%c", mode[i]);
		}
		printf("\n");
	}

	do {
		if(strlen(binary) > 128) {
			return ZINT_ERROR_TOO_LONG;
		}

		data_block = mode[position];
		short_data_block_length = 0;
		do {
			short_data_block_length++;
		} while (((short_data_block_length + position) < length) && (mode[position + short_data_block_length] == data_block));

		switch(data_block) {
			case 'K':
				/* Kanji mode */
				/* Mode indicator */
				concat(binary, "K");
				*kanji_used = 1;

				/* Character count indicator */
				buffer[0] = short_data_block_length;
				buffer[1] = '\0';
				concat(binary, buffer);

				if(debug) { printf("Kanji block (length %d)\n\t", short_data_block_length); }

				/* Character representation */
				for(i = 0; i < short_data_block_length; i++) {
					int jis = jisdata[position + i];
					int msb, lsb, prod;

					if(jis > 0x9fff) { jis -= 0xc140; }
					msb = (jis & 0xff00) >> 4;
					lsb = (jis & 0xff);
					prod = (msb * 0xc0) + lsb;
				
					qr_bscan(binary, prod, 0x1000);
					
					if(debug) { printf("0x%4X ", prod); }

					if(strlen(binary) > 128) {
						return ZINT_ERROR_TOO_LONG;
					}
				}

				if(debug) { printf("\n"); }

				break;
			case 'B':
				/* Byte mode */
				/* Mode indicator */
				concat(binary, "B");
				*byte_used = 1;

				/* Character count indicator */
				buffer[0] = short_data_block_length;
				buffer[1] = '\0';
				concat(binary, buffer);

				if(debug) { printf("Byte block (length %d)\n\t", short_data_block_length); }

				/* Character representation */
				for(i = 0; i < short_data_block_length; i++) {
					int byte = jisdata[position + i];
					
					qr_bscan(binary, byte, 0x80);
					
					if(debug) { printf("0x%4X ", byte); }

					if(strlen(binary) > 128) {
						return ZINT_ERROR_TOO_LONG;
					}
				}

				if(debug) { printf("\n"); }

				break;
			case 'A':
				/* Alphanumeric mode */
				/* Mode indicator */
				concat(binary, "A");
				*alphanum_used = 1;

				/* Character count indicator */
				buffer[0] = short_data_block_length;
				buffer[1] = '\0';
				concat(binary, buffer);

				if(debug) { printf("Alpha block (length %d)\n\t", short_data_block_length); }

				/* Character representation */
				i = 0; 
				while ( i < short_data_block_length ) {
					int count;
					int first = 0, second = 0, prod;

					first = posn(RHODIUM, (char) jisdata[position + i]);
					count = 1;
					prod = first;

					if(mode[position + i + 1] == 'A') {
						second = posn(RHODIUM, (char) jisdata[position + i + 1]);
						count = 2;
						prod = (first * 45) + second;
					}
				
					qr_bscan(binary, prod, 1 << (5 * count)); /* count = 1..2 */
					
					if(debug) { printf("0x%4X ", prod); }

					if(strlen(binary) > 128) {
						return ZINT_ERROR_TOO_LONG;
					}

					i += 2;
				};

				if(debug) { printf("\n"); }

				break;
			case 'N':
				/* Numeric mode */
				/* Mode indicator */
				concat(binary, "N");

				/* Character count indicator */
				buffer[0] = short_data_block_length;
				buffer[1] = '\0';
				concat(binary, buffer);

				if(debug) { printf("Number block (length %d)\n\t", short_data_block_length); }

				/* Character representation */
				i = 0;
				while ( i < short_data_block_length ) {
					int count;
					int first = 0, second = 0, third = 0, prod;

					first = posn(NEON, (char) jisdata[position + i]);
					count = 1;
					prod = first;

					if(mode[position + i + 1] == 'N') {
						second = posn(NEON, (char) jisdata[position + i + 1]);
						count = 2;
						prod = (prod * 10) + second;
					}

					if(mode[position + i + 2] == 'N') {
						third = posn(NEON, (char) jisdata[position + i + 2]);
						count = 3;
						prod = (prod * 10) + third;
					}
				
					qr_bscan(binary, prod, 1 << (3 * count)); /* count = 1..3 */
					
					if(debug) { printf("0x%4X (%d)", prod, prod); }
					
					if(strlen(binary) > 128) {
						return ZINT_ERROR_TOO_LONG;
					}
					
					i += 3;
				};
				
				if(debug) { printf("\n"); }
				
				break;
		}
		
		position += short_data_block_length;
	} while (position < length - 1) ;
	
	return 0;
}

void get_bitlength(int count[], char stream[]) {
	int length, i;
	
	length = strlen(stream);
	
	for(i = 0; i < 4; i++) {
		count[i] = 0;
	}
	
	i = 0;
	do {
		if((stream[i] == '0') || (stream[i] == '1')) {
			count[0]++;
			count[1]++;
			count[2]++;
			count[3]++;
			i++;
		} else {
			switch(stream[i]) {
				case 'K':
					count[2] += 5;
					count[3] += 7;
					i += 2;
					break;
				case 'B':
					count[2] += 6;
					count[3] += 8;
					i += 2;
					break;
				case 'A':
					count[1] += 4;
					count[2] += 6;
					count[3] += 8;
					i += 2;
					break;
				case 'N':
					count[0] += 3;
					count[1] += 5;
					count[2] += 7;
					count[3] += 9;
					i += 2;
					break;
			}
		}
	} while (i < length);
}

void microqr_expand_binary(char binary_stream[], char full_stream[], int version)
{
	int i, length;
	
	length = strlen(binary_stream);
	
	i = 0;
	do {
		switch(binary_stream[i]) {
			case '1': concat(full_stream, "1"); i++; break;
			case '0': concat(full_stream, "0"); i++; break;
			case 'N':
				/* Numeric Mode */
				/* Mode indicator */
				switch(version) {
					case 1: concat(full_stream, "0"); break;
					case 2: concat(full_stream, "00"); break;
					case 3: concat(full_stream, "000"); break;
				}
				
				/* Character count indicator */
				qr_bscan(full_stream, binary_stream[i + 1], 4 << version); /* version = 0..3 */
				
				i += 2;
				break;
			case 'A':
				/* Alphanumeric Mode */
				/* Mode indicator */
				switch(version) {
					case 1: concat(full_stream, "1"); break;
					case 2: concat(full_stream, "01"); break;
					case 3: concat(full_stream, "001"); break;
				}
				
				/* Character count indicator */
				qr_bscan(full_stream, binary_stream[i + 1], 2 << version); /* version = 1..3 */
				
				i += 2;
				break;
			case 'B':
				/* Byte Mode */
				/* Mode indicator */
				switch(version) {
					case 2: concat(full_stream, "10"); break;
					case 3: concat(full_stream, "010"); break;
				}
				
				/* Character count indicator */
				qr_bscan(full_stream, binary_stream[i + 1], 2 << version); /* version = 2..3 */
				
				i += 2;
				break;
			case 'K':
				/* Kanji Mode */
				/* Mode indicator */
				switch(version) {
					case 2: concat(full_stream, "11"); break;
					case 3: concat(full_stream, "011"); break;
				}
				
				/* Character count indicator */
				qr_bscan(full_stream, binary_stream[i + 1], 1 << version); /* version = 2..3 */
				
				i += 2;
				break;
		}
			
	} while (i < length);
}

void micro_qr_m1(char binary_data[])
{
	int i, latch;
	int bits_total, bits_left, remainder;
	int data_codewords, ecc_codewords;
	unsigned char data_blocks[4], ecc_blocks[3];
	
	bits_total = 20;
	latch = 0;
	
	/* Add terminator */
	bits_left = bits_total - strlen(binary_data);
	if(bits_left <= 3) {
		for(i = 0; i < bits_left; i++) {
			concat(binary_data, "0");
		}
		latch = 1;
	} else {
		concat(binary_data, "000");
	}
	
	if(latch == 0) {
		/* Manage last (4-bit) block */
		bits_left = bits_total - strlen(binary_data);
		if(bits_left <= 4) {
			for(i = 0; i < bits_left; i++) {
				concat(binary_data, "0");
			}
			latch = 1;
		}
	}

	if(latch == 0) {
		/* Complete current byte */
		remainder = 8 - (strlen(binary_data) % 8);
		if(remainder == 8) { remainder = 0; }
		for(i = 0; i < remainder; i++) {
			concat(binary_data, "0");
		}
		
		/* Add padding */
		bits_left = bits_total - strlen(binary_data);
		if(bits_left > 4) {
			remainder = (bits_left - 4) / 8;
			for(i = 0; i < remainder; i++) {
				concat(binary_data, i & 1 ? "00010001" : "11101100"); 
			}
		}
		concat(binary_data, "0000");
	}
	
	data_codewords = 3;
	ecc_codewords = 2;
	
	/* Copy data into codewords */
	for(i = 0; i < (data_codewords - 1); i++) {
		data_blocks[i] = 0;
		if(binary_data[i * 8] == '1') { data_blocks[i] += 0x80; }
		if(binary_data[(i * 8) + 1] == '1') { data_blocks[i] += 0x40; }
		if(binary_data[(i * 8) + 2] == '1') { data_blocks[i] += 0x20; }
		if(binary_data[(i * 8) + 3] == '1') { data_blocks[i] += 0x10; }
		if(binary_data[(i * 8) + 4] == '1') { data_blocks[i] += 0x08; }
		if(binary_data[(i * 8) + 5] == '1') { data_blocks[i] += 0x04; }
		if(binary_data[(i * 8) + 6] == '1') { data_blocks[i] += 0x02; }
		if(binary_data[(i * 8) + 7] == '1') { data_blocks[i] += 0x01; }
	}
	data_blocks[2] = 0;
	if(binary_data[16] == '1') { data_blocks[2] += 0x08; }
	if(binary_data[17] == '1') { data_blocks[2] += 0x04; }
	if(binary_data[18] == '1') { data_blocks[2] += 0x02; }
	if(binary_data[19] == '1') { data_blocks[2] += 0x01; }
	
	/* Calculate Reed-Solomon error codewords */
	rs_init_gf(0x11d);
	rs_init_code(ecc_codewords, 0);
	rs_encode(data_codewords,data_blocks,ecc_blocks);
	rs_free();
	
	/* Add Reed-Solomon codewords to binary data */
	for(i = 0; i < ecc_codewords; i++) {
		qr_bscan(binary_data, ecc_blocks[ecc_codewords - i - 1], 0x80);
	}
}

void micro_qr_m2(char binary_data[], int ecc_mode)
{
	int i, latch;
	int bits_total, bits_left, remainder;
	int data_codewords, ecc_codewords;
	unsigned char data_blocks[6], ecc_blocks[7];
	
	latch = 0;
	
	if(ecc_mode == LEVEL_L) { bits_total = 40; }
	if(ecc_mode == LEVEL_M) { bits_total = 32; }
	
	/* Add terminator */
	bits_left = bits_total - strlen(binary_data);
	if(bits_left <= 5) {
		for(i = 0; i < bits_left; i++) {
			concat(binary_data, "0");
		}
		latch = 1;
	} else {
		concat(binary_data, "00000");
	}

	if(latch == 0) {
		/* Complete current byte */
		remainder = 8 - (strlen(binary_data) % 8);
		if(remainder == 8) { remainder = 0; }
		for(i = 0; i < remainder; i++) {
			concat(binary_data, "0");
		}
		
		/* Add padding */
		bits_left = bits_total - strlen(binary_data);
		remainder = bits_left / 8;
		for(i = 0; i < remainder; i++) {
			concat(binary_data, i & 1 ? "00010001" : "11101100"); 
		}
	}
	
	if(ecc_mode == LEVEL_L) { data_codewords = 5; ecc_codewords = 5; }
	if(ecc_mode == LEVEL_M) { data_codewords = 4; ecc_codewords = 6; }
	
	/* Copy data into codewords */
	for(i = 0; i < data_codewords; i++) {
		data_blocks[i] = 0;
		if(binary_data[i * 8] == '1') { data_blocks[i] += 0x80; }
		if(binary_data[(i * 8) + 1] == '1') { data_blocks[i] += 0x40; }
		if(binary_data[(i * 8) + 2] == '1') { data_blocks[i] += 0x20; }
		if(binary_data[(i * 8) + 3] == '1') { data_blocks[i] += 0x10; }
		if(binary_data[(i * 8) + 4] == '1') { data_blocks[i] += 0x08; }
		if(binary_data[(i * 8) + 5] == '1') { data_blocks[i] += 0x04; }
		if(binary_data[(i * 8) + 6] == '1') { data_blocks[i] += 0x02; }
		if(binary_data[(i * 8) + 7] == '1') { data_blocks[i] += 0x01; }
	}
	
	/* Calculate Reed-Solomon error codewords */
	rs_init_gf(0x11d);
	rs_init_code(ecc_codewords, 0);
	rs_encode(data_codewords,data_blocks,ecc_blocks);
	rs_free();
	
	/* Add Reed-Solomon codewords to binary data */
	for(i = 0; i < ecc_codewords; i++) {
		qr_bscan(binary_data, ecc_blocks[ecc_codewords - i - 1], 0x80);
	}
	
	return;
}

void micro_qr_m3(char binary_data[], int ecc_mode)
{
	int i, latch;
	int bits_total, bits_left, remainder;
	int data_codewords, ecc_codewords;
	unsigned char data_blocks[12], ecc_blocks[9];
	
	latch = 0;
	
	if(ecc_mode == LEVEL_L) { bits_total = 84; }
	if(ecc_mode == LEVEL_M) { bits_total = 68; }
	
	/* Add terminator */
	bits_left = bits_total - strlen(binary_data);
	if(bits_left <= 7) {
		for(i = 0; i < bits_left; i++) {
			concat(binary_data, "0");
		}
		latch = 1;
	} else {
		concat(binary_data, "0000000");
	}
	
	if(latch == 0) {
		/* Manage last (4-bit) block */
		bits_left = bits_total - strlen(binary_data);
		if(bits_left <= 4) {
			for(i = 0; i < bits_left; i++) {
				concat(binary_data, "0");
			}
			latch = 1;
		}
	}
	
	if(latch == 0) {
		/* Complete current byte */
		remainder = 8 - (strlen(binary_data) % 8);
		if(remainder == 8) { remainder = 0; }
		for(i = 0; i < remainder; i++) {
			concat(binary_data, "0");
		}
		
		/* Add padding */
		bits_left = bits_total - strlen(binary_data);
		if(bits_left > 4) {
			remainder = (bits_left - 4) / 8;
			for(i = 0; i < remainder; i++) {
				concat(binary_data, i & 1 ? "00010001" : "11101100");
			}
		}
		concat(binary_data, "0000");
	}
	
	if(ecc_mode == LEVEL_L) { data_codewords = 11; ecc_codewords = 6; }
	if(ecc_mode == LEVEL_M) { data_codewords = 9; ecc_codewords = 8; }
	
	/* Copy data into codewords */
	for(i = 0; i < (data_codewords - 1); i++) {
		data_blocks[i] = 0;
		if(binary_data[i * 8] == '1') { data_blocks[i] += 0x80; }
		if(binary_data[(i * 8) + 1] == '1') { data_blocks[i] += 0x40; }
		if(binary_data[(i * 8) + 2] == '1') { data_blocks[i] += 0x20; }
		if(binary_data[(i * 8) + 3] == '1') { data_blocks[i] += 0x10; }
		if(binary_data[(i * 8) + 4] == '1') { data_blocks[i] += 0x08; }
		if(binary_data[(i * 8) + 5] == '1') { data_blocks[i] += 0x04; }
		if(binary_data[(i * 8) + 6] == '1') { data_blocks[i] += 0x02; }
		if(binary_data[(i * 8) + 7] == '1') { data_blocks[i] += 0x01; }
	}
	
	if(ecc_mode == LEVEL_L) {
		data_blocks[11] = 0;
		if(binary_data[80] == '1') { data_blocks[2] += 0x08; }
		if(binary_data[81] == '1') { data_blocks[2] += 0x04; }
		if(binary_data[82] == '1') { data_blocks[2] += 0x02; }
		if(binary_data[83] == '1') { data_blocks[2] += 0x01; }
	}
	
	if(ecc_mode == LEVEL_M) {
		data_blocks[9] = 0;
		if(binary_data[64] == '1') { data_blocks[2] += 0x08; }
		if(binary_data[65] == '1') { data_blocks[2] += 0x04; }
		if(binary_data[66] == '1') { data_blocks[2] += 0x02; }
		if(binary_data[67] == '1') { data_blocks[2] += 0x01; }
	}
	
	/* Calculate Reed-Solomon error codewords */
	rs_init_gf(0x11d);
	rs_init_code(ecc_codewords, 0);
	rs_encode(data_codewords,data_blocks,ecc_blocks);
	rs_free();
	
	/* Add Reed-Solomon codewords to binary data */
	for(i = 0; i < ecc_codewords; i++) {
		qr_bscan(binary_data, ecc_blocks[ecc_codewords - i - 1], 0x80);
	}
	
	return;
}

void micro_qr_m4(char binary_data[], int ecc_mode)
{
	int i, latch;
	int bits_total, bits_left, remainder;
	int data_codewords, ecc_codewords;
	unsigned char data_blocks[17], ecc_blocks[15];
	
	latch = 0;
	
	if(ecc_mode == LEVEL_L) { bits_total = 128; }
	if(ecc_mode == LEVEL_M) { bits_total = 112; }
	if(ecc_mode == LEVEL_Q) { bits_total = 80; }
	
	/* Add terminator */
	bits_left = bits_total - strlen(binary_data);
	if(bits_left <= 9) {
		for(i = 0; i < bits_left; i++) {
			concat(binary_data, "0");
		}
		latch = 1;
	} else {
		concat(binary_data, "000000000");
	}
	
	if(latch == 0) {
		/* Complete current byte */
		remainder = 8 - (strlen(binary_data) % 8);
		if(remainder == 8) { remainder = 0; }
		for(i = 0; i < remainder; i++) {
			concat(binary_data, "0");
		}
	
		/* Add padding */
		bits_left = bits_total - strlen(binary_data);
		remainder = bits_left / 8;
		for(i = 0; i < remainder; i++) {
			concat(binary_data, i & 1 ? "00010001" : "11101100");
		}
	}
	
	if(ecc_mode == LEVEL_L) { data_codewords = 16; ecc_codewords = 8; }
	if(ecc_mode == LEVEL_M) { data_codewords = 14; ecc_codewords = 10; }
	if(ecc_mode == LEVEL_Q) { data_codewords = 10; ecc_codewords = 14; }
	
	/* Copy data into codewords */
	for(i = 0; i < data_codewords; i++) {
		data_blocks[i] = 0;
		if(binary_data[i * 8] == '1') { data_blocks[i] += 0x80; }
		if(binary_data[(i * 8) + 1] == '1') { data_blocks[i] += 0x40; }
		if(binary_data[(i * 8) + 2] == '1') { data_blocks[i] += 0x20; }
		if(binary_data[(i * 8) + 3] == '1') { data_blocks[i] += 0x10; }
		if(binary_data[(i * 8) + 4] == '1') { data_blocks[i] += 0x08; }
		if(binary_data[(i * 8) + 5] == '1') { data_blocks[i] += 0x04; }
		if(binary_data[(i * 8) + 6] == '1') { data_blocks[i] += 0x02; }
		if(binary_data[(i * 8) + 7] == '1') { data_blocks[i] += 0x01; }
	}
	
	/* Calculate Reed-Solomon error codewords */
	rs_init_gf(0x11d);
	rs_init_code(ecc_codewords, 0);
	rs_encode(data_codewords,data_blocks,ecc_blocks);
	rs_free();
	
	/* Add Reed-Solomon codewords to binary data */
	for(i = 0; i < ecc_codewords; i++) {
		qr_bscan(binary_data, ecc_blocks[ecc_codewords - i - 1], 0x80);
	}
}

void micro_setup_grid(unsigned char* grid, int size)
{
	int i, toggle = 1;

	/* Add timing patterns */
	for(i = 0; i < size; i++) {
		if(toggle == 1) {
			grid[i] = 0x21;
			grid[(i * size)] = 0x21;
			toggle = 0;
		} else {
			grid[i] = 0x20;
			grid[(i * size)] = 0x20;
			toggle = 1;
		}
	}
	
	/* Add finder patterns */
	place_finder(grid, size, 0, 0);
	
	/* Add separators */
	for(i = 0; i < 7; i++) {
		grid[(7 * size) + i] = 0x10;
		grid[(i * size) + 7] = 0x10;
	}
	grid[(7 * size) + 7] = 0x10;
	
	
	/* Reserve space for format information */
	for(i = 0; i < 8; i++) {
		grid[(8 * size) + i] += 0x20;
		grid[(i * size) + 8] += 0x20;
	}
	grid[(8 * size) + 8] += 20;
}

void micro_populate_grid(unsigned char* grid, int size, char full_stream[])
{
	int direction = 1; /* up */
	int row = 0; /* right hand side */
	
	int i, n, x, y;
	
	n = strlen(full_stream);
	y = size - 1;
	i = 0;
	do {
		x = (size - 2) - (row * 2);

		if(!(grid[(y * size) + (x + 1)] & 0xf0)) {
			if (full_stream[i] == '1') {
				grid[(y * size) + (x + 1)] = 0x01;
			} else {
				grid[(y * size) + (x + 1)] = 0x00;
			}
			i++;
		}
		
		if(i < n) {
			if(!(grid[(y * size) + x] & 0xf0)) {
				if (full_stream[i] == '1') {
					grid[(y * size) + x] = 0x01;
				} else {
					grid[(y * size) + x] = 0x00;
				}
				i++;
			}
		}
		
		if(direction) { y--; } else { y++; }
		if(y == 0) {
			/* reached the top */
			row++;
			y = 1;
			direction = 0;
		}
		if(y == size) {
			/* reached the bottom */
			row++;
			y = size - 1;
			direction = 1;
		}
	} while (i < n);
}

int micro_evaluate(unsigned char *grid, int size, int pattern)
{
	int sum1, sum2, i, filter = 0, retval;
	
	switch(pattern) {
		case 0: filter = 0x01; break;
		case 1: filter = 0x02; break;
		case 2: filter = 0x04; break;
		case 3: filter = 0x08; break;
	}
	
	sum1 = 0;
	sum2 = 0;
	for(i = 1; i < size; i++) {
		if(grid[(i * size) + size - 1] & filter) { sum1++; }
		if(grid[((size - 1) * size) + i] & filter) { sum2++; }
	}
	
	if(sum1 <= sum2) { retval = (sum1 * 16) + sum2; } else { retval = (sum2 * 16) + sum1; }
	
	return retval;
}

int micro_apply_bitmask(unsigned char *grid, int size)
{
	int x, y;
	unsigned char p;
	int pattern, value[8];
	int best_val, best_pattern;
	int bit;
	
#ifndef _MSC_VER
	unsigned char mask[size * size];
	unsigned char eval[size * size];
#else
	unsigned char* mask = (unsigned char *)_alloca((size * size) * sizeof(unsigned char));
	unsigned char* eval = (unsigned char *)_alloca((size * size) * sizeof(unsigned char));
#endif

	/* Perform data masking */
	for(x = 0; x < size; x++) {
		for(y = 0; y < size; y++) {
			mask[(y * size) + x] = 0x00;
			
			if (!(grid[(y * size) + x] & 0xf0)) {
				if((y & 1) == 0) {
					mask[(y * size) + x] += 0x01;
				}

				if((((y / 2) + (x / 3)) & 1) == 0) {
					mask[(y * size) + x] += 0x02;
				}
				
				if(((((y * x) & 1) + ((y * x) % 3)) & 1) == 0) {
					mask[(y * size) + x] += 0x04;
				}
				
				if(((((y + x) & 1) + ((y * x) % 3)) & 1) == 0) {
					mask[(y * size) + x] += 0x08;
				}
			}
		}
	}
	
	for(x = 0; x < size; x++) {
		for(y = 0; y < size; y++) {
			if(grid[(y * size) + x] & 0x01) { p = 0xff; } else { p = 0x00; }
			
			eval[(y * size) + x] = mask[(y * size) + x] ^ p;
		}
	}
	
	
	/* Evaluate result */
	for(pattern = 0; pattern < 8; pattern++) {
		value[pattern] = micro_evaluate(eval, size, pattern);
	}
	
	best_pattern = 0;
	best_val = value[0];
	for(pattern = 1; pattern < 4; pattern++) {
		if(value[pattern] > best_val) {
			best_pattern = pattern;
			best_val = value[pattern];
		}
	}
	
	/* Apply mask */
	for(x = 0; x < size; x++) {
		for(y = 0; y < size; y++) {
			bit = 0;
			switch(best_pattern) {
				case 0: if(mask[(y * size) + x] & 0x01) { bit = 1; } break;
				case 1: if(mask[(y * size) + x] & 0x02) { bit = 1; } break;
				case 2: if(mask[(y * size) + x] & 0x04) { bit = 1; } break;
				case 3: if(mask[(y * size) + x] & 0x08) { bit = 1; } break;
			}
			if(bit == 1) {
				if(grid[(y * size) + x] & 0x01) {
					grid[(y * size) + x] = 0x00;
				} else {
					grid[(y * size) + x] = 0x01;
				}
			}
		}
	}
	
	return best_pattern;
}

int microqr(struct zint_symbol *symbol, unsigned char source[], int length)
{
	int i, j, glyph, size;
	char binary_stream[200];
	char full_stream[200];
	int utfdata[40];
	int jisdata[40];
	char mode[40];
	int error_number, kanji_used = 0, alphanum_used = 0, byte_used = 0;
	int version_valid[4];
	int binary_count[4];
	int ecc_level, autoversion, version;
	int n_count, a_count, bitmask, format, format_full;
	
	if(length > 35) {
		strcpy(symbol->errtxt, "Input data too long");
		return ZINT_ERROR_TOO_LONG;
	}
	
	for(i = 0; i < 4; i++) {
		version_valid[i] = 1;
	}

	switch(symbol->input_mode) {
		case DATA_MODE:
			for(i = 0; i < length; i++) {
				jisdata[i] = (int)source[i];
			}
			break;
		default:
			/* Convert Unicode input to Shift-JIS */
			error_number = utf8toutf16(symbol, source, utfdata, &length);
			if(error_number != 0) { return error_number; }
			
			for(i = 0; i < length; i++) {
				if(utfdata[i] <= 0xff) {
					jisdata[i] = utfdata[i];
				} else {
					j = 0;
					glyph = 0;
					do {
						if(sjis_lookup[j * 2] == utfdata[i]) {
							glyph = sjis_lookup[(j * 2) + 1];
						}
						j++;
					} while ((j < 6843) && (glyph == 0));
					if(glyph == 0) {
						strcpy(symbol->errtxt, "Invalid character in input data");
						return ZINT_ERROR_INVALID_DATA;
					}
					jisdata[i] = glyph;
				}
			}
			break;
	}
	
	define_mode(mode, jisdata, length, 0);
	
	n_count = 0;
	a_count = 0;
	for(i = 0; i < length; i++) {
		if((jisdata[i] >= '0') && (jisdata[i] <= '9')) { n_count++; }
		if(in_alpha(jisdata[i])) { a_count++; }
	}
	
	if(a_count == length) {
		/* All data can be encoded in Alphanumeric mode */
		for(i = 0; i < length; i++) {
			mode[i] = 'A';
		}
	}
	
	if(n_count == length) {
		/* All data can be encoded in Numeric mode */
		for(i = 0; i < length; i++) {
			mode[i] = 'N';
		}
	}
	
	error_number = micro_qr_intermediate(binary_stream, jisdata, mode, length, &kanji_used, &alphanum_used, &byte_used);
	if(error_number != 0) {
		strcpy(symbol->errtxt, "Input data too long");
		return error_number;
	}
	
	get_bitlength(binary_count, binary_stream);
	
	/* Eliminate possivle versions depending on type of content */
	if(byte_used) {
		version_valid[0] = 0;
		version_valid[1] = 0;
	}
	
	if(alphanum_used) {
		version_valid[0] = 0;
	}
	
	if(kanji_used) {
		version_valid[0] = 0;
		version_valid[1] = 0;
	}
	
	/* Eliminate possible versions depending on length of binary data */
	if(binary_count[0] > 20) { version_valid[0] = 0; }
	if(binary_count[1] > 40) { version_valid[1] = 0; }
	if(binary_count[2] > 84) { version_valid[2] = 0; }
	if(binary_count[3] > 128) { 
		strcpy(symbol->errtxt, "Input data too long");
		return ZINT_ERROR_TOO_LONG;
	}
	
	/* Eliminate possible versions depending on error correction level specified */
	ecc_level = LEVEL_L;
	if((symbol->option_1 >= 1) && (symbol->option_2 <= 4)) {
		ecc_level = symbol->option_1;
	}
	
	if(ecc_level == LEVEL_H) {
		strcpy(symbol->errtxt, "Error correction level H not available");
		return ZINT_ERROR_INVALID_OPTION;
	}
	
	if(ecc_level == LEVEL_Q) {
		version_valid[0] = 0;
		version_valid[1] = 0;
		version_valid[2] = 0;
		if(binary_count[3] > 80) {
			strcpy(symbol->errtxt, "Input data too long");
			return ZINT_ERROR_TOO_LONG;
		}
	}
	
	if(ecc_level == LEVEL_M) {
		version_valid[0] = 0;
		if(binary_count[1] > 32) { version_valid[1] = 0; }
		if(binary_count[2] > 68) { version_valid[2] = 0; }
		if(binary_count[3] > 112) {
			strcpy(symbol->errtxt, "Input data too long");
			return ZINT_ERROR_TOO_LONG;
		}
	}
	
	autoversion = 3;
	if(version_valid[2]) { autoversion = 2; }
	if(version_valid[1]) { autoversion = 1; }
	if(version_valid[0]) { autoversion = 0; }
	
	version = autoversion;
	/* Get version from user */
	if((symbol->option_2 >= 1) && (symbol->option_2 <= 4)) {
		if(symbol->option_2 >= autoversion) {
			version = symbol->option_2;
		}
	}
	
	/* If there is enough unused space then increase the error correction level */
	if(version == 3) {
		if(binary_count[3] <= 112) { ecc_level = LEVEL_M; }
		if(binary_count[3] <= 80) { ecc_level = LEVEL_Q; }
	}
	
	if(version == 2) {
		if(binary_count[2] <= 68) { ecc_level = LEVEL_M; }
	}
	
	if(version == 1) {
		if(binary_count[1] <= 32) { ecc_level = LEVEL_M; }
	}
	
	strcpy(full_stream, "");
	microqr_expand_binary(binary_stream, full_stream, version);
	
	switch(version) {
		case 0: micro_qr_m1(full_stream); break;
		case 1: micro_qr_m2(full_stream, ecc_level); break;
		case 2: micro_qr_m3(full_stream, ecc_level); break;
		case 3: micro_qr_m4(full_stream, ecc_level); break;
	}
	
	size = micro_qr_sizes[version];
#ifndef _MSC_VER
	unsigned char grid[size * size];
#else
	unsigned char* grid = (unsigned char *)_alloca((size * size) * sizeof(unsigned char));
#endif
	
	for(i = 0; i < size; i++) {
		for(j = 0; j < size; j++) {
			grid[(i * size) + j] = 0;
		}
	}
	
	micro_setup_grid(grid, size);
	micro_populate_grid(grid, size, full_stream);
	bitmask = micro_apply_bitmask(grid, size);
	
	/* Add format data */
	format = 0;
	switch(version) {
		case 1: switch(ecc_level) {
				case 1: format = 1; break;
				case 2: format = 2; break;
			}
			break;
		case 2: switch(ecc_level) {
				case 1: format = 3; break;
				case 2: format = 4; break;
			}
			break;
		case 3: switch(ecc_level) {
				case 1: format = 5; break;
				case 2: format = 6; break;
				case 3: format = 7; break;
			}
			break;
	}
	
	format_full = qr_annex_c1[(format << 2) + bitmask];
	
	if(format_full & 0x4000) { grid[(8 * size) + 1] += 0x01; } 
	if(format_full & 0x2000) { grid[(8 * size) + 2] += 0x01; } 
	if(format_full & 0x1000) { grid[(8 * size) + 3] += 0x01; } 
	if(format_full & 0x800) { grid[(8 * size) + 4] += 0x01; } 
	if(format_full & 0x400) { grid[(8 * size) + 5] += 0x01; } 
	if(format_full & 0x200) { grid[(8 * size) + 6] += 0x01; } 
	if(format_full & 0x100) { grid[(8 * size) + 7] += 0x01; } 
	if(format_full & 0x80) { grid[(8 * size) + 8] += 0x01; } 
	if(format_full & 0x40) { grid[(7 * size) + 8] += 0x01; } 
	if(format_full & 0x20) { grid[(6 * size) + 8] += 0x01; } 
	if(format_full & 0x10) { grid[(5 * size) + 8] += 0x01; } 
	if(format_full & 0x08) { grid[(4 * size) + 8] += 0x01; } 
	if(format_full & 0x04) { grid[(3 * size) + 8] += 0x01; } 
	if(format_full & 0x02) { grid[(2 * size) + 8] += 0x01; } 
	if(format_full & 0x01) { grid[(1 * size) + 8] += 0x01; } 
	
	symbol->width = size;
	symbol->rows = size;
	
	for(i = 0; i < size; i++) {
		for(j = 0; j < size; j++) {
			if(grid[(i * size) + j] & 0x01) {
				set_module(symbol, i, j);
			}
		}
		symbol->row_height[i] = 1;
	}
	
	return 0;
}
