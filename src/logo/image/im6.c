#ifdef FF_HAVE_IMAGEMAGICK6

#include "image.h"
#include "common/library.h"

#include <magick/MagickCore.h>

static FF_LIBRARY_SYMBOL(ResizeImage);

static void* logoResize(const void* image, size_t width, size_t height, void* exceptionInfo)
{
    return ffResizeImage(image, width, height, UndefinedFilter, 1.0, exceptionInfo);
}

FFLogoImageResult ffLogoPrintImageIM6(FFinstance* instance, FFLogoRequestData* requestData)
{
    FF_LIBRARY_LOAD(imageMagick, instance->config.libImageMagick, FF_LOGO_IMAGE_RESULT_INIT_ERROR, "libMagickCore-6.Q16HDRI.so", 8, "libMagickCore-6.Q16.so", 8)
    FF_LIBRARY_LOAD_SYMBOL_ADDRESS(imageMagick, ffResizeImage, ResizeImage, FF_LOGO_IMAGE_RESULT_INIT_ERROR);

    FFIMData imData;
    imData.resizeFunc = logoResize;
    imData.library = imageMagick;

    FFLogoImageResult result = ffLogoPrintImageImpl(instance, requestData, &imData);

    dlclose(imageMagick);

    return result;
}

#endif
