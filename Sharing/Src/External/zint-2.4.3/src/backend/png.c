/* png.c - Handles output to PNG file */

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

#include <stdio.h>
#ifdef _MSC_VER
#include <fcntl.h>
#include <io.h>
#endif
#include <stdlib.h>
#include <string.h>
#include "common.h"

#ifdef _MSC_VER
#include <malloc.h> 
#endif /* _MSC_VER */

#ifndef NO_PNG
#include "png.h"        /* libpng header; includes zlib.h and setjmp.h */
#endif /* NO_PNG */
#include "maxipng.h"	/* Maxicode shapes */

#include "font.h"	/* Font for human readable text */

#define SSET	"0123456789ABCDEF"

#define	PNG_DATA	100
#define	BMP_DATA	200

#ifndef NO_PNG
struct mainprog_info_type {
    long width;
    long height;
    FILE *outfile;
    jmp_buf jmpbuf;
};

static void writepng_error_handler(png_structp png_ptr, png_const_charp msg)
{
    struct mainprog_info_type  *graphic;

    fprintf(stderr, "writepng libpng error: %s\n", msg);
    fflush(stderr);

    graphic = (struct mainprog_info_type*)png_get_error_ptr(png_ptr);
    if (graphic == NULL) {         /* we are completely hosed now */
        fprintf(stderr,
          "writepng severe error:  jmpbuf not recoverable; terminating.\n");
        fflush(stderr);
        return;
    }
    longjmp(graphic->jmpbuf, 1);
}

