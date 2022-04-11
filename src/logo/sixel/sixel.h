#pragma once

#ifndef FF_INCLUDED_LOGO_SIXEL_sixel
#define FF_INCLUDED_LOGO_SIXEL_sixel

#include "fastfetch.h"

#if defined(FF_HAVE_IMAGEMAGICK7) || defined(FF_HAVE_IMAGEMAGICK6)

#include <sys/ioctl.h>
#include <unistd.h>

#define MAGICKCORE_HDRI_ENABLE 1
#define MAGICKCORE_QUANTUM_DEPTH 16

typedef enum FFLogoSixelResult
{
    FF_LOGO_SIXEL_RESULT_SUCCESS,    //Logo printed
    FF_LOGO_SIXEL_RESULT_INIT_ERROR, //Failed to load library, try again with next IM version
    FF_LOGO_SIXEL_RESULT_RUN_ERROR   //Failed to load / convert image, cancle whole sixel code
} FFLogoSixelResult;

typedef void*(*FFLogoIMResizeFunc)(const void* image, size_t width, size_t height, void* exceptionInfo);
typedef bool(*FFLogoIMWriteFunc)(void* image, const void* imageInfo, void* exceptionInfo);

FFLogoSixelResult ffLogoPrintSixelImpl(FFinstance* instance, void* library, FFLogoIMResizeFunc resizeFunc, FFLogoIMWriteFunc writeFunc);

#endif

#ifdef FF_HAVE_IMAGEMAGICK7
FFLogoSixelResult ffLogoPrintSixelIM7(FFinstance* instance);
#endif

#ifdef FF_HAVE_IMAGEMAGICK6
#include <math.h>
FFLogoSixelResult ffLogoPrintSixelIM6(FFinstance* instance);
#endif


#endif
