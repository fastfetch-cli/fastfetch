#include "codec.h"

#if FF_HAVE_VA || FF_HAVE_VDPAU

    #include "common/library.h"
    #include "common/mallocHelper.h"
    #include "common/strutil.h"
    #include "common/io.h"

    #if FF_HAVE_VA
        #include <fcntl.h>
        #include <va/va.h>
        #include <va/va_drm.h>
    #endif

    #include <string.h>

    #if FF_HAVE_VDPAU
        #include <stdlib.h>
        #include <vdpau/vdpau.h>

VdpStatus vdp_device_create_x11(void* display, int screen, VdpDevice* device, VdpGetProcAddress** get_proc_address);
void* XOpenDisplay(const char* display_name);
int XCloseDisplay(void* display);
int XDefaultScreen(void* display);
    #endif

    #if FF_HAVE_VA

static FFCodecType ffCodecProfileToType(VAProfile profile) {
    switch (profile) {
        case 11: // VAProfileH263Baseline
            return FF_CODEC_TYPE_H263;

        case 12: // VAProfileJPEGBaseline
            return FF_CODEC_TYPE_MJPEG;

        case 0: // VAProfileMPEG2Simple
        case 1: // VAProfileMPEG2Main
            return FF_CODEC_TYPE_MPEG2;

        case 2: // VAProfileMPEG4Simple
        case 3: // VAProfileMPEG4AdvancedSimple
        case 4: // VAProfileMPEG4Main
            return FF_CODEC_TYPE_DIVX_XVID;

        case 5:  // VAProfileH264Baseline
        case 6:  // VAProfileH264Main
        case 7:  // VAProfileH264High
        case 13: // VAProfileH264ConstrainedBaseline
        case 15: // VAProfileH264MultiviewHigh
        case 16: // VAProfileH264StereoHigh
        case 36: // VAProfileH264High10
        case 40: // VAProfileH264High422
            return FF_CODEC_TYPE_H264;

        case 8:  // VAProfileVC1Simple
        case 9:  // VAProfileVC1Main
        case 10: // VAProfileVC1Advanced
            return FF_CODEC_TYPE_VC1;

        case 14: // VAProfileVP8Version0_3
            return FF_CODEC_TYPE_VP8;

        case 17: // VAProfileHEVCMain
        case 18: // VAProfileHEVCMain10
        case 23: // VAProfileHEVCMain12
        case 24: // VAProfileHEVCMain422_10
        case 25: // VAProfileHEVCMain422_12
        case 26: // VAProfileHEVCMain444
        case 27: // VAProfileHEVCMain444_10
        case 28: // VAProfileHEVCMain444_12
        case 29: // VAProfileHEVCSccMain
        case 30: // VAProfileHEVCSccMain10
        case 31: // VAProfileHEVCSccMain444
        case 34: // VAProfileHEVCSccMain444_10
            return FF_CODEC_TYPE_HEVC;

        case 19: // VAProfileVP9Profile0
        case 20: // VAProfileVP9Profile1
        case 21: // VAProfileVP9Profile2
        case 22: // VAProfileVP9Profile3
            return FF_CODEC_TYPE_VP9;

        case 32: // VAProfileAV1Profile0
        case 33: // VAProfileAV1Profile1
        case 39: // VAProfileAV1Profile2
            return FF_CODEC_TYPE_AV1;

        case 37: // VAProfileVVCMain10
        case 38: // VAProfileVVCMultilayerMain10
            return FF_CODEC_TYPE_VVC;

        default:
            return FF_CODEC_TYPE_UNKNOWN;
    }
}

static FFCodecShowType ffCodecGetEntrypointType(VAEntrypoint entrypoint) {
    switch (entrypoint) {
        case VAEntrypointVLD:
        case VAEntrypointIDCT:
        case VAEntrypointMoComp:
            return FF_CODEC_SHOW_TYPE_DECODER;
        case VAEntrypointEncSlice:
        case VAEntrypointEncSliceLP:
        case VAEntrypointFEI:
            return FF_CODEC_SHOW_TYPE_ENCODER;
        default:
            return FF_CODEC_SHOW_TYPE_NONE;
    }
}

