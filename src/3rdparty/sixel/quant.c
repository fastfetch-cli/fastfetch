/*
 *
 * mediancut algorithm implementation is imported from pnmcolormap.c
 * in netpbm library.
 * http://netpbm.sourceforge.net/
 *
 * *******************************************************************************
 *                  original license block of pnmcolormap.c
 * *******************************************************************************
 *
 *   Derived from ppmquant, originally by Jef Poskanzer.
 *
 *   Copyright (C) 1989, 1991 by Jef Poskanzer.
 *   Copyright (C) 2001 by Bryan Henderson.
 *
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted, provided
 *   that the above copyright notice appear in all copies and that both that
 *   copyright notice and this permission notice appear in supporting
 *   documentation.  This software is provided "as is" without express or
 *   implied warranty.
 *
 * ******************************************************************************
 *
 * Copyright (c) 2014-2018 Hayaki Saito
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
 *
 *
 */

#include "config.h"

/* STDC_HEADERS */
#include <stdlib.h>
#include <stdio.h>

#if HAVE_STRING_H
# include <string.h>
#endif  /* HAVE_STRING_H */
#if HAVE_MATH_H
#include <math.h>
#endif  /* HAVE_MATH_H */
#if HAVE_LIMITS_H
# include <limits.h>
#endif  /* HAVE_MATH_H */
#if HAVE_INTTYPES_H
# include <inttypes.h>
#endif  /* HAVE_MATH_H */

#include "quant.h"

#if HAVE_DEBUG
#define quant_trace fprintf
#else
static inline void quant_trace(FILE *f, ...) { (void) f; }
#endif

/*****************************************************************************
 *
 * quantization
 *
 *****************************************************************************/

typedef struct box* boxVector;
struct box {
    unsigned int ind;
    unsigned int colors;
    unsigned int sum;
};

typedef unsigned long sample;
typedef sample * tuple;

struct tupleint {
    /* An ordered pair of a tuple value and an integer, such as you
       would find in a tuple table or tuple hash.
       Note that this is a variable length structure.
    */
    unsigned int value;
    sample tuple[1];
    /* This is actually a variable size array -- its size is the
       depth of the tuple in question.  Some compilers do not let us
       declare a variable length array.
    */
};
typedef struct tupleint ** tupletable;

typedef struct {
    unsigned int size;
    tupletable table;
} tupletable2;

static unsigned int compareplanePlane;
    /* This is a parameter to compareplane().  We use this global variable
       so that compareplane() can be called by qsort(), to compare two
       tuples.  qsort() doesn't pass any arguments except the two tuples.
    */
static int
compareplane(const void * const arg1,
             const void * const arg2)
{
    int lhs, rhs;

    typedef const struct tupleint * const * const sortarg;
    sortarg comparandPP  = (sortarg) arg1;
    sortarg comparatorPP = (sortarg) arg2;
    lhs = (int)(*comparandPP)->tuple[compareplanePlane];
    rhs = (int)(*comparatorPP)->tuple[compareplanePlane];

    return lhs - rhs;
}


static int
sumcompare(const void * const b1, const void * const b2)
{
    return (int)((boxVector)b2)->sum - (int)((boxVector)b1)->sum;
}


static SIXELSTATUS
alloctupletable(
    tupletable          /* out */ *result,
    unsigned int const  /* in */  depth,
    unsigned int const  /* in */  size,
    sixel_allocator_t   /* in */  *allocator)
{
    SIXELSTATUS status = SIXEL_FALSE;
    enum { message_buffer_size = 256 };
    char message[message_buffer_size];
    int nwrite;
    unsigned int mainTableSize;
    unsigned int tupleIntSize;
    unsigned int allocSize;
    void * pool;
    tupletable tbl;
    unsigned int i;

    if (UINT_MAX / sizeof(struct tupleint) < size) {
        nwrite = sprintf(message,
                         "size %u is too big for arithmetic",
                         size);
        if (nwrite > 0) {
            sixel_helper_set_additional_message(message);
        }
        status = SIXEL_RUNTIME_ERROR;
        goto end;
    }

    mainTableSize = size * sizeof(struct tupleint *);
    tupleIntSize = sizeof(struct tupleint) - sizeof(sample)
        + depth * sizeof(sample);

    /* To save the enormous amount of time it could take to allocate
       each individual tuple, we do a trick here and allocate everything
       as a single malloc block and suballocate internally.
    */
    if ((UINT_MAX - mainTableSize) / tupleIntSize < size) {
        nwrite = sprintf(message,
                         "size %u is too big for arithmetic",
                         size);
        if (nwrite > 0) {
            sixel_helper_set_additional_message(message);
        }
        status = SIXEL_RUNTIME_ERROR;
        goto end;
    }

    allocSize = mainTableSize + size * tupleIntSize;

    pool = sixel_allocator_malloc(allocator, allocSize);
    if (pool == NULL) {
        sprintf(message,
                "unable to allocate %u bytes for a %u-entry "
                "tuple table",
                 allocSize, size);
        sixel_helper_set_additional_message(message);
        status = SIXEL_BAD_ALLOCATION;
        goto end;
    }
    tbl = (tupletable) pool;

    for (i = 0; i < size; ++i)
        tbl[i] = (struct tupleint *)
            ((char*)pool + mainTableSize + i * tupleIntSize);

    *result = tbl;

    status = SIXEL_OK;

end:
    return status;
}


/*
** Here is the fun part, the median-cut colormap generator.  This is based
** on Paul Heckbert's paper "Color Image Quantization for Frame Buffer
** Display", SIGGRAPH '82 Proceedings, page 297.
*/

