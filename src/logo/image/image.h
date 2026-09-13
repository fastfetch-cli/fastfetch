#pragma once

#include "../logo.h"

#if defined(FF_HAVE_IMAGEMAGICK7) || defined(FF_HAVE_IMAGEMAGICK6) || defined(_WIN32) || defined(FF_HAVE_SIXEL)

typedef enum FFLogoImageResult: uint8_t {
    FF_LOGO_IMAGE_RESULT_SUCCESS,    // Logo printed
    FF_LOGO_IMAGE_RESULT_INIT_ERROR, // Failed to load library, try again with next IM version
    FF_LOGO_IMAGE_RESULT_RUN_ERROR   // Failed to load / convert image, cancel whole sixel code
} FFLogoImageResult;

typedef struct FFLogoRequestData {
    FFLogoType type;
    FFstrbuf cacheDir;

    double characterPixelWidth;
    double characterPixelHeight;

    uint32_t logoPixelWidth;
    uint32_t logoPixelHeight;

    uint32_t logoCharacterHeight;
    uint32_t logoCharacterWidth;
} FFLogoRequestData;

// Decoded and resized image: RGBA8, straight (unassociated) alpha, no row padding
typedef struct FFImageBuffer {
    uint8_t* data;
    uint32_t width;
    uint32_t height;
} FFImageBuffer;

// Backend contract. Both functions update requestData->logoPixelWidth / logoPixelHeight
// with the real dimensions of the produced image, so the caller can derive the character
// dimensions afterwards.

// Decode the image source and resize it to the requested pixel size
bool ffImageCreate(FFLogoRequestData* requestData, FFImageBuffer* out, const char** error);
void ffImageDestroy(FFImageBuffer* buffer);

// Encode the image source as a sixel byte stream (including the DCS envelope).
// The encoder belongs to the backend: ImageMagick's SIXEL coder off Windows,
// the embedded libsixel on Windows.
bool ffImageSixelEncode(FFLogoRequestData* requestData, FFstrbuf* out, const char** error);

#endif

#ifdef FF_HAVE_IMAGEMAGICK7
bool ffImageCreateIM7(FFLogoRequestData* requestData, FFImageBuffer* out, const char** error);
bool ffImageSixelEncodeIM7(FFLogoRequestData* requestData, FFstrbuf* out, const char** error);
#endif

#ifdef FF_HAVE_IMAGEMAGICK6
bool ffImageCreateIM6(FFLogoRequestData* requestData, FFImageBuffer* out, const char** error);
bool ffImageSixelEncodeIM6(FFLogoRequestData* requestData, FFstrbuf* out, const char** error);
#endif

#ifdef _WIN32
bool ffImageCreateWIC(FFLogoRequestData* requestData, FFImageBuffer* out, const char** error);
#endif

#ifdef FF_HAVE_SIXEL
bool ffSixelEncode(const FFImageBuffer* buffer, FFstrbuf* result, const char** error);
#endif
