extern "C" {
#include "codec.h"
#include "common/library.h"
#include "common/windows/com.h"
#include "common/windows/unicode.h"
}

#include <d3d11.h>
#include <d3d12.h>
#include <dxgi.h>
#include <initguid.h>
#include <dxva.h>
#include <d3d12video.h>

static const DXGI_FORMAT FF_NATIVE_CODEC_FORMATS[] = {
    DXGI_FORMAT_420_OPAQUE,
    DXGI_FORMAT_NV12,
    DXGI_FORMAT_P010,
    DXGI_FORMAT_P016,
    DXGI_FORMAT_YUY2,
    DXGI_FORMAT_Y210,
    DXGI_FORMAT_Y216,
    DXGI_FORMAT_AYUV,
    DXGI_FORMAT_Y410,
    DXGI_FORMAT_Y416,
};

static FFCodecType ffCodecProfileToTypeDx11(const GUID& profile) {
    // clang-format off
    if (IsEqualGUID(profile, DXVA_ModeH261_A) ||
        IsEqualGUID(profile, DXVA_ModeH261_B)) {
        return FF_CODEC_TYPE_H261;
    }

    if (IsEqualGUID(profile, DXVA_ModeH263_A) ||
        IsEqualGUID(profile, DXVA_ModeH263_B) ||
        IsEqualGUID(profile, DXVA_ModeH263_C) ||
        IsEqualGUID(profile, DXVA_ModeH263_D) ||
        IsEqualGUID(profile, DXVA_ModeH263_E) ||
        IsEqualGUID(profile, DXVA_ModeH263_F)) {
        return FF_CODEC_TYPE_H263;
    }

    if (IsEqualGUID(profile, DXVA_ModeMJPEG_VLD_420) ||
        IsEqualGUID(profile, DXVA_ModeMJPEG_VLD_422) ||
        IsEqualGUID(profile, DXVA_ModeMJPEG_VLD_444) ||
        IsEqualGUID(profile, DXVA_ModeMJPEG_VLD_4444) ||
        IsEqualGUID(profile, DXVA_ModeJPEG_VLD_420) ||
        IsEqualGUID(profile, DXVA_ModeJPEG_VLD_422) ||
        IsEqualGUID(profile, DXVA_ModeJPEG_VLD_444)) {
        return FF_CODEC_TYPE_MJPEG;
    }

    if (IsEqualGUID(profile, DXVA_ModeMPEG1_A) ||
        IsEqualGUID(profile, DXVA_ModeMPEG1_VLD)) {
        return FF_CODEC_TYPE_MPEG1;
    }

    if (IsEqualGUID(profile, DXVA_ModeMPEG2_A) ||
        IsEqualGUID(profile, DXVA_ModeMPEG2_B) ||
        IsEqualGUID(profile, DXVA_ModeMPEG2_C) ||
        IsEqualGUID(profile, DXVA_ModeMPEG2_D) ||
        IsEqualGUID(profile, DXVA_ModeMPEG2and1_VLD)) {
        return FF_CODEC_TYPE_MPEG2;
    }

    if (IsEqualGUID(profile, DXVA_ModeMPEG4pt2_VLD_Simple) ||
        IsEqualGUID(profile, DXVA_ModeMPEG4pt2_VLD_AdvSimple_NoGMC) ||
        IsEqualGUID(profile, DXVA_ModeMPEG4pt2_VLD_AdvSimple_GMC)) {
        return FF_CODEC_TYPE_DIVX_XVID;
    }

    if (IsEqualGUID(profile, DXVA_ModeH264_E) ||
        IsEqualGUID(profile, DXVA_ModeH264_F) ||
        IsEqualGUID(profile, DXVA_ModeH264_VLD_WithFMOASO_NoFGT) ||
        IsEqualGUID(profile, DXVA_ModeH264_VLD_Stereo_Progressive_NoFGT) ||
        IsEqualGUID(profile, DXVA_ModeH264_VLD_Stereo_NoFGT) ||
        IsEqualGUID(profile, DXVA_ModeH264_VLD_Multiview_NoFGT)) {
        return FF_CODEC_TYPE_H264;
    }

    if (IsEqualGUID(profile, DXVA_ModeWMV8_A) ||
        IsEqualGUID(profile, DXVA_ModeWMV8_B)) {
        return FF_CODEC_TYPE_WMV8;
    }

    if (IsEqualGUID(profile, DXVA_ModeWMV9_A) ||
        IsEqualGUID(profile, DXVA_ModeWMV9_B) ||
        IsEqualGUID(profile, DXVA_ModeWMV9_C)) {
        return FF_CODEC_TYPE_WMV9;
    }

    if (IsEqualGUID(profile, DXVA_ModeVC1_A) ||
        IsEqualGUID(profile, DXVA_ModeVC1_B) ||
        IsEqualGUID(profile, DXVA_ModeVC1_C) ||
        IsEqualGUID(profile, DXVA_ModeVC1_D) ||
        IsEqualGUID(profile, DXVA_ModeVC1_D2010)) {
        return FF_CODEC_TYPE_VC1;
    }

    if (IsEqualGUID(profile, DXVA_ModeVP8_VLD)) {
        return FF_CODEC_TYPE_VP8;
    }

    if (IsEqualGUID(profile, DXVA_ModeHEVC_VLD_Main) ||
        IsEqualGUID(profile, DXVA_ModeHEVC_VLD_Main10) ||
        IsEqualGUID(profile, DXVA_ModeHEVC_VLD_Main12) ||
        IsEqualGUID(profile, DXVA_ModeHEVC_VLD_Main10_422) ||
        IsEqualGUID(profile, DXVA_ModeHEVC_VLD_Main12_422) ||
        IsEqualGUID(profile, DXVA_ModeHEVC_VLD_Main_444) ||
        IsEqualGUID(profile, DXVA_ModeHEVC_VLD_Main10_Ext) ||
        IsEqualGUID(profile, DXVA_ModeHEVC_VLD_Main10_444) ||
        IsEqualGUID(profile, DXVA_ModeHEVC_VLD_Main12_444) ||
        IsEqualGUID(profile, DXVA_ModeHEVC_VLD_Main16)) {
        return FF_CODEC_TYPE_HEVC;
    }

    if (IsEqualGUID(profile, DXVA_ModeVP9_VLD_Profile0) ||
        IsEqualGUID(profile, DXVA_ModeVP9_VLD_10bit_Profile2)) {
        return FF_CODEC_TYPE_VP9;
    }

    if (IsEqualGUID(profile, DXVA_ModeAV1_VLD_Profile0) ||
        IsEqualGUID(profile, DXVA_ModeAV1_VLD_Profile1) ||
        IsEqualGUID(profile, DXVA_ModeAV1_VLD_Profile2) ||
        IsEqualGUID(profile, DXVA_ModeAV1_VLD_12bit_Profile2) ||
        IsEqualGUID(profile, DXVA_ModeAV1_VLD_12bit_Profile2_420)) {
        return FF_CODEC_TYPE_AV1;
    }
    // clang-format on

    return FF_CODEC_TYPE_UNKNOWN;
}

