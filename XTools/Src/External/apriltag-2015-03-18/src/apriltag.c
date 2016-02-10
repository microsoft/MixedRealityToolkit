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

#include "apriltag.h"

#include <math.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
/// MICROSOFT PROJECT B CHANGES BEGIN
#ifndef _MSC_VER
#include <sys/time.h>
#endif
/// MICROSOFT PROJECT B CHANGES END

#include "common/image_u8.h"
#include "common/image_u32.h"
#include "common/zhash.h"
#include "common/zarray.h"
#include "common/matd.h"
#include "common/homography.h"
#include "common/timeprofile.h"
#include "common/math_util.h"
#include "g2d.h"

#include "common/postscript_utils.h"

/// MICROSOFT PROJECT B CHANGES BEGIN
#include "common/stackalloc.h"
/// MICROSOFT PROJECT B CHANGES END

#ifndef M_PI
# define M_PI 3.141592653589793238462643383279502884196
#endif

extern zarray_t *apriltag_quad_gradient(apriltag_detector_t *td, image_u8_t *im);
extern zarray_t *apriltag_quad_thresh(apriltag_detector_t *td, image_u8_t *im);

struct quick_decode_entry
{
    uint64_t rcode;   // the queried code
    uint16_t id;      // the tag ID (a small integer)
    uint8_t hamming;  // how many errors corrected?
    uint8_t rotation; // number of rotations [0, 3]
};

struct quick_decode
{
    int nentries;
    struct quick_decode_entry *entries;
};

/** if the bits in w were arranged in a d*d grid and that grid was
 * rotated, what would the new bits in w be?
 * The bits are organized like this (for d = 3):
 *
 *  8 7 6       2 5 8      0 1 2
 *  5 4 3  ==>  1 4 7 ==>  3 4 5    (rotate90 applied twice)
 *  2 1 0       0 3 6      6 7 8
 **/
static uint64_t rotate90(uint64_t w, uint32_t d)
{
    uint64_t wr = 0;

    for (int32_t r = d-1; r >=0; r--) {
        for (int32_t c = 0; c < d; c++) {
            int32_t b = r + d*c;

            wr = wr << 1;

            if ((w & (((uint64_t) 1) << b))!=0)
                wr |= 1;
        }
    }

    return wr;
}

void quad_destroy(struct quad *quad)
{
    if (!quad)
        return;

    matd_destroy(quad->H);
    matd_destroy(quad->Hinv);
    free(quad);
}

struct quad *quad_copy(struct quad *quad)
{
    struct quad *q = calloc(1, sizeof(struct quad));
    memcpy(q, quad, sizeof(struct quad));
    if (quad->H)
        q->H = matd_copy(quad->H);
    if (quad->Hinv)
        q->Hinv = matd_copy(quad->Hinv);
    return q;
}

void quick_decode_add(struct quick_decode *qd, uint64_t code, int id, int hamming)
{
    uint32_t bucket = code % qd->nentries;

    while (qd->entries[bucket].rcode != UINT64_MAX) {
        bucket = (bucket + 1) % qd->nentries;
    }

    qd->entries[bucket].rcode = code;
    qd->entries[bucket].id = id;
    qd->entries[bucket].hamming = hamming;
}

void quick_decode_uninit(apriltag_family_t *fam)
{
    if (!fam->impl)
        return;

    struct quick_decode *qd = (struct quick_decode*) fam->impl;
    free(qd->entries);
    free(qd);
    fam->impl = NULL;
}

void quick_decode_init(apriltag_family_t *family, int maxhamming)
{
    assert(family->impl == NULL);
    assert(family->ncodes < 65535);

    struct quick_decode *qd = calloc(1, sizeof(struct quick_decode));
    int capacity = family->ncodes;

    int nbits = family->d * family->d;

    if (maxhamming >= 1)
        capacity += family->ncodes * nbits;

    if (maxhamming >= 2)
        capacity += family->ncodes * nbits * (nbits-1);

    if (maxhamming >= 3)
        capacity += family->ncodes * nbits * (nbits-1) * (nbits-2);

    qd->nentries = capacity * 3;

//    printf("capacity %d, size: %.0f kB\n",
//           capacity, qd->nentries * sizeof(struct quick_decode_entry) / 1024.0);

    qd->entries = calloc(qd->nentries, sizeof(struct quick_decode_entry));
    if (qd->entries == NULL) {
        printf("apriltag.c: failed to allocate hamming decode table. Reduce max hamming size.\n");
        exit(-1);
    }

    for (int i = 0; i < qd->nentries; i++)
        qd->entries[i].rcode = UINT64_MAX;

    for (int i = 0; i < family->ncodes; i++) {
        uint64_t code = family->codes[i];

        // add exact code (hamming = 0)
        quick_decode_add(qd, code, i, 0);

        if (maxhamming >= 1) {
            // add hamming 1
            for (int j = 0; j < nbits; j++)
                quick_decode_add(qd, code ^ (1L << j), i, 1);
        }

        if (maxhamming >= 2) {
            // add hamming 2
            for (int j = 0; j < nbits; j++)
                for (int k = 0; k < j; k++)
                    quick_decode_add(qd, code ^ (1L << j) ^ (1L << k), i, 2);
        }

        if (maxhamming >= 3) {
            // add hamming 3
            for (int j = 0; j < nbits; j++)
                for (int k = 0; k < j; k++)
                    for (int m = 0; m < k; m++)
                        quick_decode_add(qd, code ^ (1L << j) ^ (1L << k) ^ (1L << m), i, 3);
        }

        if (maxhamming > 3) {
            printf("apriltag.c: maxhamming beyond 3 not supported\n");
        }
    }

    family->impl = qd;

    if (0) {
        int longest_run = 0;
        int run = 0;
        int run_sum = 0;
        int run_count = 0;

        // This accounting code doesn't check the last possible run that
        // occurs at the wrap-around. That's pretty insignificant.
        for (int i = 0; i < qd->nentries; i++) {
            if (qd->entries[i].rcode == UINT64_MAX) {
                if (run > 0) {
                    run_sum += run;
                    run_count ++;
                }
                run = 0;
            } else {
                run ++;
                longest_run = imax(longest_run, run);
            }
        }

        printf("quick decode: longest run: %d, average run %.3f\n", longest_run, 1.0 * run_sum / run_count);
    }
}

