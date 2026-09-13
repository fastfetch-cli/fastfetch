/*
 * this file is derived from "sixel" original version (2014-3-2)
 * http://nanno.dip.jp/softlib/man/rlogin/sixel.tar.gz
 *
 * Initial developer of this file is kmiya@culti.
 *
 * He distributes it under very permissive license which permits
 * useing, copying, modification, redistribution, and all other
 * public activities without any restrictions.
 *
 * He declares this is compatible with MIT/BSD/GPL.
 *
 * Hayaki Saito (saitoha@me.com) modified this and re-licensed
 * it under the MIT license.
 *
 * Araki Ken added high-color encoding mode(sixel_encode_highcolor)
 * extension.
 *
 */
#include "config.h"

/* STDC_HEADERS */
#include <stdio.h>
#include <stdlib.h>

#if HAVE_STRING_H
# include <string.h>
#endif  /* HAVE_STRING_H */
#if HAVE_LIMITS_H
# include <limits.h>
#endif  /* HAVE_LIMITS_H */
#if HAVE_INTTYPES_H
# include <inttypes.h>
#endif  /* HAVE_INTTYPES_H */

#include <sixel.h>
#include "output.h"
#include "dither.h"

#define DCS_START_7BIT       "\033P"
#define DCS_START_7BIT_SIZE  (sizeof(DCS_START_7BIT) - 1)
#define DCS_START_8BIT       "\220"
#define DCS_START_8BIT_SIZE  (sizeof(DCS_START_8BIT) - 1)
#define DCS_END_7BIT         "\033\\"
#define DCS_END_7BIT_SIZE    (sizeof(DCS_END_7BIT) - 1)
#define DCS_END_8BIT         "\234"
#define DCS_END_8BIT_SIZE    (sizeof(DCS_END_8BIT) - 1)
#define DCS_7BIT(x)          DCS_START_7BIT x DCS_END_7BIT
#define DCS_8BIT(x)          DCS_START_8BIT x DCS_END_8BIT
#define SCREEN_PACKET_SIZE   256

enum {
    PALETTE_HIT    = 1,
    PALETTE_CHANGE = 2
};

/* implementation */

/* GNU Screen penetration */
static void
sixel_penetrate(
    sixel_output_t  /* in */    *output,        /* output context */
    int             /* in */    nwrite,         /* output size */
    char const      /* in */    *dcs_start,     /* DCS introducer */
    char const      /* in */    *dcs_end,       /* DCS terminator */
    int const       /* in */    dcs_start_size, /* size of DCS introducer */
    int const       /* in */    dcs_end_size)   /* size of DCS terminator */
{
    int pos;
    int const splitsize = SCREEN_PACKET_SIZE
                        - dcs_start_size - dcs_end_size;

    for (pos = 0; pos < nwrite; pos += splitsize) {
        output->fn_write((char *)dcs_start, dcs_start_size, output->priv);
        output->fn_write(((char *)output->buffer) + pos,
                          nwrite - pos < splitsize ? nwrite - pos: splitsize,
                          output->priv);
        output->fn_write((char *)dcs_end, dcs_end_size, output->priv);
    }
}


static void
sixel_advance(sixel_output_t *output, int nwrite)
{
    if ((output->pos += nwrite) >= SIXEL_OUTPUT_PACKET_SIZE) {
        if (output->penetrate_multiplexer) {
            sixel_penetrate(output,
                            SIXEL_OUTPUT_PACKET_SIZE,
                            DCS_START_7BIT,
                            DCS_END_7BIT,
                            DCS_START_7BIT_SIZE,
                            DCS_END_7BIT_SIZE);
        } else {
            output->fn_write((char *)output->buffer,
                             SIXEL_OUTPUT_PACKET_SIZE, output->priv);
        }
        memcpy(output->buffer,
               output->buffer + SIXEL_OUTPUT_PACKET_SIZE,
               (size_t)(output->pos -= SIXEL_OUTPUT_PACKET_SIZE));
    }
}


static void
sixel_putc(unsigned char *buffer, unsigned char value)
{
    *buffer = value;
}


static void
sixel_puts(unsigned char *buffer, char const *value, int size)
{
    memcpy(buffer, (void *)value, (size_t)size);
}


#if HAVE_LDIV
static int
sixel_putnum_impl(char *buffer, long value, int pos)
{
    ldiv_t r;

    r = ldiv(value, 10);
    if (r.quot > 0) {
        pos = sixel_putnum_impl(buffer, r.quot, pos);
    }
    *(buffer + pos) = '0' + r.rem;
    return pos + 1;
}
#endif  /* HAVE_LDIV */


static int
sixel_putnum(char *buffer, int value)
{
    int pos;

#if HAVE_LDIV
    pos = sixel_putnum_impl(buffer, value, 0);
#else
    pos = sprintf(buffer, "%d", value);
#endif  /* HAVE_LDIV */

    return pos;
}


static SIXELSTATUS
sixel_put_flash(sixel_output_t *const output)
{
    int n;
    int nwrite;

    if (output->has_gri_arg_limit) {  /* VT240 Max 255 ? */
        while (output->save_count > 255) {
            /* argument of DECGRI('!') is limitted to 255 in real VT */
            sixel_puts(output->buffer + output->pos, "!255", 4);
            sixel_advance(output, 4);
            sixel_putc(output->buffer + output->pos, output->save_pixel);
            sixel_advance(output, 1);
            output->save_count -= 255;
        }
    }

    if (output->save_count > 3) {
        /* DECGRI Graphics Repeat Introducer ! Pn Ch */
        sixel_putc(output->buffer + output->pos, '!');
        sixel_advance(output, 1);
        nwrite = sixel_putnum((char *)output->buffer + output->pos, output->save_count);
        sixel_advance(output, nwrite);
        sixel_putc(output->buffer + output->pos, output->save_pixel);
        sixel_advance(output, 1);
    } else {
        for (n = 0; n < output->save_count; n++) {
            output->buffer[output->pos] = output->save_pixel;
            sixel_advance(output, 1);
        }
    }

    output->save_pixel = 0;
    output->save_count = 0;

    return 0;
}


static SIXELSTATUS
sixel_put_pixel(sixel_output_t *const output, int pix)
{
    SIXELSTATUS status = SIXEL_FALSE;

    if (pix < 0 || pix > '?') {
        pix = 0;
    }

    pix += '?';

    if (pix == output->save_pixel) {
        output->save_count++;
    } else {
        status = sixel_put_flash(output);
        if (SIXEL_FAILED(status)) {
            goto end;
        }
        output->save_pixel = pix;
        output->save_count = 1;
    }

    status = SIXEL_OK;

end:
    return status;
}