static tupletable2
newColorMap(unsigned int const newcolors, unsigned int const depth, sixel_allocator_t *allocator)
{
    SIXELSTATUS status = SIXEL_FALSE;
    tupletable2 colormap;
    unsigned int i;

    colormap.size = 0;
    status = alloctupletable(&colormap.table, depth, newcolors, allocator);
    if (SIXEL_FAILED(status)) {
        goto end;
    }
    if (colormap.table) {
        for (i = 0; i < newcolors; ++i) {
            unsigned int plane;
            for (plane = 0; plane < depth; ++plane)
                colormap.table[i]->tuple[plane] = 0;
        }
        colormap.size = newcolors;
    }

end:
    return colormap;
}


static boxVector
newBoxVector(
    unsigned int const  /* in */ colors,
    unsigned int const  /* in */ sum,
    unsigned int const  /* in */ newcolors,
    sixel_allocator_t   /* in */ *allocator)
{
    boxVector bv;

    bv = (boxVector)sixel_allocator_malloc(allocator,
                                           sizeof(struct box) * (size_t)newcolors);
    if (bv == NULL) {
        quant_trace(stderr, "out of memory allocating box vector table\n");
        return NULL;
    }

    /* Set up the initial box. */
    bv[0].ind = 0;
    bv[0].colors = colors;
    bv[0].sum = sum;

    return bv;
}


static void
findBoxBoundaries(tupletable2  const colorfreqtable,
                  unsigned int const depth,
                  unsigned int const boxStart,
                  unsigned int const boxSize,
                  sample             minval[],
                  sample             maxval[])
{
/*----------------------------------------------------------------------------
  Go through the box finding the minimum and maximum of each
  component - the boundaries of the box.
-----------------------------------------------------------------------------*/
    unsigned int plane;
    unsigned int i;

    for (plane = 0; plane < depth; ++plane) {
        minval[plane] = colorfreqtable.table[boxStart]->tuple[plane];
        maxval[plane] = minval[plane];
    }

    for (i = 1; i < boxSize; ++i) {
        for (plane = 0; plane < depth; ++plane) {
            sample const v = colorfreqtable.table[boxStart + i]->tuple[plane];
            if (v < minval[plane]) minval[plane] = v;
            if (v > maxval[plane]) maxval[plane] = v;
        }
    }
}



static unsigned int
largestByNorm(sample minval[], sample maxval[], unsigned int const depth)
{

    unsigned int largestDimension;
    unsigned int plane;
    sample largestSpreadSoFar;

    largestSpreadSoFar = 0;
    largestDimension = 0;
    for (plane = 0; plane < depth; ++plane) {
        sample const spread = maxval[plane]-minval[plane];
        if (spread > largestSpreadSoFar) {
            largestDimension = plane;
            largestSpreadSoFar = spread;
        }
    }
    return largestDimension;
}



static unsigned int
largestByLuminosity(sample minval[], sample maxval[], unsigned int const depth)
{
/*----------------------------------------------------------------------------
   This subroutine presumes that the tuple type is either
   BLACKANDWHITE, GRAYSCALE, or RGB (which implies pamP->depth is 1 or 3).
   To save time, we don't actually check it.
-----------------------------------------------------------------------------*/
    unsigned int retval;

    double lumin_factor[3] = {0.2989, 0.5866, 0.1145};

    if (depth == 1) {
        retval = 0;
    } else {
        /* An RGB tuple */
        unsigned int largestDimension;
        unsigned int plane;
        double largestSpreadSoFar;

        largestSpreadSoFar = 0.0;
        largestDimension = 0;

        for (plane = 0; plane < 3; ++plane) {
            double const spread =
                lumin_factor[plane] * (maxval[plane]-minval[plane]);
            if (spread > largestSpreadSoFar) {
                largestDimension = plane;
                largestSpreadSoFar = spread;
            }
        }
        retval = largestDimension;
    }
    return retval;
}



static void
centerBox(unsigned int const boxStart,
          unsigned int const boxSize,
          tupletable2  const colorfreqtable,
          unsigned int const depth,
          tuple        const newTuple)
{

    unsigned int plane;
    sample minval, maxval;
    unsigned int i;

    for (plane = 0; plane < depth; ++plane) {
        minval = maxval = colorfreqtable.table[boxStart]->tuple[plane];

        for (i = 1; i < boxSize; ++i) {
            sample v = colorfreqtable.table[boxStart + i]->tuple[plane];
            minval = minval < v ? minval: v;
            maxval = maxval > v ? maxval: v;
        }
        newTuple[plane] = (minval + maxval) / 2;
    }
}



static void
averageColors(unsigned int const boxStart,
              unsigned int const boxSize,
              tupletable2  const colorfreqtable,
              unsigned int const depth,
              tuple        const newTuple)
{
    unsigned int plane;
    sample sum;
    unsigned int i;

    for (plane = 0; plane < depth; ++plane) {
        sum = 0;

        for (i = 0; i < boxSize; ++i) {
            sum += colorfreqtable.table[boxStart + i]->tuple[plane];
        }

        newTuple[plane] = sum / boxSize;
    }
}



static void
averagePixels(unsigned int const boxStart,
              unsigned int const boxSize,
              tupletable2 const colorfreqtable,
              unsigned int const depth,
              tuple const newTuple)
{

    unsigned int n;
        /* Number of tuples represented by the box */
    unsigned int plane;
    unsigned int i;

    /* Count the tuples in question */
    n = 0;  /* initial value */
    for (i = 0; i < boxSize; ++i) {
        n += (unsigned int)colorfreqtable.table[boxStart + i]->value;
    }

    for (plane = 0; plane < depth; ++plane) {
        sample sum;

        sum = 0;

        for (i = 0; i < boxSize; ++i) {
            sum += colorfreqtable.table[boxStart + i]->tuple[plane]
                * (unsigned int)colorfreqtable.table[boxStart + i]->value;
        }

        newTuple[plane] = sum / n;
    }
}



