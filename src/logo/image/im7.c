#ifdef FF_HAVE_IMAGEMAGICK7

    #include "image.h"
    #include "common/library.h"
    #include "common/mallocHelper.h"
    #include "common/strutil.h"

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
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, DestroyImageList, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
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
        // ReadImage may return a list of images (e.g. for multi-frame formats like GIF).
        // We only need the first frame, so destroy the whole list to avoid leaking the rest.
        ffDestroyImageList(image);
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

    // The raw pixel coders write image->depth bits per sample, not 8: a 1-bit grayscale source comes
    // back as columns*4/8 bytes per row and a 16-bit one as columns*8, while the RGBA caller hands
    // the blob on as RGBA8 and derives its length from width*height*4. Pin the depth for the raw
    // formats only -- the SIXEL coder quantises on its own, so it keeps the source depth and its
    // output stays byte identical.
    if (ffStrEquals(magick, "RGBA")) {
        image->depth = 8;
    }

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
        ffDestroyImageList(image);
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

    // FFImageBuffer carries no length, so every consumer derives it from width*height*4. Refuse any
    // other size rather than let them read past the blob -- the raw coder's depth scaling used to
    // produce one (see the depth pin in im7EncodeImage).
    if (length != (size_t) requestData->logoPixelWidth * requestData->logoPixelHeight * 4) {
        if (error) {
            *error = "Image Magick did not return an RGBA8 buffer";
        }
        free(blob);
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

// Encodes pixels the caller already has. The selected-frame path composes its frame itself, so it
// can not go through ffImageSixelEncodeIM7 -- that one re-reads the source and would encode
// whatever the still path would have shown instead of the frame that was asked for.
bool ffImageSixelEncodeBufferIM7(const FFImageBuffer* buffer, FFstrbuf* out, const char** error) {
    // clang-format off
    #if _WIN32
    FF_LIBRARY_LOAD(imageMagick, false,
        "libMagickCore-7.Q16HDRI-10" FF_LIBRARY_EXTENSION, 0
    )
    #else
    FF_LIBRARY_LOAD(imageMagick, false,
        "libMagickCore-7.Q16HDRI" FF_LIBRARY_EXTENSION, 11,
        "libMagickCore-7.Q16" FF_LIBRARY_EXTENSION, 11,
        "libMagickCore-7" FF_LIBRARY_EXTENSION, 11
    )
    #endif
    // clang-format on

    FF_LIBRARY_LOAD_SYMBOL(imageMagick, MagickCoreGenesis, false)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, MagickCoreTerminus, false)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, AcquireExceptionInfo, false)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, DestroyExceptionInfo, false)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, AcquireImageInfo, false)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, DestroyImageInfo, false)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, DestroyImage, false)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, CopyMagickString, false)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, ConstituteImage, false)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, ImageToBlob, false)

    ffMagickCoreGenesis(nullptr, MagickFalse);

    ExceptionInfo* exceptionInfo = ffAcquireExceptionInfo();
    if (exceptionInfo == nullptr) {
        if (error) *error = "failed to acquire an ImageMagick exception";
        ffMagickCoreTerminus();
        return false;
    }

    Image* image = ffConstituteImage(buffer->width, buffer->height, "RGBA", CharPixel,
        buffer->data, exceptionInfo);
    if (image == nullptr) {
        if (error) *error = "failed to wrap the animation frame";
        ffDestroyExceptionInfo(exceptionInfo);
        ffMagickCoreTerminus();
        return false;
    }

    ImageInfo* imageInfoOut = ffAcquireImageInfo();
    if (imageInfoOut == nullptr) {
        if (error) *error = "failed to acquire an ImageMagick image info";
        ffDestroyImage(image);
        ffDestroyExceptionInfo(exceptionInfo);
        ffMagickCoreTerminus();
        return false;
    }
    ffCopyMagickString(imageInfoOut->magick, "SIXEL", 6);

    size_t length = 0;
    FF_AUTO_FREE void* blob = ffImageToBlob(imageInfoOut, image, &length, exceptionInfo);

    ffDestroyImageInfo(imageInfoOut);
    ffDestroyImage(image);
    ffDestroyExceptionInfo(exceptionInfo);
    ffMagickCoreTerminus();

    if (blob == nullptr || length == 0) {
        if (error) *error = "failed to encode the animation frame as sixel";
        return false;
    }

    ffStrbufSetNS(out, (uint32_t) length, (const char*) blob);

    // leak imageMagick to prevent fastfetch from crashing #552, as the static path does
    imageMagick = nullptr;
    return true;
}

