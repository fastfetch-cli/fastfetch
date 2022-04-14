#ifdef FF_HAVE_IMAGEMAGICK6

#include "image.h"
#include <magick/MagickCore.h>

static FF_LIBRARY_SYMBOL(ResizeImage);
static FF_LIBRARY_SYMBOL(WriteImage);

static void* logoResize(const void* image, size_t width, size_t height, void* exceptionInfo)
{
    return ffResizeImage(image, width, height, UndefinedFilter, 1.0, exceptionInfo);
}

static bool logoWrite(void* imageInfo, void* image, void* exceptionInfo)
{
    FF_UNUSED(exceptionInfo);
    return ffWriteImage(imageInfo, image) == MagickTrue;
}

FFLogoImageResult ffLogoPrintImageIM6(FFinstance* instance, FFLogoType type)
{
    FF_LIBRARY_LOAD(imageMagick, instance->config.libImageMagick, FF_LOGO_IMAGE_RESULT_INIT_ERROR, "libMagickCore-6.Q16HDRI.so", 8, "libMagickCore-6.Q16.so", 8)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(imageMagick, ffResizeImage, ResizeImage, FF_LOGO_IMAGE_RESULT_INIT_ERROR);
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(imageMagick, ffWriteImage, WriteImage, FF_LOGO_IMAGE_RESULT_INIT_ERROR);

    FFLogoImageResult result = ffLogoPrintImageImpl(instance, imageMagick, logoResize, logoWrite, type);

    dlclose(imageMagick);

    return result;
}

#endif