// returns an entry with hamming set to 255 if no decode was found.
static void quick_decode_codeword(apriltag_family_t *tf, uint64_t rcode,
                                  struct quick_decode_entry *entry)
{
    struct quick_decode *qd = (struct quick_decode*) tf->impl;

    for (int ridx = 0; ridx < 4; ridx++) {

        for (int bucket = rcode % qd->nentries;
             qd->entries[bucket].rcode != UINT64_MAX;
             bucket = (bucket + 1) % qd->nentries) {

            if (qd->entries[bucket].rcode == rcode) {
                *entry = qd->entries[bucket];
                entry->rotation = ridx;
                return;
            }
        }

        rcode = rotate90(rcode, tf->d);
    }

    entry->rcode = 0;
    entry->id = 65535;
    entry->hamming = 255;
    entry->rotation = 0;
}

static inline int detection_compare_function(const void *_a, const void *_b)
{
    apriltag_detection_t *a = *(apriltag_detection_t**) _a;
    apriltag_detection_t *b = *(apriltag_detection_t**) _b;

    return a->id - b->id;
}

static uint32_t rgb_scale(uint32_t rgb, float a)
{
    int r = (rgb >> 16)&0xff;
    int g = (rgb >> 8)&0xff;
    int b = (rgb >> 0)&0xff;

    r *= a;
    g *= a;
    b *= a;

    return (r<<16) | (g<<8) | b;
}

void apriltag_detector_remove_family(apriltag_detector_t *td, apriltag_family_t *fam)
{
    quick_decode_uninit(fam);
    zarray_remove_value(td->tag_families, &fam, 0);
}

void apriltag_detector_add_family(apriltag_detector_t *td, apriltag_family_t *fam)
{
    zarray_add(td->tag_families, &fam);

    // XXX Tunable, but really, 2 is a good choice. Values of >=3
    // consume prohibitively large amounts of memory, and otherwise
    // you want the largest value possible.
    if (!fam->impl)
        quick_decode_init(fam, 2);
}

void apriltag_detector_clear_families(apriltag_detector_t *td)
{
    for (int i = 0; i < zarray_size(td->tag_families); i++) {
        apriltag_family_t *fam;
        zarray_get(td->tag_families, i, &fam);
        quick_decode_uninit(fam);
    }
    zarray_clear(td->tag_families);
}

apriltag_detector_t *apriltag_detector_create()
{
    apriltag_detector_t *td = (apriltag_detector_t*) calloc(1, sizeof(apriltag_detector_t));

    td->nthreads = 1;

    td->qtp.max_nmaxima = 10;
    td->qtp.min_cluster_pixels = 5;

    td->qtp.max_line_fit_mse = 1.0;
    td->qtp.critical_rad = 10 * M_PI / 180;
    td->qtp.deglitch = 0;
    td->qtp.min_white_black_diff = 15;

    td->tag_families = zarray_create(sizeof(apriltag_family_t*));

    pthread_mutex_init(&td->mutex, NULL);

    td->tp = timeprofile_create();

    td->refine_edges = 1;
    td->refine_pose = 0;
    td->refine_decode = 0;

    td->debug = 0;

    // NB: defer initialization of td->wp so that the user can
    // override td->nthreads.

    return td;
}

void apriltag_detector_destroy(apriltag_detector_t *td)
{
    timeprofile_destroy(td->tp);
    workerpool_destroy(td->wp);

    apriltag_detector_clear_families(td);

    zarray_destroy(td->tag_families);
    free(td);
}

struct quad_decode_task
{
    int i0, i1;
    zarray_t *quads;
    apriltag_detector_t *td;

    image_u8_t *im;
    zarray_t *detections;

    image_u8_t *im_gray_samples;
    image_u8_t *im_decision;
};

struct evaluate_quad_ret
{
    int64_t rcode;
    double  score;
    matd_t  *H, *Hinv;

    int decode_status;
    struct quick_decode_entry e;
};

void quad_update_homographies(struct quad *quad)
{
    zarray_t *correspondences = zarray_create(sizeof(float[4]));

    for (int i = 0; i < 4; i++) {
        float corr[4];

        corr[0] = (i==0 || i==3) ? -1 : 1;
        corr[1] = (i==0 || i==1) ? -1 : 1;
        corr[2] = quad->p[i][0];
        corr[3] = quad->p[i][1];

        zarray_add(correspondences, &corr);
    }

    if (quad->H)
        matd_destroy(quad->H);
    if (quad->Hinv)
        matd_destroy(quad->Hinv);

    // XXX Tunable
    quad->H = homography_compute(correspondences, HOMOGRAPHY_COMPUTE_FLAG_SVD);
    quad->Hinv = matd_inverse(quad->H);

    zarray_destroy(correspondences);
}

