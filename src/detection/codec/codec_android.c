#include "codec.h"

#include "common/library.h"
#include "common/strutil.h"
#undef __INTRODUCED_IN
#define __INTRODUCED_IN(...)
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

static bool ffCodecIsHardwareAccelerated(
    AMediaCodec* codec,
    __typeof__(&AMediaCodec_getName) ffAMediaCodec_getName,
    __typeof__(&AMediaCodec_releaseName) ffAMediaCodec_releaseName) {
    if (!codec) {
        return false;
    }

    char* codecName = NULL;
    media_status_t status = ffAMediaCodec_getName(codec, &codecName);
    if (status != AMEDIA_OK || !codecName) {
        return false;
    }

    bool isHardware = !ffCodecIsLikelySoftware(codecName);
    ffAMediaCodec_releaseName(codec, codecName);
    return isHardware;
}

const char* ffDetectCodecNative(FFCodecOptions* options, FFlist* result /*list of FFCodecResult*/) {
    FF_LIBRARY_LOAD_MESSAGE(mediandk, "libmediandk.so", 0)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(mediandk, AMediaCodec_createDecoderByType)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(mediandk, AMediaCodec_createEncoderByType)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(mediandk, AMediaCodec_delete)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(mediandk, AMediaCodec_getName)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(mediandk, AMediaCodec_releaseName)

    FFCodecType decoders = FF_CODEC_TYPE_NONE;
    FFCodecType encoders = FF_CODEC_TYPE_NONE;

    for (uint32_t i = 0; i < ARRAY_SIZE(FF_CODEC_MIME_TO_TYPE); ++i) {
        const char* mime = FF_CODEC_MIME_TO_TYPE[i].mime;
        FFCodecType type = FF_CODEC_MIME_TO_TYPE[i].type;

        if ((options->showType & FF_CODEC_SHOW_TYPE_DECODER) && !(decoders & type)) {
            AMediaCodec* decoder = ffAMediaCodec_createDecoderByType(mime);
            if (decoder) {
                if (ffCodecIsHardwareAccelerated(decoder, ffAMediaCodec_getName, ffAMediaCodec_releaseName)) {
                    decoders |= type;
                }
                ffAMediaCodec_delete(decoder);
            }
        }

        if ((options->showType & FF_CODEC_SHOW_TYPE_ENCODER) && !(encoders & type)) {
            AMediaCodec* encoder = ffAMediaCodec_createEncoderByType(mime);
            if (encoder) {
                if (ffCodecIsHardwareAccelerated(encoder, ffAMediaCodec_getName, ffAMediaCodec_releaseName)) {
                    encoders |= type;
                }
                ffAMediaCodec_delete(encoder);
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

    return NULL;
}
