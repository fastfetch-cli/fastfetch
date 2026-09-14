#include "image.h"
#include "common/mallocHelper.h"
#include "common/apple/cf_helpers.h"

#include <Accelerate/Accelerate.h>
#include <CoreFoundation/CoreFoundation.h>
#include <ImageIO/ImageIO.h>

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

    size_t sourceWidth = CGImageGetWidth(image);
    size_t sourceHeight = CGImageGetHeight(image);
    if (sourceWidth == 0 || sourceHeight == 0) {
        if (error) {
            *error = "invalid image dimensions";
        }
        return false;
    }

    // Fill in the missing dimension, keeping the source aspect ratio (same as the IM path)
    uint32_t width = requestData->logoPixelWidth;
    uint32_t height = requestData->logoPixelHeight;
    if (width == 0 && height == 0) {
        width = (uint32_t) sourceWidth;
        height = (uint32_t) sourceHeight;
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

    const bool sameSize = width == sourceWidth && height == sourceHeight;

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
        // copy rows into the required tightly packed FFImageBuffer allocation.
        if (src.rowBytes == dstStride) {
            out->data = (uint8_t*) src.data;
            out->width = width;
            out->height = height;
            src.data = nullptr;
            return true;
        }

        FF_AUTO_FREE uint8_t* pixels = (uint8_t*) malloc(dstStride * (size_t) height);
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
        out->data = pixels;
        out->width = width;
        out->height = height;
        pixels = nullptr;
        return true;
    } else {
        FF_AUTO_FREE uint8_t* pixels = (uint8_t*) malloc(dstStride * (size_t) height);
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
            if (error) {
                *error = "failed to scale the image";
            }
            return false;
        }

        // Un-premultiply in place (pointwise, alpha == 0 is handled safely) so the result
        // matches the kitty f=32 / chafa CHAFA_PIXEL_RGBA8_UNASSOCIATED contract.
        vErr = vImageUnpremultiplyData_ARGB8888(&dst, &dst, kvImageNoFlags);
        if (vErr != kvImageNoError) {
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
            if (error) {
                *error = "failed to reorder image channels";
            }
            return false;
        }

        out->data = pixels;
        out->width = width;
        out->height = height;
        pixels = nullptr; // Ownership is transferred to `out`
        return true;
    }
}
