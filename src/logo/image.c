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

    Image* image = ffReadImage(imageInfoIn, exceptionInfo);
    ffDestroyImageInfo(imageInfoIn);
    if(image == NULL)
    {
        ffDestroyExceptionInfo(exceptionInfo);
        dlclose(imageMagick);
        return false;
    }

    double characterPixelWidth = winsize.ws_xpixel / (double) winsize.ws_col;
    double characterPixelHeight = winsize.ws_ypixel / (double) winsize.ws_row;

    //We keep the values as double, to have more precise calculations later
    double imagePixelWidth = (double) instance->config.logoWidth * characterPixelWidth;
    double imagePixelHeight = (imagePixelWidth / (double) image->columns) * (double) image->rows;

    image = ffResizeImage(image, (size_t) imagePixelWidth, (size_t) imagePixelHeight, UndefinedFilter, exceptionInfo);
    if(image == NULL)
    {
        ffDestroyExceptionInfo(exceptionInfo);
        dlclose(imageMagick);
        return false;
    }

    ImageInfo* imageInfoOut = ffAcquireImageInfo();
    if(imageInfoOut == NULL)
    {
        ffDestroyImage(image);
        ffDestroyExceptionInfo(exceptionInfo);
        dlclose(imageMagick);
        return false;
    }

    ffPrintChar(' ', instance->config.logoPaddingLeft);

    imageInfoOut->file = stdout;
    ffCopyMagickString(imageInfoOut->magick, "SIXEL", 6);

    MagickBooleanType writeResult = ffWriteImage(imageInfoOut, image, exceptionInfo);

    ffDestroyImageInfo(imageInfoOut);
    ffDestroyImage(image);
    ffDestroyExceptionInfo(exceptionInfo);
    dlclose(imageMagick);

    if(writeResult == MagickFalse)
        return false;

    instance->state.logoHeight = (uint32_t) (imagePixelHeight / characterPixelHeight);
    instance->state.logoWidth =  (uint32_t) (imagePixelWidth / characterPixelWidth);
    instance->state.logoWidth += instance->config.logoPaddingLeft + instance->config.logoPaddingRight;

    fputs("\033[9999999D", stdout);

    if(instance->state.logoHeight > 0)
    {
        printf("\033[%uA", instance->state.logoHeight);
        return true;
    }

    return false;
}

#endif

void ffLogoPrintSixel(FFinstance* instance)
{
    bool sixelPrinted = false;

    #ifdef FF_HAVE_IMAGEMAGICK
        sixelPrinted = printSixel(instance);
    #endif

    if(!sixelPrinted)
        ffLogoPrintBuiltinDetected(instance);
}
