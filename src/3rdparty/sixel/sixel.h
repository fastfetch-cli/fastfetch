/*
 * Copyright (c) 2014-2020 Hayaki Saito
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

#include <stddef.h>  /* for size_t */

#ifndef LIBSIXEL_SIXEL_H
#define LIBSIXEL_SIXEL_H

/*
 * fastfetch: upstream defines this as `__declspec(dllexport)` on Windows, but
 * this vendored subset is compiled into libfastfetch statically rather than
 * into a standalone DLL. Keeping the attribute would (a) add every libsixel
 * symbol to fastfetch's export table and (b) make these declarations disagree
 * with the ones in dither.h / output.h / quant.h / allocator.h, which do not
 * carry it (clang: -Wdll-attribute-on-redeclaration).
 * Re-apply this change when re-syncing with upstream.
 */
#define SIXELAPI

#define LIBSIXEL_VERSION "1.8.7-r2"
#define LIBSIXEL_ABI_VERSION "1:6:0"

typedef unsigned char sixel_index_t;

/* limitations */
#define SIXEL_OUTPUT_PACKET_SIZE     16384
#define SIXEL_PALETTE_MIN            2
#define SIXEL_PALETTE_MAX            256
#define SIXEL_USE_DEPRECATED_SYMBOLS 1
#define SIXEL_ALLOCATE_BYTES_MAX     10248UL * 1024UL * 128UL   /* up to 128M */
#define SIXEL_WIDTH_LIMIT            1000000
#define SIXEL_HEIGHT_LIMIT           1000000

/* loader settings */
#define SIXEL_DEFALUT_GIF_DELAY         1

/* return value */
typedef int SIXELSTATUS;
#define SIXEL_OK                0x0000                          /* succeeded */
#define SIXEL_FALSE             0x1000                          /* failed */

#define SIXEL_RUNTIME_ERROR     (SIXEL_FALSE         | 0x0100)  /* runtime error */
#define SIXEL_LOGIC_ERROR       (SIXEL_FALSE         | 0x0200)  /* logic error */
#define SIXEL_FEATURE_ERROR     (SIXEL_FALSE         | 0x0300)  /* feature not enabled */
#define SIXEL_LIBC_ERROR        (SIXEL_FALSE         | 0x0400)  /* errors caused by curl */
#define SIXEL_CURL_ERROR        (SIXEL_FALSE         | 0x0500)  /* errors occures in libc functions */
#define SIXEL_JPEG_ERROR        (SIXEL_FALSE         | 0x0600)  /* errors occures in libjpeg functions */
#define SIXEL_PNG_ERROR         (SIXEL_FALSE         | 0x0700)  /* errors occures in libpng functions */
#define SIXEL_GDK_ERROR         (SIXEL_FALSE         | 0x0800)  /* errors occures in gdk functions */
#define SIXEL_GD_ERROR          (SIXEL_FALSE         | 0x0900)  /* errors occures in gd functions */
#define SIXEL_STBI_ERROR        (SIXEL_FALSE         | 0x0a00)  /* errors occures in stb_image functions */
#define SIXEL_STBIW_ERROR       (SIXEL_FALSE         | 0x0b00)  /* errors occures in stb_image_write functions */

#define SIXEL_INTERRUPTED       (SIXEL_OK            | 0x0001)  /* interrupted by a signal */

#define SIXEL_BAD_ALLOCATION    (SIXEL_RUNTIME_ERROR | 0x0001)  /* malloc() failed */
#define SIXEL_BAD_ARGUMENT      (SIXEL_RUNTIME_ERROR | 0x0002)  /* bad argument detected */
#define SIXEL_BAD_INPUT         (SIXEL_RUNTIME_ERROR | 0x0003)  /* bad input detected */
#define SIXEL_BAD_INTEGER_OVERFLOW (SIXEL_RUNTIME_ERROR | 0x0004)  /* integer overflow */

#define SIXEL_NOT_IMPLEMENTED   (SIXEL_FEATURE_ERROR | 0x0001)  /* feature not implemented */

#define SIXEL_SUCCEEDED(status) (((status) & 0x1000) == 0)
#define SIXEL_FAILED(status)    (((status) & 0x1000) != 0)

/* method for finding the largest dimension for splitting,
 * and sorting by that component */
#define SIXEL_LARGE_AUTO   0x0   /* choose automatically the method for finding the largest
                                    dimension */
#define SIXEL_LARGE_NORM   0x1   /* simply comparing the range in RGB space */
#define SIXEL_LARGE_LUM    0x2   /* transforming into luminosities before the comparison */

/* method for choosing a color from the box */
#define SIXEL_REP_AUTO            0x0  /* choose automatically the method for selecting
                                          representative color from each box */
