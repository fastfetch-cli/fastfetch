#pragma once

#include "fastfetch.h"
#include "modules/codec/option.h"

typedef enum FF_A_PACKED FFCodecType {
    FF_CODEC_TYPE_NONE = 0,
    FF_CODEC_TYPE_UNKNOWN = UINT32_C(1) << 0,
    FF_CODEC_TYPE_H261 = UINT32_C(1) << 1,
    FF_CODEC_TYPE_H263 = UINT32_C(1) << 2,
    FF_CODEC_TYPE_MJPEG = UINT32_C(1) << 3,
    FF_CODEC_TYPE_MPEG1 = UINT32_C(1) << 4,
    FF_CODEC_TYPE_MPEG2 = UINT32_C(1) << 5,
    FF_CODEC_TYPE_DIVX_XVID = UINT32_C(1) << 6,
    FF_CODEC_TYPE_H264 = UINT32_C(1) << 7,
    FF_CODEC_TYPE_WMV8 = UINT32_C(1) << 8,
    FF_CODEC_TYPE_WMV9 = UINT32_C(1) << 9,
    FF_CODEC_TYPE_VC1 = UINT32_C(1) << 10,
    FF_CODEC_TYPE_VP8 = UINT32_C(1) << 11,
    FF_CODEC_TYPE_HEVC = UINT32_C(1) << 12,
    FF_CODEC_TYPE_VP9 = UINT32_C(1) << 13,
    FF_CODEC_TYPE_AV1 = UINT32_C(1) << 14,
    FF_CODEC_TYPE_VVC = UINT32_C(1) << 15,
    FF_CODEC_TYPE_DOLBY_VISION_HEVC = UINT32_C(1) << 16,
    FF_CODEC_TYPE_PRORES = UINT32_C(1) << 17,
    FF_CODEC_TYPE_PRORES_RAW = UINT32_C(1) << 18,
    FF_CODEC_TYPE_JPEG_XL = UINT32_C(1) << 19,
    FF_CODEC_TYPE_MAX = FF_CODEC_TYPE_JPEG_XL,
    FF_CODEC_TYPE_FORCE_UNSIGNED = UINT32_MAX,
} FFCodecType;

typedef struct FFCodecResult {
    FFstrbuf gpu;
    FFCodecType decoders;
    FFCodecType encoders;
    const char* platformApi;
} FFCodecResult;

const char* ffDetectCodec(FFCodecOptions* options, FFlist* result /*list of FFCodecResult*/);
const char* ffDetectCodecNative(FFCodecOptions* options, FFlist* result /*list of FFCodecResult*/);

#ifdef FF_HAVE_VULKAN
const char* ffDetectCodecVulkan(FFlist* result /*list of FFCodecResult*/);
#endif
