#include "codec.h"

#include <VideoToolbox/VideoToolbox.h>
#include "common/apple/cf_helpers.h"

static const struct {
    CMVideoCodecType codec;
    FFCodecType type;
} FF_CODEC_CODECS[] = {
    { 'h263', FF_CODEC_TYPE_H263 },              // kCMVideoCodecType_H263
    { 'jpeg', FF_CODEC_TYPE_MJPEG },             // kCMVideoCodecType_JPEG
    { 'dmb1', FF_CODEC_TYPE_MJPEG },             // kCMVideoCodecType_JPEG_OpenDML
    { 'mp1v', FF_CODEC_TYPE_MPEG1 },             // kCMVideoCodecType_MPEG1Video
    { 'mp2v', FF_CODEC_TYPE_MPEG2 },             // kCMVideoCodecType_MPEG2Video
    { 'mp4v', FF_CODEC_TYPE_DIVX_XVID },         // kCMVideoCodecType_MPEG4Video
    { 'avc1', FF_CODEC_TYPE_H264 },              // kCMVideoCodecType_H264
    { 'hvc1', FF_CODEC_TYPE_HEVC },              // kCMVideoCodecType_HEVC
    { 'muxa', FF_CODEC_TYPE_HEVC },              // kCMVideoCodecType_HEVCWithAlpha
    { 'dvh1', FF_CODEC_TYPE_DOLBY_VISION_HEVC }, // kCMVideoCodecType_DolbyVisionHEVC
    { 'dish', FF_CODEC_TYPE_HEVC },              // kCMVideoCodecType_DisparityHEVC
    { 'deph', FF_CODEC_TYPE_HEVC },              // kCMVideoCodecType_DepthHEVC
    { 'vp09', FF_CODEC_TYPE_VP9 },               // kCMVideoCodecType_VP9
    { 'av01', FF_CODEC_TYPE_AV1 },               // kCMVideoCodecType_AV1
    { 'ap4x', FF_CODEC_TYPE_PRORES },            // kCMVideoCodecType_AppleProRes4444XQ
    { 'ap4h', FF_CODEC_TYPE_PRORES },            // kCMVideoCodecType_AppleProRes4444
    { 'apch', FF_CODEC_TYPE_PRORES },            // kCMVideoCodecType_AppleProRes422HQ
    { 'apcn', FF_CODEC_TYPE_PRORES },            // kCMVideoCodecType_AppleProRes422
    { 'apcs', FF_CODEC_TYPE_PRORES },            // kCMVideoCodecType_AppleProRes422LT
    { 'apco', FF_CODEC_TYPE_PRORES },            // kCMVideoCodecType_AppleProRes422Proxy
    { 'aprn', FF_CODEC_TYPE_PRORES_RAW },        // kCMVideoCodecType_AppleProResRAW
    { 'aprh', FF_CODEC_TYPE_PRORES_RAW },        // kCMVideoCodecType_AppleProResRAWHQ
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

const char* ffDetectCodecNative(FFCodecOptions* options, FFlist* result /* list of FFCodecResult */) {
    FFCodecType decoders = options->showType & FF_CODEC_SHOW_TYPE_DECODER ? ffCodecDetectDecoders() : FF_CODEC_TYPE_NONE;
    FFCodecType encoders = options->showType & FF_CODEC_SHOW_TYPE_ENCODER ? ffCodecDetectEncoders() : FF_CODEC_TYPE_NONE;

    if (decoders != FF_CODEC_TYPE_NONE || encoders != FF_CODEC_TYPE_NONE) {
        FFCodecResult* item = FF_LIST_ADD(FFCodecResult, *result);
        ffStrbufInitStatic(&item->gpu, "Default");
        item->decoders = decoders;
        item->encoders = encoders;
        item->platformApi = "VideoToolbox";
    }

    return NULL;
}