static tupletable2
colormapFromBv(unsigned int const newcolors,
               boxVector const bv,
               unsigned int const boxes,
               tupletable2 const colorfreqtable,
               unsigned int const depth,
               int const methodForRep,
               sixel_allocator_t *allocator)
{
    /*
    ** Ok, we've got enough boxes.  Now choose a representative color for
    ** each box.  There are a number of possible ways to make this choice.
    ** One would be to choose the center of the box; this ignores any structure
    ** within the boxes.  Another method would be to average all the colors in
    ** the box - this is the method specified in Heckbert's paper.  A third
    ** method is to average all the pixels in the box.
    */
    tupletable2 colormap;
    unsigned int bi;

    colormap = newColorMap(newcolors, depth, allocator);
    if (!colormap.size) {
        return colormap;
    }

    for (bi = 0; bi < boxes; ++bi) {
        switch (methodForRep) {
        case SIXEL_REP_CENTER_BOX:
            centerBox(bv[bi].ind, bv[bi].colors,
                      colorfreqtable, depth,
                      colormap.table[bi]->tuple);
            break;
        case SIXEL_REP_AVERAGE_COLORS:
            averageColors(bv[bi].ind, bv[bi].colors,
                          colorfreqtable, depth,
                          colormap.table[bi]->tuple);
            break;
        case SIXEL_REP_AVERAGE_PIXELS:
            averagePixels(bv[bi].ind, bv[bi].colors,
                          colorfreqtable, depth,
                          colormap.table[bi]->tuple);
            break;
        default:
            quant_trace(stderr, "Internal error: "
                                "invalid value of methodForRep: %d\n",
                        methodForRep);
        }
    }
    return colormap;
}


static SIXELSTATUS
splitBox(boxVector const bv,
         unsigned int *const boxesP,
         unsigned int const bi,
         tupletable2 const colorfreqtable,
         unsigned int const depth,
         int const methodForLargest)
{
/*----------------------------------------------------------------------------
   Split Box 'bi' in the box vector bv (so that bv contains one more box
   than it did as input).  Split it so that each new box represents about
   half of the pixels in the distribution given by 'colorfreqtable' for
   the colors in the original box, but with distinct colors in each of the
   two new boxes.

   Assume the box contains at least two colors.
-----------------------------------------------------------------------------*/
    SIXELSTATUS status = SIXEL_FALSE;
    unsigned int const boxStart = bv[bi].ind;
    unsigned int const boxSize  = bv[bi].colors;
    unsigned int const sm       = bv[bi].sum;

    enum { max_depth= 16 };
    sample minval[max_depth];
    sample maxval[max_depth];

    /* assert(max_depth >= depth); */

    unsigned int largestDimension;
        /* number of the plane with the largest spread */
    unsigned int medianIndex;
    unsigned int lowersum;
        /* Number of pixels whose value is "less than" the median */

    findBoxBoundaries(colorfreqtable, depth, boxStart, boxSize,
                      minval, maxval);

    /* Find the largest dimension, and sort by that component.  I have
       included two methods for determining the "largest" dimension;
       first by simply comparing the range in RGB space, and second by
       transforming into luminosities before the comparison.
    */
    switch (methodForLargest) {
    case SIXEL_LARGE_NORM:
        largestDimension = largestByNorm(minval, maxval, depth);
        break;
    case SIXEL_LARGE_LUM:
        largestDimension = largestByLuminosity(minval, maxval, depth);
        break;
    default:
        sixel_helper_set_additional_message(
            "Internal error: invalid value of methodForLargest.");
        status = SIXEL_LOGIC_ERROR;
        goto end;
    }

    /* TODO: I think this sort should go after creating a box,
       not before splitting.  Because you need the sort to use
       the SIXEL_REP_CENTER_BOX method of choosing a color to
       represent the final boxes
    */

    /* Set the gross global variable 'compareplanePlane' as a
       parameter to compareplane(), which is called by qsort().
    */
    compareplanePlane = largestDimension;
    qsort((char*) &colorfreqtable.table[boxStart], boxSize,
          sizeof(colorfreqtable.table[boxStart]),
          compareplane);

    {
        /* Now find the median based on the counts, so that about half
           the pixels (not colors, pixels) are in each subdivision.  */

        unsigned int i;

        lowersum = colorfreqtable.table[boxStart]->value; /* initial value */
        for (i = 1; i < boxSize - 1 && lowersum < sm / 2; ++i) {
            lowersum += colorfreqtable.table[boxStart + i]->value;
        }
        medianIndex = i;
    }
    /* Split the box, and sort to bring the biggest boxes to the top.  */

    bv[bi].colors = medianIndex;
    bv[bi].sum = lowersum;
    bv[*boxesP].ind = boxStart + medianIndex;
    bv[*boxesP].colors = boxSize - medianIndex;
    bv[*boxesP].sum = sm - lowersum;
    ++(*boxesP);
    qsort((char*) bv, *boxesP, sizeof(struct box), sumcompare);

    status = SIXEL_OK;

end:
    return status;
}



