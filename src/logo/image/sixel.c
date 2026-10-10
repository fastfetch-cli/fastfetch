#include "image.h"

#ifdef FF_HAVE_SIXEL

    #include <string.h>
    #include <sixel.h>
    // The transparent path needs two things the public header does not declare: the palette mapping
    // -- which is where libsixel applies the Floyd-Steinberg dithering, so doing it by hand would
    // band the gradients -- and the palette size the encoder's keycolor handling is built around.
    // The vendored copy is pinned to the exact revision this build compiles, so there is no
    // version skew to guard against.
    #include "dither.h"

static int sixelWriteCallback(char* data, int size, void* priv) {
    ffStrbufAppendNS((FFstrbuf*) priv, (uint32_t) size, data);
    return 1; // non-zero means "keep going"
}

// Only a fully transparent pixel is left out. A partially transparent one can not be represented --
// a sixel has no way to blend it with whatever the terminal has behind it -- and painting it in its
// unblended colour is closer to the source than dropping it would be.
static bool hasTransparentPixels(const FFImageBuffer* buffer) {
    const uint8_t* pixels = buffer->data;
    const uint32_t pixelCount = buffer->width * buffer->height;
    for (uint32_t i = 0; i < pixelCount; ++i) {
        if (pixels[i * 4 + 3] == 0) {
            return true;
        }
    }

    return false;
}

// Encodes an image with transparent pixels as an indexed image whose transparent pixels carry a
// keycolor: that entry is left out of the palette definition and its pixels are left out of the
// pixel data, so nothing is drawn there and the terminal background shows through.
static SIXELSTATUS sixelEncodeTransparent(sixel_dither_t* dither, const FFImageBuffer* buffer, sixel_output_t* output) {
    const int width = (int) buffer->width;
    const int height = (int) buffer->height;

    // Map the pixels onto the palette through libsixel's own lookup, then hand the result back as
    // an indexed image. This is the same mapping the opaque path gets from sixel_encode.
    sixel_index_t* indices = sixel_dither_apply_palette(dither, buffer->data, width, height);
    if (indices == nullptr) {
        return SIXEL_RUNTIME_ERROR;
    }

    // The keycolor is the first index past the quantized palette, so it is never a colour that a
    // pixel was mapped to. Asking for one colour less than the maximum is what guarantees the
    // index exists.
    const int keycolor = sixel_dither_get_num_of_palette_colors(dither);
    const uint8_t* pixels = buffer->data;
    for (uint32_t i = 0; i < (uint32_t) width * (uint32_t) height; ++i) {
        if (pixels[i * 4 + 3] == 0) {
            indices[i] = (sixel_index_t) keycolor;
        }
    }

    // The encoder sizes its per-colour map from ncolors and skips the keycolor inside that range,
    // so the keycolor has to be counted in it. Its own palette entry is never read: leaving the
    // definition out is exactly what the keycolor means.
    int ncolors = keycolor + 1;
    if (ncolors < 3) {
        // Two colours mean "the terminal's default black and white palette, no definitions needed"
        // to the encoder, which would be wrong here: one of the two is the keycolor, not a colour.
        // The padding entry is written out so no uninitialised palette byte reaches the output.
        ncolors = 3;
        memset(dither->palette + keycolor * 3, 0, (size_t) (ncolors - keycolor) * 3);
    }
    dither->ncolors = ncolors;

    sixel_dither_set_transparent(dither, keycolor);
    // PAL8 hands the indices over as they are, instead of mapping the pixels a second time
    sixel_dither_set_pixelformat(dither, SIXEL_PIXELFORMAT_PAL8);

    SIXELSTATUS status = sixel_encode(indices, width, height, 1, dither, output);

    sixel_allocator_free(dither->allocator, indices);
    return status;
}

bool ffSixelEncode(const FFImageBuffer* buffer, FFstrbuf* result, const char** error) {
    // A source without fully transparent pixels takes the opaque path, unchanged
    const bool transparent = hasTransparentPixels(buffer);

    sixel_dither_t* dither = nullptr;
    if (sixel_dither_new(&dither, transparent ? SIXEL_PALETTE_MAX - 1 : SIXEL_PALETTE_MAX, nullptr) != SIXEL_OK) {
        if (error) {
            *error = "sixel_dither_new() failed";
        }
        return false;
    }

    // Feed RGBA8 directly and let libsixel do the palette quantization and dithering
    if (sixel_dither_initialize(dither, buffer->data, (int) buffer->width, (int) buffer->height, SIXEL_PIXELFORMAT_RGBA8888, SIXEL_LARGE_AUTO, SIXEL_REP_AUTO, SIXEL_QUALITY_HIGH) != SIXEL_OK) {
        sixel_dither_unref(dither);
        if (error) {
            *error = "sixel_dither_initialize() failed";
        }
        return false;
    }

    sixel_output_t* output = nullptr;
    if (sixel_output_new(&output, sixelWriteCallback, result, nullptr) != SIXEL_OK) {
        sixel_dither_unref(dither);
        if (error) {
            *error = "sixel_output_new() failed";
        }
        return false;
    }

    // The depth parameter is unused by libsixel; the DCS envelope is emitted by default
    SIXELSTATUS status = transparent
        ? sixelEncodeTransparent(dither, buffer, output)
        : sixel_encode(buffer->data, (int) buffer->width, (int) buffer->height, 4, dither, output);

    sixel_output_unref(output);
    sixel_dither_unref(dither);

    if (status != SIXEL_OK || result->length == 0) {
        if (error) {
            *error = "sixel_encode() failed";
        }
        return false;
    }
    return true;
}

#endif
