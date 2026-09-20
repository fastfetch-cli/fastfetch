#include "codec.h"

#include "common/android/api.h"
#include "common/strutil.h"
#include <media/NdkMediaCodec.h>

static const struct {
    const char* mime;
    FFCodecType type;
} FF_CODEC_MIME_TO_TYPE[] = {
    { "video/3gpp", FF_CODEC_TYPE_H263 },
    { "video/h263", FF_CODEC_TYPE_H263 },
    { "video/mjpeg", FF_CODEC_TYPE_MJPEG },
    { "video/mpeg2", FF_CODEC_TYPE_MPEG2 },
    { "video/mp2v-es", FF_CODEC_TYPE_MPEG2 },
    { "video/mp4v-es", FF_CODEC_TYPE_DIVX_XVID },
    { "video/avc", FF_CODEC_TYPE_H264 },
    { "video/hevc", FF_CODEC_TYPE_HEVC },
    { "video/x-vnd.on2.vp8", FF_CODEC_TYPE_VP8 },
    { "video/x-vnd.on2.vp9", FF_CODEC_TYPE_VP9 },
    { "video/av01", FF_CODEC_TYPE_AV1 },
    { "video/vvc", FF_CODEC_TYPE_VVC },
};

static bool ffCodecIsLikelySoftware(const char* codecName) {
    if (!codecName) {
        return true;
    }

    return ffStrStartsWith(codecName, "OMX.google.") ||
        ffStrStartsWith(codecName, "c2.android.") ||
        ffStrStartsWith(codecName, "OMX.ffmpeg.") ||
        ffStrStartsWith(codecName, "OMX.PV.");
}

// Only the name query is newer than the API level this build targets: AMediaCodec_getName and
// AMediaCodec_releaseName are API 28, while creating and deleting a codec is API 21.
FF_ANDROID_REQUIRES_API(28) static bool ffCodecIsHardwareAccelerated(AMediaCodec* codec) {
    if (!codec) {
        return false;
    }

    char* codecName = nullptr;
    media_status_t status = AMediaCodec_getName(codec, &codecName);
    if (status != AMEDIA_OK || !codecName) {
        return false;
    }

    bool isHardware = !ffCodecIsLikelySoftware(codecName);
    AMediaCodec_releaseName(codec, codecName);
    return isHardware;
}

FF_ANDROID_REQUIRES_API(28) static const char* ffDetectCodecNativeImpl(FFCodecOptions* options, FFlist* result /*list of FFCodecResult*/) {
    FFCodecType decoders = FF_CODEC_TYPE_NONE;
    FFCodecType encoders = FF_CODEC_TYPE_NONE;

    for (uint32_t i = 0; i < ARRAY_SIZE(FF_CODEC_MIME_TO_TYPE); ++i) {
        const char* mime = FF_CODEC_MIME_TO_TYPE[i].mime;
        FFCodecType type = FF_CODEC_MIME_TO_TYPE[i].type;

        if ((options->showType & FF_CODEC_SHOW_TYPE_DECODER) && !(decoders & type)) {
            AMediaCodec* decoder = AMediaCodec_createDecoderByType(mime);
            if (decoder) {
                if (ffCodecIsHardwareAccelerated(decoder)) {
                    decoders |= type;
                }
                AMediaCodec_delete(decoder);
            }
        }

        if ((options->showType & FF_CODEC_SHOW_TYPE_ENCODER) && !(encoders & type)) {
            AMediaCodec* encoder = AMediaCodec_createEncoderByType(mime);
            if (encoder) {
                if (ffCodecIsHardwareAccelerated(encoder)) {
                    encoders |= type;
                }
                AMediaCodec_delete(encoder);
            }
        }
    }

    if (decoders != FF_CODEC_TYPE_NONE || encoders != FF_CODEC_TYPE_NONE) {
        FFCodecResult* item = FF_LIST_ADD(FFCodecResult, *result);
        ffStrbufInitStatic(&item->gpu, "Default");
        item->decoders = decoders;
        item->encoders = encoders;
        item->platformApi = "AMediaCodec";
    }

    return nullptr;
}

const char* ffDetectCodecNative(FFCodecOptions* options, FFlist* result /*list of FFCodecResult*/) {
    if (FF_ANDROID_API_AT_LEAST(28)) {
        return ffDetectCodecNativeImpl(options, result);
    }

    // Reading the codec name is the only way to tell a hardware codec from a software one. Without
    // it there is nothing to report, and listing the codecs as if they were accelerated would be a
    // guess, so say why instead.
    return "AMediaCodec_getName() requires Android 9 (API 28)";
}