static SIXELSTATUS
mediancut(tupletable2 const colorfreqtable,
          unsigned int const depth,
          unsigned int const newcolors,
          int const methodForLargest,
          int const methodForRep,
          tupletable2 *const colormapP,
          sixel_allocator_t *allocator)
{
/*----------------------------------------------------------------------------
   Compute a set of only 'newcolors' colors that best represent an
   image whose pixels are summarized by the histogram
   'colorfreqtable'.  Each tuple in that table has depth 'depth'.
   colorfreqtable.table[i] tells the number of pixels in the subject image
   have a particular color.

   As a side effect, sort 'colorfreqtable'.
-----------------------------------------------------------------------------*/
    boxVector bv;
    unsigned int bi;
    unsigned int boxes;
    int multicolorBoxesExist;
    unsigned int i;
    unsigned int sum;
    SIXELSTATUS status = SIXEL_FALSE;

    sum = 0;

    for (i = 0; i < colorfreqtable.size; ++i) {
        sum += colorfreqtable.table[i]->value;
    }

    /* There is at least one box that contains at least 2 colors; ergo,
       there is more splitting we can do.  */
    bv = newBoxVector(colorfreqtable.size, sum, newcolors, allocator);
    if (bv == NULL) {
        goto end;
    }
    boxes = 1;
    multicolorBoxesExist = (colorfreqtable.size > 1);

    /* Main loop: split boxes until we have enough. */
    while (boxes < newcolors && multicolorBoxesExist) {
        /* Find the first splittable box. */
        for (bi = 0; bi < boxes && bv[bi].colors < 2; ++bi)
            ;
        if (bi >= boxes) {
            multicolorBoxesExist = 0;
        } else {
            status = splitBox(bv, &boxes, bi,
                              colorfreqtable, depth,
                              methodForLargest);
            if (SIXEL_FAILED(status)) {
                goto end;
            }
        }
    }
    *colormapP = colormapFromBv(newcolors, bv, boxes,
                                colorfreqtable, depth,
                                methodForRep, allocator);

    sixel_allocator_free(allocator, bv);

    status = SIXEL_OK;

end:
    return status;
}


static unsigned int
computeHash(unsigned char const *data, unsigned int const depth)
{
    unsigned int hash = 0;
    unsigned int n;

    for (n = 0; n < depth; n++) {
        hash |= (unsigned int)(data[depth - 1 - n] >> 3) << n * 5;
    }

    return hash;
}


static SIXELSTATUS
computeHistogram(unsigned char const    /* in */  *data,
                 unsigned int           /* in */  length,
                 unsigned long const    /* in */  depth,
                 tupletable2 * const    /* out */ colorfreqtableP,
                 int const              /* in */  qualityMode,
                 sixel_allocator_t      /* in */  *allocator)
{
    SIXELSTATUS status = SIXEL_FALSE;
    typedef unsigned short unit_t;
    unsigned int i, n;
    unit_t *histogram = NULL;
    unit_t *refmap = NULL;
    unit_t *ref;
    unit_t *it;
    unsigned int bucket_index;
    unsigned int step;
    unsigned int max_sample;

    switch (qualityMode) {
    case SIXEL_QUALITY_LOW:
        max_sample = 18383;
        break;
    case SIXEL_QUALITY_HIGH:
        max_sample = 1118383;
        break;
    case SIXEL_QUALITY_FULL:
    default:
        max_sample = 4003079;
        break;
    }

    step = length / depth / max_sample * depth;
    if (step <= 0) {
        step = depth;
    }

    quant_trace(stderr, "making histogram...\n");

    histogram = (unit_t *)sixel_allocator_calloc(allocator,
                                                 (size_t)(1 << depth * 5),
                                                 sizeof(unit_t));
    if (histogram == NULL) {
        sixel_helper_set_additional_message(
            "unable to allocate memory for histogram.");
        status = SIXEL_BAD_ALLOCATION;
        goto end;
    }
    it = ref = refmap
        = (unsigned short *)sixel_allocator_malloc(allocator,
                                                   (size_t)(1 << depth * 5) * sizeof(unit_t));
    if (!it) {
        sixel_helper_set_additional_message(
            "unable to allocate memory for lookup table.");
        status = SIXEL_BAD_ALLOCATION;
        goto end;
    }

    for (i = 0; i < length; i += step) {
        bucket_index = computeHash(data + i, 3);
        if (histogram[bucket_index] == 0) {
            *ref++ = bucket_index;
        }
        if (histogram[bucket_index] < (unsigned int)(1 << sizeof(unsigned short) * 8) - 1) {
            histogram[bucket_index]++;
        }
    }

    colorfreqtableP->size = (unsigned int)(ref - refmap);

    status = alloctupletable(&colorfreqtableP->table, depth, (unsigned int)(ref - refmap), allocator);
    if (SIXEL_FAILED(status)) {
        goto end;
    }
    for (i = 0; i < colorfreqtableP->size; ++i) {
        if (histogram[refmap[i]] > 0) {
            colorfreqtableP->table[i]->value = histogram[refmap[i]];
            for (n = 0; n < depth; n++) {
                colorfreqtableP->table[i]->tuple[depth - 1 - n]
                    = (sample)((*it >> n * 5 & 0x1f) << 3);
            }
        }
        it++;
    }

    quant_trace(stderr, "%u colors found\n", colorfreqtableP->size);

    status = SIXEL_OK;

end:
    sixel_allocator_free(allocator, refmap);
    sixel_allocator_free(allocator, histogram);

    return status;
}


