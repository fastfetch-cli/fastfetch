/*
 * Copyright (c) 2014-2019 Hayaki Saito
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

#include "config.h"

/* STDC_HEADERS */
#include <stdio.h>
#include <stdlib.h>

#if HAVE_MEMORY_H
# include <memory.h>
#endif  /* HAVE_MEMORY_H */

#include <sixel.h>

static void
get_rgb(unsigned char const *data,
        int const pixelformat,
        int depth,
        unsigned char *r,
        unsigned char *g,
        unsigned char *b)
{
    unsigned int pixels = 0;
#if SWAP_BYTES
    unsigned int low;
    unsigned int high;
#endif
    int count = 0;

    while (count < depth) {
        pixels = *(data + count) | (pixels << 8);
        count++;
    }

    /* TODO: we should swap bytes (only necessary on LSByte first hardware?) */
#if SWAP_BYTES
    if (depth == 2) {
        low    = pixels & 0xff;
        high   = (pixels >> 8) & 0xff;
        pixels = (low << 8) | high;
    }
#endif

    switch (pixelformat) {
    case SIXEL_PIXELFORMAT_RGB555:
        *r = ((pixels >> 10) & 0x1f) << 3;
        *g = ((pixels >>  5) & 0x1f) << 3;
        *b = ((pixels >>  0) & 0x1f) << 3;
        break;
    case SIXEL_PIXELFORMAT_RGB565:
        *r = ((pixels >> 11) & 0x1f) << 3;
        *g = ((pixels >>  5) & 0x3f) << 2;
        *b = ((pixels >>  0) & 0x1f) << 3;
        break;
    case SIXEL_PIXELFORMAT_RGB888:
        *r = (pixels >> 16) & 0xff;
        *g = (pixels >>  8) & 0xff;
        *b = (pixels >>  0) & 0xff;
        break;
    case SIXEL_PIXELFORMAT_BGR555:
        *r = ((pixels >>  0) & 0x1f) << 3;
        *g = ((pixels >>  5) & 0x1f) << 3;
        *b = ((pixels >> 10) & 0x1f) << 3;
        break;
    case SIXEL_PIXELFORMAT_BGR565:
        *r = ((pixels >>  0) & 0x1f) << 3;
        *g = ((pixels >>  5) & 0x3f) << 2;
        *b = ((pixels >> 11) & 0x1f) << 3;
        break;
    case SIXEL_PIXELFORMAT_BGR888:
        *r = (pixels >>  0) & 0xff;
        *g = (pixels >>  8) & 0xff;
        *b = (pixels >> 16) & 0xff;
        break;
    case SIXEL_PIXELFORMAT_RGBA8888:
        *r = (pixels >> 24) & 0xff;
        *g = (pixels >> 16) & 0xff;
        *b = (pixels >>  8) & 0xff;
        break;
    case SIXEL_PIXELFORMAT_ARGB8888:
        *r = (pixels >> 16) & 0xff;
        *g = (pixels >>  8) & 0xff;
        *b = (pixels >>  0) & 0xff;
        break;
    case SIXEL_PIXELFORMAT_BGRA8888:
        *r = (pixels >>  8) & 0xff;
        *g = (pixels >> 16) & 0xff;
        *b = (pixels >> 24) & 0xff;
        break;
    case SIXEL_PIXELFORMAT_ABGR8888:
        *r = (pixels >>  0) & 0xff;
        *g = (pixels >>  8) & 0xff;
        *b = (pixels >> 16) & 0xff;
        break;
    case SIXEL_PIXELFORMAT_GA88:
        *r = *g = *b = (pixels >> 8) & 0xff;
        break;
    case SIXEL_PIXELFORMAT_G8:
    case SIXEL_PIXELFORMAT_AG88:
        *r = *g = *b = pixels & 0xff;
        break;
    default:
        *r = *g = *b = 0;
        break;
    }
}


