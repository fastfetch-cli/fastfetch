#include "image.h"
#include "common/mallocHelper.h"
#include "common/apple/cf_helpers.h"

#include <Accelerate/Accelerate.h>
#include <CoreFoundation/CoreFoundation.h>
#include <ImageIO/ImageIO.h>

// ---------------------------------------------------------------------------------------------
// Shared with the still path
// ---------------------------------------------------------------------------------------------

// Fills in the missing dimension, keeping the source aspect ratio (same as the IM path).
static bool imageIOResolveTargetSize(FFLogoRequestData* requestData, uint32_t sourceWidth, uint32_t sourceHeight, const char** error) {
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
        if (error) {
            *error = "invalid target dimensions";
        }
        return false;
    }

    requestData->logoPixelWidth = width;
    requestData->logoPixelHeight = height;
    return true;
}

// Decodes one CGImage into tightly packed straight (unassociated) RGBA8888 at width x height.
// Both paths need exactly this -- the still path once, the animation path once per frame.
static bool imageIOConvert(CGImageRef image, uint32_t width, uint32_t height, uint8_t** outPixels, const char** error) {
    const bool sameSize = width == CGImageGetWidth(image) && height == CGImageGetHeight(image);

    // Decode straight into RGBA for the same-size fast path. For resizing, decode
    // into premultiplied ARGB so interpolation does not bleed transparent RGB into
    // visible edge pixels.
    // vImageBuffer_InitWithCGImage handles format conversion, color management and
    // byte order in one call. NULL colorSpace means sRGB, matching kCGColorSpaceSRGB.
    const vImage_CGImageFormat format = {
        .bitsPerComponent = 8,
        .bitsPerPixel = 32,
        .colorSpace = nullptr,
        .bitmapInfo = (CGBitmapInfo) ((sameSize ? kCGImageAlphaLast : kCGImageAlphaPremultipliedFirst) | kCGImageByteOrder32Big),
        .version = 0,
        .decode = nullptr,
        .renderingIntent = kCGRenderingIntentDefault
    };

    vImage_Buffer src = {};
    vImage_Error vErr = vImageBuffer_InitWithCGImage(&src, (vImage_CGImageFormat*) &format, nullptr, image, kvImageNoFlags);
    if (vErr != kvImageNoError) {
        if (error) {
            *error = "failed to decode the image with vImage";
        }
        return false;
    }

    // Final output buffer: straight (unassociated) RGBA8888 at the target size.
    const size_t dstStride = (size_t) width * 4;
    if (sameSize) {
        // The requested format is already straight RGBA. If vImage did not add
        // row padding, transfer its allocation directly to the caller; otherwise
        // copy rows into the required tightly packed allocation.
        if (src.rowBytes == dstStride) {
            *outPixels = (uint8_t*) src.data;
            return true;
        }

        uint8_t* pixels = (uint8_t*) malloc(dstStride * (size_t) height);
        if (pixels == nullptr) {
            free(src.data);
            if (error) {
                *error = "out of memory";
            }
            return false;
        }
        for (uint32_t y = 0; y < height; ++y) {
            memcpy(pixels + (size_t) y * dstStride,
                (const uint8_t*) src.data + (size_t) y * src.rowBytes,
                dstStride);
        }
        free(src.data);
        *outPixels = pixels;
        return true;
    }

    uint8_t* pixels = (uint8_t*) malloc(dstStride * (size_t) height);
    if (pixels == nullptr) {
        free(src.data);
        if (error) {
            *error = "out of memory";
        }
        return false;
    }

    const vImage_Buffer dst = {
        .data = pixels,
        .width = width,
        .height = height,
        .rowBytes = dstStride
    };

    // Resample while still premultiplied: interpolating premultiplied data is the
    // correct way to scale (it avoids the dark fringes you get from averaging
    // straight-alpha RGB). NULL temp buffer lets vImage allocate internally.
    vErr = vImageScale_ARGB8888(&src, &dst, nullptr, kvImageHighQualityResampling);
    free(src.data);
    if (vErr != kvImageNoError) {
        free(pixels);
        if (error) {
            *error = "failed to scale the image";
        }
        return false;
    }

    // Un-premultiply in place (pointwise, alpha == 0 is handled safely) so the result
    // matches the kitty f=32 / chafa CHAFA_PIXEL_RGBA8_UNASSOCIATED contract.
    vErr = vImageUnpremultiplyData_ARGB8888(&dst, &dst, kvImageNoFlags);
    if (vErr != kvImageNoError) {
        free(pixels);
        if (error) {
            *error = "failed to un-premultiply the image";
        }
        return false;
    }

    // Reorder ARGB -> RGBA in place (permute supports in-place when data/rowBytes match).
    // The downstream consumers and the WIC backend all expect R,G,B,A byte order.
    const uint8_t permuteMap[4] = { 1, 2, 3, 0 }; // A,R,G,B -> R,G,B,A
    vErr = vImagePermuteChannels_ARGB8888(&dst, &dst, permuteMap, kvImageNoFlags);
    if (vErr != kvImageNoError) {
        free(pixels);
        if (error) {
            *error = "failed to reorder image channels";
        }
        return false;
    }

    *outPixels = pixels;
    return true;
}

