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

#ifndef LIBSIXEL_OUTPUT_H
#define LIBSIXEL_OUTPUT_H

typedef struct sixel_node {
    struct sixel_node *next;
    int pal;
    int sx;
    int mx;
    char *map;
} sixel_node_t;

struct sixel_output {

    int ref;
    sixel_allocator_t *allocator;

    /* compatiblity flags */

    /* 0: 7bit terminal,
     * 1: 8bit terminal */
    unsigned char has_8bit_control;

    /* 0: the terminal has sixel scrolling
     * 1: the terminal does not have sixel scrolling */
    unsigned char has_sixel_scrolling;

    /* 1: the argument of repeat introducer(DECGRI) is not limitted
       0: the argument of repeat introducer(DECGRI) is limitted 255 */
    unsigned char has_gri_arg_limit;

    /* 0: DECSDM set (CSI ? 80 h) enables sixel scrolling
       1: DECSDM set (CSI ? 80 h) disables sixel scrolling */
    unsigned char has_sdm_glitch;

    /* 0: do not skip DCS envelope
     * 1: skip DCS envelope */
    unsigned char skip_dcs_envelope;

    /* PALETTETYPE_AUTO: select palette type automatically
     * PALETTETYPE_HLS : HLS color space
     * PALETTETYPE_RGB : RGB color space */
    unsigned char palette_type;

    sixel_write_function fn_write;

    int save_pixel;
    int save_count;
    int active_palette;

    sixel_node_t *node_top;
    sixel_node_t *node_free;

    int penetrate_multiplexer;
    int encode_policy;

    void *priv;
    int pos;
    unsigned char buffer[1];
};

#endif /* LIBSIXEL_OUTPUT_H */

/* emacs Local Variables:      */
/* emacs mode: c               */
/* emacs tab-width: 4          */
/* emacs indent-tabs-mode: nil */
/* emacs c-basic-offset: 4     */
/* emacs End:                  */
/* vim: set expandtab ts=4 sts=4 sw=4 : */
/* EOF */
