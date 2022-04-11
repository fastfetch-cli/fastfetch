#include "sixel.h"

#ifdef FF_HAVE_IMAGEMAGICK7

#include <MagickCore/MagickCore.h>

static FF_LIBRARY_SYMBOL(ResizeImage);
static FF_LIBRARY_SYMBOL(WriteImage)

static void* logoResizeIM7(const void* image, size_t width, size_t height, void* exceptionInfo)
{
    return ffResizeImage(image, width, height, UndefinedFilter, exceptionInfo);
}

static bool logoWriteIM7(void* image, const void* imageInfo, void* exceptionInfo)
{
    return ffWriteImage(imageInfo, image, exceptionInfo) == MagickTrue;
}

bool ffLogoPrintSixelIM7(FFinstance* instance)
{
    FF_LIBRARY_LOAD(imageMagick, instance->config.libImageMagick, false, "libMagickCore-7.Q16HDRI.so", 11, "libMagickCore-7.Q16.so", 11)

    FF_LIBRARY_LOAD_SYMBOL_ADRESS(imageMagick, ffResizeImage, ResizeImage, false);
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(imageMagick, ffWriteImage, WriteImage, false);

    return ffLogoPrintSixelImpl(instance, imageMagick, logoResizeIM7, logoWriteIM7);
}

#endif
