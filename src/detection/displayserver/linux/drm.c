#include "displayserver_linux.h"
#include "common/io.h"
#include "common/edidHelper.h"
#include "common/strutil.h"
#include "common/mallocHelper.h"

#ifdef __linux__
    #include <dirent.h>

static const char* drmParseSysfs(FFDisplayServerResult* result) {
    const char* drmDirPath = "/sys/class/drm/";

    FF_AUTO_CLOSE_DIR DIR* dirp = opendir(drmDirPath);
    if (dirp == NULL) {
        return "opendir(drmDirPath) failed";
    }

    FF_STRBUF_AUTO_DESTROY drmDir = ffStrbufCreateA(64);
    ffStrbufAppendS(&drmDir, drmDirPath);

    uint32_t drmDirLength = drmDir.length;

    struct dirent* entry;
    while ((entry = readdir(dirp)) != NULL) {
        if (entry->d_name[0] == '.') {
            continue;
        }

        ffStrbufAppendS(&drmDir, entry->d_name);
        uint32_t drmDirWithDnameLength = drmDir.length;

        char buf;
        ffStrbufAppendS(&drmDir, "/enabled");
        if (ffReadFileData(drmDir.chars, sizeof(buf), &buf) <= 0 || buf != 'e') {
            /* read failed or enabled != "enabled" */
            ffStrbufSubstrBefore(&drmDir, drmDirWithDnameLength);
            ffStrbufAppendS(&drmDir, "/status");
            buf = 'd';
            ffReadFileData(drmDir.chars, sizeof(buf), &buf);
            if (buf != 'c') {
                /* read failed or status != "connected" */
                ffStrbufSubstrBefore(&drmDir, drmDirLength);
                continue;
            }
        }

        unsigned width = 0, height = 0, physicalWidth = 0, physicalHeight = 0;
        double refreshRate = 0;
        FF_STRBUF_AUTO_DESTROY name = ffStrbufCreate();

        ffStrbufSubstrBefore(&drmDir, drmDirWithDnameLength);
        ffStrbufAppendS(&drmDir, "/edid");

        const char* plainName = entry->d_name;
        if (ffStrStartsWith(plainName, "card")) {
            const char* tmp = strchr(plainName + strlen("card"), '-');
            if (tmp) {
                plainName = tmp + 1;
            }
        }

        uint8_t edidData[512];
        ssize_t edidLength = ffReadFileData(drmDir.chars, ARRAY_SIZE(edidData), edidData);
        if (edidLength <= 0 || edidLength % 128 != 0) {
            edidLength = 0;
            ffStrbufSubstrBefore(&drmDir, drmDirWithDnameLength);
            ffStrbufAppendS(&drmDir, "/modes");

            char modes[32];
            if (ffReadFileData(drmDir.chars, ARRAY_SIZE(modes), modes) >= 3) {
                sscanf(modes, "%ux%u", &width, &height);
                ffStrbufAppendS(&name, plainName);
            }
        } else {
            ffEdidGetName(edidData, &name);
            ffEdidGetPreferredResolutionAndRefreshRate(edidData, &width, &height, &refreshRate);
            ffEdidGetPhysicalSize(edidData, &physicalWidth, &physicalHeight);
        }

        FFDisplayResult* item = ffdsAppendDisplay(
            result,
            width,
            height,
            refreshRate,
            0,
            0,
            0,
            0,
            0,
            &name,
            ffdsGetDisplayType(plainName),
            false,
            0,
            physicalWidth,
            physicalHeight,
            "sysfs-drm");
        if (item && edidLength) {
            item->hdrStatus = ffEdidGetHdrCompatible(edidData, (uint32_t) edidLength) ? FF_DISPLAY_HDR_STATUS_SUPPORTED : FF_DISPLAY_HDR_STATUS_UNSUPPORTED;
            ffEdidGetSerialAndManufactureDate(edidData, &item->serial, &item->manufactureYear, &item->manufactureWeek);
        }

        ffStrbufSubstrBefore(&drmDir, drmDirLength);
    }

    return NULL;
}
#endif

#if FF_HAVE_DRM || __has_include(<drm/drm.h>)

    #include <fcntl.h>
    #include <sys/ioctl.h>
    #if FF_HAVE_DRM
        #include <drm.h>
        #include <drm_mode.h>
    #else
        #define FF_HAVE_DRM 1
        #include <drm/drm.h>
        #include <drm/drm_mode.h>
    #endif