static SIXELSTATUS
sixel_node_new(sixel_node_t **np, sixel_allocator_t *allocator)
{
    SIXELSTATUS status = SIXEL_FALSE;

    *np = (sixel_node_t *)sixel_allocator_malloc(allocator,
                                                 sizeof(sixel_node_t));
    if (np == NULL) {
        sixel_helper_set_additional_message(
            "sixel_node_new: sixel_allocator_malloc() failed.");
        status = SIXEL_BAD_ALLOCATION;
        goto end;
    }

    status = SIXEL_OK;

end:
    return status;
}

static void
sixel_node_del(sixel_output_t *output, sixel_node_t *np)
{
    sixel_node_t *tp;

    if ((tp = output->node_top) == np) {
        output->node_top = np->next;
    } else {
        while (tp->next != NULL) {
            if (tp->next == np) {
                tp->next = np->next;
                break;
            }
            tp = tp->next;
        }
    }

    np->next = output->node_free;
    output->node_free = np;
}


static SIXELSTATUS
sixel_put_node(
    sixel_output_t /* in */     *output,  /* output context */
    int            /* in/out */ *x,       /* header position */
    sixel_node_t   /* in */     *np,      /* node object */
    int            /* in */     ncolors,  /* number of palette colors */
    int            /* in */     keycolor) /* transparent color number */
{
    SIXELSTATUS status = SIXEL_FALSE;
    int nwrite;

    if (ncolors != 2 || keycolor == (-1)) {
        /* designate palette index */
        if (output->active_palette != np->pal) {
            sixel_putc(output->buffer + output->pos, '#');
            sixel_advance(output, 1);
            nwrite = sixel_putnum((char *)output->buffer + output->pos, np->pal);
            sixel_advance(output, nwrite);
            output->active_palette = np->pal;
        }
    }

    for (; *x < np->sx; ++*x) {
        status = sixel_put_pixel(output, 0);
        if (SIXEL_FAILED(status)) {
            goto end;
        }
    }

    for (; *x < np->mx; ++*x) {
        status = sixel_put_pixel(output, np->map[*x]);
        if (SIXEL_FAILED(status)) {
            goto end;
        }
    }

    status = sixel_put_flash(output);
    if (SIXEL_FAILED(status)) {
        goto end;
    }

end:
    return status;
}


static SIXELSTATUS
sixel_encode_header(int width, int height, sixel_output_t *output)
{
    SIXELSTATUS status = SIXEL_FALSE;
    int nwrite;
    int p[3] = {0, 0, 0};
    int pcount = 3;
    int use_raster_attributes = 1;

    output->pos = 0;

    if (!output->skip_dcs_envelope) {
        if (output->has_8bit_control) {
            sixel_puts(output->buffer + output->pos,
                       DCS_START_8BIT,
                       DCS_START_8BIT_SIZE);
            sixel_advance(output, DCS_START_8BIT_SIZE);
        } else {
            sixel_puts(output->buffer + output->pos,
                       DCS_START_7BIT,
                       DCS_START_7BIT_SIZE);
            sixel_advance(output, DCS_START_7BIT_SIZE);
        }
    }

    if (p[2] == 0) {
        pcount--;
        if (p[1] == 0) {
            pcount--;
            if (p[0] == 0) {
                pcount--;
            }
        }
    }

    if (pcount > 0) {
        nwrite = sixel_putnum((char *)output->buffer + output->pos, p[0]);
        sixel_advance(output, nwrite);
        if (pcount > 1) {
            sixel_putc(output->buffer + output->pos, ';');
            sixel_advance(output, 1);
            nwrite = sixel_putnum((char *)output->buffer + output->pos, p[1]);
            sixel_advance(output, nwrite);
            if (pcount > 2) {
                sixel_putc(output->buffer + output->pos, ';');
                sixel_advance(output, 1);
                nwrite = sixel_putnum((char *)output->buffer + output->pos, p[2]);
                sixel_advance(output, nwrite);
            }
        }
    }

    sixel_putc(output->buffer + output->pos, 'q');
    sixel_advance(output, 1);

    if (use_raster_attributes) {
        sixel_puts(output->buffer + output->pos, "\"1;1;", 5);
        sixel_advance(output, 5);
        nwrite = sixel_putnum((char *)output->buffer + output->pos, width);
        sixel_advance(output, nwrite);
        sixel_putc(output->buffer + output->pos, ';');
        sixel_advance(output, 1);
        nwrite = sixel_putnum((char *)output->buffer + output->pos, height);
        sixel_advance(output, nwrite);
    }

    status = SIXEL_OK;

    return status;
}


static SIXELSTATUS
output_rgb_palette_definition(
    sixel_output_t /* in */ *output,
    unsigned char  /* in */ *palette,
    int            /* in */ n,
    int            /* in */ keycolor
)
{
    SIXELSTATUS status = SIXEL_FALSE;
    int nwrite;

    if (n != keycolor) {
        /* DECGCI Graphics Color Introducer  # Pc ; Pu; Px; Py; Pz */
        sixel_putc(output->buffer + output->pos, '#');
        sixel_advance(output, 1);
        nwrite = sixel_putnum((char *)output->buffer + output->pos, n);
        sixel_advance(output, nwrite);
        sixel_puts(output->buffer + output->pos, ";2;", 3);
        sixel_advance(output, 3);
        nwrite = sixel_putnum((char *)output->buffer + output->pos,
                              (palette[n * 3 + 0] * 100 + 127) / 255);
        sixel_advance(output, nwrite);
        sixel_putc(output->buffer + output->pos, ';');
        sixel_advance(output, 1);
        nwrite = sixel_putnum((char *)output->buffer + output->pos,
                              (palette[n * 3 + 1] * 100 + 127) / 255);
        sixel_advance(output, nwrite);
        sixel_putc(output->buffer + output->pos, ';');
        sixel_advance(output, 1);
        nwrite = sixel_putnum((char *)output->buffer + output->pos,
                              (palette[n * 3 + 2] * 100 + 127) / 255);
        sixel_advance(output, nwrite);
    }

    status = SIXEL_OK;

    return status;
}


