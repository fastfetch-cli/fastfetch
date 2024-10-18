#include "displayserver.h"
#include "detection/os/os.h"
#include "util/windows/unicode.h"
#include "util/windows/registry.h"
#include "util/mallocHelper.h"
#include "util/edidHelper.h"

#include <dwmapi.h>
#include <WinUser.h>
#include <wchar.h>

typedef struct FFMonitorInfo
{
    HMONITOR handle;
    MONITORINFOEXW info;
} FFMonitorInfo;

static CALLBACK BOOL MonitorEnumProc(
    HMONITOR hMonitor,
    FF_MAYBE_UNUSED HDC hdc,
    FF_MAYBE_UNUSED LPRECT lpRect,
    LPARAM lParam
)
{
    FFlist* monitors = (FFlist*) lParam;
    FFMonitorInfo* newMonitor = ffListAdd(monitors);
    newMonitor->handle = hMonitor;
    newMonitor->info.cbSize = sizeof(newMonitor->info);

    return GetMonitorInfoW(hMonitor, (MONITORINFO*) &newMonitor->info);
}

static void detectDisplays(FFDisplayServerResult* ds)
{
    FF_LIST_AUTO_DESTROY monitors = ffListCreate(sizeof(FFMonitorInfo));
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM) &monitors);

    DISPLAYCONFIG_PATH_INFO paths[128];
    uint32_t pathCount = ARRAY_SIZE(paths);
    DISPLAYCONFIG_MODE_INFO modes[256];
    uint32_t modeCount = ARRAY_SIZE(modes);

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

            DISPLAYCONFIG_SOURCE_DEVICE_NAME sourceName = {
                .header = {
                    .type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME,
                    .size = sizeof(sourceName),
                    .adapterId = path->sourceInfo.adapterId,
                    .id = path->sourceInfo.id,
                },
            };

            FFMonitorInfo* monitorInfo = NULL;
            if (DisplayConfigGetDeviceInfo(&sourceName.header) == ERROR_SUCCESS)
            {
                FF_LIST_FOR_EACH(FFMonitorInfo, item, monitors)
                {
                    if (wcsncmp(item->info.szDevice, sourceName.viewGdiDeviceName, ARRAY_SIZE(sourceName.viewGdiDeviceName)) == 0)
                    {
                        monitorInfo = item;
                        break;
                    }
                }
            }
            if (!monitorInfo) continue;

            FF_STRBUF_AUTO_DESTROY name = ffStrbufCreate();
            uint32_t physicalWidth = 0, physicalHeight = 0;

            DISPLAYCONFIG_TARGET_DEVICE_NAME targetName = {
                .header = {
                    .type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME,
                    .size = sizeof(targetName),
                    .adapterId = path->targetInfo.adapterId,
                    .id = path->targetInfo.id,
                },
            };
            uint8_t edidData[1024];
            DWORD edidLength = 0;

            if(DisplayConfigGetDeviceInfo(&targetName.header) == ERROR_SUCCESS)
            {
                wchar_t regPath[256] = L"SYSTEM\\CurrentControlSet\\Enum";
                wchar_t* pRegPath = regPath + strlen("SYSTEM\\CurrentControlSet\\Enum");
                wchar_t* pDevPath = targetName.monitorDevicePath + strlen("\\\\?");
                while (*pDevPath && *pDevPath != L'{')
                {
                    if (*pDevPath == L'#')
                        *pRegPath = L'\\';
                    else
                        *pRegPath = *pDevPath;
                    ++pRegPath;
                    ++pDevPath;
                    assert(pRegPath < regPath + ARRAY_SIZE(regPath) + strlen("Device Parameters"));
                }
                wcscpy(pRegPath, L"Device Parameters");

                edidLength = ARRAY_SIZE(edidData);
                if (RegGetValueW(HKEY_LOCAL_MACHINE, regPath, L"EDID", RRF_RT_REG_BINARY, NULL, edidData, &edidLength) == ERROR_SUCCESS &&
                    edidLength > 0 && edidLength % 128 == 0)
                {
                    ffEdidGetName(edidData, &name);
                    ffEdidGetPhysicalSize(edidData, &physicalWidth, &physicalHeight);
                }
                else
                {
                    edidLength = 0;
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

            uint32_t width = modes[path->sourceInfo.modeInfoIdx].sourceMode.width;
            uint32_t height = modes[path->sourceInfo.modeInfoIdx].sourceMode.height;
            if (path->targetInfo.rotation == DISPLAYCONFIG_ROTATION_ROTATE90 ||
                path->targetInfo.rotation == DISPLAYCONFIG_ROTATION_ROTATE270)
            {
                uint32_t temp = width;
                width = height;
                height = temp;
                temp = physicalWidth;
                physicalWidth = physicalHeight;
                physicalHeight = temp;
            }

            uint32_t rotation;
            switch (path->targetInfo.rotation)
            {
                case DISPLAYCONFIG_ROTATION_ROTATE90: rotation = 90; break;
                case DISPLAYCONFIG_ROTATION_ROTATE180: rotation = 180; break;
                case DISPLAYCONFIG_ROTATION_ROTATE270: rotation = 270; break;
                default: rotation = 0; break;
            }

            FFDisplayResult* display = ffdsAppendDisplay(ds,
                width,
                height,
                path->targetInfo.refreshRate.Numerator / (double) path->targetInfo.refreshRate.Denominator,
                (uint32_t) (monitorInfo->info.rcMonitor.right - monitorInfo->info.rcMonitor.left),
                (uint32_t) (monitorInfo->info.rcMonitor.bottom - monitorInfo->info.rcMonitor.top),
                rotation,
                &name,
                path->targetInfo.outputTechnology == DISPLAYCONFIG_OUTPUT_TECHNOLOGY_OTHER ? FF_DISPLAY_TYPE_UNKNOWN :
                    path->targetInfo.outputTechnology == DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INTERNAL ||
                    path->targetInfo.outputTechnology == DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DISPLAYPORT_EMBEDDED ||
                    path->targetInfo.outputTechnology == DISPLAYCONFIG_OUTPUT_TECHNOLOGY_UDI_EMBEDDED
                    ? FF_DISPLAY_TYPE_BUILTIN : FF_DISPLAY_TYPE_EXTERNAL,
                !!(monitorInfo->info.dwFlags & MONITORINFOF_PRIMARY),
                (uint64_t)(uintptr_t) monitorInfo->handle,
                physicalWidth,
                physicalHeight,
                "GDI"
            );

            if (display)
            {
                DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO advColorInfo = {
                    .header = {
                        .type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO,
                        .size = sizeof(advColorInfo),
                        .adapterId = path->targetInfo.adapterId,
                        .id = path->targetInfo.id,
                    }
                };
                if (DisplayConfigGetDeviceInfo(&advColorInfo.header) == ERROR_SUCCESS)
                {
                    if (advColorInfo.advancedColorEnabled)
                        display->hdrStatus = FF_DISPLAY_HDR_STATUS_ENABLED;
                    else if (advColorInfo.advancedColorSupported)
                        display->hdrStatus = FF_DISPLAY_HDR_STATUS_SUPPORTED;
                    else
                        display->hdrStatus = FF_DISPLAY_HDR_STATUS_UNSUPPORTED;
                    display->bitDepth = (uint8_t) advColorInfo.bitsPerColorChannel;
                }
                else
                    display->hdrStatus = FF_DISPLAY_HDR_STATUS_UNKNOWN;
                if (edidLength > 0)
                    ffEdidGetSerialAndManufactureDate(edidData, &display->serial, &display->manufactureYear, &display->manufactureWeek);
            }
        }
    }
}

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds)
{
    BOOL enabled;
    if(SUCCEEDED(DwmIsCompositionEnabled(&enabled)) && enabled)
    {
        ffStrbufSetStatic(&ds->wmProcessName, "dwm.exe");
        ffStrbufSetStatic(&ds->wmPrettyName, "Desktop Window Manager");
    }
    else
    {
        // `explorer.exe` only provides a subset of WM functions, as well as the taskbar and desktop icons.
        // While a window itself is drawn by kernel (GDI). Killing `explorer.exe` won't affect how windows are displayed generally.
        ffStrbufSetStatic(&ds->wmProcessName, "explorer.exe");
        ffStrbufSetStatic(&ds->wmPrettyName, "Internal");
    }

    detectDisplays(ds);

    //https://github.com/hykilpikonna/hyfetch/blob/master/neofetch#L2067
    const FFOSResult* os = ffDetectOS();
    if(
        ffStrbufEqualS(&os->version, "11") ||
        ffStrbufEqualS(&os->version, "10") ||
        ffStrbufEqualS(&os->version, "2022") ||
        ffStrbufEqualS(&os->version, "2019") ||
        ffStrbufEqualS(&os->version, "2016")
    ) ffStrbufSetStatic(&ds->dePrettyName, "Fluent");
    else if(
        ffStrbufEqualS(&os->version, "8") ||
        ffStrbufEqualS(&os->version, "8.1") ||
        ffStrbufEqualS(&os->version, "2012 R2") ||
        ffStrbufEqualS(&os->version, "2012")
    ) ffStrbufSetStatic(&ds->dePrettyName, "Metro");
    else
        ffStrbufSetStatic(&ds->dePrettyName, "Aero");
}
