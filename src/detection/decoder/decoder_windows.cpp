extern "C" {
#include "decoder.h"
#include "common/library.h"
#include "common/windows/com.h"
#include "common/windows/unicode.h"
}

#include <d3d11.h>
#include <dxgi.h>
#include <initguid.h>
#include <dxva.h>

static const DXGI_FORMAT FF_NATIVE_DECODER_FORMATS[] = {
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

static FFDecoderType ffDecoderProfileToType(const GUID& profile) {
    // clang-format off
    if (IsEqualGUID(profile, DXVA_ModeH261_A) ||
        IsEqualGUID(profile, DXVA_ModeH261_B)) {
        return FF_DECODER_TYPE_H261;
    }

    if (IsEqualGUID(profile, DXVA_ModeH263_A) ||
        IsEqualGUID(profile, DXVA_ModeH263_B) ||
        IsEqualGUID(profile, DXVA_ModeH263_C) ||
        IsEqualGUID(profile, DXVA_ModeH263_D) ||
        IsEqualGUID(profile, DXVA_ModeH263_E) ||
        IsEqualGUID(profile, DXVA_ModeH263_F)) {
        return FF_DECODER_TYPE_H263;
    }

    if (IsEqualGUID(profile, DXVA_ModeMJPEG_VLD_420) ||
        IsEqualGUID(profile, DXVA_ModeMJPEG_VLD_422) ||
        IsEqualGUID(profile, DXVA_ModeMJPEG_VLD_444) ||
        IsEqualGUID(profile, DXVA_ModeMJPEG_VLD_4444) ||
        IsEqualGUID(profile, DXVA_ModeJPEG_VLD_420) ||
        IsEqualGUID(profile, DXVA_ModeJPEG_VLD_422) ||
        IsEqualGUID(profile, DXVA_ModeJPEG_VLD_444)) {
        return FF_DECODER_TYPE_MJPEG;
    }

    if (IsEqualGUID(profile, DXVA_ModeMPEG1_A) ||
        IsEqualGUID(profile, DXVA_ModeMPEG1_VLD)) {
        return FF_DECODER_TYPE_MPEG1;
    }

    if (IsEqualGUID(profile, DXVA_ModeMPEG2_A) ||
        IsEqualGUID(profile, DXVA_ModeMPEG2_B) ||
        IsEqualGUID(profile, DXVA_ModeMPEG2_C) ||
        IsEqualGUID(profile, DXVA_ModeMPEG2_D) ||
        IsEqualGUID(profile, DXVA_ModeMPEG2and1_VLD)) {
        return FF_DECODER_TYPE_MPEG2;
    }

    if (IsEqualGUID(profile, DXVA_ModeMPEG4pt2_VLD_Simple) ||
        IsEqualGUID(profile, DXVA_ModeMPEG4pt2_VLD_AdvSimple_NoGMC) ||
        IsEqualGUID(profile, DXVA_ModeMPEG4pt2_VLD_AdvSimple_GMC)) {
        return FF_DECODER_TYPE_DIVX_XVID;
    }

    if (IsEqualGUID(profile, DXVA_ModeH264_E) ||
        IsEqualGUID(profile, DXVA_ModeH264_F) ||
        IsEqualGUID(profile, DXVA_ModeH264_VLD_WithFMOASO_NoFGT) ||
        IsEqualGUID(profile, DXVA_ModeH264_VLD_Stereo_Progressive_NoFGT) ||
        IsEqualGUID(profile, DXVA_ModeH264_VLD_Stereo_NoFGT) ||
        IsEqualGUID(profile, DXVA_ModeH264_VLD_Multiview_NoFGT)) {
        return FF_DECODER_TYPE_H264;
    }

    if (IsEqualGUID(profile, DXVA_ModeWMV8_A) ||
        IsEqualGUID(profile, DXVA_ModeWMV8_B)) {
        return FF_DECODER_TYPE_WMV8;
    }

    if (IsEqualGUID(profile, DXVA_ModeWMV9_A) ||
        IsEqualGUID(profile, DXVA_ModeWMV9_B) ||
        IsEqualGUID(profile, DXVA_ModeWMV9_C)) {
        return FF_DECODER_TYPE_WMV9;
    }

    if (IsEqualGUID(profile, DXVA_ModeVC1_A) ||
        IsEqualGUID(profile, DXVA_ModeVC1_B) ||
        IsEqualGUID(profile, DXVA_ModeVC1_C) ||
        IsEqualGUID(profile, DXVA_ModeVC1_D) ||
        IsEqualGUID(profile, DXVA_ModeVC1_D2010)) {
        return FF_DECODER_TYPE_VC1;
    }

    if (IsEqualGUID(profile, DXVA_ModeVP8_VLD)) {
        return FF_DECODER_TYPE_VP8;
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
        return FF_DECODER_TYPE_HEVC;
    }

    if (IsEqualGUID(profile, DXVA_ModeVP9_VLD_Profile0) ||
        IsEqualGUID(profile, DXVA_ModeVP9_VLD_10bit_Profile2)) {
        return FF_DECODER_TYPE_VP9;
    }

    if (IsEqualGUID(profile, DXVA_ModeAV1_VLD_Profile0) ||
        IsEqualGUID(profile, DXVA_ModeAV1_VLD_Profile1) ||
        IsEqualGUID(profile, DXVA_ModeAV1_VLD_Profile2) ||
        IsEqualGUID(profile, DXVA_ModeAV1_VLD_12bit_Profile2) ||
        IsEqualGUID(profile, DXVA_ModeAV1_VLD_12bit_Profile2_420)) {
        return FF_DECODER_TYPE_AV1;
    }
    // clang-format on

    return FF_DECODER_TYPE_UNKNOWN;
}

static bool ffDecoderProfileHasNativeOutput(ID3D11VideoDevice* videoDevice, const GUID& profile) {
    for (uint32_t i = 0; i < ARRAY_SIZE(FF_NATIVE_DECODER_FORMATS); ++i) {
        BOOL supported = FALSE;
        if (SUCCEEDED(videoDevice->CheckVideoDecoderFormat(&profile, FF_NATIVE_DECODER_FORMATS[i], &supported)) && supported) {
            return true;
        }
    }

    return false;
}

const char* ffDetectDecoder(FF_A_UNUSED FFDecoderOptions* options, FFlist* result /*list of FFDecoderResult*/) {
    FF_LIBRARY_LOAD_MESSAGE(dxgi, "dxgi" FF_LIBRARY_EXTENSION, 1)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(dxgi, CreateDXGIFactory1)
    FF_LIBRARY_LOAD_MESSAGE(d3d11, "d3d11" FF_LIBRARY_EXTENSION, 1)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(d3d11, D3D11CreateDevice)

    const char* error = ffInitCom();
    if (error) {
        return error;
    }

    IDXGIFactory1* FF_AUTO_RELEASE_COM_OBJECT factory = nullptr;
    if (FAILED(ffCreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**) &factory)) || !factory) {
        return "CreateDXGIFactory1() failed";
    }

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
            continue;
        }

        ID3D11VideoDevice* FF_AUTO_RELEASE_COM_OBJECT videoDevice = nullptr;
        if (FAILED(d3dDevice->QueryInterface(__uuidof(ID3D11VideoDevice), (void**) &videoDevice)) || !videoDevice) {
            continue;
        }

        FFDecoderResult* gpuResult = nullptr;
        UINT profileCount = videoDevice->GetVideoDecoderProfileCount();
        for (UINT profileIndex = 0; profileIndex < profileCount; ++profileIndex) {
            GUID profile;
            if (FAILED(videoDevice->GetVideoDecoderProfile(profileIndex, &profile))) {
                continue;
            }

            if (!ffDecoderProfileHasNativeOutput(videoDevice, profile)) {
                continue;
            }

            if (!gpuResult) {
                gpuResult = FF_LIST_ADD(FFDecoderResult, *result);
                ffStrbufInitWS(&gpuResult->gpu, desc.Description);
                gpuResult->types = FF_DECODER_TYPE_NONE;
            }
            gpuResult->types = (FFDecoderType) (((uint32_t) gpuResult->types) | ((uint32_t) ffDecoderProfileToType(profile)));
        }
    }

    return nullptr;
}
