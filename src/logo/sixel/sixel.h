#pragma once

#ifndef FF_INCLUDED_LOGO_SIXEL_sixel
#define FF_INCLUDED_LOGO_SIXEL_sixel

#include "fastfetch.h"

#ifdef FF_HAVE_IMAGEMAGICK7
bool ffLogoPrintSixelIM7(FFinstance* instance);
#endif

#ifdef FF_HAVE_IMAGEMAGICK6
#ifndef isnan
    #define isnan(x) ((x) != (x))
#endif
bool ffLogoPrintSixelIM6(FFinstance* instance);
#endif

#if defined(FF_HAVE_IMAGEMAGICK7) || defined(FF_HAVE_IMAGEMAGICK6)

#include <sys/ioctl.h>
#include <unistd.h>

#define MAGICKCORE_HDRI_ENABLE 1
#define MAGICKCORE_QUANTUM_DEPTH 16

typedef void*(*FFLogoIMResizeFunc)(const void* image, size_t width, size_t height, void* exceptionInfo);
typedef bool(*FFLogoIMWriteFunc)(void* image, const void* imageInfo, void* exceptionInfo);

bool ffLogoPrintSixelImpl(FFinstance* instance, void* library, FFLogoIMResizeFunc resizeFunc, FFLogoIMWriteFunc writeFunc);

#endif

#endif