static FFCodecType ffCodecProfileToTypeDx12(const GUID& profile) {
    // clang-format off
    if (IsEqualGUID(profile, D3D12_VIDEO_DECODE_PROFILE_MPEG1_AND_MPEG2)) {
        return FFCodecType(FF_CODEC_TYPE_MPEG1 | FF_CODEC_TYPE_MPEG2);
    }

    if (IsEqualGUID(profile, D3D12_VIDEO_DECODE_PROFILE_MPEG2)) {
        return FF_CODEC_TYPE_MPEG2;
    }

    if (IsEqualGUID(profile, D3D12_VIDEO_DECODE_PROFILE_MPEG4PT2_SIMPLE) ||
        IsEqualGUID(profile, D3D12_VIDEO_DECODE_PROFILE_MPEG4PT2_ADVSIMPLE_NOGMC)) {
        return FF_CODEC_TYPE_DIVX_XVID;
    }

    if (IsEqualGUID(profile, D3D12_VIDEO_DECODE_PROFILE_H264) ||
        IsEqualGUID(profile, D3D12_VIDEO_DECODE_PROFILE_H264_STEREO_PROGRESSIVE) ||
        IsEqualGUID(profile, D3D12_VIDEO_DECODE_PROFILE_H264_STEREO) ||
        IsEqualGUID(profile, D3D12_VIDEO_DECODE_PROFILE_H264_MULTIVIEW)) {
        return FF_CODEC_TYPE_H264;
    }

    if (IsEqualGUID(profile, D3D12_VIDEO_DECODE_PROFILE_VC1) ||
        IsEqualGUID(profile, D3D12_VIDEO_DECODE_PROFILE_VC1_D2010)) {
        return FF_CODEC_TYPE_VC1;
    }

    if (IsEqualGUID(profile, D3D12_VIDEO_DECODE_PROFILE_VP8)) {
        return FF_CODEC_TYPE_VP8;
    }

    if (IsEqualGUID(profile, D3D12_VIDEO_DECODE_PROFILE_HEVC_MAIN) ||
        IsEqualGUID(profile, D3D12_VIDEO_DECODE_PROFILE_HEVC_MAIN10)) {
        return FF_CODEC_TYPE_HEVC;
    }

    if (IsEqualGUID(profile, D3D12_VIDEO_DECODE_PROFILE_VP9) ||
        IsEqualGUID(profile, D3D12_VIDEO_DECODE_PROFILE_VP9_10BIT_PROFILE2)) {
        return FF_CODEC_TYPE_VP9;
    }

    if (IsEqualGUID(profile, D3D12_VIDEO_DECODE_PROFILE_AV1_PROFILE0) ||
        IsEqualGUID(profile, D3D12_VIDEO_DECODE_PROFILE_AV1_PROFILE1) ||
        IsEqualGUID(profile, D3D12_VIDEO_DECODE_PROFILE_AV1_PROFILE2) ||
        IsEqualGUID(profile, D3D12_VIDEO_DECODE_PROFILE_AV1_12BIT_PROFILE2) ||
        IsEqualGUID(profile, D3D12_VIDEO_DECODE_PROFILE_AV1_12BIT_PROFILE2_420)) {
        return FF_CODEC_TYPE_AV1;
    }
    // clang-format on

    return FF_CODEC_TYPE_UNKNOWN;
}