int png_pixel_plot(struct zint_symbol *symbol, int image_height, int image_width, char *pixelbuf, int rotate_angle)
{
	struct mainprog_info_type wpng_info;
	struct mainprog_info_type *graphic;
	
#ifndef _MSC_VER
	unsigned char outdata[image_width * 3];
#else
	unsigned char* outdata = (unsigned char*)_alloca(image_width * 3);
#endif
	png_structp  png_ptr;
	png_infop  info_ptr;
	graphic = &wpng_info;
	unsigned char *image_data;
	int i, row, column, errno;
	int fgred, fggrn, fgblu, bgred, bggrn, bgblu;

	switch(rotate_angle) {
		case 0:
		case 180:
			graphic->width = image_width;
			graphic->height = image_height;
			break;
		case 90:
		case 270:
			graphic->width = image_height;
			graphic->height = image_width;
			break;
	}

	/* sort out colour options */
	to_upper((unsigned char*)symbol->fgcolour);
	to_upper((unsigned char*)symbol->bgcolour);

	if(strlen(symbol->fgcolour) != 6) {
		strcpy(symbol->errtxt, "Malformed foreground colour target");
		return ZINT_ERROR_INVALID_OPTION;
	}
	if(strlen(symbol->bgcolour) != 6) {
		strcpy(symbol->errtxt, "Malformed background colour target");
		return ZINT_ERROR_INVALID_OPTION;
	}
	errno = is_sane(SSET, (unsigned char*)symbol->fgcolour, strlen(symbol->fgcolour));
	if (errno == ZINT_ERROR_INVALID_DATA) {
		strcpy(symbol->errtxt, "Malformed foreground colour target");
		return ZINT_ERROR_INVALID_OPTION;
	}
	errno = is_sane(SSET, (unsigned char*)symbol->bgcolour, strlen(symbol->bgcolour));
	if (errno == ZINT_ERROR_INVALID_DATA) {
		strcpy(symbol->errtxt, "Malformed background colour target");
		return ZINT_ERROR_INVALID_OPTION;
	}

	fgred = (16 * ctoi(symbol->fgcolour[0])) + ctoi(symbol->fgcolour[1]);
	fggrn = (16 * ctoi(symbol->fgcolour[2])) + ctoi(symbol->fgcolour[3]);
	fgblu = (16 * ctoi(symbol->fgcolour[4])) + ctoi(symbol->fgcolour[5]);
	bgred = (16 * ctoi(symbol->bgcolour[0])) + ctoi(symbol->bgcolour[1]);
	bggrn = (16 * ctoi(symbol->bgcolour[2])) + ctoi(symbol->bgcolour[3]);
	bgblu = (16 * ctoi(symbol->bgcolour[4])) + ctoi(symbol->bgcolour[5]);

	/* Open output file in binary mode */
	if((symbol->output_options & BARCODE_STDOUT) != 0) {
#ifdef _MSC_VER
		if (-1 == _setmode(_fileno(stdout), _O_BINARY)) {
			strcpy(symbol->errtxt, "Can't open output file");
			return ZINT_ERROR_FILE_ACCESS;
		}
#endif
		graphic->outfile = stdout;
	} else {
		if (!(graphic->outfile = fopen(symbol->outfile, "wb"))) {
			strcpy(symbol->errtxt, "Can't open output file");
			return ZINT_ERROR_FILE_ACCESS;
		}
	}

	/* Set up error handling routine as proc() above */
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, graphic, writepng_error_handler, NULL);
	if (!png_ptr) {
		strcpy(symbol->errtxt, "Out of memory");
		return ZINT_ERROR_MEMORY;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, NULL);
		strcpy(symbol->errtxt, "Out of memory");
		return ZINT_ERROR_MEMORY;
	}

	/* catch jumping here */
	if (setjmp(graphic->jmpbuf)) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		strcpy(symbol->errtxt, "libpng error occurred");
		return ZINT_ERROR_MEMORY;
	}

	/* open output file with libpng */
	png_init_io(png_ptr, graphic->outfile);

	/* set compression */
	png_set_compression_level(png_ptr,9);

	/* set Header block */
	png_set_IHDR(png_ptr, info_ptr, graphic->width, graphic->height,
		     8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
       PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	/* write all chunks up to (but not including) first IDAT */
	png_write_info(png_ptr, info_ptr);

	/* set up the transformations:  for now, just pack low-bit-depth pixels
	into bytes (one, two or four pixels per byte) */
	png_set_packing(png_ptr);

	/* Pixel Plotting */

	switch(rotate_angle) {
		case 0: /* Plot the right way up */
			for(row = 0; row < image_height; row++) {
				for(column = 0; column < image_width; column++) {
					i = column * 3;
					switch(*(pixelbuf + (image_width * row) + column))
					{
						case '1':
							outdata[i] = fgred;
							outdata[i + 1] = fggrn;
							outdata[i + 2] = fgblu;
							break;
						default:
							outdata[i] = bgred;
							outdata[i + 1] = bggrn;
							outdata[i + 2] = bgblu;
							break;

					}
				}
				/* write row contents to file */
				image_data = outdata;
				png_write_row(png_ptr, image_data);
			}
			break;
		case 90: /* Plot 90 degrees clockwise */
			for(row = 0; row < image_width; row++) {
				for(column = 0; column < image_height; column++) {
					i = column * 3;
					switch(*(pixelbuf + (image_width * (image_height - column - 1)) + row))
					{
						case '1':
							outdata[i] = fgred;
							outdata[i + 1] = fggrn;
							outdata[i + 2] = fgblu;
							break;
						default:
							outdata[i] = bgred;
							outdata[i + 1] = bggrn;
							outdata[i + 2] = bgblu;
							break;

					}
				}

				/* write row contents to file */
				image_data = outdata;
				png_write_row(png_ptr, image_data);
			}
			break;
		case 180: /* Plot upside down */
			for(row = 0; row < image_height; row++) {
				for(column = 0; column < image_width; column++) {
					i = column * 3;
					switch(*(pixelbuf + (image_width * (image_height - row - 1)) + (image_width - column - 1)))
					{
						case '1':
							outdata[i] = fgred;
							outdata[i + 1] = fggrn;
							outdata[i + 2] = fgblu;
							break;
						default:
							outdata[i] = bgred;
							outdata[i + 1] = bggrn;
							outdata[i + 2] = bgblu;
							break;

					}
				}

				/* write row contents to file */
				image_data = outdata;
				png_write_row(png_ptr, image_data);
			}
			break;
		case 270: /* Plot 90 degrees anti-clockwise */
			for(row = 0; row < image_width; row++) {
				for(column = 0; column < image_height; column++) {
					i = column * 3;
					switch(*(pixelbuf + (image_width * column) + (image_width - row - 1)))
					{
						case '1':
							outdata[i] = fgred;
							outdata[i + 1] = fggrn;
							outdata[i + 2] = fgblu;
							break;
						default:
							outdata[i] = bgred;
							outdata[i + 1] = bggrn;
							outdata[i + 2] = bgblu;
							break;

					}
				}

				/* write row contents to file */
				image_data = outdata;
				png_write_row(png_ptr, image_data);
			}
			break;
	}

	/* End the file */
	png_write_end(png_ptr, NULL);

	/* make sure we have disengaged */
	if (png_ptr && info_ptr) png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(wpng_info.outfile);
	return 0;
}
#endif /* NO_PNG */