// compute a "score" for a quad that is independent of tag family
// encoding (but dependent upon the tag geometry) by considering the
// contrast around the exterior of the tag.
double quad_goodness(apriltag_family_t *family, image_u8_t *im, struct quad *quad)
{
    // when sampling from the white border, how much white border do
    // we actually consider valid, measured in bit-cell units? (the
    // outside portions are often intruded upon, so it could be advantageous to use
    // less than the "nominal" 1.0. (Less than 1.0 not well tested.)

    // XXX Tunable
    float white_border = 1;

    // in tag coordinates, how big is each bit cell?
    double bit_size = 2.0 / (2*family->black_border + family->d);
//    double inv_bit_size = 1.0 / bit_size;

    int32_t xmin = INT32_MAX, xmax = 0, ymin = INT32_MAX, ymax = 0;

    for (int i = 0; i < 4; i++) {
        double tx = (i == 0 || i == 3) ? -1 - bit_size : 1 + bit_size;
        double ty = (i == 0 || i == 1) ? -1 - bit_size : 1 + bit_size;
        double x, y;

        homography_project(quad->H, tx, ty, &x, &y);
        xmin = imin(xmin, x);
        xmax = imax(xmax, x);
        ymin = imin(ymin, y);
        ymax = imax(ymax, y);
    }

    // clamp bounding box to image dimensions
    xmin = imax(0, xmin);
    xmax = imin(im->width-1, xmax);
    ymin = imax(0, ymin);
    ymax = imin(im->height-1, ymax);

//    int nbits = family->d * family->d;

    int64_t W1 = 0, B1 = 0, Wn = 0, Bn = 0;

    float wsz = bit_size*white_border;
    float bsz = bit_size*family->black_border;

    matd_t *Hinv = quad->Hinv;
//    matd_t *H = quad->H;

    // iterate over all the pixels in the tag. (Iterating in pixel space)
    for (int y = ymin; y <= ymax; y++) {

        // we'll incrementally compute the homography
        // projections. Begin by evaluating the homogeneous position
        // [(xmin - .5f), y, 1]. Then, we'll update as we stride in
        // the +x direction.
        double Hx = MATD_EL(Hinv, 0, 0) * (.5 + (int) xmin) +
            MATD_EL(Hinv, 0, 1) * (y + .5) + MATD_EL(Hinv, 0, 2);
        double Hy = MATD_EL(Hinv, 1, 0) * (.5 + (int) xmin) +
            MATD_EL(Hinv, 1, 1) * (y + .5) + MATD_EL(Hinv, 1, 2);
        double Hh = MATD_EL(Hinv, 2, 0) * (.5 + (int) xmin) +
            MATD_EL(Hinv, 2, 1) * (y + .5) + MATD_EL(Hinv, 2, 2);

        for (int x = xmin; x <= xmax;  x++) {
            // project the pixel center.
            double tx, ty;

            // divide by homogeneous coordinate
            tx = Hx / Hh;
            ty = Hy / Hh;

            // if we move x one pixel to the right, here's what
            // happens to our three pre-normalized coordinates.
            Hx += MATD_EL(Hinv, 0, 0);
            Hy += MATD_EL(Hinv, 1, 0);
            Hh += MATD_EL(Hinv, 2, 0);

            float txa = fabsf((float) tx), tya = fabsf((float) ty);
            float xymax = fmaxf(txa, tya);

//            if (txa >= 1 + wsz || tya >= 1 + wsz)
            if (xymax >= 1 + wsz)
                continue;

            uint8_t v = im->buf[y*im->stride + x];

            // it's within the white border?
//            if (txa >= 1 || tya >= 1) {
            if (xymax >= 1) {
                W1 += v;
                Wn ++;
                continue;
            }

            // it's within the black border?
//            if (txa >= 1 - bsz || tya >= 1 - bsz) {
            if (xymax >= 1 - bsz) {
                B1 += v;
                Bn ++;
                continue;
            }

            // it must be a data bit. We don't do anything with these.
            continue;
        }
    }

    // score = average margin between white and black pixels near border.
    return 1.0 * W1 / Wn - 1.0 * B1 / Bn;
}

