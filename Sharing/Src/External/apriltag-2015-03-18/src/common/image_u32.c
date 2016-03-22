/* (C) 2013-2015, The Regents of The University of Michigan
All rights reserved.

This software may be available under alternative licensing
terms. Contact Edwin Olson, ebolson@umich.edu, for more information.

   Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "pnm.h"
#include "image_u32.h"

// least common multiple of 64 (sandy bridge cache line) and 64 (stride
// needed for 16byte-wide RGBA processing)
#define DEFAULT_ALIGNMENT 64

image_u32_t *image_u32_create(int width, int height)
{
    return image_u32_create_alignment(width, height, DEFAULT_ALIGNMENT);
}

// alignment specified in units of uint32
image_u32_t *image_u32_create_alignment(int width, int height, int alignment)
{
    image_u32_t *im = (image_u32_t*) calloc(1, sizeof(image_u32_t));

    im->width  = width;
    im->height = height;
    im->stride = width;

    if ((im->stride % alignment) != 0)
        im->stride += alignment - (im->stride % alignment);

    im->buf = (uint32_t*) calloc(im->height*im->stride, sizeof(uint32_t));

    return im;
}

image_u32_t *image_u32_create_from_u8(image_u8_t *in)
{
    image_u32_t *out = image_u32_create(in->width, in->height);

    for (int y = 0; y < in->height; y++) {
        for (int x = 0; x < in->width; x++) {
            uint32_t v = in->buf[in->stride*y+x];
            v = (v<<16) | (v<<8) | v;
            out->buf[out->stride*y+x] = v;
        }
    }

    return out;
}


image_u32_t *image_u32_copy(const image_u32_t *in)
{
    image_u32_t *copy = calloc(1, sizeof(image_u32_t));

    copy->width  = in->width;
    copy->height = in->height;
    copy->stride = in->stride;

    copy->buf = malloc(in->height*in->stride*sizeof(uint32_t));
    memcpy(copy->buf, in->buf, in->height*in->stride*sizeof(uint32_t));

    return copy;
}

void image_u32_destroy(image_u32_t *im)
{
    if (im == NULL)
        return;

    free(im->buf);
    free(im);
}

////////////////////////////////////////////////////////////
// PNM file i/o

// Create an RGBA image from PNM
image_u32_t *image_u32_create_from_pnm(const char *path)
{
    pnm_t *pnm = pnm_create_from_file(path);
    if (pnm == NULL)
        return NULL;

    image_u32_t *im = NULL;

    switch (pnm->format) {
        case PNM_FORMAT_GRAY: {
            im = image_u32_create(pnm->width, pnm->height);

            for (int y = 0; y < im->height; y++) {
                for (int x = 0; x < im->width; x++) {
                    uint8_t gray = pnm->buf[y*im->width + x];
                    im->buf[y*im->stride + x] = 0xff000000 | (gray << 16) | (gray << 8) | (gray << 0);
                }
            }

            break;
        }

        case PNM_FORMAT_RGB: {
            im = image_u32_create(pnm->width, pnm->height);

            // Gray conversion for RGB is gray = (r + g + g + b)/4
            for (int y = 0; y < im->height; y++) {
                for (int x = 0; x < im->width; x++) {
                    uint8_t a = 0xff;
                    uint8_t r = pnm->buf[y*im->width*3 + 3*x];
                    uint8_t g = pnm->buf[y*im->width*3 + 3*x+1];
                    uint8_t b = pnm->buf[y*im->width*3 + 3*x+2];

                    im->buf[y*im->stride + x] = (a & 0xff) << 24 | (b & 0xff) << 16 | (g & 0xff) << 8 | (r & 0xff);
                }
            }

            break;
        }
    }

    pnm_destroy(pnm);
    return im;
}

int image_u32_write_pnm(const image_u32_t *im, const char *path)
{
    FILE *f = fopen(path, "wb");
    int res = 0;

    if (f == NULL) {
        res = -1;
        goto finish;
    }

    // Only outputs to RGB
    fprintf(f, "P6\n%d %d\n255\n", im->width, im->height);

    for (int y = 0; y < im->height; y++) {
        for (int x = 0; x < im->width; x++) {
            uint32_t abgr = im->buf[y*im->stride + x];
            uint8_t r = (uint8_t)((abgr >> 0) & 0xff);
            uint8_t g = (uint8_t)((abgr >> 8) & 0xff);
            uint8_t b = (uint8_t)((abgr >> 16) & 0xff);

            fwrite(&r, 1, 1, f);
            fwrite(&g, 1, 1, f);
            fwrite(&b, 1, 1, f);
        }
    }

finish:
    if (f != NULL)
        fclose(f);

    return res;
}

// only widths 1 and 3 supported
void image_u32_draw_line(image_u32_t *im, float x0, float y0, float x1, float y1, uint32_t v, int width)
{
    double dist = sqrtf((y1-y0)*(y1-y0) + (x1-x0)*(x1-x0));
    double delta = 0.5 / dist;

    // terrible line drawing code
    for (float f = 0; f <= 1; f += delta) {
        int x = ((int) (x0*f + x1*(1-f)));
        int y = ((int) (y0*f + y1*(1-f)));

        if (x < 0 || y < 0 || x+1 >= im->width || y+1 >= im->height)
            continue;

        int idx = y*im->stride + x;
        im->buf[idx] = v;
        if (width > 1) {
            im->buf[idx+1] = v;
            im->buf[idx+im->stride] = v;
            im->buf[idx+1+im->stride] = v;
        }
    }
}

void image_u32_draw_circle(image_u32_t *im, float x0, float y0, float r, uint32_t v)
{
    r = r*r;

    for (int y = y0-r; y <= y0+r; y++) {
        for (int x = x0-r; x <= x0+r; x++) {
            float d = (x-x0)*(x-x0) + (y-y0)*(y-y0);
            if (d > r)
                continue;

            if (x >= 0 && x < im->width && y >= 0 && y < im->height) {
                int idx = y*im->stride + x;
                im->buf[idx] = v;
            }
        }
    }
}

// for each pixel, take the green channel, scale it, then create a gray color from it.
void image_u32_scale_gray(image_u32_t *im, float scale)
{
    for (int y = 0; y < im->height; y++) {
        for (int x = 0; x < im->width; x++) {
            uint32_t v = im->buf[y*im->stride + x];
            uint8_t g = (v>>0) & 0xff;
            g *= scale;
            im->buf[y*im->stride + x] = 0x00 | (g<<16) | (g<<8) | (g<<0);
        }
    }
}
