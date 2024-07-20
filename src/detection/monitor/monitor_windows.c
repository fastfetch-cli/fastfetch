#include "monitor.h"

#include "util/edidHelper.h"
#include "util/windows/registry.h"

#include <windows.h>

const char* ffDetectMonitor(FFlist* results)
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
        NULL) != ERROR_SUCCESS)
        return "QueryDisplayConfig() failed";

    if (pathCount == 0)
        return "QueryDisplayConfig() returns 0 paths";

    for (uint32_t i = 0; i < pathCount; ++i)
    {
        DISPLAYCONFIG_PATH_INFO* path = &paths[i];

        DISPLAYCONFIG_TARGET_DEVICE_NAME targetName = {
            .header = {
                .type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME,
                .size = sizeof(targetName),
                .adapterId = path->targetInfo.adapterId,
                .id = path->targetInfo.id,
            },
        };
        if (DisplayConfigGetDeviceInfo(&targetName.header) != ERROR_SUCCESS)
            continue;

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
            assert(pRegPath < regPath + sizeof(regPath) / sizeof(wchar_t) + strlen("Device Parameters"));
        }
        wcscpy(pRegPath, L"Device Parameters");

        uint8_t edidData[1024];
        DWORD edidLength = sizeof(edidData);

        if (RegGetValueW(HKEY_LOCAL_MACHINE, regPath, L"EDID", RRF_RT_REG_BINARY, NULL, edidData, &edidLength) == ERROR_SUCCESS &&
            edidLength > 0 && edidLength % 128 == 0)
        {
            uint32_t width, height;
            ffEdidGetPhysicalResolution(edidData, &width, &height);
            if (width == 0 || height == 0) continue;

            FFMonitorResult* display = (FFMonitorResult*) ffListAdd(results);
            display->width = width;
            display->height = height;
            ffEdidGetSerialAndManufactureDate(edidData, &display->serial, &display->manufactureYear, &display->manufactureWeek);
            ffStrbufInit(&display->name);
            ffEdidGetName(edidData, &display->name);
            ffEdidGetPhysicalSize(edidData, &display->physicalWidth, &display->physicalHeight);
            display->refreshRate = 0;
            display->hdrCompatible = false;

            DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO advColorInfo = {
                .header = {
                    .type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO,
                    .size = sizeof(advColorInfo),
                    .adapterId = path->targetInfo.adapterId,
                    .id = path->targetInfo.id,
                }
            };
            if (DisplayConfigGetDeviceInfo(&advColorInfo.header) == ERROR_SUCCESS)
                display->hdrCompatible = !!advColorInfo.advancedColorSupported;
            else
                display->hdrCompatible = ffEdidGetHdrCompatible(edidData, (uint32_t) edidLength);

            DISPLAYCONFIG_TARGET_PREFERRED_MODE preferredMode = {
                .header = {
                    .type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_PREFERRED_MODE,
                    .size = sizeof(preferredMode),
                    .adapterId = path->targetInfo.adapterId,
                    .id = path->targetInfo.id,
                }
            };
            if (DisplayConfigGetDeviceInfo(&preferredMode.header) == ERROR_SUCCESS)
            {
                if (preferredMode.width == width && preferredMode.height == height)
                {
                    DISPLAYCONFIG_RATIONAL freq = preferredMode.targetMode.targetVideoSignalInfo.vSyncFreq;
                    display->refreshRate = freq.Numerator / (double) freq.Denominator;
                }
            }

            DISPLAYCONFIG_VIDEO_SIGNAL_INFO current = modes[path->targetInfo.modeInfoIdx].targetMode.targetVideoSignalInfo;
            if (current.activeSize.cx == width && current.activeSize.cy == height)
            {
                double refreshRate = current.vSyncFreq.Numerator / (double) current.vSyncFreq.Denominator;
                if (refreshRate > display->refreshRate) display->refreshRate = refreshRate;
            }
        }
    }
    return NULL;
}
