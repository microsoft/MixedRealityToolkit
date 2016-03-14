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

#include <stdint.h>
#include <stdlib.h>
#include "image_f32.h"

image_f32_t *image_f32_create(int width, int height)
{
    image_f32_t *fim = (image_f32_t*) calloc(1, sizeof(image_f32_t));

    fim->width = width;
    fim->height = height;
    fim->stride = width; // XXX do better alignment

    fim->buf = calloc(fim->height * fim->stride, sizeof(float));

    return fim;
}

// scales by 1/255u
image_f32_t *image_f32_create_from_u8(image_u8_t *im)
{
    image_f32_t *fim = image_f32_create(im->width, im->height);

    for (int y = 0; y < fim->height; y++)
        for (int x = 0; x < fim->width; x++)
            fim->buf[y*fim->stride + x] = im->buf[y*im->stride + x] / 255.0f;

    return fim;
}

void image_f32_destroy(image_f32_t *im)
{
    free(im->buf);
    free(im);
}
