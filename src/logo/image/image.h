#pragma once

#ifndef FF_INCLUDED_LOGO_SIXEL_sixel
#define FF_INCLUDED_LOGO_SIXEL_sixel

#include "fastfetch.h"

#if defined(FF_HAVE_IMAGEMAGICK7) || defined(FF_HAVE_IMAGEMAGICK6)

#define MAGICKCORE_HDRI_ENABLE 1
#define MAGICKCORE_QUANTUM_DEPTH 16

typedef enum FFLogoImageResult
{
    FF_LOGO_IMAGE_RESULT_SUCCESS,    //Logo printed
    FF_LOGO_IMAGE_RESULT_INIT_ERROR, //Failed to load library, try again with next IM version
    FF_LOGO_IMAGE_RESULT_RUN_ERROR   //Failed to load / convert image, cancle whole sixel code
} FFLogoImageResult;

typedef void*(*FFLogoIMResizeFunc)(const void* image, size_t width, size_t height, void* exceptionInfo);
typedef bool(*FFLogoIMWriteFunc)(void* imageInfo, void* image, void* exceptionInfo);

FFLogoImageResult ffLogoPrintImageImpl(FFinstance* instance, void* library, FFLogoIMResizeFunc resizeFunc, FFLogoIMWriteFunc writeFunc, FFLogoType type);

#endif

#ifdef FF_HAVE_IMAGEMAGICK7
FFLogoImageResult ffLogoPrintImageIM7(FFinstance* instance, FFLogoType type);
#endif

#ifdef FF_HAVE_IMAGEMAGICK6
#include <math.h>
FFLogoImageResult ffLogoPrintImageIM6(FFinstance* instance, FFLogoType type);
#endif


#endif
