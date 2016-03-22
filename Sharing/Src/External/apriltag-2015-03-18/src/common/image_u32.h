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

#ifndef _IMAGE_U32_H
#define _IMAGE_U32_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct image_u32 image_u32_t;
struct image_u32
{
    int width, height;
    int stride;

    uint32_t *buf;
};

#include "image_u8.h"

/////////////////////////////////////
// IMPORTANT NOTE ON BYTE ORDER
//
// This format is designed to make dealing with 4 color channel images
// efficient by loading an entire pixel into a single uint32_t. This
// raises a number of endian-ness and format issues.
//
// The SUPPORTED format is byte-ordered: R, G, B, A. Whether the R
// channel ends up mapped to high-order bits or low-order bits depends
// on the endianness of your platform..
//
// On little-endian machines (x86, ARM), this will look like:
//
//     uint32_t v = (a<<24) + (b<<16) + (g<<8) + (r<<0)
//
// On big-endian machines, this will look like:
//
//     uint32_t v = (r<<24) + (g<<16) + (b<<8) + (a<<0)
//
// Obviously, you can do whatever you want, but if you don't adhere
// to this convention, you may find your color channels oddly swapped around
// if you convert between formats.
//
// Since most platforms are little endian, you could simply assume
// little-endian ordering and add:
//
// #ifdef __ORDER_BIG_ENDIAN
//   #error big endian not supported
// #endif

/////////////////////////////////////

// Create or load an image. returns NULL on failure
image_u32_t *image_u32_create(int width, int height);
image_u32_t *image_u32_create_alignment(int width, int height, int alignment);
image_u32_t *image_u32_create_from_pnm(const char *path);
    image_u32_t *image_u32_create_from_u8(image_u8_t *in);

image_u32_t *image_u32_copy(const image_u32_t *in);

void image_u32_destroy(image_u32_t *im);

// Write a pnm. Returns 0 on success
// Currently only supports GRAY and ABGR. Does not write out alpha for ABGR
int image_u32_write_pnm(const image_u32_t *im, const char *path);

// only widths 1 and 3 supported
    void image_u32_draw_line(image_u32_t *im, float x0, float y0, float x1, float y1, uint32_t v, int width);

    void image_u32_draw_circle(image_u32_t *im, float x0, float y0, float r, uint32_t v);

// for each pixel, take the green channel, scale it, then create a gray color from it.
    void image_u32_scale_gray(image_u32_t *im, float scale);

#ifdef __cplusplus
}
#endif

#endif