// ---------------------------------------------------------------------------------------------
// Animation
//
// ImageMagick reads an animated GIF as a list of *sub-frames*: every image holds only the frame's
// own rectangle in image->columns x image->rows, and where it belongs on the logical screen in
// image->page. CoalesceImages turns that into canvas-sized frames, which is the shape the session
// contract asks for.
//
// Its idea of composition is not a browser's, though, so three things are steered before it runs.
// All three were measured against an independent reference compositor rather than guessed, and so
// was the fourth difference, which cannot be steered and is normalised after the fact instead. The
// measurements and the memory cost that comes with reusing the composition are in
// doc/kitty-animation.md §11-E.
// ---------------------------------------------------------------------------------------------

// The entry points the session calls have to outlive ffImageAnimationOpenIM7, so unlike the
// locals im7EncodeImage loads they belong to the session.
typedef struct FFIm7Animation {
    FF_LIBRARY_SYMBOL(ResizeImage)
    FF_LIBRARY_SYMBOL(ImageToBlob)
    FF_LIBRARY_SYMBOL(DestroyImageList)
    FF_LIBRARY_SYMBOL(DestroyImageInfo)
    FF_LIBRARY_SYMBOL(DestroyExceptionInfo)
    FF_LIBRARY_SYMBOL(MagickCoreTerminus)
    FF_LIBRARY_SYMBOL(SetImageAlphaChannel)

    Image* images;         // coalesced, so every frame is canvas sized
    Image* cursor;         // the next frame to hand out
    ImageInfo* blobInfo;   // magick "RGBA", reused for every frame
    ExceptionInfo* exceptionInfo;
    uint32_t nextIndex;
    uint32_t outputWidth;
    uint32_t outputHeight;
    int32_t minGap;
} FFIm7Animation;

// The aspect-ratio rule the static path applies inline, needed here for the canvas rather than
// for a decoded frame.
static bool im7ResolveTargetSize(FFLogoRequestData* requestData, uint32_t sourceWidth, uint32_t sourceHeight, const char** error) {
    uint32_t width = requestData->logoPixelWidth;
    uint32_t height = requestData->logoPixelHeight;
    if (width == 0 && height == 0) {
        width = sourceWidth;
        height = sourceHeight;
    } else if (width == 0) {
        width = (uint32_t) ((double) sourceWidth / (double) sourceHeight * height);
    } else if (height == 0) {
        height = (uint32_t) ((double) sourceHeight / (double) sourceWidth * width);
    }

    if (width == 0 || height == 0) {
        if (error) *error = "invalid target dimensions";
        return false;
    }

    requestData->logoPixelWidth = width;
    requestData->logoPixelHeight = height;
    return true;
}