static SIXELSTATUS
output_hls_palette_definition(
    sixel_output_t /* in */ *output,
    unsigned char  /* in */ *palette,
    int            /* in */ n,
    int            /* in */ keycolor
)
{
    SIXELSTATUS status = SIXEL_FALSE;
    int h;
    int l;
    int s;
    int r;
    int g;
    int b;
    int max;
    int min;
    int nwrite;

    if (n != keycolor) {
        r = palette[n * 3 + 0];
        g = palette[n * 3 + 1];
        b = palette[n * 3 + 2];
        max = r > g ? (r > b ? r: b): (g > b ? g: b);
        min = r < g ? (r < b ? r: b): (g < b ? g: b);
        l = ((max + min) * 100 + 255) / 510;
        if (max == min) {
            h = s = 0;
        } else {
            if (l < 50) {
                s = ((max - min) * 100) / (max + min);
            } else {
                s = ((max - min) * 100) / ((255 - max) + (255 - min));
            }
            if (r == max) {
                h = 120 + (g - b) * 60 / (max - min);
            } else if (g == max) {
                h = 240 + (b - r) * 60 / (max - min);
            } else if (r < g) /* if (b == max) */ {
                h = 360 + (r - g) * 60 / (max - min);
            } else {
                h = 0 + (r - g) * 60 / (max - min);
            }
        }
        /* DECGCI Graphics Color Introducer  # Pc ; Pu; Px; Py; Pz */
        sixel_putc(output->buffer + output->pos, '#');
        sixel_advance(output, 1);
        nwrite = sixel_putnum((char *)output->buffer + output->pos, n);
        sixel_advance(output, nwrite);
        sixel_puts(output->buffer + output->pos, ";1;", 3);
        sixel_advance(output, 3);
        nwrite = sixel_putnum((char *)output->buffer + output->pos, h);
        sixel_advance(output, nwrite);
        sixel_putc(output->buffer + output->pos, ';');
        sixel_advance(output, 1);
        nwrite = sixel_putnum((char *)output->buffer + output->pos, l);
        sixel_advance(output, nwrite);
        sixel_putc(output->buffer + output->pos, ';');
        sixel_advance(output, 1);
        nwrite = sixel_putnum((char *)output->buffer + output->pos, s);
        sixel_advance(output, nwrite);
    }

    status = SIXEL_OK;
    return status;
}


static SIXELSTATUS
sixel_encode_body(
    sixel_index_t       /* in */ *pixels,
    int                 /* in */ width,
    int                 /* in */ height,
    unsigned char       /* in */ *palette,
    int                 /* in */ ncolors,
    int                 /* in */ keycolor,
    int                 /* in */ bodyonly,
    sixel_output_t      /* in */ *output,
    unsigned char       /* in */ *palstate,
    sixel_allocator_t   /* in */ *allocator)
{
    SIXELSTATUS status = SIXEL_FALSE;
    int x;
    int y;
    int i;
    int n;
    int c;
    int sx;
    int mx;
    int len;
    int pix;
    char *map = NULL;
    int check_integer_overflow;
    sixel_node_t *np, *tp, top;
    int fillable;

    if (ncolors < 1) {
        status = SIXEL_BAD_ARGUMENT;
        goto end;
    }
    len = ncolors * width;
    output->active_palette = (-1);

    map = (char *)sixel_allocator_calloc(allocator,
                                         (size_t)len,
                                         sizeof(char));
    if (map == NULL) {
        sixel_helper_set_additional_message(
            "sixel_encode_body: sixel_allocator_calloc() failed.");
        status = SIXEL_BAD_ALLOCATION;
        goto end;
    }

    if (!bodyonly && (ncolors != 2 || keycolor == (-1))) {
        if (output->palette_type == SIXEL_PALETTETYPE_HLS) {
            for (n = 0; n < ncolors; n++) {
                status = output_hls_palette_definition(output, palette, n, keycolor);
                if (SIXEL_FAILED(status)) {
                    goto end;
                }
            }
        } else {
            for (n = 0; n < ncolors; n++) {
                status = output_rgb_palette_definition(output, palette, n, keycolor);
                if (SIXEL_FAILED(status)) {
                    goto end;
                }
            }
        }
    }

    for (y = i = 0; y < height; y++) {
        if (output->encode_policy != SIXEL_ENCODEPOLICY_SIZE) {
            fillable = 0;
        } else if (palstate) {
            /* high color sixel */
            pix = pixels[(y - i) * width];
            if (pix >= ncolors) {
                fillable = 0;
            } else {
                fillable = 1;
            }
        } else {
            /* normal sixel */
            fillable = 1;
        }
        for (x = 0; x < width; x++) {
            if (y > INT_MAX / width) {
                /* integer overflow */
                sixel_helper_set_additional_message(
                    "sixel_encode_body: integer overflow detected."
                    " (y > INT_MAX)");
                status = SIXEL_BAD_INTEGER_OVERFLOW;
                goto end;
            }
            check_integer_overflow = y * width;
            if (check_integer_overflow > INT_MAX - x) {
                /* integer overflow */
                sixel_helper_set_additional_message(
                    "sixel_encode_body: integer overflow detected."
                    " (y * width > INT_MAX - x)");
                status = SIXEL_BAD_INTEGER_OVERFLOW;
                goto end;
            }
            pix = pixels[check_integer_overflow + x];  /* color index */
            if (pix >= 0 && pix < ncolors && pix != keycolor) {
                if (pix > INT_MAX / width) {
                    /* integer overflow */
                    sixel_helper_set_additional_message(
                        "sixel_encode_body: integer overflow detected."
                        " (pix > INT_MAX / width)");
                    status = SIXEL_BAD_INTEGER_OVERFLOW;
                    goto end;
                }
                check_integer_overflow = pix * width;
                if (check_integer_overflow > INT_MAX - x) {
                    /* integer overflow */
                    sixel_helper_set_additional_message(
                        "sixel_encode_body: integer overflow detected."
                        " (pix * width > INT_MAX - x)");
                    status = SIXEL_BAD_INTEGER_OVERFLOW;
                    goto end;
                }
                map[pix * width + x] |= (1 << i);
            }
            else if (!palstate) {
                fillable = 0;
            }
        }

        if (++i < 6 && (y + 1) < height) {
            continue;
        }

        for (c = 0; c < ncolors; c++) {
            for (sx = 0; sx < width; sx++) {
                if (*(map + c * width + sx) == 0) {
                    continue;
                }

                for (mx = sx + 1; mx < width; mx++) {
                    if (*(map + c * width + mx) != 0) {
                        continue;
                    }

                    for (n = 1; (mx + n) < width; n++) {
                        if (*(map + c * width + mx + n) != 0) {
                            break;
                        }
                    }

                    if (n >= 10 || (mx + n) >= width) {
                        break;
                    }
                    mx = mx + n - 1;
                }

                if ((np = output->node_free) != NULL) {
                    output->node_free = np->next;
                } else {
                    status = sixel_node_new(&np, allocator);
                    if (SIXEL_FAILED(status)) {
                        goto end;
                    }
                }

                np->pal = c;
                np->sx = sx;
                np->mx = mx;
                np->map = map + c * width;

                top.next = output->node_top;
                tp = &top;

                while (tp->next != NULL) {
                    if (np->sx < tp->next->sx) {
                        break;
                    } else if (np->sx == tp->next->sx && np->mx > tp->next->mx) {
                        break;
                    }
                    tp = tp->next;
                }

                np->next = tp->next;
                tp->next = np;
                output->node_top = top.next;

                sx = mx - 1;
            }

        }

        if (y != 5) {
            /* DECGNL Graphics Next Line */
            output->buffer[output->pos] = '-';
            sixel_advance(output, 1);
        }

        for (x = 0; (np = output->node_top) != NULL;) {
            sixel_node_t *next;
            if (x > np->sx) {
                /* DECGCR Graphics Carriage Return */
                output->buffer[output->pos] = '$';
                sixel_advance(output, 1);
                x = 0;
            }

            if (fillable) {
                memset(np->map + np->sx, (1 << i) - 1, (size_t)(np->mx - np->sx));
            }
            status = sixel_put_node(output, &x, np, ncolors, keycolor);
            if (SIXEL_FAILED(status)) {
                goto end;
            }
            next = np->next;
            sixel_node_del(output, np);
            np = next;

            while (np != NULL) {
                if (np->sx < x) {
                    np = np->next;
                    continue;
                }

                if (fillable) {
                    memset(np->map + np->sx, (1 << i) - 1, (size_t)(np->mx - np->sx));
                }
                status = sixel_put_node(output, &x, np, ncolors, keycolor);
                if (SIXEL_FAILED(status)) {
                    goto end;
                }
                next = np->next;
                sixel_node_del(output, np);
                np = next;
            }

            fillable = 0;
        }

        i = 0;
        memset(map, 0, (size_t)len);
    }

    if (palstate) {
        output->buffer[output->pos] = '$';
        sixel_advance(output, 1);
    }

    status = SIXEL_OK;

end:
    /* free nodes */
    while ((np = output->node_free) != NULL) {
        output->node_free = np->next;
        sixel_allocator_free(allocator, np);
    }
    output->node_top = NULL;

    sixel_allocator_free(allocator, map);

    return status;
}