static int
computeColorMapFromInput(unsigned char const *data,
                         unsigned int const length,
                         unsigned int const depth,
                         unsigned int const reqColors,
                         int const methodForLargest,
                         int const methodForRep,
                         int const qualityMode,
                         tupletable2 * const colormapP,
                         unsigned int *origcolors,
                         sixel_allocator_t *allocator)
{
/*----------------------------------------------------------------------------
   Produce a colormap containing the best colors to represent the
   image stream in file 'ifP'.  Figure it out using the median cut
   technique.

   The colormap will have 'reqcolors' or fewer colors in it, unless
   'allcolors' is true, in which case it will have all the colors that
   are in the input.

   The colormap has the same maxval as the input.

   Put the colormap in newly allocated storage as a tupletable2
   and return its address as *colormapP.  Return the number of colors in
   it as *colorsP and its maxval as *colormapMaxvalP.

   Return the characteristics of the input file as
   *formatP and *freqPamP.  (This information is not really
   relevant to our colormap mission; just a fringe benefit).
-----------------------------------------------------------------------------*/
    SIXELSTATUS status = SIXEL_FALSE;
    tupletable2 colorfreqtable = {0, NULL};
    unsigned int i;
    unsigned int n;

    status = computeHistogram(data, length, depth,
                              &colorfreqtable, qualityMode, allocator);
    if (SIXEL_FAILED(status)) {
        goto end;
    }
    if (origcolors) {
        *origcolors = colorfreqtable.size;
    }

    if (colorfreqtable.size <= reqColors) {
        quant_trace(stderr,
                    "Image already has few enough colors (<=%d).  "
                    "Keeping same colors.\n", reqColors);
        /* *colormapP = colorfreqtable; */
        colormapP->size = colorfreqtable.size;
        status = alloctupletable(&colormapP->table, depth, colorfreqtable.size, allocator);
        if (SIXEL_FAILED(status)) {
            goto end;
        }
        for (i = 0; i < colorfreqtable.size; ++i) {
            colormapP->table[i]->value = colorfreqtable.table[i]->value;
            for (n = 0; n < depth; ++n) {
                colormapP->table[i]->tuple[n] = colorfreqtable.table[i]->tuple[n];
            }
        }
    } else {
        quant_trace(stderr, "choosing %d colors...\n", reqColors);
        status = mediancut(colorfreqtable, depth, reqColors,
                           methodForLargest, methodForRep, colormapP, allocator);
        if (SIXEL_FAILED(status)) {
            goto end;
        }
        quant_trace(stderr, "%d colors are choosed.\n", colorfreqtable.size);
    }

    status = SIXEL_OK;

end:
    sixel_allocator_free(allocator, colorfreqtable.table);
    return status;
}


/* diffuse error energy to surround pixels */
static void
error_diffuse(unsigned char /* in */    *data,      /* base address of pixel buffer */
              int           /* in */    pos,        /* address of the destination pixel */
              int           /* in */    depth,      /* color depth in bytes */
              int           /* in */    error,      /* error energy */
              int           /* in */    numerator,  /* numerator of diffusion coefficient */
              int           /* in */    denominator /* denominator of diffusion coefficient */)
{
    int c;

    data += pos * depth;

    c = *data + error * numerator / denominator;
    if (c < 0) {
        c = 0;
    }
    if (c >= 1 << 8) {
        c = (1 << 8) - 1;
    }
    *data = (unsigned char)c;
}


static void
diffuse_none(unsigned char *data, int width, int height,
             int x, int y, int depth, int error)
{
    /* unused */ (void) data;
    /* unused */ (void) width;
    /* unused */ (void) height;
    /* unused */ (void) x;
    /* unused */ (void) y;
    /* unused */ (void) depth;
    /* unused */ (void) error;
}


static void
diffuse_fs(unsigned char *data, int width, int height,
           int x, int y, int depth, int error)
{
    int pos;

    pos = y * width + x;

    /* Floyd Steinberg Method
     *          curr    7/16
     *  3/16    5/48    1/16
     */
    if (x < width - 1 && y < height - 1) {
        /* add error to the right cell */
        error_diffuse(data, pos + width * 0 + 1, depth, error, 7, 16);
        /* add error to the left-bottom cell */
        error_diffuse(data, pos + width * 1 - 1, depth, error, 3, 16);
        /* add error to the bottom cell */
        error_diffuse(data, pos + width * 1 + 0, depth, error, 5, 16);
        /* add error to the right-bottom cell */
        error_diffuse(data, pos + width * 1 + 1, depth, error, 1, 16);
    }
}


static void
diffuse_atkinson(unsigned char *data, int width, int height,
                 int x, int y, int depth, int error)
{
    int pos;

    pos = y * width + x;

    /* Atkinson's Method
     *          curr    1/8    1/8
     *   1/8     1/8    1/8
     *           1/8
     */
    if (y < height - 2) {
        /* add error to the right cell */
        error_diffuse(data, pos + width * 0 + 1, depth, error, 1, 8);
        /* add error to the 2th right cell */
        error_diffuse(data, pos + width * 0 + 2, depth, error, 1, 8);
        /* add error to the left-bottom cell */
        error_diffuse(data, pos + width * 1 - 1, depth, error, 1, 8);
        /* add error to the bottom cell */
        error_diffuse(data, pos + width * 1 + 0, depth, error, 1, 8);
        /* add error to the right-bottom cell */
        error_diffuse(data, pos + width * 1 + 1, depth, error, 1, 8);
        /* add error to the 2th bottom cell */
        error_diffuse(data, pos + width * 2 + 0, depth, error, 1, 8);
    }
}


static void
diffuse_jajuni(unsigned char *data, int width, int height,
               int x, int y, int depth, int error)
{
    int pos;

    pos = y * width + x;

    /* Jarvis, Judice & Ninke Method
     *                  curr    7/48    5/48
     *  3/48    5/48    7/48    5/48    3/48
     *  1/48    3/48    5/48    3/48    1/48
     */
    if (pos < (height - 2) * width - 2) {
        error_diffuse(data, pos + width * 0 + 1, depth, error, 7, 48);
        error_diffuse(data, pos + width * 0 + 2, depth, error, 5, 48);
        error_diffuse(data, pos + width * 1 - 2, depth, error, 3, 48);
        error_diffuse(data, pos + width * 1 - 1, depth, error, 5, 48);
        error_diffuse(data, pos + width * 1 + 0, depth, error, 7, 48);
        error_diffuse(data, pos + width * 1 + 1, depth, error, 5, 48);
        error_diffuse(data, pos + width * 1 + 2, depth, error, 3, 48);
        error_diffuse(data, pos + width * 2 - 2, depth, error, 1, 48);
        error_diffuse(data, pos + width * 2 - 1, depth, error, 3, 48);
        error_diffuse(data, pos + width * 2 + 0, depth, error, 5, 48);
        error_diffuse(data, pos + width * 2 + 1, depth, error, 3, 48);
        error_diffuse(data, pos + width * 2 + 2, depth, error, 1, 48);
    }
}


