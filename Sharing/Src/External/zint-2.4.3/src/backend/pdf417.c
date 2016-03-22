/* pdf417.c - Handles PDF417 stacked symbology */

/*  Zint - A barcode generating program using libpng
    Copyright (C) 2008 Robin Stuart <robin@zint.org.uk>
    Portions Copyright (C) 2004 Grandzebu
    Bug Fixes thanks to KL Chin <klchin@users.sourceforge.net>

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

/*  This code is adapted from "Code barre PDF 417 / PDF 417 barcode" v2.5.0
    which is Copyright (C) 2004 (Grandzebu).
    The original code can be downloaded from http://grandzebu.net/index.php */

/* NOTE: symbol->option_1 is used to specify the security level (i.e. control the
   number of check codewords)

   symbol->option_2 is used to adjust the width of the resulting symbol (i.e. the
   number of codeword columns not including row start and end data) */

/* @(#) $Id: pdf417.c,v 1.21 2010/01/28 17:55:59 hooper114 Exp $ */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#ifndef _MSC_VER
#include <stdint.h>
#else
#include <malloc.h> 
#include "ms_stdint.h"
#endif 
#include "pdf417.h"
#include "common.h"
#include "large.h"

/* 
   Three figure numbers in comments give the location of command equivalents in the
   original Visual Basic source code file pdf417.frm
   this code retains some original (French) procedure and variable names to ease conversion */

/* text mode processing tables */

static const int asciix[95] = { 7, 8, 8, 4, 12, 4, 4, 8, 8, 8, 12, 4, 12, 12, 12, 12, 4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 12, 8, 8, 4, 8, 8, 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 8, 8, 8, 4, 8, 8, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 8, 8, 8, 8 };

static const int asciiy[95] = { 26, 10, 20, 15, 18, 21, 10, 28, 23, 24, 22, 20, 13, 16, 17, 19, 0, 1, 2, 3,
	4, 5, 6, 7, 8, 9, 14, 0, 1, 23, 2, 25, 3, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 4, 5, 6, 24, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
	11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 21, 27, 9 };

/* Automatic sizing table */

static const int MicroAutosize[56] =
{	4, 6, 7, 8, 10, 12, 13, 14, 16, 18, 19, 20, 24, 29, 30, 33, 34, 37, 39, 46, 54, 58, 70, 72, 82, 90, 108, 126,
	1, 14, 2, 7, 3, 25, 8, 16, 5, 17, 9, 6, 10, 11, 28, 12, 19, 13, 29, 20, 30, 21, 22, 31, 23, 32, 33, 34
};

int liste[2][1000]; /* global - okay, so I got _almost_ everything local! */

/* 866 */

int quelmode(char codeascii)
{
	int mode = BYT;
	if ((codeascii == '\t') || (codeascii == '\n') || (codeascii == '\r') || ((codeascii >= ' ') && (codeascii <= '~'))) { mode = TEX; }
	else if((codeascii >= '0') && (codeascii <= '9')) { mode = NUM; }
	/* 876 */

	return mode;
}

/* 844 */
void regroupe(int *indexliste)
{
	int i, j;

	/* bring together same type blocks */
	if(*(indexliste) > 1) {
		i = 1;
		while(i < *(indexliste)) {
			if(liste[1][i - 1] == liste[1][i]) {
				/* bring together */
				liste[0][i - 1] = liste[0][i - 1] + liste[0][i];
				j = i + 1;

				/* decreace the list */
				while(j < *(indexliste)) {
					liste[0][j - 1] = liste[0][j];
					liste[1][j - 1] = liste[1][j];
					j++;
				}
				*(indexliste) = *(indexliste) - 1;
				i--;
			}
			i++;
		}
	}
	/* 865 */
}