static bool ffCodecProfileHasNativeOutput(ID3D11VideoDevice* videoDevice, const GUID& profile) {
    for (uint32_t i = 0; i < ARRAY_SIZE(FF_NATIVE_CODEC_FORMATS); ++i) {
        BOOL supported = FALSE;
        if (SUCCEEDED(videoDevice->CheckVideoDecoderFormat(&profile, FF_NATIVE_CODEC_FORMATS[i], &supported)) && supported) {
            return true;
        }
    }

    return false;
}

static bool ffCodecProfileHasNativeOutput(ID3D12VideoDevice* videoDevice, const GUID& profile, UINT nodeIndex) {
    for (uint32_t i = 0; i < ARRAY_SIZE(FF_NATIVE_CODEC_FORMATS); ++i) {
        D3D12_FEATURE_DATA_VIDEO_DECODE_SUPPORT support = {
            .NodeIndex = nodeIndex,
            .Configuration = {
                .DecodeProfile = profile,
                .BitstreamEncryption = D3D12_BITSTREAM_ENCRYPTION_TYPE_NONE,
                .InterlaceType = D3D12_VIDEO_FRAME_CODED_INTERLACE_TYPE_NONE,
            },
            .Width = 1920,
            .Height = 1080,
            .DecodeFormat = FF_NATIVE_CODEC_FORMATS[i],
            .FrameRate = {
                .Numerator = 30,
                .Denominator = 1,
            },
            .BitRate = 10000000,
            .SupportFlags = D3D12_VIDEO_DECODE_SUPPORT_FLAG_NONE,
            .ConfigurationFlags = D3D12_VIDEO_DECODE_CONFIGURATION_FLAG_NONE,
            .DecodeTier = D3D12_VIDEO_DECODE_TIER_NOT_SUPPORTED,
        };

        if (SUCCEEDED(videoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO_DECODE_SUPPORT, &support, sizeof(support))) &&
            (support.SupportFlags & D3D12_VIDEO_DECODE_SUPPORT_FLAG_SUPPORTED)) {
            return true;
        }
    }

    return false;
}

static bool ffCodecEncoderSupportedD3d12(ID3D12VideoDevice* videoDevice, D3D12_VIDEO_ENCODER_CODEC codec, UINT nodeIndex) {
    D3D12_FEATURE_DATA_VIDEO_ENCODER_RATE_CONTROL_MODE encoderMode = {};
    encoderMode.NodeIndex = nodeIndex;
    encoderMode.Codec = codec;
    encoderMode.RateControlMode = D3D12_VIDEO_ENCODER_RATE_CONTROL_MODE_CQP;

    return SUCCEEDED(videoDevice->CheckFeatureSupport(
               D3D12_FEATURE_VIDEO_ENCODER_RATE_CONTROL_MODE,
               &encoderMode,
               sizeof(encoderMode))) &&
        encoderMode.IsSupported;
}

static FFCodecType ffCodecEncoderToType(D3D12_VIDEO_ENCODER_CODEC codec) {
    switch (codec) {
        case D3D12_VIDEO_ENCODER_CODEC_H264:
            return FF_CODEC_TYPE_H264;
        case D3D12_VIDEO_ENCODER_CODEC_HEVC:
            return FF_CODEC_TYPE_HEVC;
        case D3D12_VIDEO_ENCODER_CODEC_AV1:
            return FF_CODEC_TYPE_AV1;
        default:
            return FF_CODEC_TYPE_UNKNOWN;
    }
}