#define SIXEL_REP_CENTER_BOX      0x1  /* choose the center of the box */
#define SIXEL_REP_AVERAGE_COLORS  0x2  /* choose the average all the color
                                          in the box (specified in Heckbert's paper) */
#define SIXEL_REP_AVERAGE_PIXELS  0x3  /* choose the average all the pixels in the box */

/* method for diffusing */
#define SIXEL_DIFFUSE_AUTO        0x0  /* choose diffusion type automatically */
#define SIXEL_DIFFUSE_NONE        0x1  /* don't diffuse */
#define SIXEL_DIFFUSE_ATKINSON    0x2  /* diffuse with Bill Atkinson's method */
#define SIXEL_DIFFUSE_FS          0x3  /* diffuse with Floyd-Steinberg method */
#define SIXEL_DIFFUSE_JAJUNI      0x4  /* diffuse with Jarvis, Judice & Ninke method */
#define SIXEL_DIFFUSE_STUCKI      0x5  /* diffuse with Stucki's method */
#define SIXEL_DIFFUSE_BURKES      0x6  /* diffuse with Burkes' method */
#define SIXEL_DIFFUSE_A_DITHER    0x7  /* positionally stable arithmetic dither */
#define SIXEL_DIFFUSE_X_DITHER    0x8  /* positionally stable arithmetic xor based dither */

/* quality modes */
#define SIXEL_QUALITY_AUTO        0x0  /* choose quality mode automatically */
#define SIXEL_QUALITY_HIGH        0x1  /* high quality palette construction */
#define SIXEL_QUALITY_LOW         0x2  /* low quality palette construction */
#define SIXEL_QUALITY_FULL        0x3  /* full quality palette construction */
#define SIXEL_QUALITY_HIGHCOLOR   0x4  /* high color */

/* built-in dither */
#define SIXEL_BUILTIN_MONO_DARK   0x0  /* monochrome terminal with dark background */
#define SIXEL_BUILTIN_MONO_LIGHT  0x1  /* monochrome terminal with light background */
#define SIXEL_BUILTIN_XTERM16     0x2  /* xterm 16color */
#define SIXEL_BUILTIN_XTERM256    0x3  /* xterm 256color */
#define SIXEL_BUILTIN_VT340_MONO  0x4  /* vt340 monochrome */
#define SIXEL_BUILTIN_VT340_COLOR 0x5  /* vt340 color */
#define SIXEL_BUILTIN_G1          0x6  /* 1bit grayscale */
#define SIXEL_BUILTIN_G2          0x7  /* 2bit grayscale */
#define SIXEL_BUILTIN_G4          0x8  /* 4bit grayscale */
#define SIXEL_BUILTIN_G8          0x9  /* 8bit grayscale */

/* offset value of pixelFormat */
#define SIXEL_FORMATTYPE_COLOR     (0)
#define SIXEL_FORMATTYPE_GRAYSCALE (1 << 6)
#define SIXEL_FORMATTYPE_PALETTE   (1 << 7)

/* pixelformat type of input image
   NOTE: for compatibility, the value of PIXELFORAMT_COLOR_RGB888 must be 3 */
#define SIXEL_PIXELFORMAT_RGB555   (SIXEL_FORMATTYPE_COLOR     | 0x01) /* 15bpp */
#define SIXEL_PIXELFORMAT_RGB565   (SIXEL_FORMATTYPE_COLOR     | 0x02) /* 16bpp */
#define SIXEL_PIXELFORMAT_RGB888   (SIXEL_FORMATTYPE_COLOR     | 0x03) /* 24bpp */
#define SIXEL_PIXELFORMAT_BGR555   (SIXEL_FORMATTYPE_COLOR     | 0x04) /* 15bpp */
#define SIXEL_PIXELFORMAT_BGR565   (SIXEL_FORMATTYPE_COLOR     | 0x05) /* 16bpp */
#define SIXEL_PIXELFORMAT_BGR888   (SIXEL_FORMATTYPE_COLOR     | 0x06) /* 24bpp */
#define SIXEL_PIXELFORMAT_ARGB8888 (SIXEL_FORMATTYPE_COLOR     | 0x10) /* 32bpp */
#define SIXEL_PIXELFORMAT_RGBA8888 (SIXEL_FORMATTYPE_COLOR     | 0x11) /* 32bpp */
#define SIXEL_PIXELFORMAT_ABGR8888 (SIXEL_FORMATTYPE_COLOR     | 0x12) /* 32bpp */
#define SIXEL_PIXELFORMAT_BGRA8888 (SIXEL_FORMATTYPE_COLOR     | 0x13) /* 32bpp */
#define SIXEL_PIXELFORMAT_G1       (SIXEL_FORMATTYPE_GRAYSCALE | 0x00) /* 1bpp grayscale */
#define SIXEL_PIXELFORMAT_G2       (SIXEL_FORMATTYPE_GRAYSCALE | 0x01) /* 2bpp grayscale */
#define SIXEL_PIXELFORMAT_G4       (SIXEL_FORMATTYPE_GRAYSCALE | 0x02) /* 4bpp grayscale */
#define SIXEL_PIXELFORMAT_G8       (SIXEL_FORMATTYPE_GRAYSCALE | 0x03) /* 8bpp grayscale */
#define SIXEL_PIXELFORMAT_AG88     (SIXEL_FORMATTYPE_GRAYSCALE | 0x13) /* 16bpp gray+alpha */
#define SIXEL_PIXELFORMAT_GA88     (SIXEL_FORMATTYPE_GRAYSCALE | 0x23) /* 16bpp gray+alpha */
#define SIXEL_PIXELFORMAT_PAL1     (SIXEL_FORMATTYPE_PALETTE   | 0x00) /* 1bpp palette */
#define SIXEL_PIXELFORMAT_PAL2     (SIXEL_FORMATTYPE_PALETTE   | 0x01) /* 2bpp palette */
#define SIXEL_PIXELFORMAT_PAL4     (SIXEL_FORMATTYPE_PALETTE   | 0x02) /* 4bpp palette */
#define SIXEL_PIXELFORMAT_PAL8     (SIXEL_FORMATTYPE_PALETTE   | 0x03) /* 8bpp palette */

/* palette type */
#define SIXEL_PALETTETYPE_AUTO     0   /* choose palette type automatically */
#define SIXEL_PALETTETYPE_HLS      1   /* HLS colorspace */
#define SIXEL_PALETTETYPE_RGB      2   /* RGB colorspace */

/* policies of SIXEL encoding */
#define SIXEL_ENCODEPOLICY_AUTO    0   /* choose encoding policy automatically */
#define SIXEL_ENCODEPOLICY_FAST    1   /* encode as fast as possible */
#define SIXEL_ENCODEPOLICY_SIZE    2   /* encode to as small sixel sequence as possible */

/* method for re-sampling */
#define SIXEL_RES_NEAREST          0   /* Use nearest neighbor method */
#define SIXEL_RES_GAUSSIAN         1   /* Use guaussian filter */
#define SIXEL_RES_HANNING          2   /* Use hanning filter */
#define SIXEL_RES_HAMMING          3   /* Use hamming filter */
#define SIXEL_RES_BILINEAR         4   /* Use bilinear filter */
#define SIXEL_RES_WELSH            5   /* Use welsh filter */
#define SIXEL_RES_BICUBIC          6   /* Use bicubic filter */
#define SIXEL_RES_LANCZOS2         7   /* Use lanczos-2 filter */
#define SIXEL_RES_LANCZOS3         8   /* Use lanczos-3 filter */
#define SIXEL_RES_LANCZOS4         9   /* Use lanczos-4 filter */

/* image format */
#define SIXEL_FORMAT_GIF           0x0 /* read only */
#define SIXEL_FORMAT_PNG           0x1 /* read/write */
#define SIXEL_FORMAT_BMP           0x2 /* read only */
#define SIXEL_FORMAT_JPG           0x3 /* read only */
#define SIXEL_FORMAT_TGA           0x4 /* read only */
#define SIXEL_FORMAT_WBMP          0x5 /* read only with --with-gd configure option */
#define SIXEL_FORMAT_TIFF          0x6 /* read only */
#define SIXEL_FORMAT_SIXEL         0x7 /* read only */
#define SIXEL_FORMAT_PNM           0x8 /* read only */
#define SIXEL_FORMAT_GD2           0x9 /* read only with --with-gd configure option */
#define SIXEL_FORMAT_PSD           0xa /* read only */
#define SIXEL_FORMAT_HDR           0xb /* read only */

/* loop mode */
#define SIXEL_LOOP_AUTO            0   /* honer the setting of GIF header */
#define SIXEL_LOOP_FORCE           1   /* always enable loop */
#define SIXEL_LOOP_DISABLE         2   /* always disable loop */

/* setopt flags */
#define SIXEL_OPTFLAG_INPUT             ('i')  /* -i, --input: specify input file name. */
#define SIXEL_OPTFLAG_OUTPUT            ('o')  /* -o, --output: specify output file name. */
#define SIXEL_OPTFLAG_OUTFILE           ('o')  /* -o, --outfile: specify output file name. */
#define SIXEL_OPTFLAG_7BIT_MODE         ('7')  /* -7, --7bit-mode: for 7bit terminals or printers (default) */
#define SIXEL_OPTFLAG_8BIT_MODE         ('8')  /* -8, --8bit-mode: for 8bit terminals or printers */
#define SIXEL_OPTFLAG_HAS_GRI_ARG_LIMIT ('R')  /* -R, --gri-limit: limit arguments of DECGRI('!') to 255 */
#define SIXEL_OPTFLAG_COLORS            ('p')  /* -p COLORS, --colors=COLORS: specify number of colors */
#define SIXEL_OPTFLAG_MAPFILE           ('m')  /* -m FILE, --mapfile=FILE: specify set of colors */
#define SIXEL_OPTFLAG_MONOCHROME        ('e')  /* -e, --monochrome: output monochrome sixel image */
#define SIXEL_OPTFLAG_INSECURE          ('k')  /* -k, --insecure: allow to connect to SSL sites without certs */
#define SIXEL_OPTFLAG_INVERT            ('i')  /* -i, --invert: assume the terminal background color */
#define SIXEL_OPTFLAG_HIGH_COLOR        ('I')  /* -I, --high-color: output 15bpp sixel image */
#define SIXEL_OPTFLAG_USE_MACRO         ('u')  /* -u, --use-macro: use DECDMAC and DEVINVM sequences */
#define SIXEL_OPTFLAG_MACRO_NUMBER      ('n')  /* -n MACRONO, --macro-number=MACRONO:
                                                  specify macro register number */
#define SIXEL_OPTFLAG_COMPLEXION_SCORE  ('C')  /* -C COMPLEXIONSCORE, --complexion-score=COMPLEXIONSCORE:
                                                  specify an number argument for the score of
                                                  complexion correction. */
#define SIXEL_OPTFLAG_IGNORE_DELAY      ('g')  /* -g, --ignore-delay: render GIF animation without delay */
#define SIXEL_OPTFLAG_STATIC            ('S')  /* -S, --static: render animated GIF as a static image */
#define SIXEL_OPTFLAG_DIFFUSION         ('d')  /* -d DIFFUSIONTYPE, --diffusion=DIFFUSIONTYPE:
                                                  choose diffusion method which used with -p option.
                                                  DIFFUSIONTYPE is one of them:
                                                    auto     -> choose diffusion type
                                                                automatically (default)
                                                    none     -> do not diffuse
                                                    fs       -> Floyd-Steinberg method
                                                    atkinson -> Bill Atkinson's method
                                                    jajuni   -> Jarvis, Judice & Ninke
                                                    stucki   -> Stucki's method
                                                    burkes   -> Burkes' method
                                                    a_dither -> positionally stable
                                                                arithmetic dither
                                                    a_dither -> positionally stable
                                                                arithmetic xor based dither
                                                */
#define SIXEL_OPTFLAG_FIND_LARGEST      ('f')  /* -f FINDTYPE, --find-largest=FINDTYPE:
                                                  choose method for finding the largest
                                                  dimension of median cut boxes for
                                                  splitting, make sense only when -p
                                                  option (color reduction) is
                                                  specified
                                                  FINDTYPE is one of them:
                                                    auto -> choose finding method
                                                            automatically (default)
                                                    norm -> simply comparing the
                                                            range in RGB space
                                                    lum  -> transforming into
                                                            luminosities before the
                                                            comparison
                                                */
#define SIXEL_OPTFLAG_SELECT_COLOR      ('s')  /* -s SELECTTYPE, --select-color=SELECTTYPE
                                                  choose the method for selecting
                                                  representative color from each
                                                  median-cut box, make sense only
                                                  when -p option (color reduction) is
                                                  specified
                                                  SELECTTYPE is one of them:
                                                    auto      -> choose selecting
                                                                 method automatically
                                                                 (default)
                                                    center    -> choose the center of
                                                                 the box
                                                    average    -> calculate the color
                                                                 average into the box
                                                    histogram -> similar with average
                                                                 but considers color
                                                                 histogram
                                                */
#define SIXEL_OPTFLAG_CROP              ('c')  /* -c REGION, --crop=REGION:
                                                  crop source image to fit the
                                                  specified geometry. REGION should
                                                  be formatted as '%dx%d+%d+%d'
                                                */
#define SIXEL_OPTFLAG_WIDTH             ('w')  /* -w WIDTH, --width=WIDTH:
                                                  resize image to specified width
                                                  WIDTH is represented by the
                                                  following syntax
                                                    auto       -> preserving aspect
                                                                  ratio (default)
                                                    <number>%  -> scale width with
                                                                  given percentage
                                                    <number>   -> scale width with
                                                                  pixel counts
                                                    <number>px -> scale width with
                                                                  pixel counts
                                                */
#define SIXEL_OPTFLAG_HEIGHT            ('h')  /* -h HEIGHT, --height=HEIGHT:
                                                   resize image to specified height
                                                   HEIGHT is represented by the
                                                   following syntax
                                                     auto       -> preserving aspect
                                                                   ratio (default)
                                                     <number>%  -> scale height with
                                                                   given percentage
                                                     <number>   -> scale height with
                                                                   pixel counts
                                                     <number>px -> scale height with
                                                                   pixel counts
                                                */
#define SIXEL_OPTFLAG_RESAMPLING        ('r')  /* -r RESAMPLINGTYPE, --resampling=RESAMPLINGTYPE:
                                                  choose resampling filter used
                                                  with -w or -h option (scaling)
                                                  RESAMPLINGTYPE is one of them:
                                                    nearest  -> Nearest-Neighbor
                                                                method
                                                    gaussian -> Gaussian filter
                                                    hanning  -> Hanning filter
                                                    hamming  -> Hamming filter
                                                    bilinear -> Bilinear filter
                                                                (default)
                                                    welsh    -> Welsh filter
                                                    bicubic  -> Bicubic filter
                                                    lanczos2 -> Lanczos-2 filter
                                                    lanczos3 -> Lanczos-3 filter
                                                    lanczos4 -> Lanczos-4 filter
                                                */
#define SIXEL_OPTFLAG_QUALITY           ('q')  /* -q QUALITYMODE, --quality=QUALITYMODE:
                                                  select quality of color
                                                  quanlization.
                                                    auto -> decide quality mode
                                                            automatically (default)
                                                    low  -> low quality and high
                                                            speed mode
                                                    high -> high quality and low
                                                            speed mode
                                                    full -> full quality and careful
                                                            speed mode
                                                */
#define SIXEL_OPTFLAG_LOOPMODE          ('l')  /* -l LOOPMODE, --loop-control=LOOPMODE:
                                                  select loop control mode for GIF
                                                  animation.
                                                    auto    -> honor the setting of
                                                               GIF header (default)
                                                    force   -> always enable loop
                                                    disable -> always disable loop
                                                */
#define SIXEL_OPTFLAG_PALETTE_TYPE      ('t')  /* -t PALETTETYPE, --palette-type=PALETTETYPE:
                                                  select palette color space type
                                                    auto -> choose palette type
                                                            automatically (default)
                                                    hls  -> use HLS color space
                                                    rgb  -> use RGB color space
                                                */
#define SIXEL_OPTFLAG_BUILTIN_PALETTE   ('b')  /* -b BUILTINPALETTE, --builtin-palette=BUILTINPALETTE:
                                                  select built-in palette type
                                                    xterm16    -> X default 16 color map
                                                    xterm256   -> X default 256 color map
                                                    vt340mono  -> VT340 monochrome map
                                                    vt340color -> VT340 color map
                                                    gray1      -> 1bit grayscale map
                                                    gray2      -> 2bit grayscale map
                                                    gray4      -> 4bit grayscale map
                                                    gray8      -> 8bit grayscale map
                                                */
#define SIXEL_OPTFLAG_ENCODE_POLICY     ('E')  /* -E ENCODEPOLICY, --encode-policy=ENCODEPOLICY:
                                                  select encoding policy
                                                    auto -> choose encoding policy
                                                            automatically (default)
                                                    fast -> encode as fast as possible
                                                    size -> encode to as small sixel
                                                            sequence as possible
                                                */
#define SIXEL_OPTFLAG_BGCOLOR           ('B')  /* -B BGCOLOR, --bgcolor=BGCOLOR:
                                                  specify background color
                                                  BGCOLOR is represented by the
                                                  following syntax
                                                    #rgb
                                                    #rrggbb
                                                    #rrrgggbbb
                                                    #rrrrggggbbbb
                                                    rgb:r/g/b
                                                    rgb:rr/gg/bb
                                                    rgb:rrr/ggg/bbb
                                                    rgb:rrrr/gggg/bbbb
                                                */
#define SIXEL_OPTFLAG_PENETRATE         ('P')  /* -P, --penetrate:
                                                  penetrate GNU Screen using DCS
                                                  pass-through sequence */
#define SIXEL_OPTFLAG_PIPE_MODE         ('D')  /* -D, --pipe-mode: (deprecated)
                                                  read source images from stdin continuously */
#define SIXEL_OPTFLAG_VERBOSE           ('v')  /* -v, --verbose: show debugging info */
#define SIXEL_OPTFLAG_VERSION           ('V')  /* -V, --version: show version and license info */
#define SIXEL_OPTFLAG_HELP              ('H')  /* -H, --help: show this help */

#if SIXEL_USE_DEPRECATED_SYMBOLS
/* output character size */
enum characterSize {
    CSIZE_7BIT = 0,  /* 7bit character */
    CSIZE_8BIT = 1   /* 8bit character */
};

/* method for finding the largest dimension for splitting,
 * and sorting by that component */
enum methodForLargest {
    LARGE_AUTO = 0,  /* choose automatically the method for finding the largest
                        dimension */
    LARGE_NORM = 1,  /* simply comparing the range in RGB space */
    LARGE_LUM  = 2   /* transforming into luminosities before the comparison */
};

/* method for choosing a color from the box */
enum methodForRep {
    REP_AUTO           = 0, /* choose automatically the method for selecting
                               representative color from each box */
    REP_CENTER_BOX     = 1, /* choose the center of the box */
    REP_AVERAGE_COLORS = 2, /* choose the average all the color
                               in the box (specified in Heckbert's paper) */
    REP_AVERAGE_PIXELS = 3  /* choose the average all the pixels in the box */
};

/* method for diffusing */
enum methodForDiffuse {
    DIFFUSE_AUTO     = 0, /* choose diffusion type automatically */
    DIFFUSE_NONE     = 1, /* don't diffuse */
    DIFFUSE_ATKINSON = 2, /* diffuse with Bill Atkinson's method */
    DIFFUSE_FS       = 3, /* diffuse with Floyd-Steinberg method */
    DIFFUSE_JAJUNI   = 4, /* diffuse with Jarvis, Judice & Ninke method */
    DIFFUSE_STUCKI   = 5, /* diffuse with Stucki's method */
    DIFFUSE_BURKES   = 6, /* diffuse with Burkes' method */
    DIFFUSE_A_DITHER = 7, /* positionally stable arithmetic dither */
    DIFFUSE_X_DITHER = 8  /* positionally stable arithmetic xor based dither */
};

/* quality modes */
enum qualityMode {
    QUALITY_AUTO      = 0, /* choose quality mode automatically */
    QUALITY_HIGH      = 1, /* high quality palette construction */
    QUALITY_LOW       = 2, /* low quality palette construction */
    QUALITY_FULL      = 3, /* full quality palette construction */
    QUALITY_HIGHCOLOR = 4  /* high color */
};

/* built-in dither */
enum builtinDither {
    BUILTIN_MONO_DARK   = 0, /* monochrome terminal with dark background */
    BUILTIN_MONO_LIGHT  = 1, /* monochrome terminal with dark background */
    BUILTIN_XTERM16     = 2, /* xterm 16color */
    BUILTIN_XTERM256    = 3, /* xterm 256color */
    BUILTIN_VT340_MONO  = 4, /* vt340 monochrome */
    BUILTIN_VT340_COLOR = 5  /* vt340 color */
};

/* offset value of enum pixelFormat */
enum formatType {
    FORMATTYPE_COLOR     = 0,
    FORMATTYPE_GRAYSCALE = 1 << 6,
    FORMATTYPE_PALETTE   = 1 << 7
};

/* pixelformat type of input image
   NOTE: for compatibility, the value of PIXELFORAMT_COLOR_RGB888 must be 3 */
enum pixelFormat {
    PIXELFORMAT_RGB555   = FORMATTYPE_COLOR     | 0x01, /* 15bpp */
    PIXELFORMAT_RGB565   = FORMATTYPE_COLOR     | 0x02, /* 16bpp */
    PIXELFORMAT_RGB888   = FORMATTYPE_COLOR     | 0x03, /* 24bpp */
    PIXELFORMAT_BGR555   = FORMATTYPE_COLOR     | 0x04, /* 15bpp */
    PIXELFORMAT_BGR565   = FORMATTYPE_COLOR     | 0x05, /* 16bpp */
    PIXELFORMAT_BGR888   = FORMATTYPE_COLOR     | 0x06, /* 24bpp */
    PIXELFORMAT_ARGB8888 = FORMATTYPE_COLOR     | 0x10, /* 32bpp */
    PIXELFORMAT_RGBA8888 = FORMATTYPE_COLOR     | 0x11, /* 32bpp */
    PIXELFORMAT_G1       = FORMATTYPE_GRAYSCALE | 0x00, /* 1bpp grayscale */
    PIXELFORMAT_G2       = FORMATTYPE_GRAYSCALE | 0x01, /* 2bpp grayscale */
    PIXELFORMAT_G4       = FORMATTYPE_GRAYSCALE | 0x02, /* 4bpp grayscale */
    PIXELFORMAT_G8       = FORMATTYPE_GRAYSCALE | 0x03, /* 8bpp grayscale */
    PIXELFORMAT_AG88     = FORMATTYPE_GRAYSCALE | 0x13, /* 16bpp gray+alpha */
    PIXELFORMAT_GA88     = FORMATTYPE_GRAYSCALE | 0x23, /* 16bpp gray+alpha */
    PIXELFORMAT_PAL1     = FORMATTYPE_PALETTE   | 0x00, /* 1bpp palette */
    PIXELFORMAT_PAL2     = FORMATTYPE_PALETTE   | 0x01, /* 2bpp palette */
    PIXELFORMAT_PAL4     = FORMATTYPE_PALETTE   | 0x02, /* 4bpp palette */
    PIXELFORMAT_PAL8     = FORMATTYPE_PALETTE   | 0x03  /* 8bpp palette */
};

/* palette type */
enum paletteType {
    PALETTETYPE_AUTO = 0,     /* choose palette type automatically */
    PALETTETYPE_HLS  = 1,     /* HLS colorspace */
    PALETTETYPE_RGB  = 2      /* RGB colorspace */
};

/* policies of SIXEL encoding */
enum encodePolicy {
    ENCODEPOLICY_AUTO = 0,    /* choose encoding policy automatically */
    ENCODEPOLICY_FAST = 1,    /* encode as fast as possible */
    ENCODEPOLICY_SIZE = 2     /* encode to as small sixel sequence as possible */
};

/* method for re-sampling */
enum methodForResampling {
    RES_NEAREST  = 0,  /* Use nearest neighbor method */
    RES_GAUSSIAN = 1,  /* Use guaussian filter */
    RES_HANNING  = 2,  /* Use hanning filter */
    RES_HAMMING  = 3,  /* Use hamming filter */
    RES_BILINEAR = 4,  /* Use bilinear filter */
    RES_WELSH    = 5,  /* Use welsh filter */
    RES_BICUBIC  = 6,  /* Use bicubic filter */
    RES_LANCZOS2 = 7,  /* Use lanczos-2 filter */
    RES_LANCZOS3 = 8,  /* Use lanczos-3 filter */
    RES_LANCZOS4 = 9   /* Use lanczos-4 filter */
};
#endif

typedef void *(* sixel_malloc_t)(size_t);
typedef void *(* sixel_calloc_t)(size_t, size_t);
typedef void *(* sixel_realloc_t)(void *, size_t);
typedef void (* sixel_free_t)(void *);

struct sixel_allocator;
typedef struct sixel_allocator sixel_allocator_t;

#ifdef __cplusplus
extern "C" {
#endif

/* create allocator object */
SIXELAPI SIXELSTATUS
sixel_allocator_new(
    sixel_allocator_t   /* out */ **ppallocator,  /* allocator object to be created */
    sixel_malloc_t      /* in */  fn_malloc,      /* custom malloc() function */
    sixel_calloc_t      /* in */  fn_calloc,      /* custom calloc() function */
    sixel_realloc_t     /* in */  fn_realloc,     /* custom realloc() function */
    sixel_free_t        /* in */  fn_free);       /* custom free() function */

/* increase reference count of allocator object (thread-unsafe) */
SIXELAPI void
sixel_allocator_ref(
    sixel_allocator_t /* in */ *allocator);  /* allocator object to be
                                                increment reference counter */

/* decrease reference count of output context object (thread-unsafe) */
SIXELAPI void
sixel_allocator_unref(sixel_allocator_t *allocator);

/* call custom malloc() */
SIXELAPI void *
sixel_allocator_malloc(
    sixel_allocator_t   /* in */ *allocator,  /* allocator object */
    size_t              /* in */ n);          /* allocation size */

/* call custom calloc() */
SIXELAPI void *
sixel_allocator_calloc(
    sixel_allocator_t   /* in */ *allocator,  /* allocator object */
    size_t              /* in */ nelm,        /* allocation size */
    size_t              /* in */ elsize);     /* allocation size */

/* call custom realloc() */
SIXELAPI void *
sixel_allocator_realloc(
    sixel_allocator_t   /* in */ *allocator,  /* allocator object */
    void                /* in */ *p,          /* existing buffer to be re-allocated */
    size_t              /* in */ n);          /* re-allocation size */

/* call custom free() */
SIXELAPI void
sixel_allocator_free(
    sixel_allocator_t   /* in */ *allocator,  /* allocator object */
    void                /* in */ *p);         /* existing buffer to be freed */

#ifdef HAVE_TESTS
extern volatile int sixel_debug_malloc_counter;

void *
sixel_bad_malloc(size_t size);

void *
sixel_bad_calloc(size_t count, size_t size);

void *
sixel_bad_realloc(void *ptr, size_t size);
#endif  /* HAVE_TESTS */

#ifdef __cplusplus
}
#endif

/* output context manipulation API */

struct sixel_output;
typedef struct sixel_output sixel_output_t;
typedef int (* sixel_write_function)(char *data, int size, void *priv);

#ifdef __cplusplus
extern "C" {
#endif

/* create new output context object */
SIXELAPI SIXELSTATUS
sixel_output_new(
    sixel_output_t          /* out */ **output,     /* output object to be created */
    sixel_write_function    /* in */  fn_write,     /* callback for output sixel */
    void                    /* in */ *priv,         /* private data given as
                                                       3rd argument of fn_write */
    sixel_allocator_t       /* in */  *allocator);  /* allocator, null if you use
                                                       default allocator */

/* deprecated: create an output object */
SIXELAPI __attribute__((deprecated)) sixel_output_t *
sixel_output_create(
    sixel_write_function /* in */ fn_write, /* callback for output sixel */
    void                 /* in */ *priv);   /* private data given as
                                               3rd argument of fn_write */
/* destroy output context object */
SIXELAPI void
sixel_output_destroy(sixel_output_t /* in */ *output); /* output context */

/* increase reference count of output context object (thread-unsafe) */
SIXELAPI void
sixel_output_ref(sixel_output_t /* in */ *output);     /* output context */

/* decrease reference count of output context object (thread-unsafe) */
SIXELAPI void
sixel_output_unref(sixel_output_t /* in */ *output);   /* output context */

/* get 8bit output mode which indicates whether it uses C1 control characters */
SIXELAPI int
sixel_output_get_8bit_availability(
    sixel_output_t /* in */ *output);   /* output context */

/* set 8bit output mode state */
SIXELAPI void
sixel_output_set_8bit_availability(
    sixel_output_t /* in */ *output,       /* output context */
    int            /* in */ availability); /* 0: do not use 8bit characters
                                              1: use 8bit characters */

/* set whether limit arguments of DECGRI('!') to 255 */
SIXELAPI void
sixel_output_set_gri_arg_limit(
    sixel_output_t /* in */ *output, /* output context */
    int            /* in */ value);  /* 0: don't limit arguments of DECGRI
                                        1: limit arguments of DECGRI to 255 */

/* set GNU Screen penetration feature enable or disable */
SIXELAPI void
sixel_output_set_penetrate_multiplexer(
    sixel_output_t /* in */ *output,    /* output context */
    int            /* in */ penetrate); /* 0: penetrate GNU Screen
                                           1: do not penetrate GNU Screen */

/* set whether we skip DCS envelope */
SIXELAPI void
sixel_output_set_skip_dcs_envelope(
    sixel_output_t /* in */ *output,   /* output context */
    int            /* in */ skip);     /* 0: output DCS envelope
                                          1: do not output DCS envelope */

/* set palette type: RGB or HLS */
SIXELAPI void
sixel_output_set_palette_type(
    sixel_output_t /* in */ *output,      /* output context */
    int            /* in */ palettetype); /* PALETTETYPE_RGB: RGB palette
                                             PALETTETYPE_HLS: HLS palette */

/* set encodeing policy: auto, fast or size */
SIXELAPI void
sixel_output_set_encode_policy(
    sixel_output_t /* in */ *output,    /* output context */
    int            /* in */ encode_policy);


#ifdef __cplusplus
}
#endif


/* color quantization API */

/* handle type of dither context object */
struct sixel_dither;
typedef struct sixel_dither sixel_dither_t;

#ifdef __cplusplus
extern "C" {
#endif

/* create dither context object */
SIXELAPI SIXELSTATUS
sixel_dither_new(
    sixel_dither_t      /* out */   **ppdither,  /* dither object to be created */
    int                 /* in */    ncolors,     /* required colors */
    sixel_allocator_t   /* in */    *allocator); /* allocator, null if you use
                                                    default allocator */

/* create dither context object */
SIXELAPI __attribute__((deprecated)) sixel_dither_t *
sixel_dither_create(int /* in */ ncolors); /* number of colors */

/* get built-in dither context object */
SIXELAPI sixel_dither_t *
sixel_dither_get(int builtin_dither); /* ID of built-in dither object */

/* destroy dither context object */
SIXELAPI void
sixel_dither_destroy(sixel_dither_t *dither); /* dither context object */

/* increase reference count of dither context object (thread-unsafe) */
SIXELAPI void
sixel_dither_ref(sixel_dither_t *dither); /* dither context object */

/* decrease reference count of dither context object (thread-unsafe) */
SIXELAPI void
sixel_dither_unref(sixel_dither_t *dither); /* dither context object */

/* initialize internal palette from specified pixel buffer */
SIXELAPI SIXELSTATUS
sixel_dither_initialize(
    sixel_dither_t *dither,                    /* dither context object */
    unsigned char /* in */ *data,              /* sample image */
    int           /* in */ width,              /* image width */
    int           /* in */ height,             /* image height */
    int           /* in */ pixelformat,        /* one of enum pixelFormat */
    int           /* in */ method_for_largest, /* method for finding the largest dimension */
    int           /* in */ method_for_rep,     /* method for choosing a color from the box */
    int           /* in */ quality_mode);      /* quality of histogram processing */

/* set diffusion type, choose from enum methodForDiffuse */
SIXELAPI void
sixel_dither_set_diffusion_type(
    sixel_dither_t /* in */ *dither,   /* dither context object */
    int /* in */ method_for_diffuse);  /* one of enum methodForDiffuse */

/* get number of palette colors */
SIXELAPI int
sixel_dither_get_num_of_palette_colors(
    sixel_dither_t /* in */ *dither);  /* dither context object */

/* get number of histogram colors */
SIXELAPI int
sixel_dither_get_num_of_histogram_colors(
    sixel_dither_t /* in */ *dither);  /* dither context object */

SIXELAPI __attribute__((deprecated)) int /* typoed! remains for compatibility. */
sixel_dither_get_num_of_histgram_colors(
    sixel_dither_t /* in */ *dither);  /* dither context object */

/* get palette */
SIXELAPI unsigned char *
sixel_dither_get_palette(
    sixel_dither_t /* in */ *dither);  /* dither context object */

/* set palette */
SIXELAPI void
sixel_dither_set_palette(
    sixel_dither_t /* in */ *dither,   /* dither context object */
    unsigned char  /* in */ *palette);

/* set the factor of complexion color correcting */
SIXELAPI void
sixel_dither_set_complexion_score(
    sixel_dither_t /* in */ *dither,   /* dither context object */
    int            /* in */ score);    /* complexion score (>= 1) */

/* set whether omitting palette difinition */
SIXELAPI void
sixel_dither_set_body_only(
    sixel_dither_t /* in */ *dither,   /* dither context object */
    int            /* in */ bodyonly); /* 0: output palette section(default)
                                          1: do not output palette section */
/* set whether optimize palette size */
SIXELAPI void
sixel_dither_set_optimize_palette(
    sixel_dither_t /* in */ *dither,   /* dither context object */
    int            /* in */ do_opt);   /* 0: optimize palette size
                                          1: don't optimize palette size */
/* set pixelformat */
SIXELAPI void
sixel_dither_set_pixelformat(
    sixel_dither_t /* in */ *dither,      /* dither context object */
    int            /* in */ pixelformat); /* one of enum pixelFormat */

/* set transparent */
SIXELAPI void
sixel_dither_set_transparent(
    sixel_dither_t /* in */ *dither,      /* dither context object */
    int            /* in */ transparent); /* transparent color index */

#ifdef __cplusplus
}
#endif

/* converter API */

typedef void * (* sixel_allocator_function)(size_t size);

#ifdef __cplusplus
extern "C" {
#endif

/* convert pixels into sixel format and write it to output context */
SIXELAPI SIXELSTATUS
sixel_encode(
    unsigned char  /* in */ *pixels,     /* pixel bytes */
    int            /* in */  width,      /* image width */
    int            /* in */  height,     /* image height */
    int            /* in */  depth,      /* color depth: now unused */
    sixel_dither_t /* in */ *dither,     /* dither context */
    sixel_output_t /* in */ *context);   /* output context */

/* convert sixel data into indexed pixel bytes and palette data */
SIXELAPI SIXELSTATUS
sixel_decode_raw(
    unsigned char       /* in */  *p,           /* sixel bytes */
    int                 /* in */  len,          /* size of sixel bytes */
    unsigned char       /* out */ **pixels,     /* decoded pixels */
    int                 /* out */ *pwidth,      /* image width */
    int                 /* out */ *pheight,     /* image height */
    unsigned char       /* out */ **palette,    /* ARGB palette */
    int                 /* out */ *ncolors,     /* palette size (<= 256) */
    sixel_allocator_t   /* in */  *allocator);  /* allocator object or null */

SIXELAPI __attribute__((deprecated)) SIXELSTATUS
sixel_decode(
    unsigned char            /* in */  *sixels,    /* sixel bytes */
    int                      /* in */  size,       /* size of sixel bytes */
    unsigned char            /* out */ **pixels,   /* decoded pixels */
    int                      /* out */ *pwidth,    /* image width */
    int                      /* out */ *pheight,   /* image height */
    unsigned char            /* out */ **palette,  /* RGBA palette */
    int                      /* out */ *ncolors,   /* palette size (<= 256) */
    sixel_allocator_function /* in */  fn_malloc); /* malloc function */

#ifdef __cplusplus
}
#endif

/* helper API */

#ifdef __cplusplus
extern "C" {
#endif

SIXELAPI void
sixel_helper_set_additional_message(
    const char      /* in */  *message         /* error message */
);

SIXELAPI char const *
sixel_helper_get_additional_message(void);

/* convert error status code int formatted string */
SIXELAPI char const *
sixel_helper_format_error(
    SIXELSTATUS     /* in */  status           /* status code */
);

/* compute pixel depth from pixelformat */
SIXELAPI int
sixel_helper_compute_depth(
    int /* in */ pixelformat /* one of enum pixelFormat */
);

/* convert pixelFormat into PIXELFORMAT_RGB888 */
SIXELAPI SIXELSTATUS
sixel_helper_normalize_pixelformat(
    unsigned char       /* out */ *dst,             /* destination buffer */
    int                 /* out */ *dst_pixelformat, /* converted pixelformat */
    unsigned char const /* in */  *src,             /* source pixels */
    int                 /* in */  src_pixelformat,  /* format of source image */
    int                 /* in */  width,            /* width of source image */
    int                 /* in */  height            /* height of source image */
);

/* scale image to specified size */
SIXELAPI SIXELSTATUS
sixel_helper_scale_image(
    unsigned char       /* out */ *dst,                  /* destination buffer */
    unsigned char const /* in */  *src,                  /* source image data */
    int                 /* in */  srcw,                  /* source image width */
    int                 /* in */  srch,                  /* source image height */
    int                 /* in */  pixelformat,           /* one of enum pixelFormat */
    int                 /* in */  dstw,                  /* destination image width */
    int                 /* in */  dsth,                  /* destination image height */
    int                 /* in */  method_for_resampling, /* one of methodForResampling */
    sixel_allocator_t   /* in */  *allocator             /* allocator object */
);

#ifdef __cplusplus
}
#endif


/* image loader/writer API */

#if SIXEL_USE_DEPRECATED_SYMBOLS
enum imageFormat {
    FORMAT_GIF   =  0, /* read only */
    FORMAT_PNG   =  1, /* read/write */
    FORMAT_BMP   =  2, /* read only */
    FORMAT_JPG   =  3, /* read only */
    FORMAT_TGA   =  4, /* read only */
    FORMAT_WBMP  =  5, /* read only with --with-gd configure option */
    FORMAT_TIFF  =  6, /* read only */
    FORMAT_SIXEL =  7, /* read only */
    FORMAT_PNM   =  8, /* read only */
    FORMAT_GD2   =  9, /* read only with --with-gd configure option */
    FORMAT_PSD   = 10, /* read only */
    FORMAT_HDR   = 11  /* read only */
};

/* loop mode */
enum loopControl {
    LOOP_AUTO    = 0,  /* honer the setting of GIF header */
    LOOP_FORCE   = 1,  /* always enable loop */
    LOOP_DISABLE = 2   /* always disable loop */
};
#endif

/* handle type of dither context object */
struct sixel_frame;
typedef struct sixel_frame sixel_frame_t;

#ifdef __cplusplus
extern "C" {
#endif

/* constructor of frame object */
SIXELAPI SIXELSTATUS
sixel_frame_new(
    sixel_frame_t       /* out */ **ppframe,    /* frame object to be created */
    sixel_allocator_t   /* in */  *allocator);  /* allocator, null if you use
                                                   default allocator */
/* deprecated version of sixel_frame_new() */
SIXELAPI __attribute__((deprecated)) sixel_frame_t *
sixel_frame_create(void);

/* increase reference count of frame object (thread-unsafe) */
SIXELAPI void
sixel_frame_ref(sixel_frame_t /* in */ *frame);

/* decrease reference count of frame object (thread-unsafe) */
SIXELAPI void
sixel_frame_unref(sixel_frame_t /* in */ *frame);

/* initialize frame object with a pixel buffer */
SIXELAPI SIXELSTATUS
sixel_frame_init(
    sixel_frame_t   /* in */ *frame,        /* frame object to be initialize */
    unsigned char   /* in */ *pixels,       /* pixel buffer */
    int             /* in */ width,         /* pixel width of buffer */
    int             /* in */ height,        /* pixel height of buffer */
    int             /* in */ pixelformat,   /* pixelformat of buffer */
    unsigned char   /* in */ *palette,      /* palette for buffer or NULL */
    int             /* in */ ncolors        /* number of palette colors or (-1) */
);

/* get pixels */
SIXELAPI unsigned char *
sixel_frame_get_pixels(sixel_frame_t /* in */ *frame);  /* frame object */

/* get palette */
SIXELAPI unsigned char *
sixel_frame_get_palette(sixel_frame_t /* in */ *frame);  /* frame object */

/* get width */
SIXELAPI int
sixel_frame_get_width(sixel_frame_t /* in */ *frame);  /* frame object */

/* get height */
SIXELAPI int
sixel_frame_get_height(sixel_frame_t /* in */ *frame);  /* frame object */

/* get ncolors */
SIXELAPI int
sixel_frame_get_ncolors(sixel_frame_t /* in */ *frame);  /* frame object */

/* get pixelformat */
SIXELAPI int
sixel_frame_get_pixelformat(sixel_frame_t /* in */ *frame);  /* frame object */

/* get transparent */
SIXELAPI int
sixel_frame_get_transparent(sixel_frame_t /* in */ *frame);  /* frame object */

/* get transparent */
SIXELAPI int
sixel_frame_get_multiframe(sixel_frame_t /* in */ *frame);  /* frame object */

/* get delay */
SIXELAPI int
sixel_frame_get_delay(sixel_frame_t /* in */ *frame);  /* frame object */

/* get frame no */
SIXELAPI int
sixel_frame_get_frame_no(sixel_frame_t /* in */ *frame);  /* frame object */

/* get loop no */
SIXELAPI int
sixel_frame_get_loop_no(sixel_frame_t /* in */ *frame);  /* frame object */

/* strip alpha from RGBA/ARGB formatted pixbuf */
SIXELAPI int
sixel_frame_strip_alpha(
    sixel_frame_t  /* in */ *frame,
    unsigned char  /* in */ *bgcolor);

/* resize a frame to given size with specified resampling filter */
SIXELAPI SIXELSTATUS
sixel_frame_resize(
    sixel_frame_t  /* in */ *frame,
    int            /* in */ width,
    int            /* in */ height,
    int            /* in */ method_for_resampling);

/* clip frame */
SIXELAPI SIXELSTATUS
sixel_frame_clip(
    sixel_frame_t  /* in */ *frame,
    int            /* in */ x,
    int            /* in */ y,
    int            /* in */ width,
    int            /* in */ height);

typedef SIXELSTATUS (* sixel_load_image_function)(
    sixel_frame_t /* in */     *frame,
    void          /* in/out */ *context);

/* Note: this function returns SIXEL_OK without calling FN_LOAD when the file
   content is empty or 1-byte LF.  This implies an assumption that CONTEXT is
   initialized to be the default value for the empty file before calling this
   function.  If it is not the case, the caller needs to properly detect it and
   handle this, or otherwise, CONTEXT can be used uninitialized in subsequent
   codes. */
SIXELAPI SIXELSTATUS
sixel_helper_load_image_file(
    char const                /* in */     *filename,     /* source file name */
    int                       /* in */     fstatic,       /* whether to extract static image */
    int                       /* in */     fuse_palette,  /* whether to use paletted image */
    int                       /* in */     reqcolors,     /* requested number of colors */
    unsigned char             /* in */     *bgcolor,      /* background color */
    int                       /* in */     loop_control,  /* one of enum loopControl */
    sixel_load_image_function /* in */     fn_load,       /* callback */
    int                       /* in */     finsecure,     /* true if do not verify SSL */
    int const                 /* in */     *cancel_flag,  /* cancel flag */
    void                      /* in/out */ *context,      /* private data for callback */
    sixel_allocator_t         /* in */     *allocator);   /* allocator object */

/* write image to file */
SIXELAPI SIXELSTATUS
sixel_helper_write_image_file(
    unsigned char       /* in */ *data,        /* source pixel data */
    int                 /* in */ width,        /* source data width */
    int                 /* in */ height,       /* source data height */
    unsigned char       /* in */ *palette,     /* palette of source data */
    int                 /* in */ pixelformat,  /* source pixelFormat */
    char const          /* in */ *filename,    /* destination filename */
    int                 /* in */ imageformat,  /* one of enum imageformat */
    sixel_allocator_t   /* in */ *allocator);  /* allocator object */

#ifdef __cplusplus
}
#endif


/* easy encoder API */

/* handle type of dither context object */
struct sixel_encoder;
typedef struct sixel_encoder sixel_encoder_t;

#ifdef __cplusplus
extern "C" {
#endif

/* create encoder object */
SIXELAPI SIXELSTATUS
sixel_encoder_new(
    sixel_encoder_t     /* out */ **ppencoder, /* encoder object to be created */
    sixel_allocator_t   /* in */  *allocator); /* allocator, null if you use
                                                  default allocator */

/* deprecated version of sixel_decoder_new() */
SIXELAPI __attribute__((deprecated)) sixel_encoder_t *
sixel_encoder_create(void);

/* increase reference count of encoder object (thread-unsafe) */
SIXELAPI void
sixel_encoder_ref(sixel_encoder_t /* in */ *encoder);

/* decrease reference count of encoder object (thread-unsafe) */
SIXELAPI void
sixel_encoder_unref(sixel_encoder_t /* in */ *encoder);

/* set cancel state flag to encoder object */
SIXELAPI SIXELSTATUS
sixel_encoder_set_cancel_flag(
    sixel_encoder_t /* in */ *encoder,
    int             /* in */ *cancel_flag);

/* set an option flag to encoder object */
SIXELAPI SIXELSTATUS
sixel_encoder_setopt(
    sixel_encoder_t /* in */ *encoder,
    int             /* in */ arg,
    char const      /* in */ *optarg);

/* load source data from specified file and encode it to SIXEL format */
SIXELAPI SIXELSTATUS
sixel_encoder_encode(
    sixel_encoder_t /* in */ *encoder,
    char const      /* in */ *filename);

/* encode specified pixel data to SIXEL format
 * output to encoder->outfd */
SIXELAPI SIXELSTATUS
sixel_encoder_encode_bytes(
    sixel_encoder_t     /* in */    *encoder,
    unsigned char       /* in */    *bytes,
    int                 /* in */    width,
    int                 /* in */    height,
    int                 /* in */    pixelformat,
    unsigned char       /* in */    *palette,
    int                 /* in */    ncolors);

#ifdef __cplusplus
}
#endif


/* easy encoder API */

/* handle type of dither context object */
struct sixel_decoder;
typedef struct sixel_decoder sixel_decoder_t;

#ifdef __cplusplus
extern "C" {
#endif

/* create decoder object */
SIXELAPI SIXELSTATUS
sixel_decoder_new(
    sixel_decoder_t    /* out */ **ppdecoder,  /* decoder object to be created */
    sixel_allocator_t  /* in */  *allocator);  /* allocator, null if you use
                                                  default allocator */

/* deprecated version of sixel_decoder_new() */
SIXELAPI __attribute__((deprecated)) sixel_decoder_t *
sixel_decoder_create(void);

/* increase reference count of decoder object (thread-unsafe) */
SIXELAPI void
sixel_decoder_ref(sixel_decoder_t *decoder);

/* decrease reference count of decoder object (thread-unsafe) */
SIXELAPI void
sixel_decoder_unref(sixel_decoder_t *decoder);

/* set an option flag to decoder object */
SIXELAPI SIXELSTATUS
sixel_decoder_setopt(
    sixel_decoder_t /* in */ *decoder,  /* decoder object */
    int             /* in */ arg,       /* one of SIXEL_OPTFLAG_*** */
    char const      /* in */ *optarg);  /* null or an argument of optflag */

/* load source data from stdin or the file specified with
   SIXEL_OPTFLAG_INPUT flag, and decode it */
SIXELAPI SIXELSTATUS
sixel_decoder_decode(
    sixel_decoder_t /* in */ *decoder);

#ifdef __cplusplus
}
#endif

#endif  /* LIBSIXEL_SIXEL_H */

/* emacs Local Variables:      */
/* emacs mode: c               */
/* emacs tab-width: 4          */
/* emacs indent-tabs-mode: nil */
/* emacs c-basic-offset: 4     */
/* emacs End:                  */
/* vim: set expandtab ts=4 sts=4 sw=4 : */
/* EOF */