/* 478 */
void pdfsmooth(int *indexliste)
{
	int i, crnt, last, next, length;

	for(i = 0; i < *(indexliste); i++) {
		crnt = liste[1][i];
		length = liste[0][i];
		if(i != 0) { last = liste[1][i - 1]; } else { last = FALSE; }
		if(i != *(indexliste) - 1) { next = liste[1][i + 1]; } else { next = FALSE; }

		if(crnt == NUM) {
			if(i == 0) { /* first block */
				if(*(indexliste) > 1) { /* and there are others */
					if((next == TEX) && (length < 8)) { liste[1][i] = TEX;}
					if((next == BYT) && (length == 1)) { liste[1][i] = BYT; }
				}
			} else {
				if(i == *(indexliste) - 1) { /* last block */
					if((last == TEX) && (length < 7)) { liste[1][i] = TEX; }
					if((last == BYT) && (length == 1)) { liste[1][i] = BYT; }
				} else { /* not first or last block */
					if(((last == BYT) && (next == BYT)) && (length < 4)) { liste[1][i] = BYT; }
					if(((last == BYT) && (next == TEX)) && (length < 4)) { liste[1][i] = TEX; }
					if(((last == TEX) && (next == BYT)) && (length < 5)) { liste[1][i] = TEX; }
					if(((last == TEX) && (next == TEX)) && (length < 8)) { liste[1][i] = TEX; }
				}
			}
		}
	}
	regroupe(indexliste);
	/* 520 */
	for(i = 0; i < *(indexliste); i++) {
		crnt = liste[1][i];
		length = liste[0][i];
		if(i != 0) { last = liste[1][i - 1]; } else { last = FALSE; }
		if(i != *(indexliste) - 1) { next = liste[1][i + 1]; } else { next = FALSE; }

		if((crnt == TEX) && (i > 0)) { /* not the first */
			if(i == *(indexliste) - 1) { /* the last one */
				if((last == BYT) && (length == 1)) { liste[1][i] = BYT; }
			} else { /* not the last one */
				if(((last == BYT) && (next == BYT)) && (length < 5)) { liste[1][i] = BYT; }
				if((((last == BYT) && (next != BYT)) || ((last != BYT) && (next == BYT))) && (length < 3)) {
					liste[1][i] = BYT;
				}
			}
		}
	}
	/* 540 */
	regroupe(indexliste);
}

/* 547 */
void textprocess(int *chainemc, int *mclength, char chaine[], int start, int length, int block)
{
	int j, indexlistet, curtable, listet[2][5000], chainet[5000], wnet;
	char codeascii;

	codeascii = 0;
	wnet = 0;

	for(j = 0; j < 1000; j++) {
		listet[0][j] = 0;
	}
	/* listet will contain the table numbers and the value of each characters */
	for(indexlistet = 0; indexlistet < length; indexlistet++) {
		codeascii = chaine[start + indexlistet];
		switch(codeascii) {
			case '\t': listet[0][indexlistet] = 12; listet[1][indexlistet] = 12; break;
			case '\n': listet[0][indexlistet] = 8; listet[1][indexlistet] = 15; break;
			case 13: listet[0][indexlistet] = 12; listet[1][indexlistet] = 11; break;
			default: listet[0][indexlistet] = asciix[codeascii - 32];
			listet[1][indexlistet] = asciiy[codeascii - 32]; break;
		}
	}

	/* 570 */
	curtable = 1; /* default table */
	for(j = 0; j < length; j++) {
		if(listet[0][j] & curtable) { /* The character is in the current table */
			chainet[wnet] = listet[1][j];
			wnet++;
		} else { /* Obliged to change table */
			int flag = FALSE; /* True if we change table for only one character */
			if (j == (length - 1)) {
				flag = TRUE;
			} else {
				if(!(listet[0][j] & listet[0][j + 1])) { flag = TRUE; }
			}

			if (flag) { /* we change only one character - look for temporary switch */
				if((listet[0][j] & 1) && (curtable == 2)) { /* T_UPP */
					chainet[wnet] = 27;
					chainet[wnet + 1] = listet[1][j];
					wnet += 2;
				}
				if(listet[0][j] & 8) { /* T_PUN */
					chainet[wnet] = 29;
					chainet[wnet + 1] = listet[1][j];
					wnet += 2;
				}
				if(!(((listet[0][j] & 1) && (curtable == 2)) || (listet[0][j] & 8))) {
					/* No temporary switch available */
					flag = FALSE;
				}
			}

			/* 599 */
			if (!(flag)) {
				int newtable;

				if(j == (length - 1)) {
					newtable = listet[0][j];
				} else {
					if(!(listet[0][j] & listet[0][j + 1])) {
						newtable = listet[0][j];
					} else {
						newtable = listet[0][j] & listet[0][j + 1];
					}
				}

				/* Maintain the first if several tables are possible */
				switch (newtable) {
					case 3:
					case 5:
					case 7:
					case 9:
					case 11:
					case 13:
					case 15:
						newtable = 1; break;
					case 6:
					case 10:
					case 14:
						newtable = 2; break;
					case 12:
						newtable = 4; break;
				}

				/* 619 - select the switch */
				switch (curtable) {
					case 1:
						switch (newtable) {
							case 2: chainet[wnet] = 27; wnet++; break;
							case 4: chainet[wnet] = 28; wnet++; break;
							case 8: chainet[wnet] = 28; wnet++; chainet[wnet] = 25; wnet++;  break;
						} break;
					case 2:
						switch (newtable) {
							case 1: chainet[wnet] = 28; wnet++; chainet[wnet] = 28; wnet++;  break;
							case 4: chainet[wnet] = 28; wnet++;  break;
							case 8: chainet[wnet] = 28; wnet++; chainet[wnet] = 25; wnet++;  break;
						} break;
					case 4:
						switch (newtable) {
							case 1: chainet[wnet] = 28; wnet++;  break;
							case 2: chainet[wnet] = 27; wnet++;  break;
							case 8: chainet[wnet] = 25; wnet++;  break;
						} break;
					case 8:
						switch (newtable) {
							case 1: chainet[wnet] = 29; wnet++;  break;
							case 2: chainet[wnet] = 29; wnet++; chainet[wnet] = 27; wnet++;  break;
							case 4: chainet[wnet] = 29; wnet++; chainet[wnet] = 28; wnet++;  break;
						} break;
				}
				curtable = newtable;
				/* 659 - at last we add the character */
				chainet[wnet] = listet[1][j];
				wnet++;
			}
		}
	}

	/* 663 */
	if (wnet & 1) {
		chainet[wnet] = 29;
		wnet++;
	}
	/* Now translate the string chainet into codewords */
	chainemc[*(mclength)] = 900;
	*(mclength) = *(mclength) + 1;

	for(j = 0; j < wnet; j+= 2) {
		int cw_number;

		cw_number = (30 * chainet[j]) + chainet[j + 1];
		chainemc[*(mclength)] = cw_number;
		*(mclength) = *(mclength) + 1;

	}
}

