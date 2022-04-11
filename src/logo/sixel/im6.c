#include "sixel.h"

#ifdef FF_HAVE_IMAGEMAGICK6

#include <magick/MagickCore.h>

static FF_LIBRARY_SYMBOL(ResizeImage);
static FF_LIBRARY_SYMBOL(WriteImage)

static void* logoResizeIM6(const void* image, size_t width, size_t height, void* exceptionInfo)
{
    return ffResizeImage(image, width, height, UndefinedFilter, 1.0, exceptionInfo);
}

static bool logoWriteIM6(void* image, const void* imageInfo, void* exceptionInfo)
{
    FF_UNUSED(exceptionInfo);
    return ffWriteImage(imageInfo, image) == MagickTrue;
}

bool ffLogoPrintSixelIM6(FFinstance* instance)
{
    FF_LIBRARY_LOAD(imageMagick, instance->config.libImageMagick, "libMagickCore-6.Q16HDRI.so", "libMagickCore-6.Q16HDRI.so.6", "libMagickCore-6.Q16.so", "libMagickCore-6.Q16.so.6")

    FF_LIBRARY_LOAD_SYMBOL_ADRESS(imageMagick, ffResizeImage, ResizeImage, false);
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(imageMagick, ffWriteImage, WriteImage, false);

    return ffLogoPrintSixelImpl(instance, imageMagick, logoResizeIM6, logoWriteIM6);
}

#endif