// returns the decision margin.
float quad_decode(apriltag_family_t *family, image_u8_t *im, struct quad *quad, struct quick_decode_entry *entry)
{
    // decode the tag binary contents by sampling the pixel
    // closest to the center of each bit cell.

    int64_t rcode = 0;

    // how wide do we assume the white border is?
    float white_border = 1.0;

    // We will compute a threshold by sampling known white/black cells around this tag.
    // This sampling is achieved by considering a set of samples along lines.
    //
    // coordinates are given in bit coordinates. ([0, fam->d]).
    //
    // { initial x, initial y, delta x, delta y, WHITE=1 }
    float patterns[] = {
        // left white column
        0 - white_border / 2.0, 0.5,
        0, 1,
        1,

        // left black column
        0 + family->black_border / 2.0, 0.5,
        0, 1,
        0,

        // right white column
        2*family->black_border + family->d + white_border / 2.0, .5,
        0, 1,
        1,

        // right black column
        2*family->black_border + family->d - family->black_border / 2.0, .5,
        0, 1,
        0,

        // top white row
        0.5, -white_border / 2.0,
        1, 0,
        1,

        // top black row
        0.5, family->black_border / 2.0,
        1, 0,
        1,

        // bottom white row
        0.5, 2*family->black_border + family->d + white_border / 2.0,
        1, 0,
        1,

        // bottom black row
        0.5, 2*family->black_border + family->d - family->black_border / 2.0,
        1, 0,
        0

        // XXX double-counts the corners.
    };

    float sums[2] = { 0, 0 };
    float counts[2] = { 0, 0 };

    for (int pattern_idx = 0; pattern_idx < sizeof(patterns)/(5*sizeof(float)); pattern_idx ++) {
        float *pattern = &patterns[pattern_idx * 5];

        int sumidx = pattern[4];

        for (int i = 0; i < 2*family->black_border + family->d; i++) {
            double tagx01 = (pattern[0] + i*pattern[2]) / (2*family->black_border + family->d);
            double tagy01 = (pattern[1] + i*pattern[3]) / (2*family->black_border + family->d);

            double tagx = 2*(tagx01-0.5);
            double tagy = 2*(tagy01-0.5);

            double px, py;
            homography_project(quad->H, tagx, tagy, &px, &py);

            // don't round
            int ix = px;
            int iy = py;
            if (ix < 0 || iy < 0 || ix >= im->width || iy >= im->height)
                continue;

            int v = im->buf[iy*im->stride + ix];

            sums[sumidx] += v;
            counts[sumidx] ++;
        }
    }

    float thresh = ((sums[0] / counts[0]) + (sums[1] / counts[1])) / 2.0;

    // compute the average decision margin (how far was each bit from
    // the decision boundary?
    float score = 0;
    float score_count = 0;

    for (int bitidx = 0; bitidx < family->d * family->d; bitidx++) {
        int bitx = bitidx % family->d;
        int bity = bitidx / family->d;

        double tagx01 = (family->black_border + bitx + 0.5) / (2*family->black_border + family->d);
        double tagy01 = (family->black_border + bity + 0.5) / (2*family->black_border + family->d);

        // scale to [-1, 1]
        double tagx = 2*(tagx01-0.5);
        double tagy = 2*(tagy01-0.5);

        double px, py;
        homography_project(quad->H, tagx, tagy, &px, &py);

        rcode = (rcode << 1);

        // don't round.
        int ix = px;
        int iy = py;

        if (ix < 0 || iy < 0 || ix >= im->width || iy >= im->height)
            continue;

        int v = im->buf[iy*im->stride + ix];

        if (v > thresh) {
            score += (v - thresh);
            score_count ++;
            rcode |= 1;
        } else {
            score += (thresh - v);
            score_count ++;
        }
    }

    quick_decode_codeword(family, rcode, entry);
    return score / score_count;
}

double score_goodness(apriltag_family_t *family, image_u8_t *im, struct quad *quad, void *user)
{
    return quad_goodness(family, im, quad);
}

double score_decodability(apriltag_family_t *family, image_u8_t *im, struct quad *quad, void *user)
{
    struct quick_decode_entry entry;

    float decision_margin = quad_decode(family, im, quad, &entry);

    // hamming trumps decision margin; maximum value for decision_margin is 255.
    return decision_margin - entry.hamming*1000;
}

// returns score of best quad
double optimize_quad_generic(apriltag_family_t *family, image_u8_t *im, struct quad *quad0,
                             float *stepsizes, int nstepsizes,
                             double (*score)(apriltag_family_t *family, image_u8_t *im, struct quad *quad, void *user),
                             void *user)
{
    struct quad *best_quad = quad_copy(quad0);
    double best_score = score(family, im, best_quad, user);

    for (int stepsize_idx = 0; stepsize_idx < nstepsizes; stepsize_idx++)  {

        int improved = 1;

        // when we make progress with a particular step size, how many
        // times will we try to perform that same step size again?
        // (max_repeat = 0 means ("don't repeat--- just move to the
        // next step size").
        // XXX Tunable
        int max_repeat = 1;

        for (int repeat = 0; repeat <= max_repeat && improved; repeat++) {

            improved = 0;

            // wiggle point i
            for (int i = 0; i < 4; i++) {

                float stepsize = stepsizes[stepsize_idx];

                // XXX Tunable (really 1 makes the best sense since)
                int nsteps = 1;

                struct quad *this_best_quad = NULL;
                double this_best_score = best_score;

                for (int sx = -nsteps; sx <= nsteps; sx++) {
                    for (int sy = -nsteps; sy <= nsteps; sy++) {
                        if (sx==0 && sy==0)
                            continue;

                        struct quad *this_quad = quad_copy(best_quad);
                        this_quad->p[i][0] = best_quad->p[i][0] + sx*stepsize;
                        this_quad->p[i][1] = best_quad->p[i][1] + sy*stepsize;
                        quad_update_homographies(this_quad);

                        double this_score = score(family, im, this_quad, user);

                        if (this_score > this_best_score) {
                            quad_destroy(this_best_quad);

                            this_best_quad = this_quad;
                            this_best_score = this_score;
                        } else {
                            quad_destroy(this_quad);
                        }
                    }
                }

                if (this_best_score > best_score) {
                    quad_destroy(best_quad);
                    best_quad = this_best_quad;
                    best_score = this_best_score;
                    improved = 1;
                }
            }
        }
    }

    matd_destroy(quad0->H);
    matd_destroy(quad0->Hinv);
    memcpy(quad0, best_quad, sizeof(struct quad)); // copy pointers
    free(best_quad);
    return best_score;
}