static SIXELSTATUS
sixel_encode_footer(sixel_output_t *output)
{
    SIXELSTATUS status = SIXEL_FALSE;

    if (!output->skip_dcs_envelope && !output->penetrate_multiplexer) {
        if (output->has_8bit_control) {
            sixel_puts(output->buffer + output->pos,
                       DCS_END_8BIT, DCS_END_8BIT_SIZE);
            sixel_advance(output, DCS_END_8BIT_SIZE);
        } else {
            sixel_puts(output->buffer + output->pos,
                       DCS_END_7BIT, DCS_END_7BIT_SIZE);
            sixel_advance(output, DCS_END_7BIT_SIZE);
        }
    }

    /* flush buffer */
    if (output->pos > 0) {
        if (output->penetrate_multiplexer) {
            sixel_penetrate(output, output->pos,
                            DCS_START_7BIT,
                            DCS_END_7BIT,
                            DCS_START_7BIT_SIZE,
                            DCS_END_7BIT_SIZE);
            output->fn_write((char *)DCS_7BIT("\033") DCS_7BIT("\\"),
                             (DCS_START_7BIT_SIZE + 1 + DCS_END_7BIT_SIZE) * 2,
                             output->priv);
        } else {
            output->fn_write((char *)output->buffer, output->pos, output->priv);
        }
    }

    status = SIXEL_OK;

    return status;
}


static SIXELSTATUS
sixel_encode_dither(
    unsigned char   /* in */ *pixels,   /* pixel bytes to be encoded */
    int             /* in */ width,     /* width of source image */
    int             /* in */ height,    /* height of source image */
    sixel_dither_t  /* in */ *dither,   /* dither context */
    sixel_output_t  /* in */ *output)   /* output context */
{
    SIXELSTATUS status = SIXEL_FALSE;
    sixel_index_t *paletted_pixels = NULL;
    sixel_index_t *input_pixels;
    size_t bufsize;

    switch (dither->pixelformat) {
    case SIXEL_PIXELFORMAT_PAL1:
    case SIXEL_PIXELFORMAT_PAL2:
    case SIXEL_PIXELFORMAT_PAL4:
    case SIXEL_PIXELFORMAT_G1:
    case SIXEL_PIXELFORMAT_G2:
    case SIXEL_PIXELFORMAT_G4:
        bufsize = (sizeof(sixel_index_t) * (size_t)width * (size_t)height * 3UL);
        paletted_pixels = (sixel_index_t *)sixel_allocator_malloc(dither->allocator, bufsize);
        if (paletted_pixels == NULL) {
            sixel_helper_set_additional_message(
                "sixel_encode_dither: sixel_allocator_malloc() failed.");
            status = SIXEL_BAD_ALLOCATION;
            goto end;
        }
        status = sixel_helper_normalize_pixelformat(paletted_pixels,
                                                    &dither->pixelformat,
                                                    pixels,
                                                    dither->pixelformat,
                                                    width, height);
        if (SIXEL_FAILED(status)) {
            goto end;
        }
        input_pixels = paletted_pixels;
        break;
    case SIXEL_PIXELFORMAT_PAL8:
    case SIXEL_PIXELFORMAT_G8:
    case SIXEL_PIXELFORMAT_GA88:
    case SIXEL_PIXELFORMAT_AG88:
        input_pixels = pixels;
        break;
    default:
        /* apply palette */
        paletted_pixels = sixel_dither_apply_palette(dither, pixels,
                                                     width, height);
        if (paletted_pixels == NULL) {
            status = SIXEL_RUNTIME_ERROR;
            goto end;
        }
        input_pixels = paletted_pixels;
        break;
    }

    status = sixel_encode_header(width, height, output);
    if (SIXEL_FAILED(status)) {
        goto end;
    }

    status = sixel_encode_body(input_pixels,
                               width,
                               height,
                               dither->palette,
                               dither->ncolors,
                               dither->keycolor,
                               dither->bodyonly,
                               output,
                               NULL,
                               dither->allocator);
    if (SIXEL_FAILED(status)) {
        goto end;
    }

    status = sixel_encode_footer(output);
    if (SIXEL_FAILED(status)) {
        goto end;
    }

end:
    sixel_allocator_free(dither->allocator, paletted_pixels);

    return status;
}

static void
dither_func_none(unsigned char *data, int width)
{
    (void) data;  /* unused */
    (void) width; /* unused */
}


static void
dither_func_fs(unsigned char *data, int width)
{
    int r, g, b;
    int error_r = data[0] & 0x7;
    int error_g = data[1] & 0x7;
    int error_b = data[2] & 0x7;

    /* Floyd Steinberg Method
     *          curr    7/16
     *  3/16    5/48    1/16
     */
    r = (data[3 + 0] + (error_r * 5 >> 4));
    g = (data[3 + 1] + (error_g * 5 >> 4));
    b = (data[3 + 2] + (error_b * 5 >> 4));
    data[3 + 0] = r > 0xff ? 0xff: r;
    data[3 + 1] = g > 0xff ? 0xff: g;
    data[3 + 2] = b > 0xff ? 0xff: b;
    r = data[width * 3 - 3 + 0] + (error_r * 3 >> 4);
    g = data[width * 3 - 3 + 1] + (error_g * 3 >> 4);
    b = data[width * 3 - 3 + 2] + (error_b * 3 >> 4);
    data[width * 3 - 3 + 0] = r > 0xff ? 0xff: r;
    data[width * 3 - 3 + 1] = g > 0xff ? 0xff: g;
    data[width * 3 - 3 + 2] = b > 0xff ? 0xff: b;
    r = data[width * 3 + 0] + (error_r * 5 >> 4);
    g = data[width * 3 + 1] + (error_g * 5 >> 4);
    b = data[width * 3 + 2] + (error_b * 5 >> 4);
    data[width * 3 + 0] = r > 0xff ? 0xff: r;
    data[width * 3 + 1] = g > 0xff ? 0xff: g;
    data[width * 3 + 2] = b > 0xff ? 0xff: b;
}