// https://gitlab.freedesktop.org/mesa/drm/-/blob/main/xf86drmMode.c#L1785
// It's not supported on Ubuntu 20.04
static inline const char* drmType2Name(uint32_t connector_type) {
    /* Keep the strings in sync with the kernel's drm_connector_enum_list in
     * drm_connector.c. */
    switch (connector_type) {
        case DRM_MODE_CONNECTOR_Unknown:
            return "Unknown";
        case DRM_MODE_CONNECTOR_VGA:
            return "VGA";
        case DRM_MODE_CONNECTOR_DVII:
            return "DVI-I";
        case DRM_MODE_CONNECTOR_DVID:
            return "DVI-D";
        case DRM_MODE_CONNECTOR_DVIA:
            return "DVI-A";
        case DRM_MODE_CONNECTOR_Composite:
            return "Composite";
        case DRM_MODE_CONNECTOR_SVIDEO:
            return "SVIDEO";
        case DRM_MODE_CONNECTOR_LVDS:
            return "LVDS";
        case DRM_MODE_CONNECTOR_Component:
            return "Component";
        case DRM_MODE_CONNECTOR_9PinDIN:
            return "DIN";
        case DRM_MODE_CONNECTOR_DisplayPort:
            return "DP";
        case DRM_MODE_CONNECTOR_HDMIA:
            return "HDMI-A";
        case DRM_MODE_CONNECTOR_HDMIB:
            return "HDMI-B";
        case DRM_MODE_CONNECTOR_TV:
            return "TV";
        case DRM_MODE_CONNECTOR_eDP:
            return "eDP";
        case DRM_MODE_CONNECTOR_VIRTUAL:
            return "Virtual";
        case DRM_MODE_CONNECTOR_DSI:
            return "DSI";
        case DRM_MODE_CONNECTOR_DPI:
            return "DPI";
        case DRM_MODE_CONNECTOR_WRITEBACK:
            return "Writeback";
        case 19 /*DRM_MODE_CONNECTOR_SPI*/:
            return "SPI";
        case 20 /*DRM_MODE_CONNECTOR_USB*/:
            return "USB";
        default:
            return "Unsupported";
    }
}

FF_A_UNUSED static const char* drmGetEdidByConnId(uint32_t connId, uint8_t* edidData, ssize_t* edidLength) {
    const char* drmDirPath = "/sys/class/drm/";

    FF_AUTO_CLOSE_DIR DIR* dirp = opendir(drmDirPath);
    if (dirp == NULL) {
        return "opendir(drmDirPath) failed";
    }

    FF_STRBUF_AUTO_DESTROY drmDir = ffStrbufCreateA(64);
    ffStrbufAppendS(&drmDir, drmDirPath);

    uint32_t drmDirLength = drmDir.length;

    struct dirent* entry;
    while ((entry = readdir(dirp)) != NULL) {
        if (entry->d_name[0] == '.') {
            continue;
        }

        ffStrbufAppendS(&drmDir, entry->d_name);
        uint32_t drmDirWithDnameLength = drmDir.length;

        char connectorId[16] = {};

        ffStrbufAppendS(&drmDir, "/connector_id");
        ffReadFileData(drmDir.chars, ARRAY_SIZE(connectorId), connectorId);
        if (strtoul(connectorId, NULL, 10) != connId) {
            ffStrbufSubstrBefore(&drmDir, drmDirLength);
            continue;
        }

        ffStrbufSubstrBefore(&drmDir, drmDirWithDnameLength);
        ffStrbufAppendS(&drmDir, "/edid");
        *edidLength = ffReadFileData(drmDir.chars, (uint32_t) *edidLength, edidData);
        return NULL;
    }

    return "Failed to match connector ID";
}