static bool ffCodecProfileHasOutput(
    VADisplay display,
    __typeof__(vaQueryConfigEntrypoints)* ffvaQueryConfigEntrypoints,
    __typeof__(vaGetConfigAttributes)* ffvaGetConfigAttributes,
    int maxEntrypoints,
    VAEntrypoint* entrypoints,
    VAProfile profile,
    FFCodecShowType entrypointType) {
    int numEntrypoints = maxEntrypoints;
    if (ffvaQueryConfigEntrypoints(display, profile, entrypoints, &numEntrypoints) != VA_STATUS_SUCCESS) {
        return false;
    }

    for (int i = 0; i < numEntrypoints; ++i) {
        if (ffCodecGetEntrypointType(entrypoints[i]) != entrypointType) {
            continue;
        }

        VAConfigAttrib attrib = {
            .type = VAConfigAttribRTFormat,
            .value = 0,
        };

        if (ffvaGetConfigAttributes(display, profile, entrypoints[i], &attrib, 1) == VA_STATUS_SUCCESS &&
            attrib.value != VA_ATTRIB_NOT_SUPPORTED &&
            attrib.value != 0) {
            return true;
        }
    }

    return false;
}

static const char* ffDetectCodecByVa(FFCodecOptions* options, FFlist* result) {
    FF_LIBRARY_LOAD_MESSAGE(libva, "libva" FF_LIBRARY_EXTENSION, 2)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libva, vaInitialize)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libva, vaTerminate)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libva, vaMaxNumProfiles)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libva, vaMaxNumEntrypoints)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libva, vaQueryConfigProfiles)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libva, vaQueryConfigEntrypoints)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libva, vaQueryVendorString)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libva, vaGetConfigAttributes)

    FF_LIBRARY_LOAD_MESSAGE(libvaDrm, "libva-drm" FF_LIBRARY_EXTENSION, 2)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libvaDrm, vaGetDisplayDRM)

    const char* error = "No DRM device could initialize VA-API";

    FF_AUTO_CLOSE_DIR DIR* dirp = opendir("/dev/dri/");
    if (dirp == NULL) {
        return "opendir(/dev/dri/) failed";
    }
    int drifd = dirfd(dirp);

    struct dirent* entry;
    while ((entry = readdir(dirp)) != NULL) {
        if (entry->d_name[0] == '.' || !ffStrStartsWith(entry->d_name, "renderD")) {
            continue;
        }

        FF_AUTO_CLOSE_FD int fd = openat(drifd, entry->d_name, O_RDONLY | O_CLOEXEC);
        if (fd < 0) {
            continue;
        }

        VADisplay display = ffvaGetDisplayDRM(fd);
        if (!display) {
            continue;
        }

        int major = 0, minor = 0;
        if (ffvaInitialize(display, &major, &minor) != VA_STATUS_SUCCESS) {
            ffvaTerminate(display);
            continue;
        }
        error = NULL;

        int maxProfiles = ffvaMaxNumProfiles(display);
        int maxEntrypoints = ffvaMaxNumEntrypoints(display);
        if (maxProfiles <= 0 || maxEntrypoints <= 0) {
            ffvaTerminate(display);
            continue;
        }

        FF_AUTO_FREE VAProfile* profiles = (VAProfile*) malloc(sizeof(VAProfile) * (size_t) maxProfiles);
        FF_AUTO_FREE VAEntrypoint* entrypoints = (VAEntrypoint*) malloc(sizeof(VAEntrypoint) * (size_t) maxEntrypoints);

        int numProfiles = maxProfiles;
        if (ffvaQueryConfigProfiles(display, profiles, &numProfiles) != VA_STATUS_SUCCESS) {
            ffvaTerminate(display);
            continue;
        }

        FFCodecType decoderTypes = FF_CODEC_TYPE_NONE;
        FFCodecType encoderTypes = FF_CODEC_TYPE_NONE;
        for (int j = 0; j < numProfiles; ++j) {
            FFCodecType type = ffCodecProfileToType(profiles[j]);

            bool hasDecoder = (options->showType & FF_CODEC_SHOW_TYPE_DECODER) &&
                !(decoderTypes & type) &&
                ffCodecProfileHasOutput(
                    display,
                    ffvaQueryConfigEntrypoints,
                    ffvaGetConfigAttributes,
                    maxEntrypoints,
                    entrypoints,
                    profiles[j],
                    FF_CODEC_SHOW_TYPE_DECODER);

            bool hasEncoder = (options->showType & FF_CODEC_SHOW_TYPE_ENCODER) &&
                !(encoderTypes & type) &&
                ffCodecProfileHasOutput(
                    display,
                    ffvaQueryConfigEntrypoints,
                    ffvaGetConfigAttributes,
                    maxEntrypoints,
                    entrypoints,
                    profiles[j],
                    FF_CODEC_SHOW_TYPE_ENCODER);

            if (!hasDecoder && !hasEncoder) {
                continue;
            }

            if (hasDecoder) {
                decoderTypes |= type;
            }
            if (hasEncoder) {
                encoderTypes |= type;
            }
        }

        if (decoderTypes != FF_CODEC_TYPE_NONE || encoderTypes != FF_CODEC_TYPE_NONE) {
            FFCodecResult* item = FF_LIST_ADD(FFCodecResult, *result);
            ffStrbufInitS(&item->gpu, ffvaQueryVendorString(display));
            item->decoders = decoderTypes;
            item->encoders = encoderTypes;
            item->platformApi = "VA-API";
        }

        ffvaTerminate(display);
    }

    return error;
}
    #endif

    #if FF_HAVE_VDPAU

