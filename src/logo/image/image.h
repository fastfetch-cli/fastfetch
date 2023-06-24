#pragma once

#ifndef FF_INCLUDED_LOGO_SIXEL_sixel
#define FF_INCLUDED_LOGO_SIXEL_sixel

#include "../logo.h"

#if defined(FF_HAVE_IMAGEMAGICK7) || defined(FF_HAVE_IMAGEMAGICK6)

typedef enum FFLogoImageResult
{
    FF_LOGO_IMAGE_RESULT_SUCCESS,    //Logo printed
    FF_LOGO_IMAGE_RESULT_INIT_ERROR, //Failed to load library, try again with next IM version
    FF_LOGO_IMAGE_RESULT_RUN_ERROR   //Failed to load / convert image, cancel whole sixel code
} FFLogoImageResult;

typedef struct FFLogoRequestData
{
    FFLogoType type;
    FFstrbuf cacheDir;

    double characterPixelWidth;
    double characterPixelHeight;

    uint32_t logoPixelWidth;
    uint32_t logoPixelHeight;

    uint32_t logoCharacterHeight;
    uint32_t logoCharacterWidth;
} FFLogoRequestData;

typedef struct FFIMData
{
    void* library;
    void*(*resizeFunc)(const void* image, size_t width, size_t height, void* exceptionInfo);
} FFIMData;

FFLogoImageResult ffLogoPrintImageImpl(FFLogoRequestData* requestData, const FFIMData* imData);

#endif

#ifdef FF_HAVE_IMAGEMAGICK7
FFLogoImageResult ffLogoPrintImageIM7(FFLogoRequestData* requestData);
#endif

#ifdef FF_HAVE_IMAGEMAGICK6
#include <math.h>
FFLogoImageResult ffLogoPrintImageIM6(FFLogoRequestData* requestData);
#endif

#endif