static void
dither_func_atkinson(unsigned char *data, int width)
{
    int r, g, b;
    int error_r = data[0] & 0x7;
    int error_g = data[1] & 0x7;
    int error_b = data[2] & 0x7;

    error_r += 4;
    error_g += 4;
    error_b += 4;

    /* Atkinson's Method
     *          curr    1/8    1/8
     *   1/8     1/8    1/8
     *           1/8
     */
    r = data[(width * 0 + 1) * 3 + 0] + (error_r >> 3);
    g = data[(width * 0 + 1) * 3 + 1] + (error_g >> 3);
    b = data[(width * 0 + 1) * 3 + 2] + (error_b >> 3);
    data[(width * 0 + 1) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 0 + 1) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 0 + 1) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 0 + 2) * 3 + 0] + (error_r >> 3);
    g = data[(width * 0 + 2) * 3 + 1] + (error_g >> 3);
    b = data[(width * 0 + 2) * 3 + 2] + (error_b >> 3);
    data[(width * 0 + 2) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 0 + 2) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 0 + 2) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 1 - 1) * 3 + 0] + (error_r >> 3);
    g = data[(width * 1 - 1) * 3 + 1] + (error_g >> 3);
    b = data[(width * 1 - 1) * 3 + 2] + (error_b >> 3);
    data[(width * 1 - 1) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 1 - 1) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 1 - 1) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 1 + 0) * 3 + 0] + (error_r >> 3);
    g = data[(width * 1 + 0) * 3 + 1] + (error_g >> 3);
    b = data[(width * 1 + 0) * 3 + 2] + (error_b >> 3);
    data[(width * 1 + 0) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 1 + 0) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 1 + 0) * 3 + 2] = b > 0xff ? 0xff: b;
    r = (data[(width * 1 + 1) * 3 + 0] + (error_r >> 3));
    g = (data[(width * 1 + 1) * 3 + 1] + (error_g >> 3));
    b = (data[(width * 1 + 1) * 3 + 2] + (error_b >> 3));
    data[(width * 1 + 1) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 1 + 1) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 1 + 1) * 3 + 2] = b > 0xff ? 0xff: b;
    r = (data[(width * 2 + 0) * 3 + 0] + (error_r >> 3));
    g = (data[(width * 2 + 0) * 3 + 1] + (error_g >> 3));
    b = (data[(width * 2 + 0) * 3 + 2] + (error_b >> 3));
    data[(width * 2 + 0) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 2 + 0) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 2 + 0) * 3 + 2] = b > 0xff ? 0xff: b;
}


static void
dither_func_jajuni(unsigned char *data, int width)
{
    int r, g, b;
    int error_r = data[0] & 0x7;
    int error_g = data[1] & 0x7;
    int error_b = data[2] & 0x7;

    error_r += 4;
    error_g += 4;
    error_b += 4;

    /* Jarvis, Judice & Ninke Method
     *                  curr    7/48    5/48
     *  3/48    5/48    7/48    5/48    3/48
     *  1/48    3/48    5/48    3/48    1/48
     */
    r = data[(width * 0 + 1) * 3 + 0] + (error_r * 7 / 48);
    g = data[(width * 0 + 1) * 3 + 1] + (error_g * 7 / 48);
    b = data[(width * 0 + 1) * 3 + 2] + (error_b * 7 / 48);
    data[(width * 0 + 1) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 0 + 1) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 0 + 1) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 0 + 2) * 3 + 0] + (error_r * 5 / 48);
    g = data[(width * 0 + 2) * 3 + 1] + (error_g * 5 / 48);
    b = data[(width * 0 + 2) * 3 + 2] + (error_b * 5 / 48);
    data[(width * 0 + 2) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 0 + 2) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 0 + 2) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 1 - 2) * 3 + 0] + (error_r * 3 / 48);
    g = data[(width * 1 - 2) * 3 + 1] + (error_g * 3 / 48);
    b = data[(width * 1 - 2) * 3 + 2] + (error_b * 3 / 48);
    data[(width * 1 - 2) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 1 - 2) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 1 - 2) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 1 - 1) * 3 + 0] + (error_r * 5 / 48);
    g = data[(width * 1 - 1) * 3 + 1] + (error_g * 5 / 48);
    b = data[(width * 1 - 1) * 3 + 2] + (error_b * 5 / 48);
    data[(width * 1 - 1) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 1 - 1) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 1 - 1) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 1 + 0) * 3 + 0] + (error_r * 7 / 48);
    g = data[(width * 1 + 0) * 3 + 1] + (error_g * 7 / 48);
    b = data[(width * 1 + 0) * 3 + 2] + (error_b * 7 / 48);
    data[(width * 1 + 0) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 1 + 0) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 1 + 0) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 1 + 1) * 3 + 0] + (error_r * 5 / 48);
    g = data[(width * 1 + 1) * 3 + 1] + (error_g * 5 / 48);
    b = data[(width * 1 + 1) * 3 + 2] + (error_b * 5 / 48);
    data[(width * 1 + 1) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 1 + 1) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 1 + 1) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 1 + 2) * 3 + 0] + (error_r * 3 / 48);
    g = data[(width * 1 + 2) * 3 + 1] + (error_g * 3 / 48);
    b = data[(width * 1 + 2) * 3 + 2] + (error_b * 3 / 48);
    data[(width * 1 + 2) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 1 + 2) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 1 + 2) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 2 - 2) * 3 + 0] + (error_r * 1 / 48);
    g = data[(width * 2 - 2) * 3 + 1] + (error_g * 1 / 48);
    b = data[(width * 2 - 2) * 3 + 2] + (error_b * 1 / 48);
    data[(width * 2 - 2) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 2 - 2) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 2 - 2) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 2 - 1) * 3 + 0] + (error_r * 3 / 48);
    g = data[(width * 2 - 1) * 3 + 1] + (error_g * 3 / 48);
    b = data[(width * 2 - 1) * 3 + 2] + (error_b * 3 / 48);
    data[(width * 2 - 1) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 2 - 1) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 2 - 1) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 2 + 0) * 3 + 0] + (error_r * 5 / 48);
    g = data[(width * 2 + 0) * 3 + 1] + (error_g * 5 / 48);
    b = data[(width * 2 + 0) * 3 + 2] + (error_b * 5 / 48);
    data[(width * 2 + 0) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 2 + 0) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 2 + 0) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 2 + 1) * 3 + 0] + (error_r * 3 / 48);
    g = data[(width * 2 + 1) * 3 + 1] + (error_g * 3 / 48);
    b = data[(width * 2 + 1) * 3 + 2] + (error_b * 3 / 48);
    data[(width * 2 + 1) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 2 + 1) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 2 + 1) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 2 + 2) * 3 + 0] + (error_r * 1 / 48);
    g = data[(width * 2 + 2) * 3 + 1] + (error_g * 1 / 48);
    b = data[(width * 2 + 2) * 3 + 2] + (error_b * 1 / 48);
    data[(width * 2 + 2) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 2 + 2) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 2 + 2) * 3 + 2] = b > 0xff ? 0xff: b;
}