static void
diffuse_stucki(unsigned char *data, int width, int height,
               int x, int y, int depth, int error)
{
    int pos;

    pos = y * width + x;

    /* Stucki's Method
     *                  curr    8/48    4/48
     *  2/48    4/48    8/48    4/48    2/48
     *  1/48    2/48    4/48    2/48    1/48
     */
    if (pos < (height - 2) * width - 2) {
        error_diffuse(data, pos + width * 0 + 1, depth, error, 1, 6);
        error_diffuse(data, pos + width * 0 + 2, depth, error, 1, 12);
        error_diffuse(data, pos + width * 1 - 2, depth, error, 1, 24);
        error_diffuse(data, pos + width * 1 - 1, depth, error, 1, 12);
        error_diffuse(data, pos + width * 1 + 0, depth, error, 1, 6);
        error_diffuse(data, pos + width * 1 + 1, depth, error, 1, 12);
        error_diffuse(data, pos + width * 1 + 2, depth, error, 1, 24);
        error_diffuse(data, pos + width * 2 - 2, depth, error, 1, 48);
        error_diffuse(data, pos + width * 2 - 1, depth, error, 1, 24);
        error_diffuse(data, pos + width * 2 + 0, depth, error, 1, 12);
        error_diffuse(data, pos + width * 2 + 1, depth, error, 1, 24);
        error_diffuse(data, pos + width * 2 + 2, depth, error, 1, 48);
    }
}


static void
diffuse_burkes(unsigned char *data, int width, int height,
               int x, int y, int depth, int error)
{
    int pos;

    pos = y * width + x;

    /* Burkes' Method
     *                  curr    4/16    2/16
     *  1/16    2/16    4/16    2/16    1/16
     */
    if (pos < (height - 1) * width - 2) {
        error_diffuse(data, pos + width * 0 + 1, depth, error, 1, 4);
        error_diffuse(data, pos + width * 0 + 2, depth, error, 1, 8);
        error_diffuse(data, pos + width * 1 - 2, depth, error, 1, 16);
        error_diffuse(data, pos + width * 1 - 1, depth, error, 1, 8);
        error_diffuse(data, pos + width * 1 + 0, depth, error, 1, 4);
        error_diffuse(data, pos + width * 1 + 1, depth, error, 1, 8);
        error_diffuse(data, pos + width * 1 + 2, depth, error, 1, 16);
    }
}

static float
mask_a (int x, int y, int c)
{
    return ((((x + c * 67) + y * 236) * 119) & 255 ) / 128.0 - 1.0;
}

static float
mask_x (int x, int y, int c)
{
    return ((((x + c * 29) ^ y* 149) * 1234) & 511 ) / 256.0 - 1.0;
}

/* lookup closest color from palette with "normal" strategy */
static int
lookup_normal(unsigned char const * const pixel,
              int const depth,
              unsigned char const * const palette,
              int const reqcolor,
              unsigned short * const cachetable,
              int const complexion)
{
    int result;
    int diff;
    int r;
    int i;
    int n;
    int distant;

    result = (-1);
    diff = INT_MAX;

    /* don't use cachetable in 'normal' strategy */
    (void) cachetable;

    for (i = 0; i < reqcolor; i++) {
        distant = 0;
        r = pixel[0] - palette[i * depth + 0];
        distant += r * r * complexion;
        for (n = 1; n < depth; ++n) {
            r = pixel[n] - palette[i * depth + n];
            distant += r * r;
        }
        if (distant < diff) {
            diff = distant;
            result = i;
        }
    }

    return result;
}


/* lookup closest color from palette with "fast" strategy */
static int
lookup_fast(unsigned char const * const pixel,
            int const depth,
            unsigned char const * const palette,
            int const reqcolor,
            unsigned short * const cachetable,
            int const complexion)
{
    int result;
    unsigned int hash;
    int diff;
    int cache;
    int i;
    int distant;

    /* don't use depth in 'fast' strategy because it's always 3 */
    (void) depth;

    result = (-1);
    diff = INT_MAX;
    hash = computeHash(pixel, 3);

    cache = cachetable[hash];
    if (cache) {  /* fast lookup */
        return cache - 1;
    }
    /* collision */
    for (i = 0; i < reqcolor; i++) {
        distant = 0;
#if 0
        for (n = 0; n < 3; ++n) {
            r = pixel[n] - palette[i * 3 + n];
            distant += r * r;
        }
#elif 1  /* complexion correction */
        distant = (pixel[0] - palette[i * 3 + 0]) * (pixel[0] - palette[i * 3 + 0]) * complexion
                + (pixel[1] - palette[i * 3 + 1]) * (pixel[1] - palette[i * 3 + 1])
                + (pixel[2] - palette[i * 3 + 2]) * (pixel[2] - palette[i * 3 + 2])
                ;
#endif
        if (distant < diff) {
            diff = distant;
            result = i;
        }
    }
    cachetable[hash] = result + 1;

    return result;
}


static int
lookup_mono_darkbg(unsigned char const * const pixel,
                   int const depth,
                   unsigned char const * const palette,
                   int const reqcolor,
                   unsigned short * const cachetable,
                   int const complexion)
{
    int n;
    int distant;

    /* unused */ (void) palette;
    /* unused */ (void) cachetable;
    /* unused */ (void) complexion;

    distant = 0;
    for (n = 0; n < depth; ++n) {
        distant += pixel[n];
    }
    return distant >= 128 * reqcolor ? 1: 0;
}