int bmp_pixel_plot(struct zint_symbol *symbol, int image_height, int image_width, char *pixelbuf, int rotate_angle)
{
	int i, row, column, errno;
	int fgred, fggrn, fgblu, bgred, bggrn, bgblu;

	switch(rotate_angle) {
		case 0:
		case 180:
			symbol->bitmap_width = image_width;
			symbol->bitmap_height = image_height;
			break;
		case 90:
		case 270:			
			symbol->bitmap_width = image_height;
			symbol->bitmap_height = image_width;
			break;
	}

	if (symbol->bitmap != NULL)
		free(symbol->bitmap);

    symbol->bitmap = (char *) malloc(image_width * image_height * 3);


	/* sort out colour options */
	to_upper((unsigned char*)symbol->fgcolour);
	to_upper((unsigned char*)symbol->bgcolour);

	if(strlen(symbol->fgcolour) != 6) {
		strcpy(symbol->errtxt, "Malformed foreground colour target");
		return ZINT_ERROR_INVALID_OPTION;
	}
	if(strlen(symbol->bgcolour) != 6) {
		strcpy(symbol->errtxt, "Malformed background colour target");
		return ZINT_ERROR_INVALID_OPTION;
	}
	errno = is_sane(SSET, (unsigned char*)symbol->fgcolour, strlen(symbol->fgcolour));
	if (errno == ZINT_ERROR_INVALID_DATA) {
		strcpy(symbol->errtxt, "Malformed foreground colour target");
		return ZINT_ERROR_INVALID_OPTION;
	}
	errno = is_sane(SSET, (unsigned char*)symbol->bgcolour, strlen(symbol->fgcolour));
	if (errno == ZINT_ERROR_INVALID_DATA) {
		strcpy(symbol->errtxt, "Malformed background colour target");
		return ZINT_ERROR_INVALID_OPTION;
	}

	fgred = (16 * ctoi(symbol->fgcolour[0])) + ctoi(symbol->fgcolour[1]);
	fggrn = (16 * ctoi(symbol->fgcolour[2])) + ctoi(symbol->fgcolour[3]);
	fgblu = (16 * ctoi(symbol->fgcolour[4])) + ctoi(symbol->fgcolour[5]);
	bgred = (16 * ctoi(symbol->bgcolour[0])) + ctoi(symbol->bgcolour[1]);
	bggrn = (16 * ctoi(symbol->bgcolour[2])) + ctoi(symbol->bgcolour[3]);
	bgblu = (16 * ctoi(symbol->bgcolour[4])) + ctoi(symbol->bgcolour[5]);

	/* Pixel Plotting */
	i = 0;	
	switch(rotate_angle) {
		case 0: /* Plot the right way up */
			for(row = 0; row < image_height; row++) {
				for(column = 0; column < image_width; column++) {
					switch(*(pixelbuf + (image_width * row) + column))
					{
						case '1':
							symbol->bitmap[i++] = fgred;
							symbol->bitmap[i++] = fggrn;
							symbol->bitmap[i++] = fgblu;
							break;
						default:
							symbol->bitmap[i++] = bgred;
							symbol->bitmap[i++] = bggrn;
							symbol->bitmap[i++] = bgblu;
							break;

					}
				}
			}
			break;
		case 90: /* Plot 90 degrees clockwise */			
			for(row = 0; row < image_width; row++) {
				for(column = 0; column < image_height; column++) {
					switch(*(pixelbuf + (image_width * (image_height - column - 1)) + row))
					{
						case '1':
							symbol->bitmap[i++] = fgred;
							symbol->bitmap[i++] = fggrn;
							symbol->bitmap[i++] = fgblu;
							break;
						default:
							symbol->bitmap[i++] = bgred;
							symbol->bitmap[i++] = bggrn;
							symbol->bitmap[i++] = bgblu;
							break;

					}
				}
			}
			break;
		case 180: /* Plot upside down */
			for(row = 0; row < image_height; row++) {
				for(column = 0; column < image_width; column++) {
					switch(*(pixelbuf + (image_width * (image_height - row - 1)) + (image_width - column - 1)))
					{
						case '1':
							symbol->bitmap[i++] = fgred;
							symbol->bitmap[i++] = fggrn;
							symbol->bitmap[i++] = fgblu;
							break;
						default:
							symbol->bitmap[i++] = bgred;
							symbol->bitmap[i++] = bggrn;
							symbol->bitmap[i++] = bgblu;
							break;

					}
				}
			}
			break;
		case 270: /* Plot 90 degrees anti-clockwise */
			for(row = 0; row < image_width; row++) {
				for(column = 0; column < image_height; column++) {
					switch(*(pixelbuf + (image_width * column) + (image_width - row - 1)))
					{
						case '1':
							symbol->bitmap[i++] = fgred;
							symbol->bitmap[i++] = fggrn;
							symbol->bitmap[i++] = fgblu;
							break;
						default:
							symbol->bitmap[i++] = bgred;
							symbol->bitmap[i++] = bggrn;
							symbol->bitmap[i++] = bgblu;
							break;

					}
				}
			}
			break;
	}

	return 0;
}

int png_to_file(struct zint_symbol *symbol, int image_height, int image_width, char *pixelbuf, int rotate_angle, int image_type)
{
	int error_number;
	float scaler = symbol->scale;
	char *scaled_pixelbuf;
	int horiz, vert, i;
	int scale_width, scale_height;

	if(scaler == 0) { scaler = 0.5; }
	scale_width = image_width * scaler;
	scale_height = image_height * scaler;

	/* Apply scale options by creating another pixel buffer */
	if (!(scaled_pixelbuf = (char *) malloc(scale_width * scale_height))) {
		printf("Insufficient memory for pixel buffer");
		return ZINT_ERROR_ENCODING_PROBLEM;
	} else {
		for(i = 0; i < (scale_width * scale_height); i++) {
			*(scaled_pixelbuf + i) = '0';
		}
	}

	for(vert = 0; vert < scale_height; vert++) {
		for(horiz = 0; horiz < scale_width; horiz++) {
			*(scaled_pixelbuf + (vert * scale_width) + horiz) = *(pixelbuf + ((int)(vert / scaler) * image_width) + (int)(horiz / scaler));
		}
	}

	if(image_type == PNG_DATA) {
#ifndef NO_PNG
		error_number = png_pixel_plot(symbol, scale_height, scale_width, scaled_pixelbuf, rotate_angle);
#else
		return ZINT_ERROR_INVALID_OPTION;
#endif
	} else {
		error_number = bmp_pixel_plot(symbol, scale_height, scale_width, scaled_pixelbuf, rotate_angle);
	}

	free(scaled_pixelbuf);

	return error_number;
}