/* 671 */
void byteprocess(int *chainemc, int *mclength, unsigned char chaine[], int start, int length, int block)
{
	int		debug	  = 0;
	int		len       = 0;
	unsigned int	chunkLen  = 0;
	uint64_t	mantisa   = 0ULL;
	uint64_t	total     = 0ULL;

	if(debug) printf("\nEntering byte mode at position %d\n", start);

	if(length == 1) {
		chainemc[(*mclength)++] = 913;
		chainemc[(*mclength)++] = chaine[start];
		if(debug) { printf("913 %d\n", chainemc[*mclength - 1]); }
	} else {
		/* select the switch for multiple of 6 bytes */
		if (length % 6 == 0) {
			chainemc[(*mclength)++] = 924;
			if(debug) printf("924 ");
		} else {
			chainemc[(*mclength)++] = 901;
			if(debug) printf("901 ");
		}

		while (len < length)
		{
			chunkLen = length - len;
			if (6 <= chunkLen)  /* Take groups of 6 */
			{
				chunkLen	= 6;
				len		+= chunkLen;
				total		= 0ULL;

				while (chunkLen--)
				{
					mantisa = chaine[start++];
					total   |= mantisa << (uint64_t)(chunkLen * 8ULL);
				}

				chunkLen = 5;

				while (chunkLen--)
				{
					chainemc[*mclength + chunkLen] = (int)(total % 900ULL);
					total /= 900ULL;
				}
				*mclength += 5;
			}
			else	/*  If it remain a group of less than 6 bytes   */
			{
				len += chunkLen;
				while (chunkLen--)
				{
					chainemc[(*mclength)++] = chaine[start++];
				}
			}
		}
	}
}

/* 712 */
void numbprocess(int *chainemc, int *mclength, char chaine[], int start, int length, int block)
{
	int j, loop, longueur, dummy[100], dumlength, diviseur, nombre;
	char chainemod[50], chainemult[100], temp;

	strcpy(chainemod, "");
	for(loop = 0; loop <= 50; loop++) {
		dummy[loop] = 0;
	}

	chainemc[*(mclength)] = 902;
	*(mclength) = *(mclength) + 1;

	j = 0;
	while(j < length) {
		dumlength = 0;
		strcpy(chainemod, "");
		longueur = length - j;
		if(longueur > 44) { longueur = 44; }
		concat(chainemod, "1");
		for(loop = 1; loop <= longueur; loop++) {
			chainemod[loop] = chaine[start + loop + j - 1];
		}
		chainemod[longueur + 1] = '\0';
		do {
			diviseur = 900;

			/* 877 - gosub Modulo */
			strcpy(chainemult, "");
			nombre = 0;
			while(strlen(chainemod) != 0) {
				nombre *= 10;
				nombre += ctoi(chainemod[0]);
				for(loop = 0; loop < strlen(chainemod); loop++) {
					chainemod[loop] = chainemod[loop + 1];
				}
				if (nombre < diviseur) {
					if (strlen(chainemult) != 0) { concat(chainemult, "0"); }
				} else {
					temp = (nombre / diviseur) + '0';
					chainemult[strlen(chainemult) + 1] = '\0';
					chainemult[strlen(chainemult)] = temp;
				}
				nombre = nombre % diviseur;
			}
			diviseur = nombre;
			/* return to 723 */

			for(loop = dumlength; loop > 0; loop--) {
				dummy[loop] = dummy[loop - 1];
			}
			dummy[0] = diviseur;
			dumlength++;
			strcpy(chainemod, chainemult);
		} while(strlen(chainemult) != 0);
		for(loop = 0; loop < dumlength; loop++) {
			chainemc[*(mclength)] = dummy[loop];
			*(mclength) = *(mclength) + 1;
		}
		j += longueur;
	}
}

