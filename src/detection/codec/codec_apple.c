#include "codec.h"

#include <VideoToolbox/VideoToolbox.h>
#include "common/apple/cf_helpers.h"

static const struct {
    CMVideoCodecType codec;
    FFCodecType type;
} FF_CODEC_CODECS[] = {
    { kCMVideoCodecType_H263, FF_CODEC_TYPE_H263 },
    { kCMVideoCodecType_JPEG, FF_CODEC_TYPE_MJPEG },
    { kCMVideoCodecType_JPEG_XL, FF_CODEC_TYPE_JPEG_XL },
    { kCMVideoCodecType_MPEG1Video, FF_CODEC_TYPE_MPEG1 },
    { kCMVideoCodecType_MPEG2Video, FF_CODEC_TYPE_MPEG2 },
    { kCMVideoCodecType_MPEG4Video, FF_CODEC_TYPE_DIVX_XVID },
    { kCMVideoCodecType_H264, FF_CODEC_TYPE_H264 },
    { kCMVideoCodecType_HEVC, FF_CODEC_TYPE_HEVC },
    { kCMVideoCodecType_HEVCWithAlpha, FF_CODEC_TYPE_HEVC },
    { kCMVideoCodecType_DolbyVisionHEVC, FF_CODEC_TYPE_DOLBY_VISION_HEVC },
    { kCMVideoCodecType_DisparityHEVC, FF_CODEC_TYPE_HEVC },
    { kCMVideoCodecType_DepthHEVC, FF_CODEC_TYPE_HEVC },
    { kCMVideoCodecType_VP9, FF_CODEC_TYPE_VP9 },
    { kCMVideoCodecType_AV1, FF_CODEC_TYPE_AV1 },
    { kCMVideoCodecType_AppleProRes4444XQ, FF_CODEC_TYPE_PRORES },
    { kCMVideoCodecType_AppleProRes4444, FF_CODEC_TYPE_PRORES },
    { kCMVideoCodecType_AppleProRes422HQ, FF_CODEC_TYPE_PRORES },
    { kCMVideoCodecType_AppleProRes422, FF_CODEC_TYPE_PRORES },
    { kCMVideoCodecType_AppleProRes422LT, FF_CODEC_TYPE_PRORES },
    { kCMVideoCodecType_AppleProRes422Proxy, FF_CODEC_TYPE_PRORES },
    { kCMVideoCodecType_AppleProResRAW, FF_CODEC_TYPE_PRORES_RAW },
    { kCMVideoCodecType_AppleProResRAWHQ, FF_CODEC_TYPE_PRORES_RAW },
};

static FFCodecType ffCodecCodecToType(CMVideoCodecType codec) {
    for (uint32_t i = 0; i < ARRAY_SIZE(FF_CODEC_CODECS); ++i) {
        if (FF_CODEC_CODECS[i].codec == codec) {
            return FF_CODEC_CODECS[i].type;
        }
    }
    return FF_CODEC_TYPE_NONE;
}

static FFCodecType ffCodecDetectEncoders(void) {
    CFArrayRef encoderList = NULL;
    if (VTCopyVideoEncoderList(NULL, &encoderList) != noErr || !encoderList) {
        return FF_CODEC_TYPE_NONE;
    }

    FFCodecType types = FF_CODEC_TYPE_NONE;
    for (uint32_t i = 0; i < CFArrayGetCount(encoderList); ++i) {
        CFDictionaryRef encoder = CFArrayGetValueAtIndex(encoderList, i);
        bool isHardwareAccelerated;
        int codec;
        if (ffCfDictGetBool(encoder, CFSTR("IsHardwareAccelerated"), &isHardwareAccelerated) != NULL ||
            !isHardwareAccelerated ||
            ffCfDictGetInt(encoder, CFSTR("CodecType"), &codec) != NULL) {
            continue;
        }
        types |= ffCodecCodecToType((CMVideoCodecType) codec);
    }

    return types;
}

static FFCodecType ffCodecDetectDecoders() {
    FFCodecType types = FF_CODEC_TYPE_NONE;
    for (uint32_t i = 0; i < ARRAY_SIZE(FF_CODEC_CODECS); ++i) {
        if (types & FF_CODEC_CODECS[i].type) {
            continue;
        }

        bool supported = VTIsHardwareDecodeSupported(FF_CODEC_CODECS[i].codec);
        if (!supported) {
            continue;
        }

        types |= FF_CODEC_CODECS[i].type;
    }

    return types;
}

const char* ffDetectCodecNative(FF_A_UNUSED FFCodecOptions* options, FFlist* result /* list of FFCodecResult */) {
    FFCodecType decoders = ffCodecDetectDecoders();
    FFCodecType encoders = ffCodecDetectEncoders();

    if (decoders != FF_CODEC_TYPE_NONE || encoders != FF_CODEC_TYPE_NONE) {
        FFCodecResult* item = FF_LIST_ADD(FFCodecResult, *result);
        ffStrbufInitStatic(&item->gpu, "Default");
        item->decoders = decoders;
        item->encoders = encoders;
        item->platformApi = "VideoToolbox";
    }

    return NULL;
}
