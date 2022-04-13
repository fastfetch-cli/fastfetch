#include "image.h"

#if defined(FF_HAVE_IMAGEMAGICK7) || defined(FF_HAVE_IMAGEMAGICK6)

//We use only the defines from here, that are exactly the same in both versions
#ifdef FF_HAVE_IMAGEMAGICK7
    #include <MagickCore/MagickCore.h>
#else
    #include <magick/MagickCore.h>
#endif

FFLogoImageResult ffLogoPrintImageImpl(FFinstance* instance, void* imageMagick, FFLogoIMResizeFunc resizeFunc, FFLogoType type)
{
    struct winsize winsize;
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsize) != 0)
        return FF_LOGO_IMAGE_RESULT_RUN_ERROR;

    FF_LIBRARY_LOAD_SYMBOL(imageMagick, AcquireExceptionInfo, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, DestroyExceptionInfo, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, AcquireImageInfo, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, DestroyImageInfo, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, CopyMagickString, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, ReadImage, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, DestroyImage, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, ImageToBlob, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, Base64Encode, FF_LOGO_IMAGE_RESULT_INIT_ERROR)

    ExceptionInfo* exceptionInfo = ffAcquireExceptionInfo();
    if(exceptionInfo == NULL)
    {
        dlclose(imageMagick);
        return FF_LOGO_IMAGE_RESULT_RUN_ERROR;
    }

    ImageInfo* imageInfoIn = ffAcquireImageInfo();
    if(imageInfoIn == NULL)
    {
        ffDestroyExceptionInfo(exceptionInfo);
        dlclose(imageMagick);
        return FF_LOGO_IMAGE_RESULT_RUN_ERROR;
    }

    //+1, because we need to copy the null byte too
    ffCopyMagickString(imageInfoIn->filename, instance->config.logoName.chars, instance->config.logoName.length + 1);

    Image* originalImage = ffReadImage(imageInfoIn, exceptionInfo);
    ffDestroyImageInfo(imageInfoIn);
    if(originalImage == NULL)
    {
        ffDestroyExceptionInfo(exceptionInfo);
        dlclose(imageMagick);
        return FF_LOGO_IMAGE_RESULT_RUN_ERROR;
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
        return FF_LOGO_IMAGE_RESULT_RUN_ERROR;
    }

    Image* resizedImage = (Image*) resizeFunc(originalImage, (size_t) imagePixelWidth, (size_t) imagePixelHeight, exceptionInfo);
    ffDestroyImage(originalImage);
    if(resizedImage == NULL)
    {
        ffDestroyExceptionInfo(exceptionInfo);
        dlclose(imageMagick);
        return FF_LOGO_IMAGE_RESULT_RUN_ERROR;
    }

    ImageInfo* imageInfoOut = ffAcquireImageInfo();
    if(imageInfoOut == NULL)
    {
        ffDestroyImage(resizedImage);
        ffDestroyExceptionInfo(exceptionInfo);
        dlclose(imageMagick);
        return FF_LOGO_IMAGE_RESULT_RUN_ERROR;
    }

    if(type == FF_LOGO_TYPE_SIXEL)
        ffCopyMagickString(imageInfoOut->magick, "SIXEL", 6);
    else //Kitty
        ffCopyMagickString(imageInfoOut->magick, "RGBA", 5);

    size_t length;
    void* data = ffImageToBlob(imageInfoOut, resizedImage, &length, exceptionInfo);

    ffDestroyImageInfo(imageInfoOut);
    ffDestroyImage(resizedImage);
    ffDestroyExceptionInfo(exceptionInfo);
    dlclose(imageMagick);

    if(data == NULL || length == 0)
        return false;

    ffPrintCharTimes(' ', instance->config.logoPaddingLeft);

    if(type == FF_LOGO_TYPE_KITTY)
    {
        void* encoded = ffBase64Encode(data, length, &length);
        free(data);
        data = encoded;
        printf("\033_Ga=T,f=32,s=%u,v=%u;", (uint32_t) imagePixelWidth, (uint32_t) imagePixelHeight);
    }

    fwrite(data, sizeof(char), length, stdout);
    free(data);

    if(type == FF_LOGO_TYPE_KITTY)
        fputs("\033\\", stdout);

    instance->state.logoHeight = (uint32_t) (imagePixelHeight / characterPixelHeight);
    instance->state.logoWidth = instance->config.logoWidth + instance->config.logoPaddingLeft + instance->config.logoPaddingRight;

    fputs("\033[9999999D", stdout);
    printf("\033[%uA", instance->state.logoHeight);

    return FF_LOGO_IMAGE_RESULT_SUCCESS;
}

#endif

bool ffLogoPrintImageIfExists(FFinstance* instance, FFLogoType type)
{
    #if !defined(FF_HAVE_IMAGEMAGICK7) && !defined(FF_HAVE_IMAGEMAGICK6)
        FF_UNUSED(instance);
    #endif

    #ifdef FF_HAVE_IMAGEMAGICK7
        FFLogoImageResult result = ffLogoPrintImageIM7(instance, type);
        if(result == FF_LOGO_IMAGE_RESULT_SUCCESS)
            return true;
        else if(result == FF_LOGO_IMAGE_RESULT_RUN_ERROR)
            return false;
    #endif

    #ifdef FF_HAVE_IMAGEMAGICK6
        return ffLogoPrintImageIM6(instance, type) == FF_LOGO_IMAGE_RESULT_SUCCESS;
    #endif

    return false;
}