void draw_bar(char *pixelbuf, int xpos, int xlen, int ypos, int ylen, int image_width, int image_height)
{
	/* Draw a rectangle */
	int i, j, png_ypos;

	png_ypos = image_height - ypos - ylen;
	/* This fudge is needed because EPS measures height from the bottom up but
	PNG measures y position from the top down */

	for(i = (xpos); i < (xpos + xlen); i++) {
		for( j = (png_ypos); j < (png_ypos + ylen); j++) {
			*(pixelbuf + (image_width * j) + i) = '1';
		}
	}
}

int bullseye_pixel(int row, int col) {
	int block_val, block_pos, return_val;

	block_val = bullseye_compressed[(row * 12) + (col / 8)];
	return_val = 0;
	block_pos = col % 8;

	switch(block_pos) {
		case 0: if((block_val & 0x80) != 0) { return_val = 1; } break;
		case 1: if((block_val & 0x40) != 0) { return_val = 1; } break;
		case 2: if((block_val & 0x20) != 0) { return_val = 1; } break;
		case 3: if((block_val & 0x10) != 0) { return_val = 1; } break;
		case 4: if((block_val & 0x08) != 0) { return_val = 1; } break;
		case 5: if((block_val & 0x04) != 0) { return_val = 1; } break;
		case 6: if((block_val & 0x02) != 0) { return_val = 1; } break;
		case 7: if((block_val & 0x01) != 0) { return_val = 1; } break;
	}

	return return_val;
}

void draw_bullseye(char *pixelbuf, int image_width, int xoffset, int yoffset)
{
	/* Central bullseye in Maxicode symbols */
	int i, j;

	for(j = 103; j < 196; j++) {
		for(i = 0; i < 93; i++) {
			if(bullseye_pixel(j - 103, i)) {
			/* if(bullseye[(((j - 103) * 93) + i)] == 1) { */
				*(pixelbuf + (image_width * j) + (image_width * yoffset) + i + 99 + xoffset) = '1';
			}
		}
	}
}

void draw_hexagon(char *pixelbuf, int image_width, int xposn, int yposn)
{
	/* Put a hexagon into the pixel buffer */
	int i, j;

	for(i = 0; i < 12; i++) {
		for(j = 0; j < 10; j++) {
			if(hexagon[(i * 10) + j] == 1) {
				*(pixelbuf + (image_width * i) + (image_width * yposn) + xposn + j) = '1';
			}
		}
	}
}

void draw_letter(char *pixelbuf, unsigned char letter, int xposn, int yposn, int smalltext, int image_width, int image_height)
{
	/* Put a letter into a position */
	int skip, i, j, glyph_no, alphabet;

	skip = 0;
	alphabet = 0;

	if(letter < 33) { skip = 1; }
	if((letter > 127) && (letter < 161)) { skip = 1; }

	if(skip == 0) {
		if(letter > 128) {
			alphabet = 1;
			glyph_no = letter - 161;
		} else {
			glyph_no = letter - 33;
		}

		if(smalltext) {
			for(i = 0; i <= 8; i++) {
				for(j = 0; j < 5; j++) {
					if(alphabet == 0) {
						if(small_font[(glyph_no * 5) + (i * 475) + j - 1] == 1) {
							*(pixelbuf + (i * image_width) + (yposn * image_width) + xposn + j) = '1';
						}
					} else {
						if(small_font_extended[(glyph_no * 5) + (i * 475) + j - 1] == 1) {
							*(pixelbuf + (i * image_width) + (yposn * image_width) + xposn + j) = '1';
						}
					}
				}
			}
		} else {
			for(i = 0; i <= 13; i++) {
				for(j = 0; j < 7 ; j++) {
					if(alphabet == 0) {
						if(ascii_font[(glyph_no * 7) + (i * 665) + j - 1] == 1) {
							*(pixelbuf + (i * image_width) + (yposn * image_width) + xposn + j) = '1';
						}
					} else {
						if(ascii_ext_font[(glyph_no * 7) + (i * 665) + j - 1] == 1) {
							*(pixelbuf + (i * image_width) + (yposn * image_width) + xposn + j) = '1';
						}
					}
				}
			}
		}
	}
}

void draw_string(char *pixbuf, char input_string[], int xposn, int yposn, int smalltext, int image_width, int image_height)
{
	/* Plot a string into the pixel buffer */
	int i, string_length, string_left_hand;

	string_length = strlen(input_string);
	string_left_hand = xposn - ((7 * string_length) / 2);

	for(i = 0; i < string_length; i++) {
		draw_letter(pixbuf, input_string[i], string_left_hand + (i * 7), yposn, smalltext, image_width, image_height);
	}

}

