#include "image.h"

#ifdef FF_HAVE_SIXEL

    #include <sixel.h> // src/3rdparty/sixel/sixel.h

static int sixelWriteCallback(char* data, int size, void* priv) {
    ffStrbufAppendNS((FFstrbuf*) priv, (uint32_t) size, data);
    return 1; // non-zero means "keep going"
}

bool ffSixelEncode(const FFImageBuffer* buffer, FFstrbuf* result, const char** error) {
    sixel_dither_t* dither = nullptr;
    if (sixel_dither_new(&dither, 256, nullptr) != SIXEL_OK) {
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
    SIXELSTATUS status = sixel_encode(buffer->data, (int) buffer->width, (int) buffer->height, 4, dither, output);

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
