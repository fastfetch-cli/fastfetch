#include "displayserver.h"
#include "detection/os/os.h"
#include "util/windows/unicode.h"

#include <dwmapi.h>
#include <WinUser.h>

typedef struct
{
    WCHAR* deviceName;
    uint32_t width;
    uint32_t height;
} DataBundle;


static CALLBACK WINBOOL enumMonitorProc(HMONITOR hMonitor, FF_MAYBE_UNUSED HDC hDC, FF_MAYBE_UNUSED LPRECT rc, LPARAM lparam)
{
    MONITORINFOEXW mi = { .cbSize = sizeof(mi) };
    DataBundle* data = (DataBundle*) lparam;
    if(GetMonitorInfoW(hMonitor, (MONITORINFO *)&mi) && wcscmp(mi.szDevice, data->deviceName) == 0)
    {
        data->width = (uint32_t) (mi.rcMonitor.right - mi.rcMonitor.left);
        data->height = (uint32_t) (mi.rcMonitor.bottom - mi.rcMonitor.top);
        return FALSE;
    }
    return TRUE;
}

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds, const FFinstance* instance)
{
    FF_UNUSED(instance);

    BOOL enabled;
    if(SUCCEEDED(DwmIsCompositionEnabled(&enabled)) && enabled == TRUE)
    {
        ffStrbufInitS(&ds->wmProcessName, "dwm.exe");
        ffStrbufInitS(&ds->wmPrettyName, "Desktop Window Manager");
    }
    else
    {
        ffStrbufInitS(&ds->wmProcessName, "internal");
        ffStrbufInitS(&ds->wmPrettyName, "internal");
    }
    ffStrbufInit(&ds->wmProtocolName);
    ffStrbufInit(&ds->deProcessName);
    ffStrbufInit(&ds->dePrettyName);
    ffStrbufInit(&ds->deVersion);
    ffListInit(&ds->displays, sizeof(FFDisplayResult));

    DISPLAYCONFIG_PATH_INFO paths[128];
    uint32_t pathCount = sizeof(paths) / sizeof(paths[0]);
    DISPLAYCONFIG_MODE_INFO modes[256];
    uint32_t modeCount = sizeof(modes) / sizeof(modes[0]);

    if (SUCCEEDED(QueryDisplayConfig(
        QDC_ONLY_ACTIVE_PATHS | QDC_VIRTUAL_MODE_AWARE | 0x00000040 /*QDC_VIRTUAL_REFRESH_RATE_AWARE*/,
        &pathCount,
        paths,
        &modeCount,
        modes,
        NULL)))
    {
        for (uint32_t i = 0; i < modeCount; ++i)
        {
            DISPLAYCONFIG_MODE_INFO* mode = &modes[i];
            if(mode->infoType != DISPLAYCONFIG_MODE_INFO_TYPE_TARGET)
                continue;

            DISPLAYCONFIG_SOURCE_DEVICE_NAME sourceName = {
                .header = {
                    .type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME,
                    .size = sizeof(sourceName),
                    .adapterId = mode->adapterId,
                    .id = mode->id,
                },
            };

            DataBundle data = {};
            if(SUCCEEDED(DisplayConfigGetDeviceInfo(&sourceName.header)))
            {
                data.deviceName = sourceName.viewGdiDeviceName;
                EnumDisplayMonitors(NULL, NULL, enumMonitorProc, (LPARAM) &data);
            }

            // Find the target (monitor) friendly name
            DISPLAYCONFIG_TARGET_DEVICE_NAME targetName = {
                .header = {
                    .type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME,
                    .size = sizeof(targetName),
                    .adapterId = mode->adapterId,
                    .id = mode->id,
                },
            };

            FF_STRBUF_AUTO_DESTROY name;
            ffStrbufInit(&name);
            if(SUCCEEDED(DisplayConfigGetDeviceInfo(&targetName.header)) && targetName.flags.friendlyNameFromEdid)
                ffStrbufSetWS(&name, targetName.monitorFriendlyDeviceName);

            ffdsAppendDisplay(ds,
                mode->targetMode.targetVideoSignalInfo.totalSize.cx,
                mode->targetMode.targetVideoSignalInfo.totalSize.cy,
                mode->targetMode.targetVideoSignalInfo.vSyncFreq.Numerator / (double) mode->targetMode.targetVideoSignalInfo.vSyncFreq.Denominator,
                data.width,
                data.height,
                &name);
        }
    }

    //https://github.com/hykilpikonna/hyfetch/blob/master/neofetch#L2067
    const FFOSResult* os = ffDetectOS(instance);
    if(
        ffStrbufEqualS(&os->version, "11") ||
        ffStrbufEqualS(&os->version, "10") ||
        ffStrbufEqualS(&os->version, "2022") ||
        ffStrbufEqualS(&os->version, "2019") ||
        ffStrbufEqualS(&os->version, "2016")
    ) ffStrbufSetS(&ds->dePrettyName, "Fluent");
    else if(
        ffStrbufEqualS(&os->version, "8") ||
        ffStrbufEqualS(&os->version, "81.") ||
        ffStrbufEqualS(&os->version, "2012 R2") ||
        ffStrbufEqualS(&os->version, "2012")
    ) ffStrbufSetS(&ds->dePrettyName, "Metro");
    else
        ffStrbufSetS(&ds->dePrettyName, "Aero");
}