int maxi_png_plot(struct zint_symbol *symbol, int rotate_angle, int data_type)
{
	int i, row, column, xposn, yposn;
	int image_height, image_width;
	char *pixelbuf;
	int error_number;
	int xoffset, yoffset;

	xoffset = symbol->border_width + symbol->whitespace_width;
	yoffset = symbol->border_width;
	image_width = 300 + (2 * xoffset * 2);
	image_height = 300 + (2 * yoffset * 2);

	if (!(pixelbuf = (char *) malloc(image_width * image_height))) {
		printf("Insifficient memory for pixel buffer");
		return ZINT_ERROR_ENCODING_PROBLEM;
	} else {
		for(i = 0; i < (image_width * image_height); i++) {
			*(pixelbuf + i) = '0';
		}
	}

	draw_bullseye(pixelbuf, image_width, (2 * xoffset), (2 * yoffset));

	for(row = 0; row < symbol->rows; row++) {
		yposn = row * 9;
		for(column = 0; column < symbol->width; column++) {
			xposn = column * 10;
			if(module_is_set(symbol, row, column)) {
				if(row & 1) {
					/* Odd (reduced) row */
					xposn += 5;
					draw_hexagon(pixelbuf, image_width, xposn + (2 * xoffset), yposn + (2 * yoffset));
				} else {
					/* Even (full) row */
					draw_hexagon(pixelbuf, image_width, xposn + (2 * xoffset), yposn + (2 * yoffset));
				}
			}
		}
	}

	if(((symbol->output_options & BARCODE_BOX) != 0) || ((symbol->output_options & BARCODE_BIND) != 0)) {
		/* boundary bars */
		draw_bar(pixelbuf, 0, image_width, 0, symbol->border_width * 2, image_width, image_height);
		draw_bar(pixelbuf, 0, image_width, 300 + (symbol->border_width * 2), symbol->border_width * 2, image_width, image_height);
	}

	if((symbol->output_options & BARCODE_BOX) != 0) {
		/* side bars */
		draw_bar(pixelbuf, 0, symbol->border_width * 2, 0, image_height, image_width, image_height);
		draw_bar(pixelbuf, 300 + ((symbol->border_width + symbol->whitespace_width + symbol->whitespace_width) * 2), symbol->border_width * 2, 0, image_height, image_width, image_height);
	}

	error_number=png_to_file(symbol, image_height, image_width, pixelbuf, rotate_angle, data_type);
	free(pixelbuf);
	return error_number;
}

void to_latin1(unsigned char source[], unsigned char preprocessed[])
{
	int j, i, input_length;

	input_length = ustrlen(source);

	j = 0;
	i = 0;
	while (i < input_length)
	{
		if(source[i] < 128) {
			preprocessed[j] = source[i];
			j++;
			i++;
		} else {
			if(source[i] == 0xC2) {
				preprocessed[j] = source[i + 1];
				j++;
				i += 2;
			}
			if(source[i] == 0xC3) {
				preprocessed[j] = source[i + 1] + 64;
				j++;
				i += 2;
			}
		}
	}
	preprocessed[j] = '\0';

	return;
}

/// MICROSOFT_PROJECT B CHANGES BEGIN
#ifdef _MSC_VER
struct StackAllocFreer
{
	StackAllocFreer(void* data) : m_data(data) {}
	~StackAllocFreer() { _freea(m_data); }
	void* m_data;
};
#endif
/// MICROSOFT PROJECT B CHANGES END

