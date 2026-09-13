#ifdef FF_HAVE_IMAGEMAGICK7

    #include "image.h"
    #include "common/library.h"
    #include "common/mallocHelper.h"

    #include <MagickCore/MagickCore.h>
    #include <stdlib.h>

static FF_LIBRARY_SYMBOL(ResizeImage)

    static void* logoResize(const void* image, size_t width, size_t height, void* exceptionInfo) {
    return ffResizeImage(image, width, height, UndefinedFilter, exceptionInfo);
}

// Decode the image source, resize it to the requested pixel size and export it as a blob
// in the given ImageMagick format ("RGBA" for kitty / chafa, "SIXEL" for sixel output).
// On success requestData->logoPixelWidth / logoPixelHeight hold the real image dimensions.
static FFLogoImageResult im7EncodeImage(FFLogoRequestData* requestData, const char* magick, uint32_t magickLength, void** outBlob, size_t* outLength) {
    // clang-format off
    #if _WIN32
    FF_LIBRARY_LOAD(imageMagick, FF_LOGO_IMAGE_RESULT_INIT_ERROR,
        "libMagickCore-7.Q16HDRI-10" FF_LIBRARY_EXTENSION, 0
    )
    #else
    FF_LIBRARY_LOAD(imageMagick, FF_LOGO_IMAGE_RESULT_INIT_ERROR,
        "libMagickCore-7.Q16HDRI" FF_LIBRARY_EXTENSION, 11,
        "libMagickCore-7.Q16" FF_LIBRARY_EXTENSION, 11,
        "libMagickCore-7" FF_LIBRARY_EXTENSION, 11
    )
    #endif
    // clang-format on
    FF_LIBRARY_LOAD_SYMBOL_ADDRESS(imageMagick, ffResizeImage, ResizeImage, FF_LOGO_IMAGE_RESULT_INIT_ERROR)

    FF_LIBRARY_LOAD_SYMBOL(imageMagick, MagickCoreGenesis, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, MagickCoreTerminus, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, AcquireExceptionInfo, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, DestroyExceptionInfo, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, AcquireImageInfo, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, DestroyImageInfo, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, ReadImage, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, DestroyImage, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, CopyMagickString, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, ImageToBlob, FF_LOGO_IMAGE_RESULT_INIT_ERROR)

    FFLogoImageResult result = FF_LOGO_IMAGE_RESULT_RUN_ERROR;
    ExceptionInfo* exceptionInfo = nullptr;
    Image* image = nullptr;
    ImageInfo* imageInfoOut = nullptr;
    FF_AUTO_FREE void* blob = nullptr;
    size_t length = 0;

    ffMagickCoreGenesis(nullptr, MagickFalse);

    exceptionInfo = ffAcquireExceptionInfo();
    if (exceptionInfo == nullptr) {
        goto cleanup;
    }

    {
        ImageInfo* imageInfoIn = ffAcquireImageInfo();
        if (imageInfoIn == nullptr) {
            goto cleanup;
        }

        //+1, because we need to copy the null byte too
        ffCopyMagickString(imageInfoIn->filename, instance.config.logo.source.chars, instance.config.logo.source.length + 1);

        image = ffReadImage(imageInfoIn, exceptionInfo);
        ffDestroyImageInfo(imageInfoIn);
        if (image == nullptr) {
            goto cleanup;
        }
    }

    if (requestData->logoPixelWidth == 0 && requestData->logoPixelHeight == 0) {
        requestData->logoPixelWidth = (uint32_t) image->columns;
        requestData->logoPixelHeight = (uint32_t) image->rows;
    } else if (requestData->logoPixelWidth == 0) {
        requestData->logoPixelWidth = (uint32_t) ((double) image->columns / (double) image->rows * requestData->logoPixelHeight);
    } else if (requestData->logoPixelHeight == 0) {
        requestData->logoPixelHeight = (uint32_t) ((double) image->rows / (double) image->columns * requestData->logoPixelWidth);
    }

    if (requestData->logoPixelWidth == 0 || requestData->logoPixelHeight == 0) {
        goto cleanup;
    }

    {
        Image* resized = logoResize(image, requestData->logoPixelWidth, requestData->logoPixelHeight, exceptionInfo);
        ffDestroyImage(image);
        image = resized;
        if (image == nullptr) {
            goto cleanup;
        }
    }

    imageInfoOut = ffAcquireImageInfo();
    if (imageInfoOut == nullptr) {
        goto cleanup;
    }

    ffCopyMagickString(imageInfoOut->magick, magick, magickLength);

    blob = ffImageToBlob(imageInfoOut, image, &length, exceptionInfo);
    if (blob == nullptr || length == 0) {
        goto cleanup;
    }

    *outBlob = blob;
    *outLength = length;
    blob = nullptr; // Ownership is transferred to the caller
    result = FF_LOGO_IMAGE_RESULT_SUCCESS;

cleanup:
    if (imageInfoOut) {
        ffDestroyImageInfo(imageInfoOut);
    }
    if (image) {
        ffDestroyImage(image);
    }
    if (exceptionInfo) {
        ffDestroyExceptionInfo(exceptionInfo);
    }
    ffMagickCoreTerminus();

    // leak imageMagick to prevent fastfetch from crashing #552
    imageMagick = nullptr;
    return result;
}

static void setError(FFLogoImageResult result, const char** error) {
    if (error) {
        *error = result == FF_LOGO_IMAGE_RESULT_INIT_ERROR
            ? "Image Magick library not found"
            : "Failed to load / convert the image source";
    }
}

bool ffImageCreateIM7(FFLogoRequestData* requestData, FFImageBuffer* out, const char** error) {
    void* blob = nullptr;
    size_t length = 0;
    FFLogoImageResult result = im7EncodeImage(requestData, "RGBA", 5, &blob, &length);
    if (result != FF_LOGO_IMAGE_RESULT_SUCCESS) {
        setError(result, error);
        return false;
    }

    out->data = blob;
    out->width = requestData->logoPixelWidth;
    out->height = requestData->logoPixelHeight;
    return true;
}

bool ffImageSixelEncodeIM7(FFLogoRequestData* requestData, FFstrbuf* out, const char** error) {
    FF_AUTO_FREE void* blob = nullptr;
    size_t length = 0;
    FFLogoImageResult result = im7EncodeImage(requestData, "SIXEL", 6, &blob, &length);
    if (result != FF_LOGO_IMAGE_RESULT_SUCCESS) {
        setError(result, error);
        return false;
    }

    ffStrbufSetNS(out, (uint32_t) length, (const char*) blob);
    return true;
}

#endif
