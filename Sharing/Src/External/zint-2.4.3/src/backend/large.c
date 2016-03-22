/* large.c - Handles binary manipulation of large numbers */

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

#include <stdio.h>
#include <string.h>
#include "common.h"
#include "large.h"

static const short int BCD[40] = {
	0, 0, 0, 0,
	1, 0, 0, 0,
	0, 1, 0, 0,
	1, 1, 0, 0,
	0, 0, 1, 0,
	1, 0, 1, 0,
	0, 1, 1, 0,
	1, 1, 1, 0,
	0, 0, 0, 1,
	1, 0, 0, 1 };

void binary_add(short int accumulator[], short int input_buffer[])
{ /* Binary addition */
	int i, carry, done;
	carry = 0;

	for(i = 0; i < 112; i++) {
		done = 0;
		if(((input_buffer[i] == 0) && (accumulator[i] == 0)) && ((carry == 0) && (done == 0))) {
			accumulator[i] = 0;
			carry = 0;
			done = 1;
		}
		if(((input_buffer[i] == 0) && (accumulator[i] == 0)) && ((carry == 1) && (done == 0))) {
			accumulator[i] = 1;
			carry = 0;
			done = 1;
		}
		if(((input_buffer[i] == 0) && (accumulator[i] == 1)) && ((carry == 0) && (done == 0))) {
			accumulator[i] = 1;
			carry = 0;
			done = 1;
		}
		if(((input_buffer[i] == 0) && (accumulator[i] == 1)) && ((carry == 1) && (done == 0))) {
			accumulator[i] = 0;
			carry = 1;
			done = 1;
		}
		if(((input_buffer[i] == 1) && (accumulator[i] == 0)) && ((carry == 0) && (done == 0))) {
			accumulator[i] = 1;
			carry = 0;
			done = 1;
		}
		if(((input_buffer[i] == 1) && (accumulator[i] == 0)) && ((carry == 1) && (done == 0))) {
			accumulator[i] = 0;
			carry = 1;
			done = 1;
		}
		if(((input_buffer[i] == 1) && (accumulator[i] == 1)) && ((carry == 0) && (done == 0))) {
			accumulator[i] = 0;
			carry = 1;
			done = 1;
		}
		if(((input_buffer[i] == 1) && (accumulator[i] == 1)) && ((carry == 1) && (done == 0))) {
			accumulator[i] = 1;
			carry = 1;
			done = 1;
		}
	}
}

void binary_subtract(short int accumulator[], short int input_buffer[])
{ 	/* 2's compliment subtraction */
	/* take input_buffer from accumulator and put answer in accumulator */
	int i;
	short int sub_buffer[112];

	for(i = 0; i < 112; i++) {
		if(input_buffer[i] == 0) {
			sub_buffer[i] = 1;
		} else {
			sub_buffer[i] = 0;
		}
	}
	binary_add(accumulator, sub_buffer);

	sub_buffer[0] = 1;

	for(i = 1; i < 112; i++) {
		sub_buffer[i] = 0;
	}
	binary_add(accumulator, sub_buffer);
}

void shiftdown(short int buffer[])
{
	int i;

	buffer[102] = 0;
	buffer[103] = 0;

	for(i = 0; i < 102; i++) {
		buffer[i] = buffer[i + 1];
	}
}

void shiftup(short int buffer[])
{
	int i;

	for(i = 102; i > 0; i--) {
		buffer[i] = buffer[i - 1];
	}

	buffer[0] = 0;
}

short int islarger(short int accum[], short int reg[])
{
	/* Returns 1 if accum[] is larger than reg[], else 0 */
	int i, latch, larger;
	latch = 0;
	i = 103;
	larger = 0;


	do {
		if((accum[i] == 1) && (reg[i] == 0)) {
			latch = 1;
			larger = 1;
		}
		if((accum[i] == 0) && (reg[i] == 1)) {
			latch = 1;
		}
		i--;
	} while ((latch == 0) && (i >= -1));

	return larger;
}

void binary_load(short int reg[], char data[], const unsigned int src_len)
{
	int read, i;
	short int temp[112] = { 0 };

	for(i = 0; i < 112; i++) {
		reg[i] = 0;
	}

	for(read = 0; read < src_len; read++) {

		for(i = 0; i < 112; i++) {
			temp[i] = reg[i];
		}

		for(i = 0; i < 9; i++) {
			binary_add(reg, temp);
		}

		temp[0] = BCD[ctoi(data[read]) * 4];
		temp[1] = BCD[(ctoi(data[read]) * 4) + 1];
		temp[2] = BCD[(ctoi(data[read]) * 4) + 2];
		temp[3] = BCD[(ctoi(data[read]) * 4) + 3];
		for(i = 4; i < 112; i++) {
			temp[i] = 0;
		}

		binary_add(reg, temp);
	}
}

void hex_dump(short int input_buffer[])
{
	int i, digit, byte_space;

	byte_space = 1;
	for(i = 100; i >= 0; i-=4) {
		digit = 0;
		digit += 1 * input_buffer[i];
		digit += 2 * input_buffer[i + 1];
		digit += 4 * input_buffer[i + 2];
		digit += 8 * input_buffer[i + 3];

		switch(digit) {
			case 0: printf("0"); break;
			case 1: printf("1"); break;
			case 2: printf("2"); break;
			case 3: printf("3"); break;
			case 4: printf("4"); break;
			case 5: printf("5"); break;
			case 6: printf("6"); break;
			case 7: printf("7"); break;
			case 8: printf("8"); break;
			case 9: printf("9"); break;
			case 10: printf("A"); break;
			case 11: printf("B"); break;
			case 12: printf("C"); break;
			case 13: printf("D"); break;
			case 14: printf("E"); break;
			case 15: printf("F"); break;
		}
		if(byte_space == 1) {
			byte_space = 0;
		} else {
			byte_space = 1;
			printf(" ");
		}
	}
	printf("\n");
}