// Three measured differences between ImageMagick's composition and a browser's, all fixed on the
// input side because the composition itself is the part worth reusing:
//
//  1. ImageMagick fills the canvas with the logical screen's background colour, opaque. Browsers
//     start transparent and never paint it. -> make every frame's background colour transparent.
//  2. A frame with no transparent index carries no alpha channel, and the canvas CoalesceImages
//     clones from it inherits that, so the transparent fill above would be discarded. -> give
//     those frames an opaque alpha channel, which is what having no transparent index means.
//  3. A frame that is never displayed (delay 0) must not erase the one before it, which is what
//     browsers and kitty do. ImageMagick honours the disposal anyway. -> drop the disposal.
static void im7SteerComposition(FFIm7Animation* session, Image* images) {
    for (Image* image = images; image != nullptr; image = image->next) {
        image->background_color.red = 0;
        image->background_color.green = 0;
        image->background_color.blue = 0;
        image->background_color.alpha = 0;
        image->background_color.alpha_trait = BlendPixelTrait;

        if (image->alpha_trait == UndefinedPixelTrait) {
            session->ffSetImageAlphaChannel(image, OpaqueAlphaChannel, session->exceptionInfo);
        }

        if (image->delay == 0 && image->dispose == BackgroundDispose) {
            image->dispose = NoneDispose;
        }
    }
}

// Where a BackgroundDispose region is cleared, ImageMagick uses the background colour it read from
// the source and only forces the alpha to 0. The RGB under a zero alpha is unused by kitty and by
// chafa, but the sixel encoder does look at it, and the Windows backend clears to zero -- so the
// pixels are normalised here rather than left to differ between the two.
static void im7ClearTransparentPixels(uint8_t* pixels, size_t length) {
    for (size_t i = 0; i + 3 < length; i += 4) {
        if (pixels[i + 3] == 0) {
            pixels[i] = 0;
            pixels[i + 1] = 0;
            pixels[i + 2] = 0;
        }
    }
}

// The source's delay in centiseconds. GIFs are read with 100 ticks per second, so the conversion
// is a no-op there, but nothing guarantees that for every format the coder may hand back.
static uint32_t im7FrameDelayCs(const Image* image) {
    if (image->ticks_per_second <= 0) {
        return (uint32_t) image->delay;
    }

    return (uint32_t) ((uint64_t) image->delay * 100u / (uint64_t) image->ticks_per_second);
}

// ImageMagick counts how often the animation is *played*; the source counts how often it repeats
// after the first play. 0 means forever and 1 means the source declared no loop count at all,
// which is the -1 the session contract uses. Checked against the Windows backend on the same
// files: no NETSCAPE block -> 1 here and -1 there, NETSCAPE 0 -> 0 and 0, NETSCAPE 3 -> 4 and 3.
static int32_t im7LoopCount(const Image* image) {
    if (image->iterations == 0) {
        return 0;
    }
    if (image->iterations == 1) {
        return -1;
    }

    const size_t loops = image->iterations - 1;
    return loops > (size_t) INT32_MAX ? INT32_MAX : (int32_t) loops;
}

static bool im7GetFrame(FFImageAnimation* animation, uint32_t index, FFImageFrame* out, const char** error) {
    FFIm7Animation* session = (FFIm7Animation*) ffImageAnimationGetImpl(animation);

    // CoalesceImages has already composed every frame, so a frame is only ever walked past once,
    // in order -- and taking one does not depend on having taken the previous one.
    if (index < session->nextIndex) {
        *error = "animation frames must be taken in order";
        return false;
    }

    while (session->nextIndex < index && session->cursor != nullptr) {
        session->cursor = session->cursor->next;
        ++session->nextIndex;
    }

    if (session->cursor == nullptr) {
        *error = "the animation has no such frame";
        return false;
    }

    const uint32_t rawDelay = im7FrameDelayCs(session->cursor);

    // kitty's mapping, which the design settled on: the raw delay is in centiseconds, the 100 ms
    // floor exists only for sources whose frames are *all* zero, and whatever is left at <= 0 means
    // "gapless" rather than 100 ms.
    const int32_t gap = (int32_t) (rawDelay > (uint32_t) session->minGap ? rawDelay : (uint32_t) session->minGap) * 10;
    out->delayMs = gap > 0 ? gap : -1;

    // Each coalesced frame is canvas sized already; the scaling to the output size is per frame,
    // exactly as it is for a still image.
    Image* resized = session->ffResizeImage(session->cursor, session->outputWidth, session->outputHeight,
        UndefinedFilter, session->exceptionInfo);
    if (resized == nullptr) {
        *error = "failed to resize the animation frame";
        return false;
    }

    size_t length = 0;
    void* blob = session->ffImageToBlob(session->blobInfo, resized, &length, session->exceptionInfo);
    session->ffDestroyImageList(resized);
    if (blob == nullptr || length != (size_t) session->outputWidth * session->outputHeight * 4) {
        if (blob != nullptr) free(blob);
        *error = "failed to export the animation frame";
        return false;
    }

    im7ClearTransparentPixels((uint8_t*) blob, length);

    out->data = (uint8_t*) blob;
    session->cursor = session->cursor->next;
    ++session->nextIndex;
    return true;
}

