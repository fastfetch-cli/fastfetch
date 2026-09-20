
#include "image.h"
#include "common/android/api.h"
#include "common/io.h"
#include "common/mallocHelper.h"

#include <android/bitmap.h>
#include <android/imagedecoder.h>

#include <fcntl.h>
#include <stdint.h>
#include <string.h>

static inline bool androidImageDecoderError(const char** error, const char* message) {
    if (error) {
        *error = message;
    }
    return false;
}

// AImageDecoder decodes to premultiplied alpha, and it can not be asked for straight alpha once a
// target size is in effect: AImageDecoder_setUnpremultipliedRequired fails with
// ANDROID_IMAGE_DECODER_INVALID_CONVERSION, documented as "Unpremultiplied is not possible due to
// an existing scale set by AImageDecoder_setTargetSize". Premultiplied is the right domain to
// scale in anyway -- interpolating straight alpha averages the colour of fully transparent pixels
// into the edges next to them, which shows up as dark fringes -- so the decoder is left alone and
// this runs afterwards, exactly as the ImageIO backend does.
static void unPremultiplyRGBA(uint8_t* data, size_t pixelCount) {
    for (size_t i = 0; i < pixelCount; ++i) {
        uint8_t* const p = data + i * 4;
        const uint8_t a = p[3];
        if (a == 0) {
            p[0] = p[1] = p[2] = 0;
        } else if (a < 255) {
            p[0] = (uint8_t) ((p[0] * 255 + a / 2) / a);
            p[1] = (uint8_t) ((p[1] * 255 + a / 2) / a);
            p[2] = (uint8_t) ((p[2] * 255 + a / 2) / a);
        }
    }
}

// Copy one decoded frame into a tightly packed, straight alpha buffer of its own.
//
// A copy rather than a hand-over: the animation path has to keep the decoder's own buffer intact,
// because AImageDecoder blends each frame into the one that is already there. The still path only
// needs the packed result, and going through here keeps the two identical.
//
// `premultiplied` is what the decoder reports about the pixels it produced, so a source that was
// decoded straight is copied and nothing else.
static uint8_t* androidPackFrame(const uint8_t* src, size_t stride, uint32_t width, uint32_t height, bool premultiplied) {
    const size_t packedStride = (size_t) width * 4;
    uint8_t* pixels = (uint8_t*) malloc(packedStride * height);
    if (pixels == nullptr) {
        return nullptr;
    }

    for (uint32_t y = 0; y < height; ++y) {
        memcpy(pixels + (size_t) y * packedStride, src + (size_t) y * stride, packedStride);
    }

    if (premultiplied) {
        unPremultiplyRGBA(pixels, (size_t) width * height);
    }
    return pixels;
}

// Everything the still and the animation path share: read the source size, resolve the requested
// pixel size, ask for RGBA8888 and let the decoder do the scaling.
FF_ANDROID_REQUIRES_API(30) static bool androidResolveDecoder(AImageDecoder* decoder, FFLogoRequestData* requestData, size_t* outStride, bool* outPremultiplied, const char** error) {
    const AImageDecoderHeaderInfo* header = AImageDecoder_getHeaderInfo(decoder);
    // Both are int32_t, and anything <= 0 is not a usable source
    const int32_t sourceWidth = AImageDecoderHeaderInfo_getWidth(header);
    const int32_t sourceHeight = AImageDecoderHeaderInfo_getHeight(header);
    if (sourceWidth <= 0 || sourceHeight <= 0) {
        return androidImageDecoderError(error, "invalid image dimensions");
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
        return androidImageDecoderError(error, "invalid target dimensions");
    }

    if (AImageDecoder_setAndroidBitmapFormat(decoder, ANDROID_BITMAP_FORMAT_RGBA_8888) != ANDROID_IMAGE_DECODER_SUCCESS) {
        return androidImageDecoderError(error, "failed to select the RGBA8888 output format");
    }

    // A source that is already the requested size needs no scaling at all, and leaving the target
    // size unset buys one more thing: AImageDecoder refuses straight alpha only *because* of a
    // scale, so unscaled output can be decoded straight and no conversion follows. That is also
    // the more accurate of the two, having no integer rounding to recover from.
    if (width == (uint32_t) sourceWidth && height == (uint32_t) sourceHeight) {
        *outPremultiplied = AImageDecoder_setUnpremultipliedRequired(decoder, true) != ANDROID_IMAGE_DECODER_SUCCESS;
    } else {
        // Scaling is part of the decode: the decoder samples the source down (or up) as it goes, so
        // there is neither a full-size intermediate nor a separate resize step.
        if (AImageDecoder_setTargetSize(decoder, (int32_t) width, (int32_t) height) != ANDROID_IMAGE_DECODER_SUCCESS) {
            return androidImageDecoderError(error, "failed to scale the image");
        }
        *outPremultiplied = true;
    }

    const size_t stride = AImageDecoder_getMinimumStride(decoder);
    if (stride < (size_t) width * 4 || stride > SIZE_MAX / height) {
        return androidImageDecoderError(error, "invalid target dimensions");
    }

    requestData->logoPixelWidth = width;
    requestData->logoPixelHeight = height;
    *outStride = stride;
    return true;
}

