#include "decoder.h"

#if FF_HAVE_DRM && FF_HAVE_VA

#include "detection/gpu/gpu.h"
#include "common/io.h"
#include "common/library.h"
#include "common/mallocHelper.h"

#include <fcntl.h>
#include <string.h>
#include <va/va.h>
#include <va/va_drm.h>
#include <xf86drm.h>

static FFDecoderType ffDecoderProfileToType(VAProfile profile) {
    switch (profile) {
        case 11: // VAProfileH263Baseline
            return FF_DECODER_TYPE_H263;

        case 12: // VAProfileJPEGBaseline
            return FF_DECODER_TYPE_MJPEG;

        case 0: // VAProfileMPEG2Simple
        case 1: // VAProfileMPEG2Main
            return FF_DECODER_TYPE_MPEG2;

        case 2: // VAProfileMPEG4Simple
        case 3: // VAProfileMPEG4AdvancedSimple
        case 4: // VAProfileMPEG4Main
            return FF_DECODER_TYPE_DIVX_XVID;

        case 5:  // VAProfileH264Baseline
        case 6:  // VAProfileH264Main
        case 7:  // VAProfileH264High
        case 13: // VAProfileH264ConstrainedBaseline
        case 15: // VAProfileH264MultiviewHigh
        case 16: // VAProfileH264StereoHigh
        case 36: // VAProfileH264High10
        case 40: // VAProfileH264High422
            return FF_DECODER_TYPE_H264;

        case 8:  // VAProfileVC1Simple
        case 9:  // VAProfileVC1Main
        case 10: // VAProfileVC1Advanced
            return FF_DECODER_TYPE_VC1;

        case 14: // VAProfileVP8Version0_3
            return FF_DECODER_TYPE_VP8;

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
            return FF_DECODER_TYPE_HEVC;

        case 19: // VAProfileVP9Profile0
        case 20: // VAProfileVP9Profile1
        case 21: // VAProfileVP9Profile2
        case 22: // VAProfileVP9Profile3
            return FF_DECODER_TYPE_VP9;

        case 32: // VAProfileAV1Profile0
        case 33: // VAProfileAV1Profile1
        case 39: // VAProfileAV1Profile2
            return FF_DECODER_TYPE_AV1;

        case 37: // VAProfileVVCMain10
        case 38: // VAProfileVVCMultilayerMain10
            return FF_DECODER_TYPE_VVC;

        default:
            return FF_DECODER_TYPE_UNKNOWN;
    }
}

static bool ffDecoderEntrypointIsDecode(VAEntrypoint entrypoint) {
    switch (entrypoint) {
        case VAEntrypointVLD:
        case VAEntrypointIDCT:
        case VAEntrypointMoComp:
            return true;
        default:
            return false;
    }
}