bool ffImageCreateImageIO(FFLogoRequestData* requestData, FFImageBuffer* out, const char** error) {
    FF_CFTYPE_AUTO_RELEASE CFURLRef url = CFURLCreateFromFileSystemRepresentation(
        kCFAllocatorDefault,
        (const UInt8*) instance.config.logo.source.chars,
        (CFIndex) instance.config.logo.source.length,
        false);
    if (url == nullptr) {
        if (error) {
            *error = "failed to create the image URL";
        }
        return false;
    }

    FF_CFTYPE_AUTO_RELEASE CGImageSourceRef source = CGImageSourceCreateWithURL(url, nullptr);
    if (source == nullptr) {
        if (error) {
            *error = "unsupported or unreadable image format";
        }
        return false;
    }

    // Only the first frame, matching ImageMagick's ReadImage (neither handles GIF animation)
    FF_CFTYPE_AUTO_RELEASE CGImageRef image = CGImageSourceCreateImageAtIndex(source, 0, nullptr);
    if (image == nullptr) {
        if (error) {
            *error = "failed to get the first frame";
        }
        return false;
    }

    const size_t sourceWidth = CGImageGetWidth(image);
    const size_t sourceHeight = CGImageGetHeight(image);
    if (sourceWidth == 0 || sourceHeight == 0) {
        if (error) {
            *error = "invalid image dimensions";
        }
        return false;
    }

    if (!imageIOResolveTargetSize(requestData, (uint32_t) sourceWidth, (uint32_t) sourceHeight, error)) {
        return false;
    }

    if (!imageIOConvert(image, requestData->logoPixelWidth, requestData->logoPixelHeight, &out->data, error)) {
        return false;
    }

    out->width = requestData->logoPixelWidth;
    out->height = requestData->logoPixelHeight;
    return true;
}

// ---------------------------------------------------------------------------------------------
// Animation
//
// ImageIO hands out *composed* canvas-sized frames: CGImageSourceCreateImageAtIndex does the
// disposal work the Windows backend has to do itself, so the session here is thin -- it keeps the
// source open, resolves the canvas size and the per-frame gaps, and converts one frame per call.
//
// Two things it does not do, both measured rather than assumed (doc/kitty-animation.md 5.1.1):
//
//  * A `disposal = background` region is cleared to opaque black, where browsers, kitty and
//    ImageMagick leave it transparent. It can not be corrected: ImageIO exposes no disposal
//    metadata at any level, so there is nothing to branch on. Accepted and documented.
//  * Such a region is cleared even when the frame it belongs to is never displayed (delay 0),
//    where browsers and kitty keep the frame before it. Same cause, same outcome -- and the
//    common case is unaffected, because every ImageMagick optimiser emits disposal none/undefined.
//
// The frame count, the canvas size and every gap come from *metadata*: no frame is decoded before
// the first one is asked for, which is what the session contract requires (a negative
// --logo-animation-frame has to be resolved before the first frame is taken).
// ---------------------------------------------------------------------------------------------