/* 366 */
int pdf417(struct zint_symbol *symbol, unsigned char chaine[], int length)
{
	int i, k, j, indexchaine, indexliste, mode, longueur, loop, mccorrection[520], offset;
	int total, chainemc[2700], mclength, c1, c2, c3, dummy[35], codeerr;
	char codebarre[140], pattern[580];
	int debug = 0;

	codeerr = 0;

	/* 456 */
	indexliste = 0;
	indexchaine = 0;

	mode = quelmode(chaine[indexchaine]);

	for(i = 0; i < 1000; i++) {
		liste[0][i] = 0;
	}

	/* 463 */
	do {
		liste[1][indexliste] = mode;
		while ((liste[1][indexliste] == mode) && (indexchaine < length)) {
			liste[0][indexliste]++;
			indexchaine++;
			mode = quelmode(chaine[indexchaine]);
		}
		indexliste++;
	} while (indexchaine < length);

	/* 474 */
	pdfsmooth(&indexliste);

	if(debug) {
		printf("Initial block pattern:\n");
		for(i = 0; i < indexliste; i++) {
			printf("Len: %d  Type: ", liste[0][i]);
			switch(liste[1][i]) {
				case TEX: printf("Text\n"); break;
				case BYT: printf("Byte\n"); break;
				case NUM: printf("Number\n"); break;
				default: printf("ERROR\n"); break;
			}
		}
	}

	/* 541 - now compress the data */
	indexchaine = 0;
	mclength = 0;
	if(symbol->output_options & READER_INIT) {
		chainemc[mclength] = 921; /* Reader Initialisation */
		mclength++;
	}
	for(i = 0; i < indexliste; i++) {
		switch(liste[1][i]) {
			case TEX: /* 547 - text mode */
				textprocess(chainemc, &mclength, (char*)chaine, indexchaine, liste[0][i], i);
				break;
			case BYT: /* 670 - octet stream mode */
				byteprocess(chainemc, &mclength, chaine, indexchaine, liste[0][i], i);
				break;
			case NUM: /* 712 - numeric mode */
				numbprocess(chainemc, &mclength, (char*)chaine, indexchaine, liste[0][i], i);
				break;
		}
		indexchaine = indexchaine + liste[0][i];
	}

	if(debug) {
		printf("\nCompressed data stream:\n");
		for(i = 0; i < mclength; i++) {
			printf("%d ", chainemc[i]);
		}
		printf("\n\n");
	}

	/* 752 - Now take care of the number of CWs per row */
	if (symbol->option_1 < 0) {
		symbol->option_1 = 6;
		if(mclength <= 863) { symbol->option_1 = 5; }
		if(mclength <= 320) { symbol->option_1 = 4; }
		if(mclength <= 160) { symbol->option_1 = 3; }
		if(mclength <= 40) { symbol->option_1 = 2; }
	}
	k = 1;
	for(loop = 1; loop <= (symbol->option_1 + 1); loop++)
	{
		k *= 2;
	}
	longueur = mclength;
	if(symbol->option_2 > 30) { symbol->option_2 = 30; }
	if(symbol->option_2 < 1) {
		symbol->option_2 = 0.5 + sqrt((longueur + k) / 3.0);
	}
	if(((longueur + k) / symbol->option_2) > 90) {
		/* stop the symbol from becoming too high */
		symbol->option_2 = symbol->option_2 + 1;
	}

	if(longueur + k > 928) {
		/* Enforce maximum codeword limit */
		return 2;
	}

	if(((longueur + k) / symbol->option_2) > 90) {
		return 4;
	}

	/* 781 - Padding calculation */
	longueur = mclength + 1 + k;
	i = 0;
	if ((longueur / symbol->option_2) < 3) {
		i = (symbol->option_2 * 3) - longueur; /* A bar code must have at least three rows */
	} else {
		if((longueur % symbol->option_2) > 0) { i = symbol->option_2 - (longueur % symbol->option_2); }
	}
	/* We add the padding */
	while (i > 0) {
		chainemc[mclength] = 900;
		mclength++;
		i--;
	}
	/* we add the length descriptor */
	for(i = mclength; i > 0; i--) {
		chainemc[i] = chainemc[i - 1];
	}
	chainemc[0] = mclength + 1;
	mclength++;

	/* 796 - we now take care of the Reed Solomon codes */
	switch(symbol->option_1) {
		case 1: offset = 2; break;
		case 2: offset = 6; break;
		case 3: offset = 14; break;
		case 4: offset = 30; break;
		case 5: offset = 62; break;
		case 6: offset = 126; break;
		case 7: offset = 254; break;
		case 8: offset = 510; break;
		default: offset = 0; break;
	}

	longueur = mclength;
	for(loop = 0; loop < 520; loop++) {
		mccorrection[loop] = 0;
	}
	total = 0;
	for(i = 0; i < longueur; i++) {
		total = (chainemc[i] + mccorrection[k - 1]) % 929;
		for(j = k - 1; j > 0; j--) {
			mccorrection[j] = (mccorrection[j - 1] + 929 - (total * coefrs[offset + j]) % 929) % 929;
		}
		mccorrection[0] = (929 - (total * coefrs[offset + j]) % 929) % 929;
	}

	/* we add these codes to the string */
	for(i = k - 1; i >= 0; i--) {
		chainemc[mclength++] = mccorrection[i] ? 929 - mccorrection[i] : 0;
	}

	/* 818 - The CW string is finished */
	c1 = (mclength / symbol->option_2 - 1) / 3;
	c2 = symbol->option_1 * 3 + (mclength / symbol->option_2 - 1) % 3;
	c3 = symbol->option_2 - 1;

	/* we now encode each row */
	for(i = 0; i <= (mclength / symbol->option_2) - 1; i++) {
		for(j = 0; j < symbol->option_2 ; j++) {
			dummy[j + 1] = chainemc[i * symbol->option_2 + j];
		}
		k = (i / 3) * 30;
		switch(i % 3) {
				/* follows this pattern from US Patent 5,243,655: 
			Row 0: L0 (row #, # of rows)         R0 (row #, # of columns)
			Row 1: L1 (row #, security level)    R1 (row #, # of rows)
			Row 2: L2 (row #, # of columns)      R2 (row #, security level)
			Row 3: L3 (row #, # of rows)         R3 (row #, # of columns)
				etc. */
			case 0:
				dummy[0] = k + c1;
				dummy[symbol->option_2 + 1] = k + c3;
				break;
			case 1:
				dummy[0] = k + c2;
				dummy[symbol->option_2 + 1] = k + c1;
				break;
			case 2:
				dummy[0] = k + c3;
				dummy[symbol->option_2 + 1] = k + c2;
				break;
		}
		strcpy(codebarre, "+*"); /* Start with a start char and a separator */
		if(symbol->symbology == BARCODE_PDF417TRUNC) {
			/* truncated - so same as before except knock off the last 5 chars */
			for(j = 0; j <= symbol->option_2;  j++) {
				switch(i % 3) {
					case 1: offset = 929; break;
					case 2: offset = 1858; break;
					default: offset = 0; break;
				}
				concat(codebarre, codagemc[offset + dummy[j]]);
				concat(codebarre, "*");
			}
		} else {
			/* normal PDF417 symbol */
			for(j = 0; j <= symbol->option_2 + 1;  j++) {
				switch(i % 3) {
					case 1: offset = 929; /* cluster(3) */ break;
					case 2: offset = 1858; /* cluster(6) */ break;
					default: offset = 0; /* cluster(0) */ break;
				}
				concat(codebarre, codagemc[offset + dummy[j]]);
				concat(codebarre, "*");
			}
			concat(codebarre, "-");
		}

		strcpy(pattern, "");
		for(loop = 0; loop < strlen(codebarre); loop++) {
			lookup(BRSET, PDFttf, codebarre[loop], pattern);
		}
		for(loop = 0; loop < strlen(pattern); loop++) {
			if(pattern[loop] == '1') { set_module(symbol, i, loop); }
		}
		if(symbol->height == 0) {
			symbol->row_height[i] = 3;
		}
	}
	symbol->rows = (mclength / symbol->option_2);
	symbol->width = strlen(pattern);

	/* 843 */
	return codeerr;
}