static bool ffDecoderProfileHasOutput(
    VADisplay display,
    __typeof__(vaQueryConfigEntrypoints)* ffvaQueryConfigEntrypoints,
    __typeof__(vaGetConfigAttributes)* ffvaGetConfigAttributes,
    int maxEntrypoints,
    VAEntrypoint* entrypoints,
    VAProfile profile
) {
    int numEntrypoints = maxEntrypoints;
    if (ffvaQueryConfigEntrypoints(display, profile, entrypoints, &numEntrypoints) != VA_STATUS_SUCCESS) {
        return false;
    }

    for (int i = 0; i < numEntrypoints; ++i) {
        if (!ffDecoderEntrypointIsDecode(entrypoints[i])) {
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

static void ffDecoderFillGpuName(const drmDevice* dev, const char* path, FFstrbuf* name) {
    ffStrbufInit(name);

    switch (dev->bustype) {
        case DRM_BUS_PCI: {
            FFGPUResult gpu = {
                .vendor = ffStrbufCreateStatic(ffGPUGetVendorString(dev->deviceinfo.pci->vendor_id)),
                .name = ffStrbufCreate(),
            };

            ffGPUFillVendorAndName(0, dev->deviceinfo.pci->vendor_id, dev->deviceinfo.pci->device_id, &gpu);
            ffStrbufSetF(name, "%s %s", gpu.vendor.chars, gpu.name.chars);

            ffStrbufDestroy(&gpu.vendor);
            ffStrbufDestroy(&gpu.name);
            break;
        }
        case DRM_BUS_PLATFORM:
            ffStrbufSetS(name, dev->deviceinfo.platform->compatible[0]);
            return;
        case DRM_BUS_HOST1X:
            ffStrbufSetS(name, dev->deviceinfo.host1x->compatible[0]);
            return;
        case DRM_BUS_USB:
            ffStrbufSetF(name, "0x%04X 0x%04X", dev->deviceinfo.usb->vendor, dev->deviceinfo.usb->product);
            return;
        default:
            ffStrbufSetStatic(name, "Unknown GPU");
            return;
    }

    if (!name->length && path && *path) {
        const char* base = strrchr(path, '/');
        ffStrbufSetS(name, base ? base + 1 : path);
    }

    if (!name->length) {
        ffStrbufSetStatic(name, "Unknown GPU");
    }
}

const char* ffDetectDecoder(FF_A_UNUSED FFDecoderOptions* options, FFlist* result /* list of FFDecoderResult */) {
    FF_LIBRARY_LOAD_MESSAGE(libdrm, "libdrm" FF_LIBRARY_EXTENSION, 2)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmGetDevices)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmFreeDevices)

    FF_LIBRARY_LOAD_MESSAGE(libva, "libva" FF_LIBRARY_EXTENSION, 2)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libva, vaInitialize)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libva, vaTerminate)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libva, vaMaxNumProfiles)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libva, vaMaxNumEntrypoints)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libva, vaQueryConfigProfiles)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libva, vaQueryConfigEntrypoints)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libva, vaGetConfigAttributes)

    FF_LIBRARY_LOAD_MESSAGE(libvaDrm, "libva-drm" FF_LIBRARY_EXTENSION, 2)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libvaDrm, vaGetDisplayDRM)

    drmDevicePtr devices[64];
    int numDevices = ffdrmGetDevices(devices, ARRAY_SIZE(devices));
    if (numDevices < 0) {
        return "drmGetDevices() failed";
    }
    if (numDevices == 0) {
        return NULL;
    }

    for (int i = 0; i < numDevices; ++i) {
        drmDevice* dev = devices[i];

        const char* path = NULL;
        if (dev->available_nodes & (1 << DRM_NODE_RENDER)) {
            path = dev->nodes[DRM_NODE_RENDER];
        } else if (dev->available_nodes & (1 << DRM_NODE_PRIMARY)) {
            path = dev->nodes[DRM_NODE_PRIMARY];
        } else {
            continue;
        }

        FF_AUTO_CLOSE_FD int fd = open(path, O_RDWR | O_CLOEXEC);
        if (fd < 0) {
            fd = open(path, O_RDONLY | O_CLOEXEC);
        }
        if (fd < 0) {
            continue;
        }

        VADisplay display = ffvaGetDisplayDRM(fd);
        if (!display) {
            continue;
        }

        int major = 0, minor = 0;
        if (ffvaInitialize(display, &major, &minor) != VA_STATUS_SUCCESS) {
            continue;
        }

        int maxProfiles = ffvaMaxNumProfiles(display);
        int maxEntrypoints = ffvaMaxNumEntrypoints(display);
        if (maxProfiles <= 0 || maxEntrypoints <= 0) {
            ffvaTerminate(display);
            continue;
        }

        FF_AUTO_FREE VAProfile* profiles = (VAProfile*) malloc(sizeof(VAProfile) * (size_t) maxProfiles);
        FF_AUTO_FREE VAEntrypoint* entrypoints = (VAEntrypoint*) malloc(sizeof(VAEntrypoint) * (size_t) maxEntrypoints);
        if (!profiles || !entrypoints) {
            ffvaTerminate(display);
            return "malloc() failed";
        }

        int numProfiles = maxProfiles;
        if (ffvaQueryConfigProfiles(display, profiles, &numProfiles) != VA_STATUS_SUCCESS) {
            ffvaTerminate(display);
            continue;
        }

        FFDecoderType types = FF_DECODER_TYPE_NONE;
        for (int j = 0; j < numProfiles; ++j) {
            if (!ffDecoderProfileHasOutput(
                    display,
                    ffvaQueryConfigEntrypoints,
                    ffvaGetConfigAttributes,
                    maxEntrypoints,
                    entrypoints,
                    profiles[j])) {
                continue;
            }

            FFDecoderType type = ffDecoderProfileToType(profiles[j]);
            types = (FFDecoderType) ((uint32_t) types | (uint32_t) type);
        }

        if (types != FF_DECODER_TYPE_NONE) {
            FFDecoderResult* item = FF_LIST_ADD(FFDecoderResult, *result);
            ffDecoderFillGpuName(dev, path, &item->gpu);
            item->types = types;
        }

        ffvaTerminate(display);
    }

    ffdrmFreeDevices(devices, numDevices);
    return NULL;
}

#else

const char* ffDetectDecoder(FFDecoderOptions* options, FFlist* result /* list of FFDecoderResult */) {
    FF_UNUSED(options, result);
    return "Fastfetch was built without DRM / VA-API headers";
}

#endif