typedef struct FFImageIOAnimation {
    CGImageSourceRef source;
    uint32_t nextIndex;
    uint32_t outputWidth;
    uint32_t outputHeight;
    int32_t minGap;
    int32_t* delaysCs; // one per frame, read once at open time; a full scan is needed for minGap anyway
} FFImageIOAnimation;

// Thin wrappers over common/apple/cf_helpers. They exist for two reasons the shared helpers
// deliberately do not cover: an absent dictionary has to be *tolerated* rather than queried
// (CFDictionaryGetValue on NULL is undefined), and every caller here treats "absent" and "of the
// wrong type" the same way, so the descriptive error string is dropped.
static CFDictionaryRef imageIOSubdict(CFDictionaryRef dict, CFStringRef key) {
    CFDictionaryRef result = nullptr;
    if (dict == nullptr || ffCfDictGetDict(dict, key, &result) != nullptr) {
        return nullptr;
    }

    return result;
}

// Both report false when the key is absent, which is how "the source declares nothing" is told
// apart from "the source declares zero".
static bool imageIODictGetDouble(CFDictionaryRef dict, CFStringRef key, double* out) {
    return dict != nullptr && ffCfDictGetDouble(dict, key, out) == nullptr;
}

static bool imageIODictGetInt(CFDictionaryRef dict, CFStringRef key, int64_t* out) {
    return dict != nullptr && ffCfDictGetInt64(dict, key, out) == nullptr;
}

// The frame's delay in centiseconds, which is the unit both GIF and APNG store.
//
// kCGImagePropertyGIFUnclampedDelayTime is the one to read, not kCGImagePropertyGIFDelayTime:
// ImageIO clamps the latter at 100 ms (and the APNG one at 50 ms), so a source whose frames are
// [0, 5, 0] centiseconds comes back as [10, 5, 10]. That would defeat kitty's rule, which floors at
// 100 ms only when *every* frame is zero and otherwise treats a zero as "gapless". Measured on
// macOS 27: mixed_delay.gif reads 0.10/0.05/0.10 clamped against 0.00/0.05/0.00 unclamped, and the
// unclamped values are the ones written into the file.
static int32_t imageIOFrameDelayCs(CFDictionaryRef frameProperties) {
    // A GIF keeps its timing in the GIF dictionary, an animated PNG in the PNG one, and the two
    // are never both present.
    CFDictionaryRef gif = imageIOSubdict(frameProperties, kCGImagePropertyGIFDictionary);
    CFDictionaryRef png = imageIOSubdict(frameProperties, kCGImagePropertyPNGDictionary);

    double seconds = 0;
    if (imageIODictGetDouble(gif, kCGImagePropertyGIFUnclampedDelayTime, &seconds) ||
        imageIODictGetDouble(gif, kCGImagePropertyGIFDelayTime, &seconds) ||
        imageIODictGetDouble(png, kCGImagePropertyAPNGUnclampedDelayTime, &seconds) ||
        imageIODictGetDouble(png, kCGImagePropertyAPNGDelayTime, &seconds)) {
        if (seconds > 0) {
            return (int32_t) lround(seconds * 100.0);
        }
    }

    return 0;
}