/* 345 */
int pdf417enc(struct zint_symbol *symbol, unsigned char source[], int length)
{
	int codeerr, error_number;

	error_number = 0;

	if((symbol->option_1 < -1) || (symbol->option_1 > 8)) {
		strcpy(symbol->errtxt, "Security value out of range");
		symbol->option_1 = -1;
		error_number = WARN_INVALID_OPTION;
	}
	if((symbol->option_2 < 0) || (symbol->option_2 > 30)) {
		strcpy(symbol->errtxt, "Number of columns out of range");
		symbol->option_2 = 0;
		error_number = WARN_INVALID_OPTION;
	}

	/* 349 */
	codeerr = pdf417(symbol, source, length);

	/* 352 */
	if(codeerr != 0) {
		switch(codeerr) {
			case 1:
				strcpy(symbol->errtxt, "No such file or file unreadable");
				error_number = ZINT_ERROR_INVALID_OPTION;
				break;
			case 2:
				strcpy(symbol->errtxt, "Input string too long");
				error_number = ZINT_ERROR_TOO_LONG;
				break;
			case 3:
				strcpy(symbol->errtxt, "Number of codewords per row too small");
				error_number = WARN_INVALID_OPTION;
				break;
			case 4:
				strcpy(symbol->errtxt, "Data too long for specified number of columns");
				error_number = ZINT_ERROR_TOO_LONG;
				break;
			default:
				strcpy(symbol->errtxt, "Something strange happened");
				error_number = ZINT_ERROR_ENCODING_PROBLEM;
				break;
		}
	}

	/* 364 */
	return error_number;
}