static void refine_edges(apriltag_detector_t *td, image_u8_t *im_orig, struct quad *quad)
{
    double lines[4][4]; // for each line, [Ex Ey nx ny]

    for (int edge = 0; edge < 4; edge++) {
        int a = edge, b = (edge + 1) & 3; // indices of the end points.

        // compute the normal to the current line estimate
        double nx = quad->p[b][1] - quad->p[a][1];
        double ny = -quad->p[b][0] + quad->p[a][0];
        double mag = sqrt(nx*nx + ny*ny);
        nx /= mag;
        ny /= mag;

        // we will now fit a NEW line by sampling points near
        // our original line that have large gradients. On really big tags,
        // we're willing to sample more to get an even better estimate.
        int nsamples = imax(16, mag / 8); // XXX tunable

        // stats for fitting a line...
        double Mx = 0, My = 0, Mxx = 0, Mxy = 0, Myy = 0, N = 0;

        for (int s = 0; s < nsamples; s++) {
            // compute a point along the line... Note, we're avoiding
            // sampling *right* at the corners, since those points are
            // the least reliable.
            double alpha = (1.0 + s) / (nsamples + 1);
            double x0 = alpha*quad->p[a][0] + (1-alpha)*quad->p[b][0];
            double y0 = alpha*quad->p[a][1] + (1-alpha)*quad->p[b][1];

            // search along the normal to this line, looking at the
            // gradients along the way. We're looking for a strong
            // response.
            double Mn = 0;
            double Mcount = 0;

            // XXX tunable: how far to search?  We want to search far
            // enough that we find the best edge, but not so far that
            // we hit other edges that aren't part of the tag. We
            // shouldn't ever have to search more than quad_decimate,
            // since otherwise we would (ideally) have started our
            // search on another pixel in the first place. Likewise,
            // for very small tags, we don't want the range to be too
            // big.
            double range = fmin(td->quad_decimate + 1, mag / 10);

            // XXX tunable step size.
            for (double n = -range; n <= range; n +=  0.25) {
                // Because of the guaranteed winding order of the
                // points in the quad, we will start inside the white
                // portion of the quad and work our way outward.
                //
                // sample to points (x1,y1) and (x2,y2) XXX tunable:
                // how far +/- to look? Small values compute the
                // gradient more precisely, but are more sensitive to
                // noise.
                double grange = 1;
                int x1 = x0 + (n + grange)*nx;
                int y1 = y0 + (n + grange)*ny;
                if (x1 < 0 || x1 >= im_orig->width || y1 < 0 || y1 >= im_orig->height)
                    continue;

                int x2 = x0 + (n - grange)*nx;
                int y2 = y0 + (n - grange)*ny;
                if (x2 < 0 || x2 >= im_orig->width || y2 < 0 || y2 >= im_orig->height)
                    continue;

                int g1 = im_orig->buf[y1*im_orig->stride + x1];
                int g2 = im_orig->buf[y2*im_orig->stride + x2];

                if (g1 < g2) // reject points whose gradient is "backwards". They can only hurt us.
                    continue;

                double weight = (g2 - g1)*(g2 - g1); // XXX tunable. What shape for weight=f(g2-g1)?

                // compute weighted average of the gradient at this point.
                Mn += weight*n;
                Mcount += weight;
            }

            // what was the average point along the line?
            double n0 = Mn / Mcount;

            // where is the point along the line?
            double bestx = x0 + n0*nx;
            double besty = y0 + n0*ny;

            // update our line fit statistics
            Mx += bestx;
            My += besty;
            Mxx += bestx*bestx;
            Mxy += bestx*besty;
            Myy += besty*besty;
            N++;
        }

        // fit a line
        double Ex = Mx / N, Ey = My / N;
        double Cxx = Mxx / N - Ex*Ex;
        double Cxy = Mxy / N - Ex*Ey;
        double Cyy = Myy / N - Ey*Ey;

        double normal_theta = .5 * atan2f(-2*Cxy, (Cyy - Cxx));
        nx = cosf(normal_theta);
        ny = sinf(normal_theta);
        lines[edge][0] = Ex;
        lines[edge][1] = Ey;
        lines[edge][2] = nx;
        lines[edge][3] = ny;
    }

    // now refit the corners of the quad
    for (int i = 0; i < 4; i++) {
        // solve for the intersection of lines (i) and (i+1)&3.
        double A00 =  lines[i][3],  A01 = -lines[(i+1)&3][3];
        double A10 =  -lines[i][2],  A11 = lines[(i+1)&3][2];
        double B0 = -lines[i][0] + lines[(i+1)&3][0];
        double B1 = -lines[i][1] + lines[(i+1)&3][1];

        double det = A00 * A11 - A10 * A01;

        // inverse.
        double W00 = A11 / det, W01 = -A01 / det;
        if (fabs(det) > 0.001) {
            // solve
            double L0 = W00*B0 + W01*B1;

            // compute intersection
            quad->p[i][0] = lines[i][0] + L0*A00;
            quad->p[i][1] = lines[i][1] + L0*A10;
        }
    }
}