static int
lookup_mono_lightbg(unsigned char const * const pixel,
                    int const depth,
                    unsigned char const * const palette,
                    int const reqcolor,
                    unsigned short * const cachetable,
                    int const complexion)
{
    int n;
    int distant;

    /* unused */ (void) palette;
    /* unused */ (void) cachetable;
    /* unused */ (void) complexion;

    distant = 0;
    for (n = 0; n < depth; ++n) {
        distant += pixel[n];
    }
    return distant < 128 * reqcolor ? 1: 0;
}


/* choose colors using median-cut method */
SIXELSTATUS
sixel_quant_make_palette(
    unsigned char          /* out */ **result,
    unsigned char const    /* in */  *data,
    unsigned int           /* in */  length,
    int                    /* in */  pixelformat,
    unsigned int           /* in */  reqcolors,
    unsigned int           /* in */  *ncolors,
    unsigned int           /* in */  *origcolors,
    int                    /* in */  methodForLargest,
    int                    /* in */  methodForRep,
    int                    /* in */  qualityMode,
    sixel_allocator_t      /* in */  *allocator)
{
    SIXELSTATUS status = SIXEL_FALSE;
    unsigned int i;
    unsigned int n;
    int ret;
    tupletable2 colormap;
    unsigned int depth;
    int result_depth;

    result_depth = sixel_helper_compute_depth(pixelformat);
    if (result_depth <= 0) {
        *result = NULL;
        goto end;
    }

    depth = (unsigned int)result_depth;

    ret = computeColorMapFromInput(data, length, depth,
                                   reqcolors, methodForLargest,
                                   methodForRep, qualityMode,
                                   &colormap, origcolors, allocator);
    if (ret != 0) {
        *result = NULL;
        goto end;
    }
    *ncolors = colormap.size;
    quant_trace(stderr, "tupletable size: %d\n", *ncolors);
    *result = (unsigned char *)sixel_allocator_malloc(allocator, *ncolors * depth);
    for (i = 0; i < *ncolors; i++) {
        for (n = 0; n < depth; ++n) {
            (*result)[i * depth + n] = colormap.table[i]->tuple[n];
        }
    }

    sixel_allocator_free(allocator, colormap.table);

    status = SIXEL_OK;

end:
    return status;
}


