#include "displayserver_linux.h"
#include "common/io/io.h"
#include "util/edidHelper.h"
#include "util/stringUtils.h"

#ifdef __linux__
#include <dirent.h>

static const char* drmParseSysfs(FFDisplayServerResult* result)
{
    const char* drmDirPath = "/sys/class/drm/";

    FF_AUTO_CLOSE_DIR DIR* dirp = opendir(drmDirPath);
    if(dirp == NULL)
        return "opendir(drmDirPath) failed";

    FF_STRBUF_AUTO_DESTROY drmDir = ffStrbufCreateA(64);
    ffStrbufAppendS(&drmDir, drmDirPath);

    uint32_t drmDirLength = drmDir.length;

    struct dirent* entry;
    while((entry = readdir(dirp)) != NULL)
    {
        if(entry->d_name[0] == '.')
            continue;

        ffStrbufAppendS(&drmDir, entry->d_name);
        uint32_t drmDirWithDnameLength = drmDir.length;

        char buf;
        ffStrbufAppendS(&drmDir, "/enabled");
        if (!ffReadFileData(drmDir.chars, sizeof(buf), &buf) || buf != 'e') {
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
        if (ffStrStartsWith(plainName, "card"))
        {
            const char* tmp = strchr(plainName + strlen("card"), '-');
            if (tmp) plainName = tmp + 1;
        }

        uint8_t edidData[512];
        ssize_t edidLength = ffReadFileData(drmDir.chars, ARRAY_SIZE(edidData), edidData);
        if(edidLength <= 0 || edidLength % 128 != 0)
        {
            edidLength = 0;
            ffStrbufSubstrBefore(&drmDir, drmDirWithDnameLength);
            ffStrbufAppendS(&drmDir, "/modes");

            char modes[32];
            if (ffReadFileData(drmDir.chars, ARRAY_SIZE(modes), modes) >= 3)
            {
                sscanf(modes, "%ux%u", &width, &height);
                ffStrbufAppendS(&name, plainName);
            }
        }
        else
        {
            ffEdidGetName(edidData, &name);
            ffEdidGetPreferredResolutionAndRefreshRate(edidData, &width, &height, &refreshRate);
            ffEdidGetPhysicalSize(edidData, &physicalWidth, &physicalHeight);
        }

        FFDisplayResult* item = ffdsAppendDisplay(
            result,
            width, height,
            refreshRate,
            0, 0,
            0,
            &name,
            ffdsGetDisplayType(plainName),
            false,
            0,
            physicalWidth,
            physicalHeight,
            "sysfs-drm"
        );
        if (item && edidLength)
        {
            item->hdrStatus = ffEdidGetHdrCompatible(edidData, (uint32_t) edidLength) ? FF_DISPLAY_HDR_STATUS_SUPPORTED : FF_DISPLAY_HDR_STATUS_UNSUPPORTED;
            ffEdidGetSerialAndManufactureDate(edidData, &item->serial, &item->manufactureYear, &item->manufactureWeek);
        }

        ffStrbufSubstrBefore(&drmDir, drmDirLength);
    }

    return NULL;
}
#endif

#ifdef FF_HAVE_DRM

#include "common/library.h"

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <fcntl.h>

// https://gitlab.freedesktop.org/mesa/drm/-/blob/main/xf86drmMode.c#L1785
// It's not supported on Ubuntu 20.04
static inline const char* drmType2Name(uint32_t connector_type)
{
    /* Keep the strings in sync with the kernel's drm_connector_enum_list in
     * drm_connector.c. */
    switch (connector_type)
    {
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

FF_MAYBE_UNUSED static const char* drmGetEdidByConnId(uint32_t connId, uint8_t* edidData, ssize_t* edidLength)
{
    const char* drmDirPath = "/sys/class/drm/";

    FF_AUTO_CLOSE_DIR DIR* dirp = opendir(drmDirPath);
    if(dirp == NULL)
        return "opendir(drmDirPath) failed";

    FF_STRBUF_AUTO_DESTROY drmDir = ffStrbufCreateA(64);
    ffStrbufAppendS(&drmDir, drmDirPath);

    uint32_t drmDirLength = drmDir.length;

    struct dirent* entry;
    while((entry = readdir(dirp)) != NULL)
    {
        if(entry->d_name[0] == '.')
            continue;

        ffStrbufAppendS(&drmDir, entry->d_name);
        uint32_t drmDirWithDnameLength = drmDir.length;

        char connectorId[16] = {};

        ffStrbufAppendS(&drmDir, "/connector_id");
        ffReadFileData(drmDir.chars, ARRAY_SIZE(connectorId), connectorId);
        if (strtoul(connectorId, NULL, 10) != connId)
        {
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

static const char* drmConnectLibdrm(FFDisplayServerResult* result)
{
    FF_LIBRARY_LOAD(libdrm, "dlopen libdrm" FF_LIBRARY_EXTENSION " failed", "libdrm" FF_LIBRARY_EXTENSION, 2)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmGetDevices)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmModeGetResources)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmModeGetConnectorCurrent)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmModeGetCrtc)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmModeGetEncoder)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmModeGetProperty)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmModeGetPropertyBlob)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmModeFreeResources)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmModeFreeCrtc)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmModeFreeEncoder)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmModeFreeConnector)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmModeFreeProperty)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmModeFreePropertyBlob)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmFreeDevices)

    drmDevice* devices[64];
    int nDevices = ffdrmGetDevices(devices, ARRAY_SIZE(devices));
    if (nDevices <= 0)
        return "drmGetDevices() failed";

    FF_STRBUF_AUTO_DESTROY name = ffStrbufCreate();

    for (int iDev = 0; iDev < nDevices; ++iDev)
    {
        drmDevice* dev = devices[iDev];

        if (!(dev->available_nodes & (1 << DRM_NODE_PRIMARY)))
            continue;

        const char* path = dev->nodes[DRM_NODE_PRIMARY];

        #if __linux__
        ffStrbufSetF(&name, "/sys/class/drm/%s/device/power/runtime_status", strrchr(path, '/') + 1);

        char buffer[8] = "";
        if (ffReadFileData(name.chars, strlen("suspend"), buffer) > 0 && ffStrStartsWith(buffer, "suspend"))
            continue;
        #endif

        FF_AUTO_CLOSE_FD int fd = open(path, O_RDONLY | O_CLOEXEC);
        if (fd < 0)
            continue;

        drmModeRes* res = ffdrmModeGetResources(fd);
        if (!res)
            continue;

        for (int iConn = 0; iConn < res->count_connectors; ++iConn)
        {
            drmModeConnector* conn = ffdrmModeGetConnectorCurrent(fd, res->connectors[iConn]);
            if (!conn)
                continue;

            if (conn->connection != DRM_MODE_DISCONNECTED)
            {
                drmModeEncoder* encoder = ffdrmModeGetEncoder(fd, conn->encoder_id);
                uint32_t width = 0, height = 0, refreshRate = 0;

                if (encoder)
                {
                    drmModeCrtc* crtc = ffdrmModeGetCrtc(fd, encoder->crtc_id);
                    if (crtc)
                    {
                        width = crtc->mode.hdisplay;
                        height = crtc->mode.vdisplay;
                        refreshRate = crtc->mode.vrefresh;
                        if (refreshRate == 0)
                        {
                            // There are weird cases that we can't get the refresh rate from the CRTC but from the modes
                            for (int iMode = 0; iMode < conn->count_modes; ++iMode)
                            {
                                drmModeModeInfo* mode = &conn->modes[iMode];
                                if (mode->clock == crtc->mode.clock && mode->htotal == crtc->mode.htotal)
                                {
                                    refreshRate = mode->vrefresh;
                                    break;
                                }
                            }
                        }
                        ffdrmModeFreeCrtc(crtc);
                    }

                    ffdrmModeFreeEncoder(encoder);
                }

                if (width == 0 || height == 0)
                {
                    // NVIDIA DRM driver seems incomplete and conn->encoder_id == 0
                    // Assume preferred resolution is used as what we do in drmParseSys
                    for (int iMode = 0; iMode < conn->count_modes; ++iMode)
                    {
                        drmModeModeInfo* mode = &conn->modes[iMode];

                        if (mode->type & DRM_MODE_TYPE_PREFERRED)
                        {
                            width = mode->hdisplay;
                            height = mode->vdisplay;
                            refreshRate = mode->vrefresh;
                            break;
                        }
                    }
                }


                ffStrbufClear(&name);
                uint16_t myear = 0, mweak = 0;
                uint32_t serial = 0;
                FFDisplayHdrStatus hdrStatus = FF_DISPLAY_HDR_STATUS_UNKNOWN;

                for (int iProp = 0; iProp < conn->count_props; ++iProp)
                {
                    drmModePropertyRes *prop = ffdrmModeGetProperty(fd, conn->props[iProp]);
                    if (!prop)
                        continue;

                    uint32_t type = prop->flags & (DRM_MODE_PROP_LEGACY_TYPE | DRM_MODE_PROP_EXTENDED_TYPE);
                    if (type == DRM_MODE_PROP_BLOB && ffStrEquals(prop->name, "EDID"))
                    {
                        drmModePropertyBlobPtr blob = NULL;

                        if (prop->count_blobs > 0 && prop->blob_ids != NULL)
                            blob = ffdrmModeGetPropertyBlob(fd, prop->blob_ids[0]);
                        else
                            blob = ffdrmModeGetPropertyBlob(fd, (uint32_t) conn->prop_values[iProp]);

                        if (blob)
                        {
                            if (blob->length >= 128)
                            {
                                ffEdidGetName(blob->data, &name);
                                hdrStatus = ffEdidGetHdrCompatible(blob->data, blob->length) ? FF_DISPLAY_HDR_STATUS_SUPPORTED : FF_DISPLAY_HDR_STATUS_UNSUPPORTED;
                                ffEdidGetSerialAndManufactureDate(blob->data, &serial, &myear, &mweak);
                            }
                            ffdrmModeFreePropertyBlob(blob);
                        }
                        break;
                    }
                    ffdrmModeFreeProperty(prop);
                }

                #if __linux__
                if (name.length == 0)
                {
                    uint8_t edidData[512];
                    ssize_t edidLength = 0;
                    drmGetEdidByConnId(conn->connector_id, edidData, &edidLength);
                    if (edidLength > 0 && edidLength % 128 == 0)
                    {
                        ffEdidGetName(edidData, &name);
                        hdrStatus = ffEdidGetHdrCompatible(edidData, (uint32_t) edidLength) ? FF_DISPLAY_HDR_STATUS_SUPPORTED : FF_DISPLAY_HDR_STATUS_UNSUPPORTED;
                        ffEdidGetSerialAndManufactureDate(edidData, &serial, &myear, &mweak);
                    }
                }
                #endif

                if (name.length == 0)
                {
                    const char* connectorTypeName = drmType2Name(conn->connector_type);
                    if (connectorTypeName == NULL)
                        connectorTypeName = "Unknown";
                    ffStrbufSetF(&name, "%s-%d", connectorTypeName, iConn + 1);
                }

                FFDisplayResult* item = ffdsAppendDisplay(result,
                    width,
                    height,
                    refreshRate,
                    0,
                    0,
                    0,
                    &name,
                    conn->connector_type == DRM_MODE_CONNECTOR_eDP || conn->connector_type == DRM_MODE_CONNECTOR_LVDS
                        ? FF_DISPLAY_TYPE_BUILTIN
                        : conn->connector_type == DRM_MODE_CONNECTOR_HDMIA || conn->connector_type == DRM_MODE_CONNECTOR_HDMIB || conn->connector_type == DRM_MODE_CONNECTOR_DisplayPort
                            ? FF_DISPLAY_TYPE_EXTERNAL : FF_DISPLAY_TYPE_UNKNOWN,
                    false,
                    conn->connector_id,
                    conn->mmWidth,
                    conn->mmHeight,
                    "libdrm"
                );

                if (item)
                {
                    item->hdrStatus = hdrStatus;
                    item->serial = serial;
                    item->manufactureYear = myear;
                    item->manufactureWeek = mweak;
                }
            }

            ffdrmModeFreeConnector(conn);
        }

        ffdrmModeFreeResources(res);
    }

    ffdrmFreeDevices(devices, nDevices);

    return NULL;
}

#endif

const char* ffdsConnectDrm(FFDisplayServerResult* result)
{
    #ifdef FF_HAVE_DRM
    if (instance.config.general.dsForceDrm != FF_DS_FORCE_DRM_TYPE_SYSFS_ONLY)
    {
        if (drmConnectLibdrm(result) == NULL)
            return NULL;
    }
    #endif

    #ifdef __linux__
    return drmParseSysfs(result);
    #endif

    return "fastfetch was compiled without drm support";
}
