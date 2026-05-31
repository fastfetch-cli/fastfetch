#pragma once

#include "fastfetch.h"
#include "modules/decoder/option.h"

typedef enum FF_A_PACKED FFDecoderType {
    FF_DECODER_TYPE_NONE = 0,
    FF_DECODER_TYPE_UNKNOWN = UINT32_C(1) << 0,
    FF_DECODER_TYPE_H261 = UINT32_C(1) << 1,
    FF_DECODER_TYPE_H263 = UINT32_C(1) << 2,
    FF_DECODER_TYPE_MJPEG = UINT32_C(1) << 3,
    FF_DECODER_TYPE_MPEG1 = UINT32_C(1) << 4,
    FF_DECODER_TYPE_MPEG2 = UINT32_C(1) << 5,
    FF_DECODER_TYPE_DIVX_XVID = UINT32_C(1) << 6,
    FF_DECODER_TYPE_H264 = UINT32_C(1) << 7,
    FF_DECODER_TYPE_WMV8 = UINT32_C(1) << 8,
    FF_DECODER_TYPE_WMV9 = UINT32_C(1) << 9,
    FF_DECODER_TYPE_VC1 = UINT32_C(1) << 10,
    FF_DECODER_TYPE_VP8 = UINT32_C(1) << 11,
    FF_DECODER_TYPE_HEVC = UINT32_C(1) << 12,
    FF_DECODER_TYPE_VP9 = UINT32_C(1) << 13,
    FF_DECODER_TYPE_AV1 = UINT32_C(1) << 14,
    FF_DECODER_TYPE_VVC = UINT32_C(1) << 15,
    FF_DECODER_TYPE_DOLBY_VISION_HEVC = UINT32_C(1) << 16,
    FF_DECODER_TYPE_PRORES = UINT32_C(1) << 17,
    FF_DECODER_TYPE_PRORES_RAW = UINT32_C(1) << 18,
    FF_DECODER_TYPE_JPEG_XL = UINT32_C(1) << 19,
    FF_DECODER_TYPE_MAX = FF_DECODER_TYPE_JPEG_XL,
    FF_DECODER_TYPE_FORCE_UNSIGNED = UINT32_MAX,
} FFDecoderType;

typedef struct FFDecoderResult {
    FFstrbuf gpu;
    FFDecoderType types;
} FFDecoderResult;

const char* ffDetectDecoder(FFDecoderOptions* options, FFlist* result /*list of FFDecoderResult*/);
