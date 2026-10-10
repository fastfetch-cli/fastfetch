/*
 * Copyright (c) 2014-2016 Hayaki Saito
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef LIBSIXEL_DITHER_H
#define LIBSIXEL_DITHER_H

#include <sixel.h>

/* dither context object */
struct sixel_dither {
    unsigned int ref;               /* reference counter */
    unsigned char *palette;         /* palette definition */
    unsigned short *cachetable;     /* cache table */
    int reqcolors;                  /* requested colors */
    int ncolors;                    /* active colors */
    int origcolors;                 /* original colors */
    int optimized;                  /* pixel is 15bpp compressable */
    int optimize_palette;           /* minimize palette size */
    int complexion;                 /* for complexion correction */
    int bodyonly;                   /* do not output palette section if true */
    int method_for_largest;         /* method for finding the largest dimention 
                                       for splitting */
    int method_for_rep;             /* method for choosing a color from the box */
    int method_for_diffuse;         /* method for diffusing */
    int quality_mode;               /* quality of histogram */
    int keycolor;                   /* background color */
    int pixelformat;                /* pixelformat for internal processing */
    sixel_allocator_t *allocator;   /* allocator */
};

#ifdef __cplusplus
extern "C" {
#endif

/* apply palette */
sixel_index_t *
sixel_dither_apply_palette(struct sixel_dither /* in */ *dither,
                           unsigned char       /* in */ *pixels,
                           int                 /* in */ width,
                           int                 /* in */ height);

#if HAVE_TESTS
int
sixel_frame_tests_main(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* LIBSIXEL_DITHER_H */

/* emacs Local Variables:      */
/* emacs mode: c               */
/* emacs tab-width: 4          */
/* emacs indent-tabs-mode: nil */
/* emacs c-basic-offset: 4     */
/* emacs End:                  */
/* vim: set expandtab ts=4 sts=4 sw=4 : */
/* EOF */