bool ffImageCreateAID(FFLogoRequestData* requestData, FFImageBuffer* out, const char** error) {
    // AImageDecoder is API 30, and fastfetch still runs on devices below that, so this is a real
    // run-time check and not a compile-time constant. See common/android/api.h for why.
    if (FF_ANDROID_API_AT_LEAST(30)) {
        FF_AUTO_CLOSE_FD int fd = open(instance.config.logo.source.chars, O_RDONLY | O_CLOEXEC);
        if (fd < 0) {
            return androidImageDecoderError(error, "failed to open the image source");
        }

        AImageDecoder* decoder = nullptr;
        if (AImageDecoder_createFromFd(fd, &decoder) != ANDROID_IMAGE_DECODER_SUCCESS) {
            return androidImageDecoderError(error, "unsupported or unreadable image format");
        }

        size_t stride = 0;
        bool premultiplied = true;
        if (!androidResolveDecoder(decoder, requestData, &stride, &premultiplied, error)) {
            AImageDecoder_delete(decoder);
            return false;
        }

        const uint32_t width = requestData->logoPixelWidth;
        const uint32_t height = requestData->logoPixelHeight;
        const size_t packedStride = (size_t) width * 4;
        const size_t bufferSize = stride * height;

        FF_AUTO_FREE uint8_t* decoded = (uint8_t*) malloc(bufferSize);
        if (decoded == nullptr) {
            AImageDecoder_delete(decoder);
            return androidImageDecoderError(error, "out of memory");
        }

        const int result = AImageDecoder_decodeImage(decoder, decoded, stride, bufferSize);
        // The decoder reads from the fd while decoding, so it outlives the decode but not the fd
        AImageDecoder_delete(decoder);
        if (result != ANDROID_IMAGE_DECODER_SUCCESS) {
            return androidImageDecoderError(error, "failed to decode the image");
        }

        // Without row padding the decoder's buffer is already laid out the way the contract wants,
        // so it is finished in place and handed over rather than copied.
        if (stride == packedStride) {
            if (premultiplied) {
                unPremultiplyRGBA(decoded, (size_t) width * height);
            }
            out->data = decoded;
            decoded = nullptr; // Ownership is transferred to `out`
        } else {
            out->data = androidPackFrame(decoded, stride, width, height, premultiplied);
            if (out->data == nullptr) {
                return androidImageDecoderError(error, "out of memory");
            }
        }

        out->width = width;
        out->height = height;
        return true;
    } else {
        return androidImageDecoderError(error, "AImageDecoder requires Android 11 (API 30) or newer");
    }
}

// ---------------------------------------------------------------------------------------------
// Animation
//
// AImageDecoder composes the frames itself. Given the same buffer for every call, it decodes the
// part of the canvas a frame actually covers and blends it over what is already there, and it
// restores the buffer for a DISPOSE_OP_PREVIOUS frame on its own. So unlike the Windows backend,
// there is no canvas to maintain here: the session keeps the decoder's buffer and hands out a copy
// of it per frame.
//
// Two things follow from the decoder only ever walking forwards:
//
//  * The frame count and every gap have to be collected up front by advancing through the whole
//    animation and rewinding, which is what the session contract asks for anyway -- a negative
//    --logo-animation-frame has to be resolved before the first frame is taken.
//  * Reaching a frame that is not the next one means rewinding and walking forward again, which is
//    slower than refusing it but keeps the iterator usable after the last frame.
//
// The loop count is the one thing AImageDecoder does not expose: it has no API for it at any
// level, so the animation is reported as looping forever, which is what an animated GIF with no
// NETSCAPE block does in every viewer anyway.
// ---------------------------------------------------------------------------------------------