// How often the animation repeats after its first play; 0 means forever and -1 means the source
// declared nothing. Neither is what ImageIO reports -- its two dictionaries count different things
// (measured on macOS 27):
//
//   GIF  : number of *plays*. NETSCAPE 3 -> 4, NETSCAPE 0 (forever) -> 0, no NETSCAPE at all -> 1.
//          That is ImageMagick's `iterations` to the digit, and the same -1 conversion applies.
//          Note that "no NETSCAPE" arrives as 1, *not* as 0: reading 0 as the missing value, or
//          passing 1 straight through, would turn "loop forever" into "play twice".
//   APNG : number of *repeats*. acTL num_plays 3 -> 3, 0 (forever) -> 0.
//
// Which dictionary the value came from is therefore the format check; there is no need to ask
// ImageIO for a UTI and string-compare it.
static int32_t imageIOLoopCount(CGImageSourceRef source) {
    FF_CFTYPE_AUTO_RELEASE CFDictionaryRef properties = CGImageSourceCopyProperties(source, nullptr);
    if (properties == nullptr) {
        return -1;
    }

    int64_t value = 0;
    if (imageIODictGetInt(imageIOSubdict(properties, kCGImagePropertyGIFDictionary), kCGImagePropertyGIFLoopCount, &value)) {
        if (value == 0) {
            return 0;
        }
        if (value == 1) {
            return -1;
        }

        const int64_t loops = value - 1;
        return loops > INT32_MAX ? INT32_MAX : (int32_t) loops;
    }

    if (imageIODictGetInt(imageIOSubdict(properties, kCGImagePropertyPNGDictionary), kCGImagePropertyAPNGLoopCount, &value)) {
        if (value == 0) {
            return 0;
        }

        return value > INT32_MAX ? INT32_MAX : (int32_t) value;
    }

    return -1;
}

static bool imageIOGetFrame(FFImageAnimation* animation, uint32_t index, FFImageFrame* out, const char** error) {
    FFImageIOAnimation* session = (FFImageIOAnimation*) ffImageAnimationGetImpl(animation);

    // ImageIO can decode an arbitrary frame on demand, unlike the Windows backend, but the session
    // contract is a sequential iterator and the composition of frame i does depend on 0..i-1, so an
    // out-of-order caller is a bug worth reporting rather than something to paper over.
    if (index < session->nextIndex) {
        *error = "animation frames must be taken in order";
        return false;
    }

    // kitty's mapping, which the design settled on: the raw delay is in centiseconds, the 100 ms
    // floor exists only for sources whose frames are *all* zero, and whatever is left at <= 0 means
    // "gapless" rather than 100 ms.
    const int32_t rawDelay = session->delaysCs[index];
    const int32_t gap = (rawDelay > session->minGap ? rawDelay : session->minGap) * 10;
    out->delayMs = gap > 0 ? gap : -1;

    FF_CFTYPE_AUTO_RELEASE CGImageRef image = CGImageSourceCreateImageAtIndex(session->source, index, nullptr);
    if (image == nullptr) {
        *error = "failed to decode the animation frame";
        return false;
    }

    if (!imageIOConvert(image, session->outputWidth, session->outputHeight, &out->data, error)) {
        return false;
    }

    session->nextIndex = index + 1;
    return true;
}

static void imageIOFreeSession(FFImageIOAnimation* session) {
    if (session->source != nullptr) {
        CFRelease(session->source);
    }
    free(session->delaysCs);
    free(session);
}

static void imageIODestroyAnimation(FFImageAnimation* animation) {
    FFImageIOAnimation* session = (FFImageIOAnimation*) ffImageAnimationGetImpl(animation);
    if (session != nullptr) {
        imageIOFreeSession(session);
    }
}