SIXELAPI int
sixel_helper_compute_depth(int pixelformat)
{
    int depth = (-1);  /* unknown */

    switch (pixelformat) {
    case SIXEL_PIXELFORMAT_ARGB8888:
    case SIXEL_PIXELFORMAT_RGBA8888:
    case SIXEL_PIXELFORMAT_ABGR8888:
    case SIXEL_PIXELFORMAT_BGRA8888:
        depth = 4;
        break;
    case SIXEL_PIXELFORMAT_RGB888:
    case SIXEL_PIXELFORMAT_BGR888:
        depth = 3;
        break;
    case SIXEL_PIXELFORMAT_RGB555:
    case SIXEL_PIXELFORMAT_RGB565:
    case SIXEL_PIXELFORMAT_BGR555:
    case SIXEL_PIXELFORMAT_BGR565:
    case SIXEL_PIXELFORMAT_AG88:
    case SIXEL_PIXELFORMAT_GA88:
        depth = 2;
        break;
    case SIXEL_PIXELFORMAT_G1:
    case SIXEL_PIXELFORMAT_G2:
    case SIXEL_PIXELFORMAT_G4:
    case SIXEL_PIXELFORMAT_G8:
    case SIXEL_PIXELFORMAT_PAL1:
    case SIXEL_PIXELFORMAT_PAL2:
    case SIXEL_PIXELFORMAT_PAL4:
    case SIXEL_PIXELFORMAT_PAL8:
        depth = 1;
        break;
    default:
        break;
    }

    return depth;
}


static void
expand_rgb(unsigned char *dst,
           unsigned char const *src,
           int width, int height,
           int pixelformat, int depth)
{
    int x;
    int y;
    int dst_offset;
    int src_offset;
    unsigned char r, g, b;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            src_offset = depth * (y * width + x);
            dst_offset = 3 * (y * width + x);
            get_rgb(src + src_offset, pixelformat, depth, &r, &g, &b);

            *(dst + dst_offset + 0) = r;
            *(dst + dst_offset + 1) = g;
            *(dst + dst_offset + 2) = b;
        }
    }
}


static SIXELSTATUS
expand_palette(unsigned char *dst, unsigned char const *src,
               int width, int height, int const pixelformat)
{
    SIXELSTATUS status = SIXEL_FALSE;
    int x;
    int y;
    int i;
    int bpp;  /* bit per plane */

    switch (pixelformat) {
    case SIXEL_PIXELFORMAT_PAL1:
    case SIXEL_PIXELFORMAT_G1:
        bpp = 1;
        break;
    case SIXEL_PIXELFORMAT_PAL2:
    case SIXEL_PIXELFORMAT_G2:
        bpp = 2;
        break;
    case SIXEL_PIXELFORMAT_PAL4:
    case SIXEL_PIXELFORMAT_G4:
        bpp = 4;
        break;
    case SIXEL_PIXELFORMAT_PAL8:
    case SIXEL_PIXELFORMAT_G8:
        for (i = 0; i < width * height; ++i, ++src) {
            *dst++ = *src;
        }
        status = SIXEL_OK;
        goto end;
    default:
        status = SIXEL_BAD_ARGUMENT;
        sixel_helper_set_additional_message(
            "expand_palette: invalid pixelformat.");
        goto end;
    }

#if HAVE_DEBUG
    fprintf(stderr, "expanding PAL%d to PAL8...\n", bpp);
#endif

    for (y = 0; y < height; ++y) {
        for (x = 0; x < width * bpp / 8; ++x) {
            for (i = 0; i < 8 / bpp; ++i) {
                *dst++ = *src >> (8 / bpp - 1 - i) * bpp & ((1 << bpp) - 1);
            }
            src++;
        }
        x = width - x * 8 / bpp;
        if (x > 0) {
            for (i = 0; i < x; ++i) {
                *dst++ = *src >> (8 - (i + 1) * bpp) & ((1 << bpp) - 1);
            }
            src++;
        }
    }

    status = SIXEL_OK;

end:
    return status;
}