typedef struct FFAndroidAnimation {
    AImageDecoder* decoder;
    uint8_t* canvas; // the decoder's buffer; every frame is blended into what is already in it
    size_t stride;
    size_t size;
    uint32_t width;
    uint32_t height;
    uint32_t nextIndex; // the frame the decoder is positioned on
    bool premultiplied; // whether the decoder's frames need un-premultiplying
    int32_t minGap;
    int32_t* delaysCs; // one per frame, in centiseconds
} FFAndroidAnimation;

FF_ANDROID_REQUIRES_API(31) static bool androidAnimationGetFrame(FFImageAnimation* animation, uint32_t index, FFImageFrame* out, const char** error) {
    FFAndroidAnimation* session = (FFAndroidAnimation*) ffImageAnimationGetImpl(animation);

    // Frames come out of the decoder in order, so anything before the current one means starting
    // over. AImageDecoder_rewind needs an animated source; it is reached here only once a frame
    // has already been taken, which means the source is one.
    if (index < session->nextIndex) {
        if (AImageDecoder_rewind(session->decoder) != ANDROID_IMAGE_DECODER_SUCCESS) {
            return androidImageDecoderError(error, "failed to rewind the animation");
        }
        session->nextIndex = 0;
    }

    // Skipped frames still have to be decoded: each frame is blended into the buffer that the
    // following one builds on, so the buffer would be wrong without them.
    while (session->nextIndex < index) {
        if (AImageDecoder_decodeImage(session->decoder, session->canvas, session->stride, session->size) != ANDROID_IMAGE_DECODER_SUCCESS) {
            return androidImageDecoderError(error, "failed to decode an animation frame");
        }
        if (AImageDecoder_advanceFrame(session->decoder) != ANDROID_IMAGE_DECODER_SUCCESS) {
            return androidImageDecoderError(error, "failed to advance to the next animation frame");
        }
        ++session->nextIndex;
    }

    if (AImageDecoder_decodeImage(session->decoder, session->canvas, session->stride, session->size) != ANDROID_IMAGE_DECODER_SUCCESS) {
        return androidImageDecoderError(error, "failed to decode the animation frame");
    }

    uint8_t* frame = androidPackFrame(session->canvas, session->stride, session->width, session->height, session->premultiplied);
    if (frame == nullptr) {
        return androidImageDecoderError(error, "out of memory");
    }

    // Position on the following frame. Past the last one this reports FINISHED, which is expected
    // and harmless: the frame just handed out is complete, and anything else the caller asks for
    // restarts above.
    AImageDecoder_advanceFrame(session->decoder);
    ++session->nextIndex;

    // The same mapping the ImageIO backend settled on: the raw gap is in centiseconds, a floor of
    // 100 ms applies only when every frame is zero, and what is left at <= 0 means gapless.
    const int32_t gap = (session->delaysCs[index] > session->minGap ? session->delaysCs[index] : session->minGap) * 10;
    out->data = frame;
    out->delayMs = gap > 0 ? gap : -1;
    return true;
}

FF_ANDROID_REQUIRES_API(31) static void androidAnimationDestroy(FFImageAnimation* animation) {
    FFAndroidAnimation* session = (FFAndroidAnimation*) ffImageAnimationGetImpl(animation);
    if (session == nullptr) {
        return;
    }

    AImageDecoder_delete(session->decoder);
    free(session->canvas);
    free(session->delaysCs);
    free(session);
}