static const struct FFCodecVdpauCodec {
    VdpDecoderProfile profile;
    FFCodecType type;
} FF_CODEC_VDPAU_CODECS[] = {
    { 0, FF_CODEC_TYPE_MPEG1 },      // VDP_DECODER_PROFILE_MPEG1
    { 1, FF_CODEC_TYPE_MPEG2 },      // VDP_DECODER_PROFILE_MPEG2_SIMPLE
    { 2, FF_CODEC_TYPE_MPEG2 },      // VDP_DECODER_PROFILE_MPEG2_MAIN
    { 12, FF_CODEC_TYPE_DIVX_XVID }, // VDP_DECODER_PROFILE_MPEG4_PART2_SP
    { 13, FF_CODEC_TYPE_DIVX_XVID }, // VDP_DECODER_PROFILE_MPEG4_PART2_ASP
    { 14, FF_CODEC_TYPE_DIVX_XVID }, // VDP_DECODER_PROFILE_DIVX4_QMOBILE
    { 15, FF_CODEC_TYPE_DIVX_XVID }, // VDP_DECODER_PROFILE_DIVX4_MOBILE
    { 16, FF_CODEC_TYPE_DIVX_XVID }, // VDP_DECODER_PROFILE_DIVX4_HOME_THEATER
    { 17, FF_CODEC_TYPE_DIVX_XVID }, // VDP_DECODER_PROFILE_DIVX4_HD_1080P
    { 18, FF_CODEC_TYPE_DIVX_XVID }, // VDP_DECODER_PROFILE_DIVX5_QMOBILE
    { 19, FF_CODEC_TYPE_DIVX_XVID }, // VDP_DECODER_PROFILE_DIVX5_MOBILE
    { 20, FF_CODEC_TYPE_DIVX_XVID }, // VDP_DECODER_PROFILE_DIVX5_HOME_THEATER
    { 21, FF_CODEC_TYPE_DIVX_XVID }, // VDP_DECODER_PROFILE_DIVX5_HD_1080P
    { 6, FF_CODEC_TYPE_H264 },       // VDP_DECODER_PROFILE_H264_BASELINE
    { 7, FF_CODEC_TYPE_H264 },       // VDP_DECODER_PROFILE_H264_MAIN
    { 8, FF_CODEC_TYPE_H264 },       // VDP_DECODER_PROFILE_H264_HIGH
    { 22, FF_CODEC_TYPE_H264 },      // VDP_DECODER_PROFILE_H264_CONSTRAINED_BASELINE
    { 23, FF_CODEC_TYPE_H264 },      // VDP_DECODER_PROFILE_H264_EXTENDED
    { 24, FF_CODEC_TYPE_H264 },      // VDP_DECODER_PROFILE_H264_PROGRESSIVE_HIGH
    { 25, FF_CODEC_TYPE_H264 },      // VDP_DECODER_PROFILE_H264_CONSTRAINED_HIGH
    { 26, FF_CODEC_TYPE_H264 },      // VDP_DECODER_PROFILE_H264_HIGH_444_PREDICTIVE
    { 9, FF_CODEC_TYPE_VC1 },        // VDP_DECODER_PROFILE_VC1_SIMPLE
    { 10, FF_CODEC_TYPE_VC1 },       // VDP_DECODER_PROFILE_VC1_MAIN
    { 11, FF_CODEC_TYPE_VC1 },       // VDP_DECODER_PROFILE_VC1_ADVANCED
    { 100, FF_CODEC_TYPE_HEVC },     // VDP_DECODER_PROFILE_HEVC_MAIN
    { 101, FF_CODEC_TYPE_HEVC },     // VDP_DECODER_PROFILE_HEVC_MAIN_10
    { 102, FF_CODEC_TYPE_HEVC },     // VDP_DECODER_PROFILE_HEVC_MAIN_STILL
    { 103, FF_CODEC_TYPE_HEVC },     // VDP_DECODER_PROFILE_HEVC_MAIN_12
    { 104, FF_CODEC_TYPE_HEVC },     // VDP_DECODER_PROFILE_HEVC_MAIN_444
    { 105, FF_CODEC_TYPE_HEVC },     // VDP_DECODER_PROFILE_HEVC_MAIN_444_10
    { 106, FF_CODEC_TYPE_HEVC },     // VDP_DECODER_PROFILE_HEVC_MAIN_444_12
    { 27, FF_CODEC_TYPE_VP9 },       // VDP_DECODER_PROFILE_VP9_PROFILE_0
    { 28, FF_CODEC_TYPE_VP9 },       // VDP_DECODER_PROFILE_VP9_PROFILE_1
    { 29, FF_CODEC_TYPE_VP9 },       // VDP_DECODER_PROFILE_VP9_PROFILE_2
    { 30, FF_CODEC_TYPE_VP9 },       // VDP_DECODER_PROFILE_VP9_PROFILE_3
    { 107, FF_CODEC_TYPE_AV1 },      // VDP_DECODER_PROFILE_AV1_MAIN
    { 108, FF_CODEC_TYPE_AV1 },      // VDP_DECODER_PROFILE_AV1_HIGH
    { 109, FF_CODEC_TYPE_AV1 },      // VDP_DECODER_PROFILE_AV1_PROFESSIONAL
};