int png_plot(struct zint_symbol *symbol, int rotate_angle, int data_type)
{
	int textdone, main_width, comp_offset, large_bar_count;
	char textpart[10], addon[6];
	float addon_text_posn, preset_height, large_bar_height;
	int i, r, textoffset, yoffset, xoffset, latch, image_width, image_height;
	char *pixelbuf;
	int addon_latch = 0, smalltext = 0;
	int this_row, block_width, plot_height, plot_yposn, textpos;
	float row_height, row_posn;
	int error_number;
	int default_text_posn;
	int next_yposn;
#ifndef _MSC_VER
	unsigned char local_text[ustrlen(symbol->text) + 1];
#else
	unsigned char* local_text = (unsigned char*)_malloca(ustrlen(symbol->text) + 1);

	/// MICROSOFT_PROJECT B CHANGES BEGIN
	// malloca'd memory is not automatically free'd when the memory goes out of scope,
	// so wrap it here to ensure we don't leak memory
	StackAllocFreer stackFreer(local_text);
	/// MICROSOFT PROJECT B CHANGES END
#endif

	if(symbol->show_hrt != 0) {
		to_latin1(symbol->text, local_text);
	} else {
		local_text[0] = '\0';
	}

	textdone = 0;
	main_width = symbol->width;
	strcpy(addon, "");
	comp_offset = 0;
	addon_text_posn = 0.0;
	row_height = 0;
	if(symbol->output_options & SMALL_TEXT) {
		smalltext = 1;
	}

	if (symbol->height == 0) {
		symbol->height = 50;
	}

	large_bar_count = 0;
	preset_height = 0.0;
	for(i = 0; i < symbol->rows; i++) {
		preset_height += symbol->row_height[i];
		if(symbol->row_height[i] == 0) {
			large_bar_count++;
		}
	}

	if (large_bar_count == 0) {
		symbol->height = preset_height;
		large_bar_height = 10;
	} else {
		large_bar_height = (symbol->height - preset_height) / large_bar_count;
	}

	while(!(module_is_set(symbol, symbol->rows - 1, comp_offset))) {
		comp_offset++;
	}

	/* Certain symbols need whitespace otherwise characters get chopped off the sides */
	if ((((symbol->symbology == BARCODE_EANX) && (symbol->rows == 1)) || (symbol->symbology == BARCODE_EANX_CC))
		|| (symbol->symbology == BARCODE_ISBNX)) {
		switch(ustrlen(local_text)) {
			case 13: /* EAN 13 */
			case 16:
			case 19:
				if(symbol->whitespace_width == 0) {
					symbol->whitespace_width = 10;
				}
				main_width = 96 + comp_offset;
				break;
			default:
				main_width = 68 + comp_offset;
		}
	}

	if (((symbol->symbology == BARCODE_UPCA) && (symbol->rows == 1)) || (symbol->symbology == BARCODE_UPCA_CC)) {
		if(symbol->whitespace_width == 0) {
			symbol->whitespace_width = 10;
			main_width = 96 + comp_offset;
		}
	}

	if (((symbol->symbology == BARCODE_UPCE) && (symbol->rows == 1)) || (symbol->symbology == BARCODE_UPCE_CC)) {
		if(symbol->whitespace_width == 0) {
			symbol->whitespace_width = 10;
			main_width = 51 + comp_offset;
		}
	}

	latch = 0;
	r = 0;
	/* Isolate add-on text */
	if(is_extendable(symbol->symbology)) {
		for(i = 0; i < ustrlen(local_text); i++) {
			if (latch == 1) {
				addon[r] = local_text[i];
				r++;
			}
			if (symbol->text[i] == '+') {
				latch = 1;
			}
		}
	}
	addon[r] = '\0';

	if(ustrlen(local_text) != 0) {
		textoffset = 9;
	} else {
		textoffset = 0;
	}

	xoffset = symbol->border_width + symbol->whitespace_width;
	yoffset = symbol->border_width;
	image_width = 2 * (symbol->width + xoffset + xoffset);
	image_height = 2 * (symbol->height + textoffset + yoffset + yoffset);

	if (!(pixelbuf = (char *) malloc(image_width * image_height))) {
		printf("Insufficient memory for pixel buffer");
		return ZINT_ERROR_ENCODING_PROBLEM;
	} else {
		for(i = 0; i < (image_width * image_height); i++) {
			*(pixelbuf + i) = '0';
		}
	}

	if(((symbol->output_options & BARCODE_BOX) != 0) || ((symbol->output_options & BARCODE_BIND) != 0)) {
		default_text_posn = image_height - 17;
	} else {
		default_text_posn = image_height - 17 - symbol->border_width - symbol->border_width;
	}

	row_posn = textoffset + yoffset;
	next_yposn = textoffset + yoffset;
	row_height = 0;

	/* Plot the body of the symbol to the pixel buffer */
	for(r = 0; r < symbol->rows; r++) {
		this_row = symbol->rows - r - 1; /* invert r otherwise plots upside down */
		row_posn += row_height;
		plot_yposn = next_yposn;
		if(symbol->row_height[this_row] == 0) {
			row_height = large_bar_height;
		} else {
			row_height = symbol->row_height[this_row];
		}
		next_yposn = (int)(row_posn + row_height);
		plot_height = next_yposn - plot_yposn;

		i = 0;
		if(module_is_set(symbol, this_row, 0)) {
			latch = 1;
		} else {
			latch = 0;
		}

		do {
			block_width = 0;
			do {
				block_width++;
			} while (module_is_set(symbol, this_row, i + block_width) == module_is_set(symbol, this_row, i));
			if((addon_latch == 0) && (r == 0) && (i > main_width)) {
				plot_height = (int)(row_height - 5.0);
				plot_yposn = (int)(row_posn - 5.0);
				addon_text_posn = row_posn + row_height - 8.0;
				addon_latch = 1;
			} 
			if(latch == 1) { 
				/* a bar */
				draw_bar(pixelbuf, (i + xoffset) * 2, block_width * 2, plot_yposn * 2, plot_height * 2, image_width, image_height);
				latch = 0;
			} else {
				/* a space */
				latch = 1;
			}
			i += block_width;

		} while (i < symbol->width);
	}

	xoffset += comp_offset;

	if ((((symbol->symbology == BARCODE_EANX) && (symbol->rows == 1)) || (symbol->symbology == BARCODE_EANX_CC)) || (symbol->symbology == BARCODE_ISBNX)) {
		/* guard bar extensions and text formatting for EAN8 and EAN13 */
		switch(ustrlen(local_text)) {
			case 8: /* EAN-8 */
			case 11:
			case 14:
				draw_bar(pixelbuf, (0 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
				draw_bar(pixelbuf, (2 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
				draw_bar(pixelbuf, (32 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
				draw_bar(pixelbuf, (34 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
				draw_bar(pixelbuf, (64 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
				draw_bar(pixelbuf, (66 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
				for(i = 0; i < 4; i++) {
					textpart[i] = symbol->text[i];
				}
				textpart[4] = '\0';
				textpos = 2 * (17 + xoffset);

				draw_string(pixelbuf, textpart, textpos, default_text_posn, smalltext, image_width, image_height);
				for(i = 0; i < 4; i++) {
					textpart[i] = symbol->text[i + 4];
				}
				textpart[4] = '\0';
				textpos = 2 * (50 + xoffset);
				draw_string(pixelbuf, textpart, textpos, default_text_posn, smalltext, image_width, image_height);
				textdone = 1;
				switch(strlen(addon)) {
					case 2:
						textpos = 2 * (xoffset + 86);
						draw_string(pixelbuf, addon, textpos, image_height - (addon_text_posn * 2) - 13, smalltext, image_width, image_height);
						break;
					case 5:
						textpos = 2 * (xoffset + 100);
						draw_string(pixelbuf, addon, textpos, image_height - (addon_text_posn * 2) - 13, smalltext, image_width, image_height);
						break;
				}

				break;
			case 13: /* EAN 13 */
			case 16:
			case 19:
				draw_bar(pixelbuf, (0 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
				draw_bar(pixelbuf, (2 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
				draw_bar(pixelbuf, (46 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
				draw_bar(pixelbuf, (48 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
				draw_bar(pixelbuf, (92 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
				draw_bar(pixelbuf, (94 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);

				textpart[0] = symbol->text[0];
				textpart[1] = '\0';
				textpos = 2 * (-7 + xoffset);
				draw_string(pixelbuf, textpart, textpos, default_text_posn, smalltext, image_width, image_height);
				for(i = 0; i < 6; i++) {
					textpart[i] = symbol->text[i + 1];
				}
				textpart[6] = '\0';
				textpos = 2 * (24 + xoffset);
				draw_string(pixelbuf, textpart, textpos, default_text_posn, smalltext, image_width, image_height);
				for(i = 0; i < 6; i++) {
					textpart[i] = symbol->text[i + 7];
				}
				textpart[6] = '\0';
				textpos = 2 * (71 + xoffset);
				draw_string(pixelbuf, textpart, textpos, default_text_posn, smalltext, image_width, image_height);
				textdone = 1;
				switch(strlen(addon)) {
					case 2:
						textpos = 2 * (xoffset + 114);
						draw_string(pixelbuf, addon, textpos, image_height - (addon_text_posn * 2) - 13, smalltext, image_width, image_height);
						break;
					case 5:
						textpos = 2 * (xoffset + 128);
						draw_string(pixelbuf, addon, textpos, image_height - (addon_text_posn * 2) - 13, smalltext, image_width, image_height);
						break;
				}
				break;

		}
	}

	if (((symbol->symbology == BARCODE_UPCA) && (symbol->rows == 1)) || (symbol->symbology == BARCODE_UPCA_CC)) {
		/* guard bar extensions and text formatting for UPCA */
		latch = 1;

		i = 0 + comp_offset;
		do {
			block_width = 0;
			do {
				block_width++;
			} while (module_is_set(symbol, symbol->rows - 1, i + block_width) == module_is_set(symbol, symbol->rows - 1, i));
			if(latch == 1) {
				/* a bar */
				draw_bar(pixelbuf, (i + xoffset - comp_offset) * 2, block_width * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
				latch = 0;
			} else {
				/* a space */
				latch = 1;
			}
			i += block_width;
		} while (i < 11 + comp_offset);
		draw_bar(pixelbuf, (46 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
		draw_bar(pixelbuf, (48 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
		latch = 1;
		i = 85 + comp_offset;
		do {
			block_width = 0;
			do {
				block_width++;
			} while (module_is_set(symbol, symbol->rows - 1, i + block_width) == module_is_set(symbol, symbol->rows - 1, i));
			if(latch == 1) {
				/* a bar */
				draw_bar(pixelbuf, (i + xoffset - comp_offset) * 2, block_width * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
				latch = 0;
			} else {
				/* a space */
				latch = 1;
			}
			i += block_width;
		} while (i < 96 + comp_offset);
		textpart[0] = symbol->text[0];
		textpart[1] = '\0';
		textpos = 2 * (-5 + xoffset);
		draw_string(pixelbuf, textpart, textpos, default_text_posn, smalltext, image_width, image_height);
		for(i = 0; i < 5; i++) {
			textpart[i] = symbol->text[i + 1];
		}
		textpart[5] = '\0';
		textpos = 2 * (27 + xoffset);
		draw_string(pixelbuf, textpart, textpos, default_text_posn, smalltext, image_width, image_height);
		for(i = 0; i < 5; i++) {
			textpart[i] = symbol->text[i + 6];
		}
		textpart[6] = '\0';
		textpos = 2 * (68 + xoffset);
		draw_string(pixelbuf, textpart, textpos, default_text_posn, smalltext, image_width, image_height);
		textpart[0] = symbol->text[11];
		textpart[1] = '\0';
		textpos = 2 * (100 + xoffset);
		draw_string(pixelbuf, textpart, textpos, default_text_posn, smalltext, image_width, image_height);
		textdone = 1;
		switch(strlen(addon)) {
			case 2:
				textpos = 2 * (xoffset + 116);
				draw_string(pixelbuf, addon, textpos, image_height - (addon_text_posn * 2) - 13, smalltext, image_width, image_height);
				break;
			case 5:
				textpos = 2 * (xoffset + 130);
				draw_string(pixelbuf, addon, textpos, image_height - (addon_text_posn * 2) - 13, smalltext, image_width, image_height);
				break;
		}

	}

	if (((symbol->symbology == BARCODE_UPCE) && (symbol->rows == 1)) || (symbol->symbology == BARCODE_UPCE_CC)) {
		/* guard bar extensions and text formatting for UPCE */
		draw_bar(pixelbuf, (0 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
		draw_bar(pixelbuf, (2 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
		draw_bar(pixelbuf, (46 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
		draw_bar(pixelbuf, (48 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);
		draw_bar(pixelbuf, (50 + xoffset) * 2, 1 * 2, (4 + (int)yoffset) * 2, 5 * 2, image_width, image_height);

		textpart[0] = symbol->text[0];
		textpart[1] = '\0';
		textpos = 2 * (-5 + xoffset);
		draw_string(pixelbuf, textpart, textpos, default_text_posn, smalltext, image_width, image_height);
		for(i = 0; i < 6; i++) {
			textpart[i] = symbol->text[i + 1];
		}
		textpart[6] = '\0';
		textpos = 2 * (24 + xoffset);
		draw_string(pixelbuf, textpart, textpos, default_text_posn, smalltext, image_width, image_height);
		textpart[0] = symbol->text[7];
		textpart[1] = '\0';
		textpos = 2 * (55 + xoffset);
		draw_string(pixelbuf, textpart, textpos, default_text_posn, smalltext, image_width, image_height);
		textdone = 1;
		switch(strlen(addon)) {
			case 2:
				textpos = 2 * (xoffset + 70);
				draw_string(pixelbuf, addon, textpos, image_height - (addon_text_posn * 2) - 13, smalltext, image_width, image_height);
				break;
			case 5:
				textpos = 2 * (xoffset + 84);
				draw_string(pixelbuf, addon, textpos, image_height - (addon_text_posn * 2) - 13, smalltext, image_width, image_height);
				break;
		}

	}

	xoffset -= comp_offset;

	/* Put boundary bars or box around symbol */
	if(((symbol->output_options & BARCODE_BOX) != 0) || ((symbol->output_options & BARCODE_BIND) != 0)) {
		/* boundary bars */
		draw_bar(pixelbuf, 0, (symbol->width + xoffset + xoffset) * 2, textoffset * 2, symbol->border_width * 2, image_width, image_height);
		draw_bar(pixelbuf, 0, (symbol->width + xoffset + xoffset) * 2, (textoffset + symbol->height + symbol->border_width) * 2, symbol->border_width * 2, image_width, image_height);
		if((symbol->output_options & BARCODE_BIND) != 0) {
			if((symbol->rows > 1) && (is_stackable(symbol->symbology) == 1)) {
				/* row binding */
				for(r = 1; r < symbol->rows; r++) {
					draw_bar(pixelbuf, xoffset * 2, symbol->width * 2, ((r * row_height) + textoffset + yoffset - 1) * 2, 2 * 2, image_width, image_height);
				}
			}
		}
	}

	if((symbol->output_options & BARCODE_BOX) != 0) {
		/* side bars */
		draw_bar(pixelbuf, 0, symbol->border_width * 2, textoffset * 2, (symbol->height + (2 * symbol->border_width)) * 2, image_width, image_height);
		draw_bar(pixelbuf, (symbol->width + xoffset + xoffset - symbol->border_width) * 2, symbol->border_width * 2, textoffset * 2, (symbol->height + (2 * symbol->border_width)) * 2, image_width, image_height);
	}

	/* Put the human readable text at the bottom */
	if((textdone == 0) && (ustrlen(local_text) != 0)) {
		textpos = (image_width / 2);
		draw_string(pixelbuf, (char*)local_text, textpos, default_text_posn, smalltext, image_width, image_height);
	}

	error_number=png_to_file(symbol, image_height, image_width, pixelbuf, rotate_angle, data_type);
	free(pixelbuf);
	return error_number;
}

#ifndef NO_PNG
int png_handle(struct zint_symbol *symbol, int rotate_angle)
{
	int error;

	if(symbol->symbology == BARCODE_MAXICODE) {
		error = maxi_png_plot(symbol, rotate_angle, PNG_DATA);
	} else {

		error = png_plot(symbol, rotate_angle, PNG_DATA);
	}

	return error;
}
#endif /* NO_PNG */

int bmp_handle(struct zint_symbol *symbol, int rotate_angle)
{
	int error;

	if(symbol->symbology == BARCODE_MAXICODE) {
		error = maxi_png_plot(symbol, rotate_angle, BMP_DATA);
	} else {
		error = png_plot(symbol, rotate_angle, BMP_DATA);
	}

	return error;
}