template <typename Func>
static void ffEnumHardwareAdapters(IDXGIFactory1* factory, Func&& onAdapter) {
    for (UINT adapterIndex = 0;; ++adapterIndex) {
        IDXGIAdapter1* FF_AUTO_RELEASE_COM_OBJECT adapter = nullptr;
        HRESULT adapterStatus = factory->EnumAdapters1(adapterIndex, &adapter);
        if (adapterStatus == DXGI_ERROR_NOT_FOUND) {
            break;
        }
        if (FAILED(adapterStatus) || !adapter) {
            continue;
        }

        DXGI_ADAPTER_DESC1 desc;
        if (FAILED(adapter->GetDesc1(&desc))) {
            continue;
        }
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
            continue;
        }

        onAdapter(adapter, desc);
    }
}

const char* detectD3d11va(FFlist* result /*list of FFCodecResult*/, IDXGIFactory1* factory) {
    FF_LIBRARY_LOAD_MESSAGE(d3d11, "d3d11" FF_LIBRARY_EXTENSION, 1)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(d3d11, D3D11CreateDevice)

    ffEnumHardwareAdapters(factory, [&](IDXGIAdapter1* adapter, const DXGI_ADAPTER_DESC1& desc) {
        ID3D11Device* FF_AUTO_RELEASE_COM_OBJECT d3dDevice = nullptr;
        D3D_FEATURE_LEVEL featureLevel;
        HRESULT deviceStatus = ffD3D11CreateDevice(
            adapter,
            D3D_DRIVER_TYPE_UNKNOWN,
            nullptr,
            D3D11_CREATE_DEVICE_VIDEO_SUPPORT,
            nullptr,
            0,
            D3D11_SDK_VERSION,
            &d3dDevice,
            &featureLevel,
            nullptr);
        if (FAILED(deviceStatus) || !d3dDevice) {
            return;
        }

        ID3D11VideoDevice* FF_AUTO_RELEASE_COM_OBJECT videoDevice = nullptr;
        if (FAILED(d3dDevice->QueryInterface(__uuidof(ID3D11VideoDevice), (void**) &videoDevice)) || !videoDevice) {
            return;
        }

        FFCodecResult* gpuResult = nullptr;
        UINT profileCount = videoDevice->GetVideoDecoderProfileCount();
        for (UINT profileIndex = 0; profileIndex < profileCount; ++profileIndex) {
            GUID profile;
            if (FAILED(videoDevice->GetVideoDecoderProfile(profileIndex, &profile))) {
                continue;
            }

            if (!ffCodecProfileHasNativeOutput(videoDevice, profile)) {
                continue;
            }

            if (!gpuResult) {
                gpuResult = FF_LIST_ADD(FFCodecResult, *result);
                ffStrbufInitWS(&gpuResult->gpu, desc.Description);
                gpuResult->decoders = FF_CODEC_TYPE_NONE;
                gpuResult->encoders = FF_CODEC_TYPE_NONE;
                gpuResult->platformApi = "d3d11va";
            }
            gpuResult->decoders = (FFCodecType) (((uint32_t) gpuResult->decoders) | ((uint32_t) ffCodecProfileToTypeDx11(profile)));
        }
    });

    return nullptr;
}

