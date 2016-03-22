/* svg.c - Scalable Vector Graphics */

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

#include <locale.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "common.h"

#define SSET	"0123456789ABCDEF"

int svg_plot(struct zint_symbol *symbol)
{
	int i, block_width, latch, r, this_row;
	float textpos, large_bar_height, preset_height, row_height, row_posn = 0.0;
	FILE *fsvg;
	int error_number = 0;
	int textoffset, xoffset, yoffset, textdone, main_width;
	char textpart[10], addon[6];
	int large_bar_count, comp_offset;
	float addon_text_posn;
	float scaler = symbol->scale;
	float default_text_posn;
	int plot_text = 1;
	const char *locale = NULL;

	row_height=0;
	textdone = 0;
	main_width = symbol->width;
	strcpy(addon, "");
	comp_offset = 0;
	addon_text_posn = 0.0;

	if((symbol->output_options & BARCODE_STDOUT) != 0) {
		fsvg = stdout;
	} else {
		fsvg = fopen(symbol->outfile, "w");
	}
	if(fsvg == NULL) {
		strcpy(symbol->errtxt, "Could not open output file");
		return ZINT_ERROR_FILE_ACCESS;
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
	error_number = is_sane(SSET, (unsigned char*)symbol->fgcolour, strlen(symbol->fgcolour));
	if (error_number == ZINT_ERROR_INVALID_DATA) {
		strcpy(symbol->errtxt, "Malformed foreground colour target");
		return ZINT_ERROR_INVALID_OPTION;
	}
	error_number = is_sane(SSET, (unsigned char*)symbol->bgcolour, strlen(symbol->bgcolour));
	if (error_number == ZINT_ERROR_INVALID_DATA) {
		strcpy(symbol->errtxt, "Malformed background colour target");
		return ZINT_ERROR_INVALID_OPTION;
	}
	locale = setlocale(LC_ALL, "C");
	
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
	large_bar_height = (symbol->height - preset_height) / large_bar_count;

	if (large_bar_count == 0) {
		symbol->height = preset_height;
	}

	while(!(module_is_set(symbol, symbol->rows - 1, comp_offset))) {
		comp_offset++;
	}

	/* Certain symbols need whitespace otherwise characters get chopped off the sides */
	if ((((symbol->symbology == BARCODE_EANX) && (symbol->rows == 1)) || (symbol->symbology == BARCODE_EANX_CC))
		|| (symbol->symbology == BARCODE_ISBNX)) {
		switch(ustrlen(symbol->text)) {
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
		for(i = 0; i < ustrlen(symbol->text); i++) {
			if (latch == 1) {
				addon[r] = symbol->text[i];
				r++;
			}
			if (symbol->text[i] == '+') {
				latch = 1;
			}
		}
	}
	addon[r] = '\0';

	if((symbol->show_hrt == 0) || (ustrlen(symbol->text) == 0)) {
		plot_text = 0;
	}
	if(plot_text) {
		textoffset = 9;
	} else {
		textoffset = 0;
	}
	xoffset = symbol->border_width + symbol->whitespace_width;
	yoffset = symbol->border_width;

	/* Start writing the header */
	fprintf(fsvg, "<?xml version=\"1.0\" standalone=\"no\"?>\n");
	fprintf(fsvg, "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\n");
	fprintf(fsvg, "   \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");
	if(symbol->symbology != BARCODE_MAXICODE) {
		fprintf(fsvg, "<svg width=\"%d\" height=\"%d\" version=\"1.1\"\n", roundup((symbol->width + xoffset + xoffset) * scaler), roundup((symbol->height + textoffset + yoffset + yoffset) * scaler));
	} else {
		fprintf(fsvg, "<svg width=\"%d\" height=\"%d\" version=\"1.1\"\n", roundup((74.0 + xoffset + xoffset) * scaler), roundup((72.0 + yoffset + yoffset) * scaler));
	}
	fprintf(fsvg, "   xmlns=\"http://www.w3.org/2000/svg\">\n");
	if(ustrlen(symbol->text) != 0) {
		fprintf(fsvg, "   <desc>%s\n", symbol->text);
	} else {
		fprintf(fsvg, "   <desc>Zint Generated Symbol\n");
	}
	fprintf(fsvg, "   </desc>\n");
	fprintf(fsvg, "\n   <g id=\"barcode\" fill=\"#%s\">\n", symbol->fgcolour);

	if(symbol->symbology != BARCODE_MAXICODE) {
		fprintf(fsvg, "      <rect x=\"0\" y=\"0\" width=\"%d\" height=\"%d\" fill=\"#%s\" />\n", roundup((symbol->width + xoffset + xoffset) * scaler), roundup((symbol->height + textoffset + yoffset + yoffset) * scaler), symbol->bgcolour);
	} else {
		fprintf(fsvg, "      <rect x=\"0\" y=\"0\" width=\"%d\" height=\"%d\" fill=\"#%s\" />\n", roundup((74.0 + xoffset + xoffset) * scaler), roundup((72.0 + yoffset + yoffset) * scaler), symbol->bgcolour);
	}

	if(((symbol->output_options & BARCODE_BOX) != 0) || ((symbol->output_options & BARCODE_BIND) != 0)) {
		default_text_posn = (symbol->height + textoffset + symbol->border_width + symbol->border_width) * scaler;
	} else {
		default_text_posn = (symbol->height + textoffset + symbol->border_width) * scaler;
	}

	if(symbol->symbology == BARCODE_MAXICODE) {
		/* Maxicode uses hexagons */
		float ax, ay, bx, by, cx, cy, dx, dy, ex, ey, fx, fy, mx, my;


		textoffset = 0.0;
		if (((symbol->output_options & BARCODE_BOX) != 0) || ((symbol->output_options & BARCODE_BIND) != 0)) {
			fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", 0.0, 0.0, (74.0 + xoffset + xoffset) * scaler, symbol->border_width * scaler);
			fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", 0.0, (72.0 + symbol->border_width) * scaler, (74.0 + xoffset + xoffset) * scaler, symbol->border_width * scaler);
		}
		if((symbol->output_options & BARCODE_BOX) != 0) {
			/* side bars */
			fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", 0.0, 0.0, symbol->border_width * scaler, (72.0 + (2 * symbol->border_width)) * scaler);
			fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", (74.0 + xoffset + xoffset - symbol->border_width) * scaler, 0.0, symbol->border_width * scaler, (72.0 + (2 * symbol->border_width)) * scaler);
		}
		fprintf(fsvg, "      <circle cx=\"%.2f\" cy=\"%.2f\" r=\"%.2f\" fill=\"#%s\" />\n", (35.76 + xoffset) * scaler, (35.60 + yoffset) * scaler, 10.85 * scaler, symbol->fgcolour);
		fprintf(fsvg, "      <circle cx=\"%.2f\" cy=\"%.2f\" r=\"%.2f\" fill=\"#%s\" />\n", (35.76 + xoffset) * scaler, (35.60 + yoffset) * scaler, 8.97 * scaler, symbol->bgcolour);
		fprintf(fsvg, "      <circle cx=\"%.2f\" cy=\"%.2f\" r=\"%.2f\" fill=\"#%s\" />\n", (35.76 + xoffset) * scaler, (35.60 + yoffset) * scaler, 7.10 * scaler, symbol->fgcolour);
		fprintf(fsvg, "      <circle cx=\"%.2f\" cy=\"%.2f\" r=\"%.2f\" fill=\"#%s\" />\n", (35.76 + xoffset) * scaler, (35.60 + yoffset) * scaler, 5.22 * scaler, symbol->bgcolour);
		fprintf(fsvg, "      <circle cx=\"%.2f\" cy=\"%.2f\" r=\"%.2f\" fill=\"#%s\" />\n", (35.76 + xoffset) * scaler, (35.60 + yoffset) * scaler, 3.31 * scaler, symbol->fgcolour);
		fprintf(fsvg, "      <circle cx=\"%.2f\" cy=\"%.2f\" r=\"%.2f\" fill=\"#%s\" />\n", (35.76 + xoffset) * scaler, (35.60 + yoffset) * scaler, 1.43 * scaler, symbol->bgcolour);
		for(r = 0; r < symbol->rows; r++) {
			for(i = 0; i < symbol->width; i++) {
				if(module_is_set(symbol, r, i)) {
					/* Dump a hexagon */
					my = r * 2.135 + 1.43;
					ay = my + 1.0 + yoffset;
					by = my + 0.5 + yoffset;
					cy = my - 0.5 + yoffset;
					dy = my - 1.0 + yoffset;
					ey = my - 0.5 + yoffset;
					fy = my + 0.5 + yoffset;
					if(r & 1) {
						mx = (2.46 * i) + 1.23 + 1.23;
					} else {
						mx = (2.46 * i) + 1.23;
					}
					ax = mx + xoffset;
					bx = mx + 0.86 + xoffset;
					cx = mx + 0.86 + xoffset;
					dx = mx + xoffset;
					ex = mx - 0.86 + xoffset;
					fx = mx - 0.86 + xoffset;
					fprintf(fsvg, "      <path d=\"M %.2f %.2f L %.2f %.2f L %.2f %.2f L %.2f %.2f L %.2f %.2f L %.2f %.2f Z\" />\n", ax * scaler, ay * scaler, bx * scaler, by * scaler, cx * scaler, cy * scaler, dx * scaler, dy * scaler, ex * scaler, ey * scaler, fx * scaler, fy * scaler);
				}
			}
		}
	}

	if(symbol->symbology != BARCODE_MAXICODE) {
		/* everything else uses rectangles (or squares) */
		/* Works from the bottom of the symbol up */
		int addon_latch = 0;

		for(r = 0; r < symbol->rows; r++) {
			this_row = r;
			if(symbol->row_height[this_row] == 0) {
				row_height = large_bar_height;
			} else {
				row_height = symbol->row_height[this_row];
			}
			row_posn = 0;
			for(i = 0; i < r; i++) {
				if(symbol->row_height[i] == 0) {
					row_posn += large_bar_height;
				} else {
					row_posn += symbol->row_height[i];
				}
			}
			row_posn += yoffset;

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
				if((addon_latch == 0) && (r == (symbol->rows - 1)) && (i > main_width)) {
					addon_text_posn = (row_posn + 8.0) * scaler;
					addon_latch = 1;
				}
				if(latch == 1) {
					/* a bar */
					if(addon_latch == 0) {
						fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", (i + xoffset) * scaler, row_posn * scaler, block_width * scaler, row_height * scaler);
					} else {
						fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", (i + xoffset) * scaler, (row_posn + 10.0) * scaler, block_width * scaler, (row_height - 5.0) * scaler);
					}
					latch = 0;
				} else {
					/* a space */
					latch = 1;
				}
				i += block_width;

			} while (i < symbol->width);
		}
	}
	/* That's done the actual data area, everything else is human-friendly */

	xoffset += comp_offset;
	row_posn = (row_posn + large_bar_height) * scaler;

	if(plot_text) {
		if ((((symbol->symbology == BARCODE_EANX) && (symbol->rows == 1)) || (symbol->symbology == BARCODE_EANX_CC)) ||
			(symbol->symbology == BARCODE_ISBNX)) {
			/* guard bar extensions and text formatting for EAN8 and EAN13 */
			switch(ustrlen(symbol->text)) {
				case 8: /* EAN-8 */
				case 11:
				case 14:
					fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", (0 + xoffset) * scaler, row_posn, scaler, 5.0 * scaler);
					fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", (2 + xoffset) * scaler, row_posn, scaler, 5.0 * scaler);
					fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", (32 + xoffset) * scaler, row_posn, scaler, 5.0 * scaler);
					fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", (34 + xoffset) * scaler, row_posn, scaler, 5.0 * scaler);
					fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", (64 + xoffset) * scaler, row_posn, scaler, 5.0 * scaler);
					fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", (66 + xoffset) * scaler, row_posn, scaler, 5.0 * scaler);
					for(i = 0; i < 4; i++) {
						textpart[i] = symbol->text[i];
					}
					textpart[4] = '\0';
					textpos = 17;
					fprintf(fsvg, "      <text x=\"%.2f\" y=\"%.2f\" text-anchor=\"middle\"\n", (textpos + xoffset) * scaler, default_text_posn);
					fprintf(fsvg, "         font-family=\"Helvetica\" font-size=\"%.1f\" fill=\"#%s\" >\n", 11.0 * scaler, symbol->fgcolour);
					fprintf(fsvg, "         %s\n", textpart);
					fprintf(fsvg, "      </text>\n");
					for(i = 0; i < 4; i++) {
						textpart[i] = symbol->text[i + 4];
					}
					textpart[4] = '\0';
					textpos = 50;
					fprintf(fsvg, "      <text x=\"%.2f\" y=\"%.2f\" text-anchor=\"middle\"\n", (textpos + xoffset) * scaler, default_text_posn);
					fprintf(fsvg, "         font-family=\"Helvetica\" font-size=\"%.1f\" fill=\"#%s\" >\n", 11.0 * scaler, symbol->fgcolour);
					fprintf(fsvg, "         %s\n", textpart);
					fprintf(fsvg, "      </text>\n");
					textdone = 1;
					switch(strlen(addon)) {
						case 2:
							textpos = xoffset + 86;
							fprintf(fsvg, "      <text x=\"%.2f\" y=\"%.2f\" text-anchor=\"middle\"\n", textpos * scaler, addon_text_posn * scaler);
							fprintf(fsvg, "         font-family=\"Helvetica\" font-size=\"%.1f\" fill=\"#%s\" >\n", 11.0 * scaler, symbol->fgcolour);
							fprintf(fsvg, "         %s\n", addon);
							fprintf(fsvg, "      </text>\n");
							break;
						case 5:
							textpos = xoffset + 100;
							fprintf(fsvg, "      <text x=\"%.2f\" y=\"%.2f\" text-anchor=\"middle\"\n", textpos * scaler, addon_text_posn * scaler);
							fprintf(fsvg, "         font-family=\"Helvetica\" font-size=\"%.1f\" fill=\"#%s\" >\n", 11.0 * scaler, symbol->fgcolour);
							fprintf(fsvg, "         %s\n", addon);
							fprintf(fsvg, "      </text>\n");
							break;
					}

					break;
				case 13: /* EAN 13 */
				case 16:
				case 19:
					fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", (0 + xoffset) * scaler, row_posn, scaler, 5.0 * scaler);
					fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", (2 + xoffset) * scaler, row_posn, scaler, 5.0 * scaler);
					fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", (46 + xoffset) * scaler, row_posn, scaler, 5.0 * scaler);
					fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", (48 + xoffset) * scaler, row_posn, scaler, 5.0 * scaler);
					fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", (92 + xoffset) * scaler, row_posn, scaler, 5.0 * scaler);
					fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", (94 + xoffset) * scaler, row_posn, scaler, 5.0 * scaler);
					textpart[0] = symbol->text[0];
					textpart[1] = '\0';
					textpos = -7;
					fprintf(fsvg, "      <text x=\"%.2f\" y=\"%.2f\" text-anchor=\"middle\"\n", (textpos + xoffset) * scaler, default_text_posn);
					fprintf(fsvg, "         font-family=\"Helvetica\" font-size=\"%.1f\" fill=\"#%s\" >\n", 11.0 * scaler, symbol->fgcolour);
					fprintf(fsvg, "         %s\n", textpart);
					fprintf(fsvg, "      </text>\n");
					for(i = 0; i < 6; i++) {
						textpart[i] = symbol->text[i + 1];
					}
					textpart[6] = '\0';
					textpos = 24;
					fprintf(fsvg, "      <text x=\"%.2f\" y=\"%.2f\" text-anchor=\"middle\"\n", (textpos + xoffset) * scaler, default_text_posn);
					fprintf(fsvg, "         font-family=\"Helvetica\" font-size=\"%.1f\" fill=\"#%s\" >\n", 11.0 * scaler, symbol->fgcolour);
					fprintf(fsvg, "         %s\n", textpart);
					fprintf(fsvg, "      </text>\n");
					for(i = 0; i < 6; i++) {
						textpart[i] = symbol->text[i + 7];
					}
					textpart[6] = '\0';
					textpos = 71;
					fprintf(fsvg, "      <text x=\"%.2f\" y=\"%.2f\" text-anchor=\"middle\"\n", (textpos + xoffset) * scaler, default_text_posn);
					fprintf(fsvg, "         font-family=\"Helvetica\" font-size=\"%.1f\" fill=\"#%s\" >\n", 11.0 * scaler, symbol->fgcolour);
					fprintf(fsvg, "         %s\n", textpart);
					fprintf(fsvg, "      </text>\n");
					textdone = 1;
					switch(strlen(addon)) {
						case 2:
							textpos = xoffset + 114;
							fprintf(fsvg, "      <text x=\"%.2f\" y=\"%.2f\" text-anchor=\"middle\"\n", textpos * scaler, addon_text_posn * scaler);
							fprintf(fsvg, "         font-family=\"Helvetica\" font-size=\"%.1f\" fill=\"#%s\" >\n", 11.0 * scaler, symbol->fgcolour);
							fprintf(fsvg, "         %s\n", addon);
							fprintf(fsvg, "      </text>\n");
							break;
						case 5:
							textpos = xoffset + 128;
							fprintf(fsvg, "      <text x=\"%.2f\" y=\"%.2f\" text-anchor=\"middle\"\n", textpos * scaler, addon_text_posn * scaler);
							fprintf(fsvg, "         font-family=\"Helvetica\" font-size=\"%.1f\" fill=\"#%s\" >\n", 11.0 * scaler, symbol->fgcolour);
							fprintf(fsvg, "         %s\n", addon);
							fprintf(fsvg, "      </text>\n");
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
					fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", (i + xoffset - comp_offset) * scaler, row_posn, block_width * scaler, 5.0 * scaler);
					latch = 0;
				} else {
					/* a space */
					latch = 1;
				}
				i += block_width;
			} while (i < 11 + comp_offset);
			fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", (46 + xoffset) * scaler, row_posn, scaler, 5.0 * scaler);
			fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", (48 + xoffset) * scaler, row_posn, scaler, 5.0 * scaler);
			latch = 1;
			i = 85 + comp_offset;
			do {
				block_width = 0;
				do {
					block_width++;
				} while (module_is_set(symbol, symbol->rows - 1, i + block_width) == module_is_set(symbol, symbol->rows - 1, i));
				if(latch == 1) {
					/* a bar */
					fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", (i + xoffset - comp_offset) * scaler, row_posn, block_width * scaler, 5.0 * scaler);
					latch = 0;
				} else {
					/* a space */
					latch = 1;
				}
				i += block_width;
			} while (i < 96 + comp_offset);
			textpart[0] = symbol->text[0];
			textpart[1] = '\0';
			textpos = -5;
			fprintf(fsvg, "      <text x=\"%.2f\" y=\"%.2f\" text-anchor=\"middle\"\n", (textpos + xoffset) * scaler, default_text_posn);
			fprintf(fsvg, "         font-family=\"Helvetica\" font-size=\"%.1f\" fill=\"#%s\" >\n", 8.0 * scaler, symbol->fgcolour);
			fprintf(fsvg, "         %s\n", textpart);
			fprintf(fsvg, "      </text>\n");
			for(i = 0; i < 5; i++) {
				textpart[i] = symbol->text[i + 1];
			}
			textpart[5] = '\0';
			textpos = 27;
			fprintf(fsvg, "      <text x=\"%.2f\" y=\"%.2f\" text-anchor=\"middle\"\n", (textpos + xoffset) * scaler, default_text_posn);
			fprintf(fsvg, "         font-family=\"Helvetica\" font-size=\"%.1f\" fill=\"#%s\" >\n", 11.0 * scaler, symbol->fgcolour);
			fprintf(fsvg, "         %s\n", textpart);
			fprintf(fsvg, "      </text>\n");
			for(i = 0; i < 5; i++) {
				textpart[i] = symbol->text[i + 6];
			}
			textpart[6] = '\0';
			textpos = 68;
			fprintf(fsvg, "      <text x=\"%.2f\" y=\"%.2f\" text-anchor=\"middle\"\n", (textpos + xoffset) * scaler, default_text_posn);
			fprintf(fsvg, "         font-family=\"Helvetica\" font-size=\"%.1f\" fill=\"#%s\" >\n", 11.0 * scaler, symbol->fgcolour);
			fprintf(fsvg, "         %s\n", textpart);
			fprintf(fsvg, "      </text>\n");
			textpart[0] = symbol->text[11];
			textpart[1] = '\0';
			textpos = 100;
			fprintf(fsvg, "      <text x=\"%.2f\" y=\"%.2f\" text-anchor=\"middle\"\n", (textpos + xoffset) * scaler, default_text_posn);
			fprintf(fsvg, "         font-family=\"Helvetica\" font-size=\"%.1f\" fill=\"#%s\" >\n", 8.0 * scaler, symbol->fgcolour);
			fprintf(fsvg, "         %s\n", textpart);
			fprintf(fsvg, "      </text>\n");
			textdone = 1;
			switch(strlen(addon)) {
				case 2:
					textpos = xoffset + 116;
					fprintf(fsvg, "      <text x=\"%.2f\" y=\"%.2f\" text-anchor=\"middle\"\n", textpos * scaler, addon_text_posn * scaler);
					fprintf(fsvg, "         font-family=\"Helvetica\" font-size=\"%.1f\" fill=\"#%s\" >\n", 11.0 * scaler, symbol->fgcolour);
					fprintf(fsvg, "         %s\n", addon);
					fprintf(fsvg, "      </text>\n");
					break;
				case 5:
					textpos = xoffset + 130;
					fprintf(fsvg, "      <text x=\"%.2f\" y=\"%.2f\" text-anchor=\"middle\"\n", textpos * scaler, addon_text_posn * scaler);
					fprintf(fsvg, "         font-family=\"Helvetica\" font-size=\"%.1f\" fill=\"#%s\" >\n", 11.0 * scaler, symbol->fgcolour);
					fprintf(fsvg, "         %s\n", addon);
					fprintf(fsvg, "      </text>\n");
					break;
			}

		}

		if (((symbol->symbology == BARCODE_UPCE) && (symbol->rows == 1)) || (symbol->symbology == BARCODE_UPCE_CC)) {
			/* guard bar extensions and text formatting for UPCE */
			fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", (0 + xoffset) * scaler, row_posn, scaler, 5.0 * scaler);
			fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", (2 + xoffset) * scaler, row_posn, scaler, 5.0 * scaler);
			fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", (46 + xoffset) * scaler, row_posn, scaler, 5.0 * scaler);
			fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", (48 + xoffset) * scaler, row_posn, scaler, 5.0 * scaler);
			fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", (50 + xoffset) * scaler, row_posn, scaler, 5.0 * scaler);
			textpart[0] = symbol->text[0];
			textpart[1] = '\0';
			textpos = -5;
			fprintf(fsvg, "      <text x=\"%.2f\" y=\"%.2f\" text-anchor=\"middle\"\n", (textpos + xoffset) * scaler, default_text_posn);
			fprintf(fsvg, "         font-family=\"Helvetica\" font-size=\"%.1f\" fill=\"#%s\" >\n", 8.0 * scaler, symbol->fgcolour);
			fprintf(fsvg, "         %s\n", textpart);
			fprintf(fsvg, "      </text>\n");
			for(i = 0; i < 6; i++) {
				textpart[i] = symbol->text[i + 1];
			}
			textpart[6] = '\0';
			textpos = 24;
			fprintf(fsvg, "      <text x=\"%.2f\" y=\"%.2f\" text-anchor=\"middle\"\n", (textpos + xoffset) * scaler, default_text_posn);
			fprintf(fsvg, "         font-family=\"Helvetica\" font-size=\"%.1f\" fill=\"#%s\" >\n", 11.0 * scaler, symbol->fgcolour);
			fprintf(fsvg, "         %s\n", textpart);
			fprintf(fsvg, "      </text>\n");
			textpart[0] = symbol->text[7];
			textpart[1] = '\0';
			textpos = 55;
			fprintf(fsvg, "      <text x=\"%.2f\" y=\"%.2f\" text-anchor=\"middle\"\n", (textpos + xoffset) * scaler, default_text_posn);
			fprintf(fsvg, "         font-family=\"Helvetica\" font-size=\"%.1f\" fill=\"#%s\" >\n", 8.0 * scaler, symbol->fgcolour);
			fprintf(fsvg, "         %s\n", textpart);
			fprintf(fsvg, "      </text>\n");
			textdone = 1;
			switch(strlen(addon)) {
				case 2:
					textpos = xoffset + 70;
					fprintf(fsvg, "      <text x=\"%.2f\" y=\"%.2f\" text-anchor=\"middle\"\n", textpos * scaler, addon_text_posn * scaler);
					fprintf(fsvg, "         font-family=\"Helvetica\" font-size=\"%.1f\" fill=\"#%s\" >\n", 11.0 * scaler, symbol->fgcolour);
					fprintf(fsvg, "         %s\n", addon);
					fprintf(fsvg, "      </text>\n");
					break;
				case 5:
					textpos = xoffset + 84;
					fprintf(fsvg, "      <text x=\"%.2f\" y=\"%.2f\" text-anchor=\"middle\"\n", textpos * scaler, addon_text_posn * scaler);
					fprintf(fsvg, "         font-family=\"Helvetica\" font-size=\"%.1f\" fill=\"#%s\" >\n", 11.0 * scaler, symbol->fgcolour);
					fprintf(fsvg, "         %s\n", addon);
					fprintf(fsvg, "      </text>\n");
					break;
			}

		}
	} /* if (plot_text) */

	xoffset -= comp_offset;

	switch(symbol->symbology) {
		case BARCODE_MAXICODE:
			/* Do nothing! (It's already been done) */
			break;
		default:
			if((symbol->output_options & BARCODE_BIND) != 0) {
				if((symbol->rows > 1) && (is_stackable(symbol->symbology) == 1)) {
					/* row binding */
					for(r = 1; r < symbol->rows; r++) {
						fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", xoffset * scaler, ((r * row_height) + yoffset - 1) * scaler, symbol->width * scaler, 2.0 * scaler);
					}
				}
			}
			if (((symbol->output_options & BARCODE_BOX) != 0) || ((symbol->output_options & BARCODE_BIND) != 0)) {
				fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", 0.0, 0.0, (symbol->width + xoffset + xoffset) * scaler, symbol->border_width * scaler);
				fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", 0.0, (symbol->height + symbol->border_width) * scaler, (symbol->width + xoffset + xoffset) * scaler, symbol->border_width * scaler);
			}
			if((symbol->output_options & BARCODE_BOX) != 0) {
				/* side bars */
				fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", 0.0, 0.0, symbol->border_width * scaler, (symbol->height + (2 * symbol->border_width)) * scaler);
				fprintf(fsvg, "      <rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" />\n", (symbol->width + xoffset + xoffset - symbol->border_width) * scaler, 0.0, symbol->border_width * scaler, (symbol->height + (2 * symbol->border_width)) * scaler);
			}
			break;
	}

	/* Put the human readable text at the bottom */
	if(plot_text && (textdone == 0)) {
		textpos = symbol->width / 2.0;
		fprintf(fsvg, "      <text x=\"%.2f\" y=\"%.2f\" text-anchor=\"middle\"\n", (textpos + xoffset) * scaler, default_text_posn);
		fprintf(fsvg, "         font-family=\"Helvetica\" font-size=\"%.1f\" fill=\"#%s\" >\n", 8.0 * scaler, symbol->fgcolour);
		fprintf(fsvg, "         %s\n", symbol->text);
		fprintf(fsvg, "      </text>\n");
	}
	fprintf(fsvg, "   </g>\n");
	fprintf(fsvg, "</svg>\n");

	if(symbol->output_options & BARCODE_STDOUT) {
		fflush(fsvg);
	} else {
		fclose(fsvg);
	}

	if (locale)
		setlocale(LC_ALL, locale);

	return error_number;
}