bool ffImageAnimationOpenAID(FFLogoRequestData* requestData, FFImageAnimation** out, const char** error) {
    // Decoding past the first frame needs API 31: AImageDecoder_advanceFrame and
    // AImageDecoderFrameInfo are both introduced there, and AImageDecoder_decodeImage only
    // documents decoding "all of the frames" from that level on.
    if (FF_ANDROID_API_AT_LEAST(31)) {
        FF_AUTO_CLOSE_FD int fd = open(instance.config.logo.source.chars, O_RDONLY | O_CLOEXEC);
        if (fd < 0) {
            return androidImageDecoderError(error, "failed to open the image source");
        }

        AImageDecoder* decoder = nullptr;
        if (AImageDecoder_createFromFd(fd, &decoder) != ANDROID_IMAGE_DECODER_SUCCESS) {
            return androidImageDecoderError(error, "unsupported or unreadable image format");
        }

        size_t stride = 0;
        bool premultiplied = true;
        if (!androidResolveDecoder(decoder, requestData, &stride, &premultiplied, error)) {
            AImageDecoder_delete(decoder);
            return false;
        }

        AImageDecoderFrameInfo* frameInfo = AImageDecoderFrameInfo_create();
        if (frameInfo == nullptr) {
            AImageDecoder_delete(decoder);
            return androidImageDecoderError(error, "out of memory");
        }

        // Walk the animation once, recording every gap on the way, so that both the frame count
        // and each frame's timing are known before the first frame is decoded.
        uint32_t capacity = 16;
        uint32_t frameCount = 0;
        int32_t* delays = (int32_t*) malloc(capacity * sizeof(*delays));
        if (delays == nullptr) {
            AImageDecoderFrameInfo_delete(frameInfo);
            AImageDecoder_delete(decoder);
            return androidImageDecoderError(error, "out of memory");
        }

        int32_t minGap = 10;
        for (;;) {
            if (frameCount == capacity) {
                const uint32_t grown = capacity * 2;
                int32_t* resized = (int32_t*) realloc(delays, grown * sizeof(*delays));
                if (resized == nullptr) {
                    free(delays);
                    AImageDecoderFrameInfo_delete(frameInfo);
                    AImageDecoder_delete(decoder);
                    return androidImageDecoderError(error, "out of memory");
                }
                delays = resized;
                capacity = grown;
            }

            int64_t nanos = 0;
            if (AImageDecoder_getFrameInfo(decoder, frameInfo) == ANDROID_IMAGE_DECODER_SUCCESS) {
                nanos = AImageDecoderFrameInfo_getDuration(frameInfo);
            }
            const int32_t cs = nanos > 0 ? (int32_t) (nanos / 10000000) : 0;
            delays[frameCount++] = cs;
            if (cs > 0) {
                minGap = 0;
            }

            if (AImageDecoder_advanceFrame(decoder) != ANDROID_IMAGE_DECODER_SUCCESS) {
                break;
            }
        }
        AImageDecoderFrameInfo_delete(frameInfo);

        // A single frame source never advanced, so it is still positioned on its only frame and
        // has nothing to rewind. Everything else stopped in the finished state.
        if (frameCount > 1 && AImageDecoder_rewind(decoder) != ANDROID_IMAGE_DECODER_SUCCESS) {
            free(delays);
            AImageDecoder_delete(decoder);
            return androidImageDecoderError(error, "failed to rewind the animation");
        }

        FFAndroidAnimation* session = (FFAndroidAnimation*) calloc(1, sizeof(*session));
        if (session == nullptr) {
            free(delays);
            AImageDecoder_delete(decoder);
            return androidImageDecoderError(error, "out of memory");
        }
        session->decoder = decoder;
        session->stride = stride;
        session->size = stride * requestData->logoPixelHeight;
        session->width = requestData->logoPixelWidth;
        session->height = requestData->logoPixelHeight;
        session->premultiplied = premultiplied;
        session->minGap = minGap;
        session->delaysCs = delays;

        session->canvas = (uint8_t*) calloc(1, session->size);
        if (session->canvas == nullptr) {
            free(session->delaysCs);
            free(session);
            AImageDecoder_delete(decoder);
            return androidImageDecoderError(error, "out of memory");
        }

        // 0: AImageDecoder has no API for the loop count, and looping forever is what a viewer
        // does with an animation that does not declare one.
        FFImageAnimation* animation = ffImageAnimationCreate(frameCount, 0, session, androidAnimationGetFrame, androidAnimationDestroy);
        if (animation == nullptr) {
            free(session->canvas);
            free(session->delaysCs);
            free(session);
            AImageDecoder_delete(decoder);
            return androidImageDecoderError(error, "out of memory");
        }

        *out = animation;
        return true;
    } else {
        return androidImageDecoderError(error, "animation support requires Android 12 (API 31) or newer");
    }
}