static void
dither_func_stucki(unsigned char *data, int width)
{
    int r, g, b;
    int error_r = data[0] & 0x7;
    int error_g = data[1] & 0x7;
    int error_b = data[2] & 0x7;

    error_r += 4;
    error_g += 4;
    error_b += 4;

    /* Stucki's Method
     *                  curr    8/48    4/48
     *  2/48    4/48    8/48    4/48    2/48
     *  1/48    2/48    4/48    2/48    1/48
     */
    r = data[(width * 0 + 1) * 3 + 0] + (error_r * 8 / 48);
    g = data[(width * 0 + 1) * 3 + 1] + (error_g * 8 / 48);
    b = data[(width * 0 + 1) * 3 + 2] + (error_b * 8 / 48);
    data[(width * 0 + 1) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 0 + 1) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 0 + 1) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 0 + 2) * 3 + 0] + (error_r * 4 / 48);
    g = data[(width * 0 + 2) * 3 + 1] + (error_g * 4 / 48);
    b = data[(width * 0 + 2) * 3 + 2] + (error_b * 4 / 48);
    data[(width * 0 + 2) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 0 + 2) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 0 + 2) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 1 - 2) * 3 + 0] + (error_r * 2 / 48);
    g = data[(width * 1 - 2) * 3 + 1] + (error_g * 2 / 48);
    b = data[(width * 1 - 2) * 3 + 2] + (error_b * 2 / 48);
    data[(width * 1 - 2) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 1 - 2) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 1 - 2) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 1 - 1) * 3 + 0] + (error_r * 4 / 48);
    g = data[(width * 1 - 1) * 3 + 1] + (error_g * 4 / 48);
    b = data[(width * 1 - 1) * 3 + 2] + (error_b * 4 / 48);
    data[(width * 1 - 1) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 1 - 1) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 1 - 1) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 1 + 0) * 3 + 0] + (error_r * 8 / 48);
    g = data[(width * 1 + 0) * 3 + 1] + (error_g * 8 / 48);
    b = data[(width * 1 + 0) * 3 + 2] + (error_b * 8 / 48);
    data[(width * 1 + 0) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 1 + 0) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 1 + 0) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 1 + 1) * 3 + 0] + (error_r * 4 / 48);
    g = data[(width * 1 + 1) * 3 + 1] + (error_g * 4 / 48);
    b = data[(width * 1 + 1) * 3 + 2] + (error_b * 4 / 48);
    data[(width * 1 + 1) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 1 + 1) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 1 + 1) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 1 + 2) * 3 + 0] + (error_r * 2 / 48);
    g = data[(width * 1 + 2) * 3 + 1] + (error_g * 2 / 48);
    b = data[(width * 1 + 2) * 3 + 2] + (error_b * 2 / 48);
    data[(width * 1 + 2) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 1 + 2) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 1 + 2) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 2 - 2) * 3 + 0] + (error_r * 1 / 48);
    g = data[(width * 2 - 2) * 3 + 1] + (error_g * 1 / 48);
    b = data[(width * 2 - 2) * 3 + 2] + (error_b * 1 / 48);
    data[(width * 2 - 2) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 2 - 2) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 2 - 2) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 2 - 1) * 3 + 0] + (error_r * 2 / 48);
    g = data[(width * 2 - 1) * 3 + 1] + (error_g * 2 / 48);
    b = data[(width * 2 - 1) * 3 + 2] + (error_b * 2 / 48);
    data[(width * 2 - 1) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 2 - 1) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 2 - 1) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 2 + 0) * 3 + 0] + (error_r * 4 / 48);
    g = data[(width * 2 + 0) * 3 + 1] + (error_g * 4 / 48);
    b = data[(width * 2 + 0) * 3 + 2] + (error_b * 4 / 48);
    data[(width * 2 + 0) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 2 + 0) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 2 + 0) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 2 + 1) * 3 + 0] + (error_r * 2 / 48);
    g = data[(width * 2 + 1) * 3 + 1] + (error_g * 2 / 48);
    b = data[(width * 2 + 1) * 3 + 2] + (error_b * 2 / 48);
    data[(width * 2 + 1) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 2 + 1) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 2 + 1) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 2 + 2) * 3 + 0] + (error_r * 1 / 48);
    g = data[(width * 2 + 2) * 3 + 1] + (error_g * 1 / 48);
    b = data[(width * 2 + 2) * 3 + 2] + (error_b * 1 / 48);
    data[(width * 2 + 2) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 2 + 2) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 2 + 2) * 3 + 2] = b > 0xff ? 0xff: b;
}


static void
dither_func_burkes(unsigned char *data, int width)
{
    int r, g, b;
    int error_r = data[0] & 0x7;
    int error_g = data[1] & 0x7;
    int error_b = data[2] & 0x7;

    error_r += 2;
    error_g += 2;
    error_b += 2;

    /* Burkes' Method
     *                  curr    4/16    2/16
     *  1/16    2/16    4/16    2/16    1/16
     */
    r = data[(width * 0 + 1) * 3 + 0] + (error_r * 4 / 16);
    g = data[(width * 0 + 1) * 3 + 1] + (error_g * 4 / 16);
    b = data[(width * 0 + 1) * 3 + 2] + (error_b * 4 / 16);
    data[(width * 0 + 1) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 0 + 1) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 0 + 1) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 0 + 2) * 3 + 0] + (error_r * 2 / 16);
    g = data[(width * 0 + 2) * 3 + 1] + (error_g * 2 / 16);
    b = data[(width * 0 + 2) * 3 + 2] + (error_b * 2 / 16);
    data[(width * 0 + 2) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 0 + 2) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 0 + 2) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 1 - 2) * 3 + 0] + (error_r * 1 / 16);
    g = data[(width * 1 - 2) * 3 + 1] + (error_g * 1 / 16);
    b = data[(width * 1 - 2) * 3 + 2] + (error_b * 1 / 16);
    data[(width * 1 - 2) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 1 - 2) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 1 - 2) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 1 - 1) * 3 + 0] + (error_r * 2 / 16);
    g = data[(width * 1 - 1) * 3 + 1] + (error_g * 2 / 16);
    b = data[(width * 1 - 1) * 3 + 2] + (error_b * 2 / 16);
    data[(width * 1 - 1) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 1 - 1) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 1 - 1) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 1 + 0) * 3 + 0] + (error_r * 4 / 16);
    g = data[(width * 1 + 0) * 3 + 1] + (error_g * 4 / 16);
    b = data[(width * 1 + 0) * 3 + 2] + (error_b * 4 / 16);
    data[(width * 1 + 0) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 1 + 0) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 1 + 0) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 1 + 1) * 3 + 0] + (error_r * 2 / 16);
    g = data[(width * 1 + 1) * 3 + 1] + (error_g * 2 / 16);
    b = data[(width * 1 + 1) * 3 + 2] + (error_b * 2 / 16);
    data[(width * 1 + 1) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 1 + 1) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 1 + 1) * 3 + 2] = b > 0xff ? 0xff: b;
    r = data[(width * 1 + 2) * 3 + 0] + (error_r * 1 / 16);
    g = data[(width * 1 + 2) * 3 + 1] + (error_g * 1 / 16);
    b = data[(width * 1 + 2) * 3 + 2] + (error_b * 1 / 16);
    data[(width * 1 + 2) * 3 + 0] = r > 0xff ? 0xff: r;
    data[(width * 1 + 2) * 3 + 1] = g > 0xff ? 0xff: g;
    data[(width * 1 + 2) * 3 + 2] = b > 0xff ? 0xff: b;
}