static void quad_decode_task(void *_u)
{
    struct quad_decode_task *task = (struct quad_decode_task*) _u;
    apriltag_detector_t *td = task->td;
    image_u8_t *im = task->im;

    for (int quadidx = task->i0; quadidx < task->i1; quadidx++) {
        struct quad *quad_original;
        zarray_get_volatile(task->quads, quadidx, &quad_original);

        // refine edges is not dependent upon the tag family, thus
        // apply this optimization BEFORE the other work.
        if (td->quad_decimate > 1 && td->refine_edges) {
            refine_edges(td, im, quad_original);
        }

        // make sure the homographies are computed...
        quad_update_homographies(quad_original);

        for (int famidx = 0; famidx < zarray_size(td->tag_families); famidx++) {
            apriltag_family_t *family;
            zarray_get(td->tag_families, famidx, &family);

            double goodness = 0;

            // since the geometry of tag families can vary, start any
            // optimization process over with the original quad.
            struct quad *quad = quad_copy(quad_original);

            // improve the quad corner positions by minimizing the
            // variance within each intra-bit area.
            if (td->refine_pose) {
                // NB: We potentially step an integer
                // number of times in each direction. To make each
                // sample as useful as possible, the step sizes should
                // not be integer multiples of each other. (I.e.,
                // probably don't use 1, 0.5, 0.25, etc.)

                // XXX Tunable
                float stepsizes[] = { 1, .4, .16, .064 };
                int nstepsizes = sizeof(stepsizes)/sizeof(float);

                goodness = optimize_quad_generic(family, im, quad, stepsizes, nstepsizes, score_goodness, NULL);
            }

            if (td->refine_decode) {
                // this optimizes decodability, but we don't report
                // that value to the user.  (so discard return value.)
                // XXX Tunable
                float stepsizes[] = { .4 };
                int nstepsizes = sizeof(stepsizes)/sizeof(float);

                optimize_quad_generic(family, im, quad, stepsizes, nstepsizes, score_decodability, NULL);
            }

            struct quick_decode_entry entry;

            float decision_margin = quad_decode(family, im, quad, &entry);
            if (entry.hamming < 255) {
                apriltag_detection_t *det = calloc(1, sizeof(apriltag_detection_t));

                det->family = family;
                det->id = entry.id;
                det->hamming = entry.hamming;
                det->goodness = goodness;
                det->decision_margin = decision_margin;

                double theta = -entry.rotation * M_PI / 2.0;
                double c = cos(theta), s = sin(theta);

                matd_t *R = matd_create(3,3);
                MATD_EL(R, 0, 0) = c;
                MATD_EL(R, 0, 1) = -s;
                MATD_EL(R, 1, 0) = s;
                MATD_EL(R, 1, 1) = c;
                MATD_EL(R, 2, 2) = 1;

                det->H = matd_op("M*M", quad->H, R);

                matd_destroy(R);

                homography_project(det->H, 0, 0, &det->c[0], &det->c[1]);

                // adjust the points in det->p so that they correspond to
                // counter-clockwise around the quad, starting at -1,-1.
                for (int i = 0; i < 4; i++) {
                    int tcx = (i == 0 || i == 3) ? -1 : 1;
                    int tcy = (i < 2) ? -1 : 1;

                    double p[2];

                    homography_project(det->H, tcx, tcy, &p[0], &p[1]);

                    det->p[i][0] = p[0];
                    det->p[i][1] = p[1];
                }

                pthread_mutex_lock(&td->mutex);
                zarray_add(task->detections, &det);
                pthread_mutex_unlock(&td->mutex);
            }

            quad_destroy(quad);
        }
    }
}

void apriltag_detection_destroy(apriltag_detection_t *det)
{
    if (det == NULL)
        return;

    matd_destroy(det->H);
    free(det);
}

