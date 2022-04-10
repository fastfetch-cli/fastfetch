#include "logo.h"

#ifdef FF_HAVE_IMAGEMAGICK

#include <MagickCore/MagickCore.h>
#include <sys/ioctl.h>
#include <unistd.h>

static bool printSixel(FFinstance* instance)
{
    struct winsize winsize;
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsize) != 0)
        return false;

    FF_LIBRARY_LOAD(imageMagick, instance->config.libImageMagick, false, "libMagickCore-7.Q16HDRI.so", "libMagickCore-7.Q16HDRI.so.10")
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, AcquireExceptionInfo, false)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, DestroyExceptionInfo, false)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, AcquireImageInfo, false)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, DestroyImageInfo, false)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, CopyMagickString, false)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, ReadImage, false)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, ResizeImage, false)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, WriteImage, false)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, DestroyImage, false)

    ExceptionInfo* exceptionInfo = ffAcquireExceptionInfo();
    if(exceptionInfo == NULL)
    {
        dlclose(imageMagick);
        return false;
    }

    ImageInfo* imageInfoIn = ffAcquireImageInfo();
    if(imageInfoIn == NULL)
    {
        ffDestroyExceptionInfo(exceptionInfo);
        dlclose(imageMagick);
        return false;
    }

    //+1, because we need to copy the null byte too
    ffCopyMagickString(imageInfoIn->filename, instance->config.logoName.chars, instance->config.logoName.length + 1);

    Image* originalImage = ffReadImage(imageInfoIn, exceptionInfo);
    ffDestroyImageInfo(imageInfoIn);
    if(originalImage == NULL)
    {
        ffDestroyExceptionInfo(exceptionInfo);
        dlclose(imageMagick);
        return false;
    }

    double characterPixelWidth = winsize.ws_xpixel / (double) winsize.ws_col;
    double characterPixelHeight = winsize.ws_ypixel / (double) winsize.ws_row;

    //We keep the values as double, to have more precise calculations later
    double imagePixelWidth = (double) instance->config.logoWidth * characterPixelWidth;
    double imagePixelHeight = (imagePixelWidth / (double) originalImage->columns) * (double) originalImage->rows;

    if(imagePixelWidth < 1.0 || imagePixelHeight < 1.0)
    {
        ffDestroyImage(originalImage);
        ffDestroyExceptionInfo(exceptionInfo);
        dlclose(imageMagick);
        return false;
    }

    Image* resizedImage = ffResizeImage(originalImage, (size_t) imagePixelWidth, (size_t) imagePixelHeight, UndefinedFilter, exceptionInfo);
    ffDestroyImage(originalImage);
    if(resizedImage == NULL)
    {
        ffDestroyExceptionInfo(exceptionInfo);
        dlclose(imageMagick);
        return false;
    }

    ImageInfo* imageInfoOut = ffAcquireImageInfo();
    if(imageInfoOut == NULL)
    {
        ffDestroyImage(resizedImage);
        ffDestroyExceptionInfo(exceptionInfo);
        dlclose(imageMagick);
        return false;
    }

    ffPrintCharTimes(' ', instance->config.logoPaddingLeft);

    imageInfoOut->file = stdout;
    ffCopyMagickString(imageInfoOut->magick, "SIXEL", 6);

    MagickBooleanType writeResult = ffWriteImage(imageInfoOut, resizedImage, exceptionInfo);

    ffDestroyImageInfo(imageInfoOut);
    ffDestroyImage(resizedImage);
    ffDestroyExceptionInfo(exceptionInfo);
    dlclose(imageMagick);

    if(writeResult == MagickFalse)
        return false;

    instance->state.logoHeight = (uint32_t) (imagePixelHeight / characterPixelHeight);
    instance->state.logoWidth = instance->config.logoWidth + instance->config.logoPaddingLeft + instance->config.logoPaddingRight;

    fputs("\033[9999999D", stdout);
    printf("\033[%uA", instance->state.logoHeight);

    return true;
}

#endif

bool ffLogoPrintSixelIfExists(FFinstance* instance)
{
    #ifdef FF_HAVE_IMAGEMAGICK
        return printSixel(instance);
    #else
        FF_UNUSED(instance);
    #endif

    return false;
}

void ffLogoPrintSixel(FFinstance* instance)
{
    if(!ffLogoPrintSixelIfExists(instance))
        ffLogoPrintBuiltinDetected(instance);
}