SIXELAPI SIXELSTATUS
sixel_helper_normalize_pixelformat(
    unsigned char       /* out */ *dst,             /* destination buffer */
    int                 /* out */ *dst_pixelformat, /* converted pixelformat */
    unsigned char const /* in */  *src,             /* source pixels */
    int                 /* in */  src_pixelformat,  /* format of source image */
    int                 /* in */  width,            /* width of source image */
    int                 /* in */  height)           /* height of source image */
{
    SIXELSTATUS status = SIXEL_FALSE;

    switch (src_pixelformat) {
    case SIXEL_PIXELFORMAT_G8:
        expand_rgb(dst, src, width, height, src_pixelformat, 1);
        *dst_pixelformat = SIXEL_PIXELFORMAT_RGB888;
        break;
    case SIXEL_PIXELFORMAT_RGB565:
    case SIXEL_PIXELFORMAT_RGB555:
    case SIXEL_PIXELFORMAT_BGR565:
    case SIXEL_PIXELFORMAT_BGR555:
    case SIXEL_PIXELFORMAT_GA88:
    case SIXEL_PIXELFORMAT_AG88:
        expand_rgb(dst, src, width, height, src_pixelformat, 2);
        *dst_pixelformat = SIXEL_PIXELFORMAT_RGB888;
        break;
    case SIXEL_PIXELFORMAT_RGB888:
    case SIXEL_PIXELFORMAT_BGR888:
        expand_rgb(dst, src, width, height, src_pixelformat, 3);
        *dst_pixelformat = SIXEL_PIXELFORMAT_RGB888;
        break;
    case SIXEL_PIXELFORMAT_RGBA8888:
    case SIXEL_PIXELFORMAT_ARGB8888:
    case SIXEL_PIXELFORMAT_BGRA8888:
    case SIXEL_PIXELFORMAT_ABGR8888:
        expand_rgb(dst, src, width, height, src_pixelformat, 4);
        *dst_pixelformat = SIXEL_PIXELFORMAT_RGB888;
        break;
    case SIXEL_PIXELFORMAT_PAL1:
    case SIXEL_PIXELFORMAT_PAL2:
    case SIXEL_PIXELFORMAT_PAL4:
        *dst_pixelformat = SIXEL_PIXELFORMAT_PAL8;
        status = expand_palette(dst, src, width, height, src_pixelformat);
        if (SIXEL_FAILED(status)) {
            goto end;
        }
        break;
    case SIXEL_PIXELFORMAT_G1:
    case SIXEL_PIXELFORMAT_G2:
    case SIXEL_PIXELFORMAT_G4:
        *dst_pixelformat = SIXEL_PIXELFORMAT_G8;
        status = expand_palette(dst, src, width, height, src_pixelformat);
        if (SIXEL_FAILED(status)) {
            goto end;
        }
        break;
    case SIXEL_PIXELFORMAT_PAL8:
        memcpy(dst, src, (size_t)(width * height));
        *dst_pixelformat = src_pixelformat;
        break;
    default:
        status = SIXEL_BAD_ARGUMENT;
        goto end;
    }

    status = SIXEL_OK;

end:
    return status;
}


#if HAVE_TESTS
static int
test1(void)
{
    unsigned char dst[3];
    int dst_pixelformat = SIXEL_PIXELFORMAT_RGB888;
    int src_pixelformat = SIXEL_PIXELFORMAT_RGB888;
    unsigned char src[] = { 0x46, 0xf3, 0xe5 };
    int ret = 0;

    int nret = EXIT_FAILURE;

    ret = sixel_helper_normalize_pixelformat(dst,
                                             &dst_pixelformat,
                                             src,
                                             src_pixelformat,
                                             1,
                                             1);
    if (ret != 0) {
        goto error;
    }
    if (dst_pixelformat != SIXEL_PIXELFORMAT_RGB888) {
        goto error;
    }
    if ((dst[0] << 16 | dst[1] << 8 | dst[2]) != (src[0] << 16 | src[1] << 8 | src[2])) {
        goto error;
    }
    return EXIT_SUCCESS;

error:
    perror("test1");
    return nret;
}


static int
test2(void)
{
    unsigned char dst[3];
    int dst_pixelformat = SIXEL_PIXELFORMAT_RGB888;
    int src_pixelformat = SIXEL_PIXELFORMAT_RGB555;
    unsigned char src[] = { 0x47, 0x9c };
    int ret = 0;

    int nret = EXIT_FAILURE;

    ret = sixel_helper_normalize_pixelformat(dst,
                                             &dst_pixelformat,
                                             src,
                                             src_pixelformat,
                                             1,
                                             1);
    if (ret != 0) {
        goto error;
    }
    if (dst_pixelformat != SIXEL_PIXELFORMAT_RGB888) {
        goto error;
    }
    if ((dst[0] >> 3 << 10 | dst[1] >> 3 << 5 | dst[2] >> 3) != (src[0] << 8 | src[1])) {
        goto error;
    }
    return EXIT_SUCCESS;

error:
    perror("test2");
    return nret;
}


