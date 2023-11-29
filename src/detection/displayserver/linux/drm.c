#include "displayserver_linux.h"
#include "common/io/io.h"
#include "util/edidHelper.h"
#include "util/stringUtils.h"

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
        if(ffStrEquals(entry->d_name, ".") || ffStrEquals(entry->d_name, ".."))
            continue;

        ffStrbufAppendS(&drmDir, entry->d_name);
        uint32_t drmDirWithDnameLength = drmDir.length;

        ffStrbufAppendS(&drmDir, "/enabled");
        char enabled = 'd'; // disabled
        ffReadFileData(drmDir.chars, sizeof(enabled), &enabled);
        if (enabled != 'e') // enabled
        {
            ffStrbufSubstrBefore(&drmDir, drmDirLength);
            continue;
        }

        ffStrbufSubstrBefore(&drmDir, drmDirWithDnameLength);
        ffStrbufAppendS(&drmDir, "/modes");

        char modes[32];
        if (ffReadFileData(drmDir.chars, sizeof(modes), modes) < 3)
        {
            ffStrbufSubstrBefore(&drmDir, drmDirLength);
            continue;
        }

        uint32_t width = 0, height = 0;

        int scanned = sscanf(modes, "%ux%u", &width, &height);
        if(scanned == 2 && width > 0 && height > 0)
        {
            ffStrbufSubstrBefore(&drmDir, drmDirLength);
            ffStrbufAppendS(&drmDir, entry->d_name);
            ffStrbufAppendS(&drmDir, "/edid");

            FF_STRBUF_AUTO_DESTROY name = ffStrbufCreate();
            uint8_t edidData[128];
            if(ffReadFileData(drmDir.chars, sizeof(edidData), edidData) == sizeof(edidData))
                ffEdidGetName(edidData, &name);
            else
            {
                const char* plainName = entry->d_name;
                if (ffStrStartsWith(plainName, "card"))
                {
                    const char* tmp = strchr(plainName + strlen("card"), '-');
                    if (tmp) plainName = tmp + 1;
                }
                ffStrbufAppendS(&name, plainName);
            }

            ffdsAppendDisplay(
                result,
                width, height,
                0,
                0, 0,
                0,
                &name,
                FF_DISPLAY_TYPE_UNKNOWN,
                false,
                0
            );
        }

        ffStrbufSubstrBefore(&drmDir, drmDirLength);
    }

    return NULL;
}

#ifdef FF_HAVE_DRM

#include "common/library.h"

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <fcntl.h>

static const char* drmConnectLibdrm(FFDisplayServerResult* result)
{
    FF_LIBRARY_LOAD(libdrm, &instance.config.library.libdrm, "dlopen(libdrm)" FF_LIBRARY_EXTENSION " failed", "libdrm" FF_LIBRARY_EXTENSION, 2)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmGetDevices)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmModeGetResources)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmModeGetConnectorCurrent)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmModeGetCrtc)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmModeGetEncoder)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmModeGetProperty)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmModeGetPropertyBlob)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmModeFreeCrtc)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmModeFreeEncoder)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmModeFreeConnector)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmModeFreeProperty)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmModeFreePropertyBlob)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmModeGetConnectorTypeName)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmFreeDevices)

    drmDevice* devices[64];
    int nDevices = ffdrmGetDevices(devices, sizeof(devices) / sizeof(devices[0]));
    if (nDevices < 0)
        return "drmGetDevices() failed";

    for (int iDev = 0; iDev < nDevices; ++iDev)
    {
        drmDevice* dev = devices[iDev];

        if (!(dev->available_nodes & (1 << DRM_NODE_PRIMARY)))
            continue;

        const char* path = dev->nodes[DRM_NODE_PRIMARY];
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

            if (conn->connection != DRM_MODE_CONNECTED)
                goto next_conn;

            drmModeEncoder* encoder = ffdrmModeGetEncoder(fd, conn->encoder_id);
            if (!encoder)
                goto next_conn;

            drmModeCrtc* crtc = ffdrmModeGetCrtc(fd, encoder->crtc_id);
            if (!crtc)
                goto next_encoder;

            {
                FF_STRBUF_AUTO_DESTROY name = ffStrbufCreate();

                for (int iProp = 0; iProp < conn->count_props; ++iProp)
                {
                    drmModePropertyRes *prop = ffdrmModeGetProperty(fd, conn->props[iProp]);
                    if (!prop)
                        continue;

                    uint32_t type = prop->flags & (DRM_MODE_PROP_LEGACY_TYPE | DRM_MODE_PROP_EXTENDED_TYPE);
                    if (type == DRM_MODE_PROP_BLOB && ffStrEquals(prop->name, "EDID"))
                    {
                        // Stangely, prop->count_blobs is 0 and prop->blob_ids is NULL
                        drmModePropertyBlobPtr blob = ffdrmModeGetPropertyBlob(fd, (uint32_t) conn->prop_values[iProp]);
                        if (blob->length >= 128)
                            ffEdidGetName(blob->data, &name);
                        ffdrmModeFreePropertyBlob(blob);
                        break;
                    }
                    ffdrmModeFreeProperty(prop);
                }

                if (name.length == 0)
                {
                    const char* connectorTypeName = ffdrmModeGetConnectorTypeName(conn->connector_type);
                    if (connectorTypeName == NULL)
                        connectorTypeName = "Unknown";
                    ffStrbufSetF(&name, "%s-%d", connectorTypeName, iConn);
                }

                uint32_t refreshRate = crtc->mode.vrefresh;
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

                ffdsAppendDisplay(
                    result,
                    crtc->width, crtc->height,
                    refreshRate,
                    0, 0,
                    0,
                    &name,
                    FF_DISPLAY_TYPE_UNKNOWN,
                    false,
                    0
                );
            }

            ffdrmModeFreeCrtc(crtc);

    next_encoder:
            ffdrmModeFreeEncoder(encoder);

    next_conn:
            ffdrmModeFreeConnector(conn);
        }
    }

    ffdrmFreeDevices(devices, nDevices);

    return NULL;
}

#endif

void ffdsConnectDrm(FFDisplayServerResult* result)
{
    #ifdef FF_HAVE_DRM
    if (drmConnectLibdrm(result) == NULL)
        return;
    #endif

    drmParseSysfs(result);
}
