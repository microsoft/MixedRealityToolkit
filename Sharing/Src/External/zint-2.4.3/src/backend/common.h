/* common.h - Header for all common functions in common.c */

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

/* Used in some logic */
#ifndef __COMMON_H
#define __COMMON_H

#ifndef FALSE
#define FALSE		0
#endif

#ifndef TRUE
#define TRUE		1
#endif

/* The most commonly used set */
#define NEON	"0123456789"

#include "zint.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern int ustrlen(const unsigned char source[]);
extern void ustrcpy(unsigned char target[], const unsigned char source[]);
extern void concat(char dest[], const char source[]);
extern void uconcat(unsigned char dest[], const unsigned char source[]);
extern int ctoi(char source);
extern char itoc(int source);
extern void to_upper(unsigned char source[]);
extern int is_sane(char test_string[], unsigned char source[], int length);
extern void lookup(char set_string[],const char *table[], char data, char dest[]);
extern int posn(char set_string[], char data);
extern void expand(struct zint_symbol *symbol, char data[]);
extern int is_stackable(int symbology);
extern int is_extendable(int symbology);
extern int roundup(float input);
extern int module_is_set(struct zint_symbol *symbol, int y_coord, int x_coord);
extern void set_module(struct zint_symbol *symbol, int y_coord, int x_coord);
extern void unset_module(struct zint_symbol *symbol, int y_coord, int x_coord);
extern int istwodigits(unsigned char source[], int position);
extern float froundup(float input);
extern int parunmodd(unsigned char llyth);
extern int latin1_process(struct zint_symbol *symbol, unsigned char source[], unsigned char preprocessed[], int *length);
extern int utf8toutf16(struct zint_symbol *symbol, unsigned char source[], int vals[], int *length);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __COMMON_H */