static int
test3(void)
{
    unsigned char dst[3];
    int dst_pixelformat = SIXEL_PIXELFORMAT_RGB888;
    int src_pixelformat = SIXEL_PIXELFORMAT_RGB565;
    unsigned char src[] = { 0x47, 0x9c };
    int ret = 0;

    int nret = EXIT_FAILURE;

    ret = sixel_helper_normalize_pixelformat(dst,
                                             &dst_pixelformat,
                                             src,
                                             src_pixelformat,
                                             1,
                                             1);
    if (ret != 0) {
        goto error;
    }
    if (dst_pixelformat != SIXEL_PIXELFORMAT_RGB888) {
        goto error;
    }
    if ((dst[0] >> 3 << 11 | dst[1] >> 2 << 5 | dst[2] >> 3) != (src[0] << 8 | src[1])) {
        goto error;
    }
    return EXIT_SUCCESS;

error:
    perror("test3");
    return nret;
}


static int
test4(void)
{
    unsigned char dst[3];
    int dst_pixelformat = SIXEL_PIXELFORMAT_RGB888;
    int src_pixelformat = SIXEL_PIXELFORMAT_BGR888;
    unsigned char src[] = { 0x46, 0xf3, 0xe5 };
    int ret = 0;

    int nret = EXIT_FAILURE;

    ret = sixel_helper_normalize_pixelformat(dst,
                                             &dst_pixelformat,
                                             src,
                                             src_pixelformat,
                                             1,
                                             1);
    if (ret != 0) {
        goto error;
    }
    if (dst_pixelformat != SIXEL_PIXELFORMAT_RGB888) {
        goto error;
    }
    if ((dst[2] << 16 | dst[1] << 8 | dst[0]) != (src[0] << 16 | src[1] << 8 | src[2])) {
        goto error;
    }
    return EXIT_SUCCESS;

error:
    perror("test4");
    return nret;
}


static int
test5(void)
{
    unsigned char dst[3];
    int dst_pixelformat = SIXEL_PIXELFORMAT_RGB888;
    int src_pixelformat = SIXEL_PIXELFORMAT_BGR555;
    unsigned char src[] = { 0x23, 0xc8 };
    int ret = 0;

    int nret = EXIT_FAILURE;

    ret = sixel_helper_normalize_pixelformat(dst,
                                             &dst_pixelformat,
                                             src,
                                             src_pixelformat,
                                             1,
                                             1);
    if (ret != 0) {
        goto error;
    }
    if (dst_pixelformat != SIXEL_PIXELFORMAT_RGB888) {
        goto error;
    }
    if ((dst[2] >> 3 << 10 | dst[1] >> 3 << 5 | dst[0] >> 3) != (src[0] << 8 | src[1])) {
        goto error;
    }
    return EXIT_SUCCESS;

error:
    perror("test5");
    return nret;
}


static int
test6(void)
{
    unsigned char dst[3];
    int dst_pixelformat = SIXEL_PIXELFORMAT_RGB888;
    int src_pixelformat = SIXEL_PIXELFORMAT_BGR565;
    unsigned char src[] = { 0x47, 0x88 };
    int ret = 0;

    int nret = EXIT_FAILURE;

    ret = sixel_helper_normalize_pixelformat(dst,
                                             &dst_pixelformat,
                                             src,
                                             src_pixelformat,
                                             1,
                                             1);
    if (ret != 0) {
        goto error;
    }
    if (dst_pixelformat != SIXEL_PIXELFORMAT_RGB888) {
        goto error;
    }
    if ((dst[2] >> 3 << 11 | dst[1] >> 2 << 5 | dst[0] >> 3) != (src[0] << 8 | src[1])) {
        goto error;
    }
    return EXIT_SUCCESS;

error:
    perror("test6");
    return nret;
}