int micro_pdf417(struct zint_symbol *symbol, unsigned char chaine[], int length)
{ /* like PDF417 only much smaller! */

	int i, k, j, indexchaine, indexliste, mode, longueur, mccorrection[50], offset;
	int total, chainemc[2700], mclength, dummy[5], codeerr;
	char codebarre[100], pattern[580];
	int variant, LeftRAPStart, CentreRAPStart, RightRAPStart, StartCluster;
	int LeftRAP, CentreRAP, RightRAP, Cluster, writer, flip, loop;
	int debug = 0;

	/* Encoding starts out the same as PDF417, so use the same code */
	codeerr = 0;

	/* 456 */
	indexliste = 0;
	indexchaine = 0;

	mode = quelmode(chaine[indexchaine]);

	for(i = 0; i < 1000; i++) {
		liste[0][i] = 0;
	}

	/* 463 */
	do {
		liste[1][indexliste] = mode;
		while ((liste[1][indexliste] == mode) && (indexchaine < length)) {
			liste[0][indexliste]++;
			indexchaine++;
			mode = quelmode(chaine[indexchaine]);
		}
		indexliste++;
	} while (indexchaine < length);

	/* 474 */
	pdfsmooth(&indexliste);

	if(debug) {
		printf("Initial mapping:\n");
		for(i = 0; i < indexliste; i++) {
			printf("len: %d   type: ", liste[0][i]);
			switch(liste[1][i]) {
				case TEX: printf("TEXT\n"); break;
				case BYT: printf("BYTE\n"); break;
				case NUM: printf("NUMBER\n"); break;
				default: printf("*ERROR*\n"); break;
			}
		}
	}

	/* 541 - now compress the data */
	indexchaine = 0;
	mclength = 0;
	if(symbol->output_options & READER_INIT) {
		chainemc[mclength] = 921; /* Reader Initialisation */
		mclength++;
	}
	for(i = 0; i < indexliste; i++) {
		switch(liste[1][i]) {
			case TEX: /* 547 - text mode */
				textprocess(chainemc, &mclength, (char*)chaine, indexchaine, liste[0][i], i);
				break;
			case BYT: /* 670 - octet stream mode */
				byteprocess(chainemc, &mclength, chaine, indexchaine, liste[0][i], i);
				break;
			case NUM: /* 712 - numeric mode */
				numbprocess(chainemc, &mclength, (char*)chaine, indexchaine, liste[0][i], i);
				break;
		}
		indexchaine = indexchaine + liste[0][i];
	}

	/* This is where it all changes! */

	if(mclength > 126) {
		strcpy(symbol->errtxt, "Input data too long");
		return ZINT_ERROR_TOO_LONG;
	}
	if(symbol->option_2 > 4) {
		strcpy(symbol->errtxt, "Specified width out of range");
		symbol->option_2 = 0;
		codeerr = WARN_INVALID_OPTION;
	}

	if(debug) {
		printf("\nEncoded Data Stream:\n");
		for(i = 0; i < mclength; i++) {
			printf("0x%02X ", chainemc[i]);
		}
		printf("\n");
	}

	/* Now figure out which variant of the symbol to use and load values accordingly */

	variant = 0;

	if((symbol->option_2 == 1) && (mclength > 20)) {
		/* the user specified 1 column but the data doesn't fit - go to automatic */
		symbol->option_2 = 0;
		strcpy(symbol->errtxt, "Specified symbol size too small for data");
		codeerr = WARN_INVALID_OPTION;
	}

	if((symbol->option_2 == 2) && (mclength > 37)) {
		/* the user specified 2 columns but the data doesn't fit - go to automatic */
		symbol->option_2 = 0;
		strcpy(symbol->errtxt, "Specified symbol size too small for data");
		codeerr = WARN_INVALID_OPTION;
	}

	if((symbol->option_2 == 3) && (mclength > 82)) {
		/* the user specified 3 columns but the data doesn't fit - go to automatic */
		symbol->option_2 = 0;
		strcpy(symbol->errtxt, "Specified symbol size too small for data");
		codeerr = WARN_INVALID_OPTION;
	}

	if(symbol->option_2 == 1) {
		/* the user specified 1 column and the data does fit */
		variant = 6;
		if(mclength <= 16) { variant = 5; }
		if(mclength <= 12) { variant = 4; }
		if(mclength <= 10) { variant = 3; }
		if(mclength <= 7) { variant = 2; }
		if(mclength <= 4) { variant = 1; }
	}

	if(symbol->option_2 == 2) {
		/* the user specified 2 columns and the data does fit */
		variant = 13;
		if(mclength <= 33) { variant = 12; }
		if(mclength <= 29) { variant = 11; }
		if(mclength <= 24) { variant = 10; }
		if(mclength <= 19) { variant = 9; }
		if(mclength <= 13) { variant = 8; }
		if(mclength <= 8) { variant = 7; }
	}

	if(symbol->option_2 == 3) {
		/* the user specified 3 columns and the data does fit */
		variant = 23;
		if(mclength <= 70) { variant = 22; }
		if(mclength <= 58) { variant = 21; }
		if(mclength <= 46) { variant = 20; }
		if(mclength <= 34) { variant = 19; }
		if(mclength <= 24) { variant = 18; }
		if(mclength <= 18) { variant = 17; }
		if(mclength <= 14) { variant = 16; }
		if(mclength <= 10) { variant = 15; }
		if(mclength <= 6) { variant = 14; }
	}

	if(symbol->option_2 == 4) {
		/* the user specified 4 columns and the data does fit */
		variant = 34;
		if(mclength <= 108) { variant = 33; }
		if(mclength <= 90) { variant = 32; }
		if(mclength <= 72) { variant = 31; }
		if(mclength <= 54) { variant = 30; }
		if(mclength <= 39) { variant = 29; }
		if(mclength <= 30) { variant = 28; }
		if(mclength <= 24) { variant = 27; }
		if(mclength <= 18) { variant = 26; }
		if(mclength <= 12) { variant = 25; }
		if(mclength <= 8) { variant = 24; }
	}

	if(variant == 0) {
		/* Zint can choose automatically from all available variations */
		for(i = 27; i >= 0; i--) {

			if(MicroAutosize[i] >= mclength) {
				variant = MicroAutosize[i + 28];
			}
		}
	}

	/* Now we have the variant we can load the data */
	variant --;
	symbol->option_2 = MicroVariants[variant]; /* columns */
	symbol->rows = MicroVariants[variant + 34]; /* rows */
	k = MicroVariants[variant + 68]; /* number of EC CWs */
	longueur = (symbol->option_2 * symbol->rows) - k; /* number of non-EC CWs */
	i = longueur - mclength; /* amount of padding required */
	offset = MicroVariants[variant + 102]; /* coefficient offset */

	if(debug) {
		printf("\nChoose symbol size:\n");
		printf("%d columns x %d rows\n", symbol->option_2, symbol->rows);
		printf("%d data codewords (including %d pads), %d ecc codewords\n", longueur, i, k);
		printf("\n");
	}

	/* We add the padding */
	while (i > 0) {
		chainemc[mclength] = 900;
		mclength++;
		i--;
	}

	/* Reed-Solomon error correction */
	longueur = mclength;
	for(loop = 0; loop < 50; loop++) {
		mccorrection[loop] = 0;
	}
	total = 0;
	for(i = 0; i < longueur; i++) {
		total = (chainemc[i] + mccorrection[k - 1]) % 929;
		for(j = k - 1; j >= 0; j--) {
			if(j == 0) {
				mccorrection[j] = (929 - (total * Microcoeffs[offset + j]) % 929) % 929;
			} else {
				mccorrection[j] = (mccorrection[j - 1] + 929 - (total * Microcoeffs[offset + j]) % 929) % 929;
			}
		}
	}

	for(j = 0; j < k; j++) {
		if(mccorrection[j] != 0) { mccorrection[j] = 929 - mccorrection[j]; }
	}
	/* we add these codes to the string */
	for(i = k - 1; i >= 0; i--) {
		chainemc[mclength] = mccorrection[i];
		mclength++;
	}

	if(debug) {
		printf("Encoded Data Stream with ECC:\n");
		for(i = 0; i < mclength; i++) {
			printf("0x%02X ", chainemc[i]);
		}
		printf("\n");
	}

	/* Now get the RAP (Row Address Pattern) start values */
	LeftRAPStart = RAPTable[variant];
	CentreRAPStart = RAPTable[variant + 34];
	RightRAPStart = RAPTable[variant + 68];
	StartCluster = RAPTable[variant + 102] / 3;

	/* That's all values loaded, get on with the encoding */

	LeftRAP = LeftRAPStart;
	CentreRAP = CentreRAPStart;
	RightRAP = RightRAPStart;
	Cluster = StartCluster; /* Cluster can be 0, 1 or 2 for Cluster(0), Cluster(3) and Cluster(6) */

	if(debug) printf("\nInternal row representation:\n");
	for(i = 0; i < symbol->rows; i++) {
		if(debug) printf("row %d: ", i);
		strcpy(codebarre, "");
		offset = 929 * Cluster;
		for(j = 0; j < 5; j++) {
			dummy[j] = 0;
		}
		for(j = 0; j < symbol->option_2 ; j++) {
			dummy[j + 1] = chainemc[i * symbol->option_2 + j];
			if(debug) printf("[%d] ", dummy[j + 1]);
		}

		/* Copy the data into codebarre */
		concat(codebarre, RAPLR[LeftRAP]);
		concat(codebarre, "1");
		concat(codebarre, codagemc[offset + dummy[1]]);
		concat(codebarre, "1");
		if(symbol->option_2 == 3) {
			concat(codebarre, RAPC[CentreRAP]);
		}
		if(symbol->option_2 >= 2) {
			concat(codebarre, "1");
			concat(codebarre, codagemc[offset + dummy[2]]);
			concat(codebarre, "1");
		}
		if(symbol->option_2 == 4) {
			concat(codebarre, RAPC[CentreRAP]);
		}
		if(symbol->option_2 >= 3) {
			concat(codebarre, "1");
			concat(codebarre, codagemc[offset + dummy[3]]);
			concat(codebarre, "1");
		}
		if(symbol->option_2 == 4) {
			concat(codebarre, "1");
			concat(codebarre, codagemc[offset + dummy[4]]);
			concat(codebarre, "1");
		}
		concat(codebarre, RAPLR[RightRAP]);
		concat(codebarre, "1"); /* stop */
		if(debug) printf("%s\n", codebarre);

		/* Now codebarre is a mixture of letters and numbers */

		writer = 0;
		flip = 1;
		strcpy(pattern, "");
		for(loop = 0; loop < strlen(codebarre); loop++) {
			if((codebarre[loop] >= '0') && (codebarre[loop] <= '9')) {
				for(k = 0; k < ctoi(codebarre[loop]); k++) {
					if(flip == 0) {
						pattern[writer] = '0';
					} else {
						pattern[writer] = '1';
					}
					writer++;
				}
				pattern[writer] = '\0';
				if(flip == 0) {
					flip = 1;
				} else {
					flip = 0;
				}
			} else {
				lookup(BRSET, PDFttf, codebarre[loop], pattern);
				writer += 5;
			}
		}
		symbol->width = writer;

		/* so now pattern[] holds the string of '1's and '0's. - copy this to the symbol */
		for(loop = 0; loop < strlen(pattern); loop++) {
			if(pattern[loop] == '1') { set_module(symbol, i, loop); }
		}
		symbol->row_height[i] = 2;

		/* Set up RAPs and Cluster for next row */
		LeftRAP++;
		CentreRAP++;
		RightRAP++;
		Cluster++;

		if(LeftRAP == 53) {
			LeftRAP = 1;
		}
		if(CentreRAP == 53) {
			CentreRAP = 1;
		}
		if(RightRAP == 53) {
			RightRAP = 1;
		}
		if(Cluster == 3) {
			Cluster = 0;
		}
	}

	return codeerr;
}

