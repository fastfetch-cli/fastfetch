#include "monitor.h"

#include "common/io/io.h"
#include "util/edidHelper.h"
#include "util/mallocHelper.h"
#include "util/stringUtils.h"
#include "util/windows/registry.h"
#include "util/windows/unicode.h"

#include <windows.h>
#include <setupapi.h>
#include <devguid.h>

static inline void wrapSetupDiDestroyDeviceInfoList(HDEVINFO* hdev)
{
    if(*hdev)
        SetupDiDestroyDeviceInfoList(*hdev);
}

const char* ffDetectMonitor(FFlist* results)
{
    //https://learn.microsoft.com/en-us/windows/win32/power/enumerating-battery-devices
    HDEVINFO hdev __attribute__((__cleanup__(wrapSetupDiDestroyDeviceInfoList))) =
        SetupDiGetClassDevsW(&GUID_DEVCLASS_MONITOR, 0, 0, DIGCF_PRESENT);
    if(hdev == INVALID_HANDLE_VALUE)
        return "SetupDiGetClassDevsW(&GUID_DEVCLASS_MONITOR) failed";

    SP_DEVINFO_DATA did = { .cbSize = sizeof(did) };
    for (DWORD idev = 0; SetupDiEnumDeviceInfo(hdev, idev, &did); ++idev)
    {
        FF_HKEY_AUTO_DESTROY hKey = SetupDiOpenDevRegKey(hdev, &did, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_QUERY_VALUE);
        if (!hKey) continue;

        FF_AUTO_FREE uint8_t* edidData = NULL;
        uint32_t edidLength = 0;
        if (!ffRegReadData(hKey, L"EDID", &edidData, &edidLength, NULL)) continue;
        if (edidLength == 0 || edidLength % 128 != 0)
            continue;

        uint32_t width, height;
        ffEdidGetPhysicalResolution(edidData, &width, &height);
        if (width == 0 || height == 0) continue;

        FFMonitorResult* display = (FFMonitorResult*) ffListAdd(results);
        display->width = width;
        display->height = height;
        ffEdidGetSerialAndManufactureDate(edidData, &display->serial, &display->manufactureYear, &display->manufactureWeek);
        display->hdrCompatible = ffEdidGetHdrCompatible(edidData, edidLength); // Doesn't work. edidLength is always 128
        ffStrbufInit(&display->name);
        ffEdidGetName(edidData, &display->name);
        ffEdidGetPhysicalSize(edidData, &display->physicalWidth, &display->physicalHeight);
        display->refreshRate = 0;
    }
    return NULL;
}