static void
dither_func_a_dither(unsigned char *data, int width, int x, int y)
{
    int c;
    float value;
    float mask;

    (void) width; /* unused */

    for (c = 0; c < 3; c ++) {
        mask = (((x + c * 17) + y * 236) * 119) & 255;
        mask = ((mask - 128) / 256.0f) ;
        value = data[c] + mask;
        if (value < 0) {
            value = 0;
        }
        value = value > 255 ? 255 : value;
        data[c] = value;
    }
}


static void
dither_func_x_dither(unsigned char *data, int width, int x, int y)
{
    int c;
    float value;
    float mask;

    (void) width;  /* unused */

    for (c = 0; c < 3; c ++) {
        mask = (((x + c * 17) ^ y * 236) * 1234) & 511;
        mask = ((mask - 128) / 512.0f) ;
        value = data[c] + mask;
        if (value < 0) {
            value = 0;
        }
        value = value > 255 ? 255 : value;
        data[c] = value;
    }
}


static void
sixel_apply_15bpp_dither(
    unsigned char *pixels,
    int x, int y, int width, int height,
    int method_for_diffuse)
{
    /* apply floyd steinberg dithering */
    switch (method_for_diffuse) {
    case SIXEL_DIFFUSE_FS:
        if (x < width - 1 && y < height - 1) {
            dither_func_fs(pixels, width);
        }
        break;
    case SIXEL_DIFFUSE_ATKINSON:
        if (x < width - 2 && y < height - 2) {
            dither_func_atkinson(pixels, width);
        }
        break;
    case SIXEL_DIFFUSE_JAJUNI:
        if (x < width - 2 && y < height - 2) {
            dither_func_jajuni(pixels, width);
        }
        break;
    case SIXEL_DIFFUSE_STUCKI:
        if (x < width - 2 && y < height - 2) {
            dither_func_stucki(pixels, width);
        }
        break;
    case SIXEL_DIFFUSE_BURKES:
        if (x < width - 2 && y < height - 1) {
            dither_func_burkes(pixels, width);
        }
        break;
    case SIXEL_DIFFUSE_A_DITHER:
        dither_func_a_dither(pixels, width, x, y);
        break;
    case SIXEL_DIFFUSE_X_DITHER:
        dither_func_x_dither(pixels, width, x, y);
        break;
    case SIXEL_DIFFUSE_NONE:
    default:
        dither_func_none(pixels, width);
        break;
    }
}