static int
test7(void)
{
    unsigned char dst[3];
    int dst_pixelformat = SIXEL_PIXELFORMAT_RGB888;
    int src_pixelformat = SIXEL_PIXELFORMAT_AG88;
    unsigned char src[] = { 0x47, 0x88 };
    int ret = 0;

    int nret = EXIT_FAILURE;

    ret = sixel_helper_normalize_pixelformat(dst,
                                             &dst_pixelformat,
                                             src,
                                             src_pixelformat,
                                             1,
                                             1);
    if (ret != 0) {
        goto error;
    }
    if (dst_pixelformat != SIXEL_PIXELFORMAT_RGB888) {
        goto error;
    }
    if (dst[0] != src[1]) {
        goto error;
    }
    return EXIT_SUCCESS;

error:
    perror("test7");
    return nret;
}


static int
test8(void)
{
    unsigned char dst[3];
    int dst_pixelformat = SIXEL_PIXELFORMAT_RGB888;
    int src_pixelformat = SIXEL_PIXELFORMAT_GA88;
    unsigned char src[] = { 0x47, 0x88 };
    int ret = 0;

    int nret = EXIT_FAILURE;

    ret = sixel_helper_normalize_pixelformat(dst,
                                             &dst_pixelformat,
                                             src,
                                             src_pixelformat,
                                             1,
                                             1);
    if (ret != 0) {
        goto error;
    }
    if (dst_pixelformat != SIXEL_PIXELFORMAT_RGB888) {
        goto error;
    }
    if (dst[0] != src[0]) {
        goto error;
    }
    return EXIT_SUCCESS;

error:
    perror("test8");
    return nret;
}


static int
test9(void)
{
    unsigned char dst[3];
    int dst_pixelformat = SIXEL_PIXELFORMAT_RGB888;
    int src_pixelformat = SIXEL_PIXELFORMAT_RGBA8888;
    unsigned char src[] = { 0x46, 0xf3, 0xe5, 0xf0 };
    int ret = 0;

    int nret = EXIT_FAILURE;

    ret = sixel_helper_normalize_pixelformat(dst,
                                             &dst_pixelformat,
                                             src,
                                             src_pixelformat,
                                             1,
                                             1);
    if (ret != 0) {
        goto error;
    }
    if (dst_pixelformat != SIXEL_PIXELFORMAT_RGB888) {
        goto error;
    }
    if (dst[0] != src[0]) {
        goto error;
    }
    if (dst[1] != src[1]) {
        goto error;
    }
    if (dst[2] != src[2]) {
        goto error;
    }
    return EXIT_SUCCESS;

error:
    perror("test8");
    return nret;
}


static int
test10(void)
{
    unsigned char dst[3];
    int dst_pixelformat = SIXEL_PIXELFORMAT_RGB888;
    int src_pixelformat = SIXEL_PIXELFORMAT_ARGB8888;
    unsigned char src[] = { 0x46, 0xf3, 0xe5, 0xf0 };
    int ret = 0;

    int nret = EXIT_FAILURE;

    ret = sixel_helper_normalize_pixelformat(dst,
                                             &dst_pixelformat,
                                             src,
                                             src_pixelformat,
                                             1,
                                             1);
    if (ret != 0) {
        goto error;
    }
    if (dst_pixelformat != SIXEL_PIXELFORMAT_RGB888) {
        goto error;
    }
    if (dst[0] != src[1]) {
        goto error;
    }
    if (dst[1] != src[2]) {
        goto error;
    }
    if (dst[2] != src[3]) {
        goto error;
    }
    return EXIT_SUCCESS;

error:
    perror("test8");
    return nret;
}


SIXELAPI int
sixel_pixelformat_tests_main(void)
{
    int nret = EXIT_FAILURE;
    size_t i;
    typedef int (* testcase)(void);

    static testcase const testcases[] = {
        test1,
        test2,
        test3,
        test4,
        test5,
        test6,
        test7,
        test8,
        test9,
        test10,
    };

    for (i = 0; i < sizeof(testcases) / sizeof(testcase); ++i) {
        nret = testcases[i]();
        if (nret != EXIT_SUCCESS) {
            goto error;
        }
    }

    nret = EXIT_SUCCESS;

error:
    return nret;
}
#endif  /* HAVE_TESTS */

/* emacs Local Variables:      */
/* emacs mode: c               */
/* emacs tab-width: 4          */
/* emacs indent-tabs-mode: nil */
/* emacs c-basic-offset: 4     */
/* emacs End:                  */
/* vim: set expandtab ts=4 sts=4 sw=4 : */
/* EOF */