static const char* drmConnectLibdrm(FFDisplayServerResult* result) {
    FF_AUTO_CLOSE_DIR DIR* dirp = opendir("/dev/dri/");
    if (dirp == NULL) {
        return "opendir(/dev/dri/) failed";
    }
    int drifd = dirfd(dirp);

    int nSuccess = 0;

    struct dirent* entry;
    while ((entry = readdir(dirp)) != NULL) {
        if (entry->d_name[0] == '.' || !ffStrStartsWith(entry->d_name, "card")) {
            continue;
        }

    #if __linux__
        char powerStatusBuf[512];
        snprintf(powerStatusBuf, ARRAY_SIZE(powerStatusBuf), "/sys/class/drm/%s/device/power/runtime_status", entry->d_name);

        char buffer[8] = "";
        if (ffReadFileData(powerStatusBuf, strlen("suspend"), buffer) > 0 && ffStrStartsWith(buffer, "suspend")) {
            continue;
        }
    #endif

        FF_AUTO_CLOSE_FD int primaryFd = openat(drifd, entry->d_name, O_RDWR | O_CLOEXEC);
        if (primaryFd < 0) {
            continue;
        }

        struct drm_mode_card_res res_ = {};
        if (ioctl(primaryFd, DRM_IOCTL_MODE_GETRESOURCES, &res_) < 0 || res_.count_connectors <= 0) {
            continue;
        }

        FF_AUTO_FREE uint32_t* connectors = malloc(res_.count_connectors * sizeof(*connectors));
        struct drm_mode_card_res res = {
            .count_connectors = res_.count_connectors,
            .connector_id_ptr = (uintptr_t) connectors,
        };

        if (ioctl(primaryFd, DRM_IOCTL_MODE_GETRESOURCES, &res) < 0) {
            continue;
        }

        ++nSuccess;

        for (uint32_t iConn = 0; iConn < res.count_connectors; ++iConn) {
            struct drm_mode_get_connector conn = {
                .connector_id = connectors[iConn]
            };
            if (ioctl(primaryFd, DRM_IOCTL_MODE_GETCONNECTOR, &conn) < 0) {
                continue;
            }

            if (conn.connection == 2 /* connector_status_disconnected */) {
                continue;
            }

            uint32_t width = 0, height = 0, refreshRate = 0;
            uint8_t bitDepth = 0;

            if (conn.encoder_id > 0) {
                struct drm_mode_get_encoder enc = {
                    .encoder_id = conn.encoder_id
                };
                if (ioctl(primaryFd, DRM_IOCTL_MODE_GETENCODER, &enc) >= 0 && enc.crtc_id > 0) {
                    struct drm_mode_crtc crtc = {
                        .crtc_id = enc.crtc_id,
                    };
                    if (ioctl(primaryFd, DRM_IOCTL_MODE_GETCRTC, &crtc) >= 0) {
                        width = crtc.mode.hdisplay;
                        height = crtc.mode.vdisplay;
                        refreshRate = crtc.mode.vrefresh;

                        if (crtc.fb_id > 0) {
                            struct drm_mode_fb_cmd fb = {
                                .fb_id = crtc.fb_id,
                            };
                            if (ioctl(primaryFd, DRM_IOCTL_MODE_GETFB, &fb) >= 0) {
                                bitDepth = (uint8_t) (fb.depth / 3);
                            }
                        }
                    }
                }
            }

            uint32_t preferredWidth = 0, preferredHeight = 0, preferredRefreshRate = 0;
            if (conn.count_modes > 0) {
                FF_AUTO_FREE struct drm_mode_modeinfo* modes = malloc(conn.count_modes * sizeof(*modes));
                struct drm_mode_get_connector connModes = {
                    .connector_id = connectors[iConn],
                    .modes_ptr = (uintptr_t) modes,
                    .count_modes = conn.count_modes
                };
                if (ioctl(primaryFd, DRM_IOCTL_MODE_GETCONNECTOR, &connModes) >= 0) {
                    for (uint32_t iMode = 0; iMode < connModes.count_modes; ++iMode) {
                        if (modes[iMode].type & DRM_MODE_TYPE_PREFERRED) {
                            preferredWidth = modes[iMode].hdisplay;
                            preferredHeight = modes[iMode].vdisplay;
                            preferredRefreshRate = modes[iMode].vrefresh;
                            break;
                        }
                    }
                }
            }

            if (width == 0 || height == 0) {
                width = preferredWidth;
                height = preferredHeight;
                refreshRate = preferredRefreshRate;
            }

            FF_STRBUF_AUTO_DESTROY name = ffStrbufCreate();
            uint16_t myear = 0, mweak = 0;
            uint32_t serial = 0;
            FFDisplayHdrStatus hdrStatus = FF_DISPLAY_HDR_STATUS_UNKNOWN;

            uint32_t* props = malloc(conn.count_props * sizeof(uint32_t));
            uint64_t* propValues = malloc(conn.count_props * sizeof(uint64_t));
            if (props && propValues && conn.count_props > 0) {
                struct drm_mode_get_connector connProps = {
                    .connector_id = connectors[iConn],
                    .props_ptr = (uintptr_t) props,
                    .prop_values_ptr = (uintptr_t) propValues,
                    .count_props = conn.count_props,
                };
                if (ioctl(primaryFd, DRM_IOCTL_MODE_GETCONNECTOR, &connProps) >= 0) {
                    for (uint32_t iProp = 0; iProp < connProps.count_props; ++iProp) {
                        struct drm_mode_get_property prop = {
                            .prop_id = props[iProp],
                        };
                        if (ioctl(primaryFd, DRM_IOCTL_MODE_GETPROPERTY, &prop) < 0) {
                            continue;
                        }

                        uint32_t type = prop.flags & (DRM_MODE_PROP_LEGACY_TYPE | DRM_MODE_PROP_EXTENDED_TYPE);
                        if (type == DRM_MODE_PROP_BLOB && ffStrEquals(prop.name, "EDID")) {
                            struct drm_mode_get_blob blob_ = {
                                .blob_id = (uint32_t) propValues[iProp]
                            };
                            if (ioctl(primaryFd, DRM_IOCTL_MODE_GETPROPBLOB, &blob_) >= 0 && blob_.length > 0) {
                                FF_AUTO_FREE void* blobData = malloc(blob_.length);
                                struct drm_mode_get_blob blob = {
                                    .blob_id = blob_.blob_id,
                                    .length = blob_.length,
                                    .data = (uintptr_t) blobData
                                };
                                if (ioctl(primaryFd, DRM_IOCTL_MODE_GETPROPBLOB, &blob) >= 0 && ffEdidIsValid(blobData, blob.length)) {
                                    ffEdidGetName(blobData, &name);
                                    hdrStatus = ffEdidGetHdrCompatible(blobData, blob.length) ? FF_DISPLAY_HDR_STATUS_SUPPORTED : FF_DISPLAY_HDR_STATUS_UNSUPPORTED;
                                    ffEdidGetSerialAndManufactureDate(blobData, &serial, &myear, &mweak);
                                }
                            }
                            break;
                        }
                    }
                }
            }

    #if __linux__
            if (name.length == 0) {
                uint8_t edidData[512];
                ssize_t edidLength = 0;
                drmGetEdidByConnId(conn.connector_id, edidData, &edidLength);
                if (edidLength > 0 && edidLength % 128 == 0) {
                    ffEdidGetName(edidData, &name);
                    hdrStatus = ffEdidGetHdrCompatible(edidData, (uint32_t) edidLength) ? FF_DISPLAY_HDR_STATUS_SUPPORTED : FF_DISPLAY_HDR_STATUS_UNSUPPORTED;
                    ffEdidGetSerialAndManufactureDate(edidData, &serial, &myear, &mweak);
                }
            }
    #endif

            if (name.length == 0) {
                const char* connectorTypeName = drmType2Name(conn.connector_type);
                if (connectorTypeName == NULL) {
                    connectorTypeName = "Unknown";
                }
                ffStrbufSetF(&name, "%s-%d", connectorTypeName, iConn + 1);
            }

            FFDisplayResult* item = ffdsAppendDisplay(result,
                width,
                height,
                refreshRate,
                0,
                preferredWidth,
                preferredHeight,
                preferredRefreshRate,
                0,
                &name,
                conn.connector_type == DRM_MODE_CONNECTOR_eDP || conn.connector_type == DRM_MODE_CONNECTOR_LVDS
                    ? FF_DISPLAY_TYPE_BUILTIN
                    : conn.connector_type == DRM_MODE_CONNECTOR_HDMIA || conn.connector_type == DRM_MODE_CONNECTOR_HDMIB || conn.connector_type == DRM_MODE_CONNECTOR_DisplayPort
                    ? FF_DISPLAY_TYPE_EXTERNAL
                    : FF_DISPLAY_TYPE_UNKNOWN,
                false,
                conn.connector_id,
                conn.mm_width,
                conn.mm_height,
                "libdrm");

            if (item) {
                item->hdrStatus = hdrStatus;
                item->serial = serial;
                item->manufactureYear = myear;
                item->manufactureWeek = mweak;
                item->bitDepth = bitDepth;
            }
        }
    }

    return nSuccess > 0 ? NULL : "No connectors found using libdrm";
}

#endif

const char* ffdsConnectDrm(FF_A_UNUSED FFDisplayServerResult* result) {
#if FF_HAVE_DRM
    if (instance.config.general.dsForceDrm != FF_DS_FORCE_DRM_TYPE_SYSFS_ONLY) {
        if (drmConnectLibdrm(result) == NULL) {
            return NULL;
        }
    }
#endif

#ifdef __linux__
    return drmParseSysfs(result);
#endif

    return "fastfetch was compiled without drm support";
}