static SIXELSTATUS
sixel_encode_highcolor(
        unsigned char *pixels, int width, int height,
        sixel_dither_t *dither, sixel_output_t *output
        )
{
    SIXELSTATUS status = SIXEL_FALSE;
    sixel_index_t *paletted_pixels = NULL;
    unsigned char *normalized_pixels = NULL;
    /* Mark sixel line pixels which have been already drawn. */
    unsigned char *marks;
    unsigned char *rgbhit;
    unsigned char *rgb2pal;
    unsigned char palhitcount[SIXEL_PALETTE_MAX];
    unsigned char palstate[SIXEL_PALETTE_MAX];
    int output_count;
    int const maxcolors = 1 << 15;
    size_t image_size;
    size_t marks_size;
    size_t normalized_size;
    size_t whole_size;
    size_t maxcolors_size;
    int x, y;
    unsigned char *dst;
    unsigned char *mptr;
    int dirty;
    int mod_y;
    int nextpal;
    int threshold;
    int pix;
    int orig_height;
    unsigned char *pal;

    /*
     * The high-color encoder keeps one palette index per input pixel
     * followed by color lookup tables and six scanlines of mark bytes.
     * Calculate all derived sizes in size_t and reject arithmetic
     * overflow before a wrapped value can reach the allocator.
     */
    maxcolors_size = (size_t)maxcolors;
    if ((size_t)height > ((size_t)-1) / (size_t)width) {
        sixel_helper_set_additional_message(
            "sixel_encode_highcolor: image size overflow.");
        status = SIXEL_BAD_INPUT;
        goto error;
    }
    image_size = (size_t)width * (size_t)height;

    if (image_size > ((size_t)-1) / 3UL) {
        sixel_helper_set_additional_message(
            "sixel_encode_highcolor: normalized size overflow.");
        status = SIXEL_BAD_INPUT;
        goto error;
    }
    normalized_size = image_size * 3UL;

    if ((size_t)width > ((size_t)-1) / 6UL) {
        sixel_helper_set_additional_message(
            "sixel_encode_highcolor: marks size overflow.");
        status = SIXEL_BAD_INPUT;
        goto error;
    }
    marks_size = (size_t)width * 6UL;

    if (image_size > (size_t)-1 - maxcolors_size ||
        image_size + maxcolors_size > (size_t)-1 - maxcolors_size ||
        image_size + maxcolors_size + maxcolors_size
            > (size_t)-1 - marks_size) {
        sixel_helper_set_additional_message(
            "sixel_encode_highcolor: whole size overflow.");
        status = SIXEL_BAD_INPUT;
        goto error;
    }
    whole_size = image_size      /* for paletted_pixels */
               + maxcolors_size  /* for rgbhit */
               + maxcolors_size  /* for rgb2pal */
               + marks_size;     /* for marks */

    if (dither->pixelformat != SIXEL_PIXELFORMAT_RGB888) {
        /* normalize pixelfromat */
        normalized_pixels = (unsigned char *)sixel_allocator_malloc(
            dither->allocator, normalized_size);
        if (normalized_pixels == NULL) {
            goto error;
        }
        status = sixel_helper_normalize_pixelformat(normalized_pixels,
                                                    &dither->pixelformat,
                                                    pixels,
                                                    dither->pixelformat,
                                                    width, height);
        if (SIXEL_FAILED(status)) {
            goto error;
        }
        pixels = normalized_pixels;
    }
    paletted_pixels = (sixel_index_t *)sixel_allocator_malloc(dither->allocator,
                                                              whole_size);
    if (paletted_pixels == NULL) {
        goto error;
    }
    rgbhit = paletted_pixels + image_size;
    memset(rgbhit, 0, maxcolors_size * 2UL + marks_size);
    rgb2pal = rgbhit + maxcolors;
    marks = rgb2pal + maxcolors;
    output_count = 0;

next:
    dst = paletted_pixels;
    nextpal = 0;
    threshold = 1;
    dirty = 0;
    mptr = marks;
    memset(palstate, 0, sizeof(palstate));
    y = mod_y = 0;

    while (1) {
        for (x = 0; x < width; x++, mptr++, dst++, pixels += 3) {
            if (*mptr) {
                *dst = 255;
            } else {
                sixel_apply_15bpp_dither(pixels,
                                         x, y, width, height,
                                         dither->method_for_diffuse);
                pix = ((pixels[0] & 0xf8) << 7) |
                      ((pixels[1] & 0xf8) << 2) |
                      ((pixels[2] >> 3) & 0x1f);

                if (!rgbhit[pix]) {
                    while (1) {
                        if (nextpal >= 255) {
                            if (threshold >= 255) {
                                break;
                            } else {
                                threshold = (threshold == 1) ? 9: 255;
                                nextpal = 0;
                            }
                        } else if (palstate[nextpal] ||
                                 palhitcount[nextpal] > threshold) {
                            nextpal++;
                        } else {
                            break;
                        }
                    }

                    if (nextpal >= 255) {
                        dirty = 1;
                        *dst = 255;
                    } else {
                        pal = dither->palette + (nextpal * 3);

                        rgbhit[pix] = 1;
                        if (output_count > 0) {
                            rgbhit[((pal[0] & 0xf8) << 7) |
                                   ((pal[1] & 0xf8) << 2) |
                                   ((pal[2] >> 3) & 0x1f)] = 0;
                        }
                        *dst = rgb2pal[pix] = nextpal++;
                        *mptr = 1;
                        palstate[*dst] = PALETTE_CHANGE;
                        palhitcount[*dst] = 1;
                        *(pal++) = pixels[0];
                        *(pal++) = pixels[1];
                        *(pal++) = pixels[2];
                    }
                } else {
                    *dst = rgb2pal[pix];
                    *mptr = 1;
                    if (!palstate[*dst]) {
                        palstate[*dst] = PALETTE_HIT;
                    }
                    if (palhitcount[*dst] < 255) {
                        palhitcount[*dst]++;
                    }
                }
            }
        }

        if (++y >= height) {
            if (dirty) {
                mod_y = 5;
            } else {
                goto end;
            }
        }
        if (dirty && (mod_y == 5 || y >= height)) {
            orig_height = height;

            if (output_count++ == 0) {
                status = sixel_encode_header(width, height, output);
                if (SIXEL_FAILED(status)) {
                    goto error;
                }
            }
            height = y;
            status = sixel_encode_body(paletted_pixels,
                                       width,
                                       height,
                                       dither->palette,
                                       255,
                                       255,
                                       dither->bodyonly,
                                       output,
                                       palstate,
                                       dither->allocator);
            if (SIXEL_FAILED(status)) {
                goto error;
            }
            if (y >= orig_height) {
              goto end;
            }
            pixels -= (6 * width * 3);
            height = orig_height - height + 6;
            goto next;
        }

        if (++mod_y == 6) {
            mptr = (unsigned char *)memset(marks, 0, marks_size);
            mod_y = 0;
        }
    }

    goto next;

end:
    if (output_count == 0) {
        status = sixel_encode_header(width, height, output);
        if (SIXEL_FAILED(status)) {
            goto error;
        }
    }
    status = sixel_encode_body(paletted_pixels,
                               width,
                               height,
                               dither->palette,
                               255,
                               255,
                               dither->bodyonly,
                               output,
                               palstate,
                               dither->allocator);
    if (SIXEL_FAILED(status)) {
        goto error;
    }

    status = sixel_encode_footer(output);
    if (SIXEL_FAILED(status)) {
        goto error;
    }

error:
    sixel_allocator_free(dither->allocator, paletted_pixels);
    sixel_allocator_free(dither->allocator, normalized_pixels);

    return status;
}


SIXELAPI SIXELSTATUS
sixel_encode(
    unsigned char  /* in */ *pixels,   /* pixel bytes */
    int            /* in */ width,     /* image width */
    int            /* in */ height,    /* image height */
    int const      /* in */ depth,     /* color depth */
    sixel_dither_t /* in */ *dither,   /* dither context */
    sixel_output_t /* in */ *output)   /* output context */
{
    SIXELSTATUS status = SIXEL_FALSE;

    (void) depth;

    /* TODO: reference counting should be thread-safe */
    sixel_dither_ref(dither);
    sixel_output_ref(output);

    if (width < 1) {
        sixel_helper_set_additional_message(
            "sixel_encode: bad width parameter."
            " (width < 1)");
        status = SIXEL_BAD_INPUT;
        goto end;
    }

    if (height < 1) {
        sixel_helper_set_additional_message(
            "sixel_encode: bad height parameter."
            " (height < 1)");
        status = SIXEL_BAD_INPUT;
        goto end;
    }

    if (width > SIXEL_WIDTH_LIMIT) {
        sixel_helper_set_additional_message(
            "sixel_encode: bad width parameter."
            " (width > SIXEL_WIDTH_LIMIT)");
        status = SIXEL_BAD_INPUT;
        goto end;
    }

    if (height > SIXEL_HEIGHT_LIMIT) {
        sixel_helper_set_additional_message(
            "sixel_encode: bad height parameter."
            " (height > SIXEL_HEIGHT_LIMIT)");
        status = SIXEL_BAD_INPUT;
        goto end;
    }

    if (dither->quality_mode == SIXEL_QUALITY_HIGHCOLOR) {
        status = sixel_encode_highcolor(pixels, width, height,
                                        dither, output);
    } else {
        status = sixel_encode_dither(pixels, width, height,
                                     dither, output);
    }

end:
    sixel_output_unref(output);
    sixel_dither_unref(dither);

    return status;
}

/* emacs Local Variables:      */
/* emacs mode: c               */
/* emacs tab-width: 4          */
/* emacs indent-tabs-mode: nil */
/* emacs c-basic-offset: 4     */
/* emacs End:                  */
/* vim: set expandtab ts=4 sts=4 sw=4 : */
/* EOF */