static void im7FreeSession(FFIm7Animation* session) {
    if (session->images) session->ffDestroyImageList(session->images);
    if (session->blobInfo) session->ffDestroyImageInfo(session->blobInfo);
    if (session->exceptionInfo) session->ffDestroyExceptionInfo(session->exceptionInfo);
    // Only ever reached once MagickCoreGenesis has run, and the symbol load in front of that has
    // already been checked, so this pointer is non-null here.
    session->ffMagickCoreTerminus();

    // The library handle is leaked on purpose, exactly as the static path does it: unloading
    // libMagickCore while the process still exits through it crashes (#552).
    free(session);
}

static void im7DestroyAnimation(FFImageAnimation* animation) {
    FFIm7Animation* session = (FFIm7Animation*) ffImageAnimationGetImpl(animation);
    if (session != nullptr) {
        im7FreeSession(session);
    }
}

bool ffImageAnimationOpenIM7(FFLogoRequestData* requestData, FFImageAnimation** out, const char** error) {
    // clang-format off
    #if _WIN32
    FF_LIBRARY_LOAD(imageMagick, false,
        "libMagickCore-7.Q16HDRI-10" FF_LIBRARY_EXTENSION, 0
    )
    #else
    FF_LIBRARY_LOAD(imageMagick, false,
        "libMagickCore-7.Q16HDRI" FF_LIBRARY_EXTENSION, 11,
        "libMagickCore-7.Q16" FF_LIBRARY_EXTENSION, 11,
        "libMagickCore-7" FF_LIBRARY_EXTENSION, 11
    )
    #endif
    // clang-format on

    FF_LIBRARY_LOAD_SYMBOL(imageMagick, MagickCoreGenesis, false)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, AcquireExceptionInfo, false)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, AcquireImageInfo, false)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, ReadImage, false)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, CoalesceImages, false)
    FF_LIBRARY_LOAD_SYMBOL(imageMagick, CopyMagickString, false)

    FFIm7Animation* session = (FFIm7Animation*) calloc(1, sizeof(*session));
    if (session == nullptr) {
        if (error) *error = "out of memory";
        return false;
    }

    // The session owns these, so every failure below has to free the session by hand rather than
    // go through im7DestroyAnimation, which expects the whole thing to be in place.
    session->ffResizeImage = (typeof(&ResizeImage)) dlsym(imageMagick, "ResizeImage");
    session->ffImageToBlob = (typeof(&ImageToBlob)) dlsym(imageMagick, "ImageToBlob");
    session->ffDestroyImageList = (typeof(&DestroyImageList)) dlsym(imageMagick, "DestroyImageList");
    session->ffDestroyImageInfo = (typeof(&DestroyImageInfo)) dlsym(imageMagick, "DestroyImageInfo");
    session->ffDestroyExceptionInfo = (typeof(&DestroyExceptionInfo)) dlsym(imageMagick, "DestroyExceptionInfo");
    session->ffMagickCoreTerminus = (typeof(&MagickCoreTerminus)) dlsym(imageMagick, "MagickCoreTerminus");
    session->ffSetImageAlphaChannel = (typeof(&SetImageAlphaChannel)) dlsym(imageMagick, "SetImageAlphaChannel");
    if (session->ffResizeImage == nullptr || session->ffImageToBlob == nullptr ||
        session->ffDestroyImageList == nullptr || session->ffDestroyImageInfo == nullptr ||
        session->ffDestroyExceptionInfo == nullptr || session->ffMagickCoreTerminus == nullptr ||
        session->ffSetImageAlphaChannel == nullptr) {
        if (error) *error = "the ImageMagick library is incomplete";
        free(session);
        return false;
    }

    // The core stays initialised for the whole session: the Image objects below belong to it, and
    // terminating it while they are alive would leave them dangling. The static path can afford to
    // initialise and terminate within one call because it never keeps an Image across calls.
    ffMagickCoreGenesis(nullptr, MagickFalse);

    session->exceptionInfo = ffAcquireExceptionInfo();
    if (session->exceptionInfo == nullptr) {
        if (error) *error = "failed to acquire an ImageMagick exception";
        im7FreeSession(session);
        return false;
    }

    ImageInfo* imageInfoIn = ffAcquireImageInfo();
    if (imageInfoIn == nullptr) {
        if (error) *error = "failed to acquire an ImageMagick image info";
        im7FreeSession(session);
        return false;
    }

    //+1, because we need to copy the null byte too
    ffCopyMagickString(imageInfoIn->filename, instance.config.logo.source.chars, instance.config.logo.source.length + 1);

    Image* raw = ffReadImage(imageInfoIn, session->exceptionInfo);
    session->ffDestroyImageInfo(imageInfoIn);
    if (raw == nullptr) {
        if (error) *error = "failed to load the image source";
        im7FreeSession(session);
        return false;
    }

    im7SteerComposition(session, raw);

    session->images = ffCoalesceImages(raw, session->exceptionInfo);
    // CoalesceImages built a new list; the one it was given is still ours to destroy.
    session->ffDestroyImageList(raw);
    if (session->images == nullptr) {
        if (error) *error = "failed to compose the animation frames";
        im7FreeSession(session);
        return false;
    }

    // The coalesced frames are canvas sized, which is what the target size is derived from.
    uint32_t frameCount = 0;
    for (Image* image = session->images; image != nullptr; image = image->next) {
        ++frameCount;
    }

    if (frameCount == 0 ||
        !im7ResolveTargetSize(requestData, (uint32_t) session->images->columns, (uint32_t) session->images->rows, error)) {
        if (frameCount == 0 && error) *error = "the image source has no frames";
        im7FreeSession(session);
        return false;
    }

    session->blobInfo = ffAcquireImageInfo();
    if (session->blobInfo == nullptr) {
        if (error) *error = "failed to acquire an ImageMagick image info";
        im7FreeSession(session);
        return false;
    }
    ffCopyMagickString(session->blobInfo->magick, "RGBA", 5);

    // kitty's rule: the 100 ms floor applies only when every frame's raw delay is <= 0. One frame
    // with a delay settles it, so the normal case stops after the first read.
    session->minGap = 10;
    for (Image* image = session->images; image != nullptr; image = image->next) {
        if (im7FrameDelayCs(image) > 0) {
            session->minGap = 0;
            break;
        }
    }

    session->cursor = session->images;
    session->nextIndex = 0;
    session->outputWidth = requestData->logoPixelWidth;
    session->outputHeight = requestData->logoPixelHeight;

    FFImageAnimation* animation = ffImageAnimationCreate(frameCount, im7LoopCount(session->images),
        session, im7GetFrame, im7DestroyAnimation);
    if (animation == nullptr) {
        if (error) *error = "out of memory";
        im7FreeSession(session);
        return false;
    }

    // leak imageMagick to prevent fastfetch from crashing #552, as the static path does
    imageMagick = nullptr;
    *out = animation;
    return true;
}

#endif
