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
    DataBundle* data = (DataBundle*) lparam;
    MONITORINFOEXW mi = { .cbSize = sizeof(mi) };
    if(GetMonitorInfoW(hMonitor, (MONITORINFO *)&mi) && wcscmp(mi.szDevice, data->deviceName) == 0)
    {
        data->width = (uint32_t) (mi.rcMonitor.right - mi.rcMonitor.left);
        data->height = (uint32_t) (mi.rcMonitor.bottom - mi.rcMonitor.top);
        return FALSE;
    }
    return TRUE;
}

static void detectDisplays(FFDisplayServerResult* ds, bool detectName)
{
    DISPLAYCONFIG_PATH_INFO paths[128];
    uint32_t pathCount = sizeof(paths) / sizeof(paths[0]);
    DISPLAYCONFIG_MODE_INFO modes[256];
    uint32_t modeCount = sizeof(modes) / sizeof(modes[0]);

    if (QueryDisplayConfig(
        QDC_ONLY_ACTIVE_PATHS,
        &pathCount,
        paths,
        &modeCount,
        modes,
        NULL) == ERROR_SUCCESS)
    {
        for (uint32_t i = 0; i < pathCount; ++i)
        {
            DISPLAYCONFIG_PATH_INFO* path = &paths[i];

            DataBundle data = {};

            DISPLAYCONFIG_SOURCE_DEVICE_NAME sourceName = {
                .header = {
                    .type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME,
                    .size = sizeof(sourceName),
                    .adapterId = path->sourceInfo.adapterId,
                    .id = path->sourceInfo.id,
                },
            };
            if (DisplayConfigGetDeviceInfo(&sourceName.header) == ERROR_SUCCESS)
            {
                data.deviceName = sourceName.viewGdiDeviceName;
                EnumDisplayMonitors(NULL, NULL, enumMonitorProc, (LPARAM) &data);
            }

            FF_STRBUF_AUTO_DESTROY name;
            ffStrbufInit(&name);

            if (detectName)
            {
                DISPLAYCONFIG_TARGET_DEVICE_NAME targetName = {
                    .header = {
                        .type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME,
                        .size = sizeof(targetName),
                        .adapterId = path->targetInfo.adapterId,
                        .id = path->targetInfo.id,
                    },
                };
                if(DisplayConfigGetDeviceInfo(&targetName.header) == ERROR_SUCCESS)
                {
                    if (targetName.flags.friendlyNameFromEdid)
                        ffStrbufSetWS(&name, targetName.monitorFriendlyDeviceName);
                    else
                    {
                        ffStrbufSetWS(&name, targetName.monitorDevicePath);
                        ffStrbufSubstrAfterFirstC(&name, '#');
                        ffStrbufSubstrBeforeFirstC(&name, '#');
                    }
                }
            }

            ffdsAppendDisplay(ds,
                modes[path->sourceInfo.modeInfoIdx].sourceMode.width,
                modes[path->sourceInfo.modeInfoIdx].sourceMode.height,
                path->targetInfo.refreshRate.Numerator / (double) path->targetInfo.refreshRate.Denominator,
                data.width,
                data.height,
                &name,
                path->targetInfo.outputTechnology == DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INTERNAL ||
                path->targetInfo.outputTechnology == DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DISPLAYPORT_EMBEDDED ||
                    path->targetInfo.outputTechnology == DISPLAYCONFIG_OUTPUT_TECHNOLOGY_UDI_EMBEDDED
                    ? FF_DISPLAY_TYPE_BUILTIN : FF_DISPLAY_TYPE_EXTERNAL
            );
        }
    }
}

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds, const FFinstance* instance)
{
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

    detectDisplays(ds, instance->config.displayDetectName);

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