const char* detectD3d12va(FFlist* result /*list of FFCodecResult*/, IDXGIFactory1* factory) {
    FF_LIBRARY_LOAD_MESSAGE(d3d12, "d3d12" FF_LIBRARY_EXTENSION, 1)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(d3d12, D3D12CreateDevice)

    static const GUID FF_D3D12_DECODE_PROFILES[] = {
        D3D12_VIDEO_DECODE_PROFILE_MPEG2,
        D3D12_VIDEO_DECODE_PROFILE_MPEG1_AND_MPEG2,
        D3D12_VIDEO_DECODE_PROFILE_H264,
        D3D12_VIDEO_DECODE_PROFILE_H264_STEREO_PROGRESSIVE,
        D3D12_VIDEO_DECODE_PROFILE_H264_STEREO,
        D3D12_VIDEO_DECODE_PROFILE_H264_MULTIVIEW,
        D3D12_VIDEO_DECODE_PROFILE_VC1,
        D3D12_VIDEO_DECODE_PROFILE_VC1_D2010,
        D3D12_VIDEO_DECODE_PROFILE_MPEG4PT2_SIMPLE,
        D3D12_VIDEO_DECODE_PROFILE_MPEG4PT2_ADVSIMPLE_NOGMC,
        D3D12_VIDEO_DECODE_PROFILE_HEVC_MAIN,
        D3D12_VIDEO_DECODE_PROFILE_HEVC_MAIN10,
        D3D12_VIDEO_DECODE_PROFILE_VP9,
        D3D12_VIDEO_DECODE_PROFILE_VP9_10BIT_PROFILE2,
        D3D12_VIDEO_DECODE_PROFILE_VP8,
        D3D12_VIDEO_DECODE_PROFILE_AV1_PROFILE0,
        D3D12_VIDEO_DECODE_PROFILE_AV1_PROFILE1,
        D3D12_VIDEO_DECODE_PROFILE_AV1_PROFILE2,
        D3D12_VIDEO_DECODE_PROFILE_AV1_12BIT_PROFILE2,
        D3D12_VIDEO_DECODE_PROFILE_AV1_12BIT_PROFILE2_420,
    };

    static const D3D12_VIDEO_ENCODER_CODEC FF_D3D12_ENCODER_CODECS[] = {
        D3D12_VIDEO_ENCODER_CODEC_H264,
        D3D12_VIDEO_ENCODER_CODEC_HEVC,
        D3D12_VIDEO_ENCODER_CODEC_AV1,
    };

    const uint32_t resultLengthBefore = result->length;

    ffEnumHardwareAdapters(factory, [&](IDXGIAdapter1* adapter, const DXGI_ADAPTER_DESC1& desc) {
        ID3D12Device* FF_AUTO_RELEASE_COM_OBJECT d3dDevice = nullptr;
        HRESULT deviceStatus = ffD3D12CreateDevice(
            adapter,
            D3D_FEATURE_LEVEL_11_0,
            __uuidof(ID3D12Device),
            (void**) &d3dDevice);
        if (FAILED(deviceStatus) || !d3dDevice) {
            return;
        }

        ID3D12VideoDevice* FF_AUTO_RELEASE_COM_OBJECT videoDevice = nullptr;
        if (FAILED(d3dDevice->QueryInterface(__uuidof(ID3D12VideoDevice), (void**) &videoDevice)) || !videoDevice) {
            return;
        }

        FFCodecType decoders = FF_CODEC_TYPE_NONE;
        FFCodecType encoders = FF_CODEC_TYPE_NONE;

        for (uint32_t profileIndex = 0; profileIndex < ARRAY_SIZE(FF_D3D12_DECODE_PROFILES); ++profileIndex) {
            const GUID& profile = FF_D3D12_DECODE_PROFILES[profileIndex];
            if (!ffCodecProfileHasNativeOutput(videoDevice, profile, 0)) {
                continue;
            }

            FFCodecType codecType = ffCodecProfileToTypeDx12(profile);
            decoders = (FFCodecType) (((uint32_t) decoders) | ((uint32_t) codecType));
        }

        for (uint32_t codecIndex = 0; codecIndex < ARRAY_SIZE(FF_D3D12_ENCODER_CODECS); ++codecIndex) {
            D3D12_VIDEO_ENCODER_CODEC codec = FF_D3D12_ENCODER_CODECS[codecIndex];
            if (!ffCodecEncoderSupportedD3d12(videoDevice, codec, 0)) {
                continue;
            }

            FFCodecType codecType = ffCodecEncoderToType(codec);
            encoders = (FFCodecType) (((uint32_t) encoders) | ((uint32_t) codecType));
        }

        if (decoders == FF_CODEC_TYPE_NONE && encoders == FF_CODEC_TYPE_NONE) {
            return;
        }

        FFCodecResult* gpuResult = FF_LIST_ADD(FFCodecResult, *result);
        ffStrbufInitWS(&gpuResult->gpu, desc.Description);
        gpuResult->decoders = decoders;
        gpuResult->encoders = encoders;
        gpuResult->platformApi = "d3d12va";
    });

    if (result->length == resultLengthBefore) {
        return "No D3D12 video acceleration support";
    }

    return nullptr;
}

const char* ffDetectCodec(FF_A_UNUSED FFCodecOptions* options, FFlist* result /*list of FFCodecResult*/) {
    FF_LIBRARY_LOAD_MESSAGE(dxgi, "dxgi" FF_LIBRARY_EXTENSION, 1)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(dxgi, CreateDXGIFactory1)

    const char* error = ffInitCom();
    if (error) {
        return error;
    }

    IDXGIFactory1* FF_AUTO_RELEASE_COM_OBJECT factory = nullptr;
    if (FAILED(ffCreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**) &factory)) || !factory) {
        return "CreateDXGIFactory1() failed";
    }

    if (detectD3d12va(result, factory) == nullptr) {
        return nullptr;
    }
    return detectD3d11va(result, factory);
}