bool ffImageAnimationOpenImageIO(FFLogoRequestData* requestData, FFImageAnimation** out, const char** error) {
    FF_CFTYPE_AUTO_RELEASE CFURLRef url = CFURLCreateFromFileSystemRepresentation(
        kCFAllocatorDefault,
        (const UInt8*) instance.config.logo.source.chars,
        (CFIndex) instance.config.logo.source.length,
        false);
    if (url == nullptr) {
        if (error) {
            *error = "failed to create the image URL";
        }
        return false;
    }

    // Frames are handed out one at a time and released by the caller, so there is nothing to gain
    // from letting ImageIO keep its own copy: without this a long animation is decoded into its
    // cache as it is played through, and the peak stops matching the still path's.
    const void* optionKeys[] = { kCGImageSourceShouldCache };
    const void* optionValues[] = { kCFBooleanFalse };
    FF_CFTYPE_AUTO_RELEASE CFDictionaryRef options = CFDictionaryCreate(kCFAllocatorDefault,
        optionKeys, optionValues, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    // Not auto-released: the session keeps it open until the last frame has been handed out.
    CGImageSourceRef source = CGImageSourceCreateWithURL(url, options);
    if (source == nullptr) {
        if (error) {
            *error = "unsupported or unreadable image format";
        }
        return false;
    }

    const size_t frameCount = CGImageSourceGetCount(source);
    if (frameCount == 0) {
        if (error) {
            *error = "the image source has no frames";
        }
        CFRelease(source);
        return false;
    }

    FFImageIOAnimation* session = (FFImageIOAnimation*) calloc(1, sizeof(*session));
    if (session == nullptr) {
        if (error) {
            *error = "out of memory";
        }
        CFRelease(source);
        return false;
    }
    session->source = source; // the session owns it from here on; every failure below frees it

    // The canvas size comes from frame 0's *properties*, not from decoding it: ImageIO hands out
    // canvas-sized frames, so this is the same value the still path reads off the decoded CGImage,
    // and reading it here keeps the contract's "no frame is decoded to open a session".
    FF_CFTYPE_AUTO_RELEASE CFDictionaryRef firstProperties = CGImageSourceCopyPropertiesAtIndex(source, 0, nullptr);
    int64_t sourceWidth = 0;
    int64_t sourceHeight = 0;
    if (!imageIODictGetInt(firstProperties, kCGImagePropertyPixelWidth, &sourceWidth) ||
        !imageIODictGetInt(firstProperties, kCGImagePropertyPixelHeight, &sourceHeight) ||
        sourceWidth <= 0 || sourceHeight <= 0) {
        if (error) {
            *error = "failed to read the animation canvas size";
        }
        imageIOFreeSession(session);
        return false;
    }

    if (!imageIOResolveTargetSize(requestData, (uint32_t) sourceWidth, (uint32_t) sourceHeight, error)) {
        imageIOFreeSession(session);
        return false;
    }
    session->outputWidth = requestData->logoPixelWidth;
    session->outputHeight = requestData->logoPixelHeight;

    session->delaysCs = (int32_t*) malloc(frameCount * sizeof(*session->delaysCs));
    if (session->delaysCs == nullptr) {
        if (error) {
            *error = "out of memory";
        }
        imageIOFreeSession(session);
        return false;
    }

    // kitty's rule: the 100 ms floor applies only when every frame's raw delay is <= 0. One frame
    // with a delay settles the floor, but every frame is still visited here so that its delay is
    // known by the time it is handed out.
    session->minGap = 10;
    for (size_t i = 0; i < frameCount; ++i) {
        FF_CFTYPE_AUTO_RELEASE CFDictionaryRef properties = CGImageSourceCopyPropertiesAtIndex(source, i, nullptr);
        session->delaysCs[i] = imageIOFrameDelayCs(properties);
        if (session->delaysCs[i] > 0) {
            session->minGap = 0;
        }
    }

    FFImageAnimation* animation = ffImageAnimationCreate((uint32_t) frameCount, imageIOLoopCount(source),
        session, imageIOGetFrame, imageIODestroyAnimation);
    if (animation == nullptr) {
        if (error) {
            *error = "out of memory";
        }
        imageIOFreeSession(session);
        return false;
    }

    *out = animation;
    return true;
}
