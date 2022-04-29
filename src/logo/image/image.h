#pragma once

#ifndef FF_INCLUDED_LOGO_SIXEL_sixel
#define FF_INCLUDED_LOGO_SIXEL_sixel

#include "../logo.h"

#if defined(FF_HAVE_IMAGEMAGICK7) || defined(FF_HAVE_IMAGEMAGICK6)

#include <sys/ioctl.h>

#define MAGICKCORE_HDRI_ENABLE 1
#define MAGICKCORE_QUANTUM_DEPTH 16

typedef struct FFLogoRequestData
{
    FFLogoType type;
    FFstrbuf cacheDir;

    struct winsize winsize;
    double characterPixelWidth;
    double characterPixelHeight;
    uint32_t imagePixelWidth;
    uint32_t imagePixelHeight;
} FFLogoRequestData;

typedef struct FFIMData
{
    void* library;
    void*(*resizeFunc)(const void* image, size_t width, size_t height, void* exceptionInfo);
} FFIMData;

FFLogoImageResult ffLogoPrintImageImpl(FFinstance* instance, FFLogoRequestData* requestData, const FFIMData* imData);

#endif

#ifdef FF_HAVE_IMAGEMAGICK7
FFLogoImageResult ffLogoPrintImageIM7(FFinstance* instance, FFLogoRequestData* requestData);
#endif

#ifdef FF_HAVE_IMAGEMAGICK6
#include <math.h>
FFLogoImageResult ffLogoPrintImageIM6(FFinstance* instance, FFLogoRequestData* requestData);
#endif


#endif
