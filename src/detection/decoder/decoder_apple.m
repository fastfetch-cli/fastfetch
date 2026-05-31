#include "decoder.h"

#import <Metal/MTLDevice.h>
#import <VideoToolbox/VideoToolbox.h>

const char* ffDetectDecoder(FF_A_UNUSED FFDecoderOptions* options, FFlist* result /* list of FFDecoderResult */) {
    static const struct {
        CMVideoCodecType codec;
        FFDecoderType type;
    } codecs[] = {
        { kCMVideoCodecType_H263, FF_DECODER_TYPE_H263 },
        { kCMVideoCodecType_JPEG, FF_DECODER_TYPE_MJPEG },
        { kCMVideoCodecType_JPEG_XL, FF_DECODER_TYPE_JPEG_XL },
        { kCMVideoCodecType_MPEG1Video, FF_DECODER_TYPE_MPEG1 },
        { kCMVideoCodecType_MPEG2Video, FF_DECODER_TYPE_MPEG2 },
        { kCMVideoCodecType_MPEG4Video, FF_DECODER_TYPE_DIVX_XVID },
        { kCMVideoCodecType_H264, FF_DECODER_TYPE_H264 },
        { kCMVideoCodecType_HEVC, FF_DECODER_TYPE_HEVC },
        { kCMVideoCodecType_HEVCWithAlpha, FF_DECODER_TYPE_HEVC },
        { kCMVideoCodecType_DolbyVisionHEVC, FF_DECODER_TYPE_DOLBY_VISION_HEVC },
        { kCMVideoCodecType_DisparityHEVC, FF_DECODER_TYPE_HEVC },
        { kCMVideoCodecType_DepthHEVC, FF_DECODER_TYPE_HEVC },
        { kCMVideoCodecType_VP9, FF_DECODER_TYPE_VP9 },
        { kCMVideoCodecType_AV1, FF_DECODER_TYPE_AV1 },
        { kCMVideoCodecType_AppleProRes4444XQ, FF_DECODER_TYPE_PRORES },
        { kCMVideoCodecType_AppleProRes4444, FF_DECODER_TYPE_PRORES },
        { kCMVideoCodecType_AppleProRes422HQ, FF_DECODER_TYPE_PRORES },
        { kCMVideoCodecType_AppleProRes422, FF_DECODER_TYPE_PRORES },
        { kCMVideoCodecType_AppleProRes422LT, FF_DECODER_TYPE_PRORES },
        { kCMVideoCodecType_AppleProRes422Proxy, FF_DECODER_TYPE_PRORES },
        { kCMVideoCodecType_AppleProResRAW, FF_DECODER_TYPE_PRORES_RAW },
        { kCMVideoCodecType_AppleProResRAWHQ, FF_DECODER_TYPE_PRORES_RAW },
    };

    FFDecoderType types = FF_DECODER_TYPE_NONE;

    for (uint32_t i = 0; i < ARRAY_SIZE(codecs); ++i) {
        if (types & codecs[i].type || !VTIsHardwareDecodeSupported(codecs[i].codec)) {
            continue;
        }
        types |= codecs[i].type;
    }

    if (types != FF_DECODER_TYPE_NONE) {
        FFDecoderResult* item = FF_LIST_ADD(FFDecoderResult, *result);
        ffStrbufInitS(&item->gpu, MTLCreateSystemDefaultDevice().name.UTF8String ?: "VideoToolbox");
        item->types = types;
    }

    return NULL;
}