/* apply color palette into specified pixel buffers */
SIXELSTATUS
sixel_quant_apply_palette(
    sixel_index_t     /* out */ *result,
    unsigned char     /* in */  *data,
    int               /* in */  width,
    int               /* in */  height,
    int               /* in */  depth,
    unsigned char     /* in */  *palette,
    int               /* in */  reqcolor,
    int               /* in */  methodForDiffuse,
    int               /* in */  foptimize,
    int               /* in */  foptimize_palette,
    int               /* in */  complexion,
    unsigned short    /* in */  *cachetable,
    int               /* in */  *ncolors,
    sixel_allocator_t /* in */  *allocator)
{
    typedef int component_t;
    enum { max_depth = 4 };
    enum { max_channel_diff_sq = 255 * 255 };
    SIXELSTATUS status = SIXEL_FALSE;
    int pos, n, x, y, sum1, sum2;
    int non_weighted_components;
    component_t offset;
    int color_index;
    long long max_complexion;
    unsigned short *indextable;
    unsigned char new_palette[SIXEL_PALETTE_MAX * 4];
    unsigned short migration_map[SIXEL_PALETTE_MAX];
    float (*f_mask) (int x, int y, int c) = NULL;
    void (*f_diffuse)(unsigned char *data, int width, int height,
                      int x, int y, int depth, int offset);
    int (*f_lookup)(unsigned char const * const pixel,
                    int const depth,
                    unsigned char const * const palette,
                    int const reqcolor,
                    unsigned short * const cachetable,
                    int const complexion);

    /* check bad reqcolor */
    if (reqcolor < 1) {
        status = SIXEL_BAD_ARGUMENT;
        sixel_helper_set_additional_message(
            "sixel_quant_apply_palette: "
            "a bad argument is detected, reqcolor < 0.");
        goto end;
    }

    /* NOTE: diffuse_jajuni, diffuse_stucki, and diffuse_burkes reference at
     * minimum the position pos + width * 1 - 2, so width must be at least 2
     * to avoid underflow.
     * On the other hand, diffuse_fs and diffuse_atkinson
     * reference pos + width * 1 - 1, but since these functions are only called
     * when width >= 1, they do not cause underflow.
     */
    if (depth != 3) {
        f_diffuse = diffuse_none;
    } else {
        switch (methodForDiffuse) {
        case SIXEL_DIFFUSE_NONE:
            f_diffuse = diffuse_none;
            break;
        case SIXEL_DIFFUSE_ATKINSON:
            f_diffuse = diffuse_atkinson;
            break;
        case SIXEL_DIFFUSE_FS:
            f_diffuse = diffuse_fs;
            break;
        case SIXEL_DIFFUSE_JAJUNI:
            /* fallback to diffuse_none if width < 2 */
            f_diffuse = width >= 2 ? diffuse_jajuni: diffuse_none;
            break;
        case SIXEL_DIFFUSE_STUCKI:
            /* fallback to diffuse_none if width < 2 */
            f_diffuse = width >= 2 ? diffuse_stucki: diffuse_none;
            break;
        case SIXEL_DIFFUSE_BURKES:
            /* fallback to diffuse_none if width < 2 */
            f_diffuse = width >= 2 ? diffuse_burkes: diffuse_none;
            break;
        case SIXEL_DIFFUSE_A_DITHER:
            f_diffuse = diffuse_none;
            f_mask = mask_a;
            break;
        case SIXEL_DIFFUSE_X_DITHER:
            f_diffuse = diffuse_none;
            f_mask = mask_x;
            break;
        default:
            quant_trace(stderr, "Internal error: invalid value of"
                                " methodForDiffuse: %d\n",
                        methodForDiffuse);
            f_diffuse = diffuse_none;
            break;
        }
    }

    f_lookup = NULL;
    if (reqcolor == 2) {
        sum1 = 0;
        sum2 = 0;
        for (n = 0; n < depth; ++n) {
            sum1 += palette[n];
        }
        for (n = depth; n < depth + depth; ++n) {
            sum2 += palette[n];
        }
        if (sum1 == 0 && sum2 == 255 * 3) {
            f_lookup = lookup_mono_darkbg;
        } else if (sum1 == 255 * 3 && sum2 == 0) {
            f_lookup = lookup_mono_lightbg;
        }
    }
    if (f_lookup == NULL) {
        if (foptimize && depth == 3) {
            f_lookup = lookup_fast;
        } else {
            f_lookup = lookup_normal;
        }
    }

    if ((f_lookup == lookup_fast || f_lookup == lookup_normal) && complexion > 1) {
        non_weighted_components = depth > 1 ? depth - 1 : 0;
        max_complexion = (INT_MAX - (long long)max_channel_diff_sq
                          * (long long)non_weighted_components)
                         / (long long)max_channel_diff_sq;
        if ((long long)complexion > max_complexion) {
            status = SIXEL_BAD_ARGUMENT;
            sixel_helper_set_additional_message(
                "sixel_quant_apply_palette: complexion parameter is too large.");
            goto end;
        }
    }

    indextable = cachetable;
    if (cachetable == NULL && f_lookup == lookup_fast) {
        indextable = (unsigned short *)sixel_allocator_calloc(allocator,
                                                              (size_t)(1 << depth * 5),
                                                              sizeof(unsigned short));
        if (!indextable) {
            quant_trace(stderr, "Unable to allocate memory for indextable.\n");
            goto end;
        }
    }

    if (foptimize_palette) {
        *ncolors = 0;

        memset(new_palette, 0x00, sizeof(SIXEL_PALETTE_MAX * depth));
        memset(migration_map, 0x00, sizeof(migration_map));

        if (f_mask) {
            for (y = 0; y < height; ++y) {
                for (x = 0; x < width; ++x) {
                    unsigned char copy[max_depth];
                    int d;
                    int val;

                    pos = y * width + x;
                    for (d = 0; d < depth; d ++) {
                        val = data[pos * depth + d] + f_mask(x, y, d) * 32;
                        copy[d] = val < 0 ? 0 : val > 255 ? 255 : val;
                    }
                    color_index = f_lookup(copy, depth,
                                           palette, reqcolor, indextable, complexion);
                    if (migration_map[color_index] == 0) {
                        result[pos] = *ncolors;
                        for (n = 0; n < depth; ++n) {
                            new_palette[*ncolors * depth + n] = palette[color_index * depth + n];
                        }
                        ++*ncolors;
                        migration_map[color_index] = *ncolors;
                    } else {
                        result[pos] = migration_map[color_index] - 1;
                    }
                }
            }
            memcpy(palette, new_palette, (size_t)(*ncolors * depth));
        } else {
            for (y = 0; y < height; ++y) {
                for (x = 0; x < width; ++x) {
                    pos = y * width + x;
                    color_index = f_lookup(data + (pos * depth), depth,
                                           palette, reqcolor, indextable, complexion);
                    if (migration_map[color_index] == 0) {
                        result[pos] = *ncolors;
                        for (n = 0; n < depth; ++n) {
                            new_palette[*ncolors * depth + n] = palette[color_index * depth + n];
                        }
                        ++*ncolors;
                        migration_map[color_index] = *ncolors;
                    } else {
                        result[pos] = migration_map[color_index] - 1;
                    }
                    for (n = 0; n < depth; ++n) {
                        offset = data[pos * depth + n] - palette[color_index * depth + n];
                        f_diffuse(data + n, width, height, x, y, depth, offset);
                    }
                }
            }
            memcpy(palette, new_palette, (size_t)(*ncolors * depth));
        }
    } else {
        if (f_mask) {
            for (y = 0; y < height; ++y) {
                for (x = 0; x < width; ++x) {
                    unsigned char copy[max_depth];
                    int d;
                    int val;

                    pos = y * width + x;
                    for (d = 0; d < depth; d ++) {
                        val = data[pos * depth + d] + f_mask(x, y, d) * 32;
                        copy[d] = val < 0 ? 0 : val > 255 ? 255 : val;
                    }
                    result[pos] = f_lookup(copy, depth,
                                           palette, reqcolor, indextable, complexion);
                }
            }
        } else {
            for (y = 0; y < height; ++y) {
                for (x = 0; x < width; ++x) {
                    pos = y * width + x;
                    color_index = f_lookup(data + (pos * depth), depth,
                                           palette, reqcolor, indextable, complexion);
                    result[pos] = color_index;
                    for (n = 0; n < depth; ++n) {
                        offset = data[pos * depth + n] - palette[color_index * depth + n];
                        f_diffuse(data + n, width, height, x, y, depth, offset);
                    }
                }
            }
        }
        *ncolors = reqcolor;
    }

    if (cachetable == NULL) {
        sixel_allocator_free(allocator, indextable);
    }

    status = SIXEL_OK;

end:
    return status;
}


void
sixel_quant_free_palette(
    unsigned char       /* in */ *data,
    sixel_allocator_t   /* in */ *allocator)
{
    sixel_allocator_free(allocator, data);
}


#if HAVE_TESTS
static int
test1(void)
{
    int nret = EXIT_FAILURE;
    sample minval[1] = { 1 };
    sample maxval[1] = { 2 };
    unsigned int retval;

    retval = largestByLuminosity(minval, maxval, 1);
    if (retval != 0) {
        goto error;
    }
    nret = EXIT_SUCCESS;

error:
    return nret;
}


SIXELAPI int
sixel_quant_tests_main(void)
{
    int nret = EXIT_FAILURE;
    size_t i;
    typedef int (* testcase)(void);

    static testcase const testcases[] = {
        test1,
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