static const char* ffDetectCodecByVdpau(FFCodecOptions* options, FFlist* result) {
    if (options->showType == FF_CODEC_SHOW_TYPE_DECODER) {
        return "VDPAU only supports decoding";
    }

    FF_LIBRARY_LOAD_MESSAGE(libX11, "libX11" FF_LIBRARY_EXTENSION, 6)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libX11, XOpenDisplay)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libX11, XCloseDisplay)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libX11, XDefaultScreen)

    FF_LIBRARY_LOAD_MESSAGE(libvdpau, "libvdpau" FF_LIBRARY_EXTENSION, 1)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libvdpau, vdp_device_create_x11)

    void* x11Display = ffXOpenDisplay(NULL);
    if (!x11Display) {
        return "XOpenDisplay() failed";
    }

    VdpDevice device = VDP_INVALID_HANDLE;
    VdpGetProcAddress* ffvdp_get_proc_address = NULL;
    if (ffvdp_device_create_x11(x11Display, ffXDefaultScreen(x11Display), &device, &ffvdp_get_proc_address) != VDP_STATUS_OK ||
        device == VDP_INVALID_HANDLE ||
        ffvdp_get_proc_address == NULL) {
        ffXCloseDisplay(x11Display);
        return "vdp_device_create_x11() failed";
    }

    VdpDeviceDestroy* ffvdp_device_destroy = NULL;
    VdpDecoderQueryCapabilities* ffvdp_decoder_query_capabilities = NULL;
    if (ffvdp_get_proc_address(device, VDP_FUNC_ID_DEVICE_DESTROY, (void**) &ffvdp_device_destroy) != VDP_STATUS_OK ||
        ffvdp_get_proc_address(device, VDP_FUNC_ID_DECODER_QUERY_CAPABILITIES, (void**) &ffvdp_decoder_query_capabilities) != VDP_STATUS_OK ||
        ffvdp_device_destroy == NULL ||
        ffvdp_decoder_query_capabilities == NULL) {
        if (ffvdp_device_destroy) {
            ffvdp_device_destroy(device);
        }
        ffXCloseDisplay(x11Display);
        return "ffvdp_get_proc_address() failed";
    }

    FFCodecType decoderTypes = FF_CODEC_TYPE_NONE;

    for (uint32_t i = 0; i < ARRAY_SIZE(FF_CODEC_VDPAU_CODECS); ++i) {
        const struct FFCodecVdpauCodec* codec = &FF_CODEC_VDPAU_CODECS[i];
        if (decoderTypes & codec->type) {
            continue;
        }
        VdpBool isSupported = VDP_FALSE;
        uint32_t maxLevel = 0, maxMacroblocks = 0, maxWidth = 0, maxHeight = 0;
        if (ffvdp_decoder_query_capabilities(device, codec->profile, &isSupported, &maxLevel, &maxMacroblocks, &maxWidth, &maxHeight) != VDP_STATUS_OK ||
            !isSupported) {
            continue;
        }

        decoderTypes |= codec->type;
    }

    ffvdp_device_destroy(device);
    ffXCloseDisplay(x11Display);

    if (decoderTypes == FF_CODEC_TYPE_NONE) {
        return NULL;
    }

    FFCodecResult* item = FF_LIST_ADD(FFCodecResult, *result);
    ffStrbufInit(&item->gpu);
    const char* driver = getenv("VDPAU_DRIVER");
    if (driver && *driver) {
        ffStrbufSetS(&item->gpu, driver);
    } else {
        ffStrbufSetStatic(&item->gpu, "Default");
    }
    item->decoders = decoderTypes;
    item->encoders = FF_CODEC_TYPE_NONE;
    item->platformApi = "VDPAU";

    return NULL;
}
    #endif

const char* ffDetectCodecNative(FFCodecOptions* options, FFlist* result /* list of FFCodecResult */) {
    FF_SUPPRESS_IO();

    #if FF_HAVE_VA
    if (ffDetectCodecByVa(options, result) == NULL) {
        return NULL;
    }
    #endif
    #if FF_HAVE_VDPAU
    if (ffDetectCodecByVdpau(options, result) == NULL) {
        return NULL;
    }
    #endif

    return "Both libva and libvdpau fail to initialize";
}

#else

const char* ffDetectCodecNative(FFCodecOptions* options, FFlist* result /* list of FFCodecResult */) {
    FF_UNUSED(options, result);
    return "Fastfetch was built without DRM / VA-API / VDPAU headers";
}

#endif