zarray_t *apriltag_detector_detect(apriltag_detector_t *td, image_u8_t *im_orig)
{
    if (zarray_size(td->tag_families) == 0) {
        zarray_t *s = zarray_create(sizeof(apriltag_detection_t*));
        printf("apriltag.c: No tag families enabled.");
        return s;
    }

    if (td->wp == NULL || td->nthreads != workerpool_get_nthreads(td->wp)) {
        workerpool_destroy(td->wp);
        td->wp = workerpool_create(td->nthreads);
    }

    timeprofile_clear(td->tp);
    timeprofile_stamp(td->tp, "init");

    ///////////////////////////////////////////////////////////
    // Step 1. Detect quads according to requested image decimation
    // and blurring parameters.
    image_u8_t *quad_im = im_orig;
    if (td->quad_decimate > 1) {
        quad_im = image_u8_decimate(im_orig, td->quad_decimate);

        timeprofile_stamp(td->tp, "decimate");
    }

    if (td->quad_sigma != 0) {
        // compute a reasonable kernel width by figuring that the
        // kernel should go out 2 std devs.
        //
        // max sigma          ksz
        // 0.499              1  (disabled)
        // 0.999              3
        // 1.499              5
        // 1.999              7

        float sigma = fabsf((float) td->quad_sigma);

        int ksz = 4 * sigma; // 2 std devs in each direction
        if ((ksz & 1) == 0)
            ksz++;

        if (ksz > 1) {

            if (td->quad_sigma > 0) {
                // Apply a blur
                image_u8_gaussian_blur(quad_im, sigma, ksz);
            } else {
                // SHARPEN the image by subtracting the low frequency components.
                image_u8_t *orig = image_u8_copy(quad_im);
                image_u8_gaussian_blur(quad_im, sigma, ksz);

                for (int y = 0; y < orig->height; y++) {
                    for (int x = 0; x < orig->width; x++) {
                        int vorig = orig->buf[y*orig->stride + x];
                        int vblur = quad_im->buf[y*quad_im->stride + x];

                        int v = 2*vorig - vblur;
                        if (v < 0)
                            v = 0;
                        if (v > 255)
                            v = 255;

                        quad_im->buf[y*quad_im->stride + x] = (uint8_t) v;
                    }
                }
                image_u8_destroy(orig);
            }
        }
    }

    timeprofile_stamp(td->tp, "blur/sharp");

    if (td->debug)
        image_u8_write_pnm(quad_im, "debug_preprocess.pnm");

//    zarray_t *quads = apriltag_quad_gradient(td, im_orig);
    zarray_t *quads = apriltag_quad_thresh(td, quad_im);

    // adjust centers of pixels so that they correspond to the
    // original full-resolution image.
    if (td->quad_decimate > 1) {
        for (int i = 0; i < zarray_size(quads); i++) {
            struct quad *q;
            zarray_get_volatile(quads, i, &q);

            for (int i = 0; i < 4; i++) {
                q->p[i][0] *= td->quad_decimate;
                q->p[i][1] *= td->quad_decimate;
            }
        }
    }

    if (quad_im != im_orig)
        image_u8_destroy(quad_im);

    zarray_t *detections = zarray_create(sizeof(apriltag_detection_t*));

    td->nquads = zarray_size(quads);

    timeprofile_stamp(td->tp, "quads");

    if (td->debug) {
        image_u8_t *im_quads = image_u8_copy(im_orig);
        image_u8_darken(im_quads);
        image_u8_darken(im_quads);

        srandom(0);

        for (int i = 0; i < zarray_size(quads); i++) {
            struct quad *quad;
            zarray_get_volatile(quads, i, &quad);

            const int bias = 100;
            int color = bias + (random() % (255-bias));

            image_u8_draw_line(im_quads, quad->p[0][0], quad->p[0][1], quad->p[1][0], quad->p[1][1], color, 1);
            image_u8_draw_line(im_quads, quad->p[1][0], quad->p[1][1], quad->p[2][0], quad->p[2][1], color, 1);
            image_u8_draw_line(im_quads, quad->p[2][0], quad->p[2][1], quad->p[3][0], quad->p[3][1], color, 1);
            image_u8_draw_line(im_quads, quad->p[3][0], quad->p[3][1], quad->p[0][0], quad->p[0][1], color, 1);
        }

        image_u8_write_pnm(im_quads, "debug_quads_raw.pnm");
        image_u8_destroy(im_quads);
    }

    ////////////////////////////////////////////////////////////////
    // Step 2. Decode tags from each quad.
    if (1) {
        image_u8_t *im_gray_samples = td->debug ? image_u8_copy(im_orig) : NULL;

        // im_decision debugging output is slow.
        image_u8_t *im_decision = td->debug ? image_u8_copy(im_orig) : NULL;

        int chunksize = 1 + zarray_size(quads) / (APRILTAG_TASKS_PER_THREAD_TARGET * td->nthreads);

		/// MICROSOFT PROJECT B CHANGES BEGIN
		stackalloc(tasks, struct quad_decode_task, zarray_size(quads) / chunksize + 1);
		/// MICROSOFT PROJECT B CHANGES END

        int ntasks = 0;
        for (int i = 0; i < zarray_size(quads); i+= chunksize) {
            tasks[ntasks].i0 = i;
            tasks[ntasks].i1 = imin(zarray_size(quads), i + chunksize);
            tasks[ntasks].quads = quads;
            tasks[ntasks].td = td;
            tasks[ntasks].im = im_orig;
            tasks[ntasks].detections = detections;

            tasks[ntasks].im_gray_samples = im_gray_samples;
            tasks[ntasks].im_decision = im_decision;

            workerpool_add_task(td->wp, quad_decode_task, &tasks[ntasks]);
            ntasks++;
        }

        workerpool_run(td->wp);

        if (im_gray_samples != NULL) {
            image_u8_write_pnm(im_gray_samples, "debug_gray_samples.pnm");
            image_u8_destroy(im_gray_samples);
        }

        if (im_decision != NULL) {
            image_u8_write_pnm(im_decision, "debug_decision.pnm");
            image_u8_destroy(im_decision);
        }

		stackfree(tasks);
    }

    if (td->debug) {
        image_u8_t *im_quads = image_u8_copy(im_orig);
        image_u8_darken(im_quads);
        image_u8_darken(im_quads);

        srandom(0);

        for (int i = 0; i < zarray_size(quads); i++) {
            struct quad *quad;
            zarray_get_volatile(quads, i, &quad);

            const int bias = 100;
            int color = bias + (random() % (255-bias));

            image_u8_draw_line(im_quads, quad->p[0][0], quad->p[0][1], quad->p[1][0], quad->p[1][1], color, 1);
            image_u8_draw_line(im_quads, quad->p[1][0], quad->p[1][1], quad->p[2][0], quad->p[2][1], color, 1);
            image_u8_draw_line(im_quads, quad->p[2][0], quad->p[2][1], quad->p[3][0], quad->p[3][1], color, 1);
            image_u8_draw_line(im_quads, quad->p[3][0], quad->p[3][1], quad->p[0][0], quad->p[0][1], color, 1);

        }

        image_u8_write_pnm(im_quads, "debug_quads_fixed.pnm");
        image_u8_destroy(im_quads);
    }

    timeprofile_stamp(td->tp, "decode+refinement");

    ////////////////////////////////////////////////////////////////
    // Step 3. Reconcile detections--- don't report the same tag more
    // than once. (Allow non-overlapping duplicate detections.)
    if (1) {
        zarray_t *poly0 = g2d_polygon_create_zeros(4);
        zarray_t *poly1 = g2d_polygon_create_zeros(4);

        for (int i0 = 0; i0 < zarray_size(detections); i0++) {

            apriltag_detection_t *det0;
            zarray_get(detections, i0, &det0);

            for (int k = 0; k < 4; k++)
                zarray_set(poly0, k, det0->p[k], NULL);

            for (int i1 = i0+1; i1 < zarray_size(detections); i1++) {

                apriltag_detection_t *det1;
                zarray_get(detections, i1, &det1);

                if (det0->id != det1->id || det0->family != det1->family)
                    continue;

                for (int k = 0; k < 4; k++)
                    zarray_set(poly1, k, det1->p[k], NULL);

                if (g2d_polygon_overlaps_polygon(poly0, poly1)) {
                    // the tags overlap. Delete one, keep the other.

                    if (det0->hamming < det1->hamming ||
                        (det0->hamming == det1->hamming && det0->goodness > det1->goodness)) {
                        // keep det0, destroy det1
                        apriltag_detection_destroy(det1);
                        zarray_remove_index(detections, i1, 1);
                        i1--; // retry the same index
                        goto retry1;
                    } else {
                        // keep det1, destroy det0
                        apriltag_detection_destroy(det0);
                        zarray_remove_index(detections, i0, 1);
                        i0--; // retry the same index.
                        goto retry0;
                    }
                }

              retry1: ;
            }

          retry0: ;
        }

        zarray_destroy(poly0);
        zarray_destroy(poly1);
    }

    timeprofile_stamp(td->tp, "reconcile");

    ////////////////////////////////////////////////////////////////
    // Produce final debug output
    if (td->debug) {

        image_u8_t *darker = image_u8_copy(im_orig);
        image_u8_darken(darker);
        image_u8_darken(darker);

        // assume letter, which is 612x792 points.
        FILE *f = fopen("debug_output.ps", "w");
        fprintf(f, "%%!PS\n\n");
        double scale = fmin(612.0/darker->width, 792.0/darker->height);
        fprintf(f, "%f %f scale\n", scale, scale);
        fprintf(f, "0 %d translate\n", darker->height);
        fprintf(f, "1 -1 scale\n");
        postscript_image(f, darker);

        image_u32_t *out = image_u32_create_from_u8(darker);
        for (int detidx = 0; detidx < zarray_size(detections); detidx++) {
            apriltag_detection_t *det;
            zarray_get(detections, detidx, &det);

            if (det->hamming > 3)
                continue;

            // d |----| c
            //   |    |
            //   |    |
            // a |----| b

            double a[2], b[2], c[2], d[2];

            homography_project(det->H, -1, -1, &a[0], &a[1]);
            homography_project(det->H,  1, -1, &b[0], &b[1]);
            homography_project(det->H,  1,  1, &c[0], &c[1]);
            homography_project(det->H, -1,  1, &d[0], &d[1]);

            float scale = ((float[]) {1.0, 0.6, 0.3, 0.1 })[det->hamming];

            image_u32_draw_line(out, a[0], a[1], b[0], b[1], rgb_scale(0xff0000, scale), 3);
            image_u32_draw_line(out, a[0], a[1], d[0], d[1], rgb_scale(0x00ff00, scale), 3);
            image_u32_draw_line(out, b[0], b[1], c[0], c[1], rgb_scale(0xff88ff, scale), 3);
            image_u32_draw_line(out, d[0], d[1], c[0], c[1], rgb_scale(0x88ffff, scale), 3);

            fprintf(f, "1 0 0 setrgbcolor %f %f moveto %f %f lineto stroke\n", a[0], a[1], b[0], b[1]);
            fprintf(f, "0 1 0 setrgbcolor %f %f moveto %f %f lineto stroke\n", a[0], a[1], d[0], d[1]);
            fprintf(f, "1 .5 1 setrgbcolor %f %f moveto %f %f lineto stroke\n", b[0], b[1], c[0], c[1]);
            fprintf(f, ".5 1 1 setrgbcolor %f %f moveto %f %f lineto stroke\n", d[0], d[1], c[0], c[1]);
        }

        image_u32_write_pnm(out, "debug_output.pnm");

        image_u8_destroy(darker);
        image_u32_destroy(out);

        fclose(f);
    }

    // deallocate
    if (td->debug) {
        FILE *f = fopen("debug_quads.ps", "w");
        fprintf(f, "%%!PS\n\n");

        image_u8_t *im = image_u8_copy(im_orig);
        image_u8_darken(im);
        image_u8_darken(im);

        // assume letter, which is 612x792 points.
        double scale = fmin(612.0/im->width, 792.0/im->height);
        fprintf(f, "%f %f scale\n", scale, scale);
        fprintf(f, "0 %d translate\n", im->height);
        fprintf(f, "1 -1 scale\n");

        postscript_image(f, im);

        for (int i = 0; i < zarray_size(quads); i++) {
            struct quad *q;
            zarray_get_volatile(quads, i, &q);

            float rgb[3];
            int bias = 100;

            for (int i = 0; i < 3; i++)
                rgb[i] = bias + (random() % (255-bias));

            fprintf(f, "%f %f %f setrgbcolor\n", rgb[0]/255.0f, rgb[1]/255.0f, rgb[2]/255.0f);
            fprintf(f, "%f %f moveto %f %f lineto %f %f lineto %f %f lineto %f %f lineto stroke\n",
                    q->p[0][0], q->p[0][1],
                    q->p[1][0], q->p[1][1],
                    q->p[2][0], q->p[2][1],
                    q->p[3][0], q->p[3][1],
                    q->p[0][0], q->p[0][1]);
        }

        fclose(f);
    }

    timeprofile_stamp(td->tp, "debug output");

    for (int i = 0; i < zarray_size(quads); i++) {
        struct quad *quad;
        zarray_get_volatile(quads, i, &quad);
        matd_destroy(quad->H);
        matd_destroy(quad->Hinv);
    }

    zarray_destroy(quads);

    zarray_sort(detections, detection_compare_function);
    timeprofile_stamp(td->tp, "cleanup");

    return detections;
}


// Call this method on each of the tags returned by apriltag_detector_detect
void apriltag_detections_destroy(zarray_t *detections)
{
    for (int i = 0; i < zarray_size(detections); i++) {
        apriltag_detection_t *det;
        zarray_get(detections, i, &det);

        apriltag_detection_destroy(det);
    }

    zarray_destroy(detections);
}
