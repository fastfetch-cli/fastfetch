#include "physicaldisplay.h"

#include "common/io/io.h"
#include "util/edidHelper.h"
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

const char* ffDetectPhysicalDisplay(FFlist* results)
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

        uint8_t edidData[256] = {};
        if (!ffRegReadData(hKey, L"EDID", edidData, sizeof(edidData), NULL)) continue;
        uint32_t width, height;
        ffEdidGetPhysicalResolution(edidData, &width, &height);
        if (width == 0 || height == 0) continue;

        FFPhysicalDisplayResult* display = (FFPhysicalDisplayResult*) ffListAdd(results);
        display->width = width;
        display->height = height;
        ffStrbufInit(&display->name);
        ffEdidGetName(edidData, &display->name);
        ffEdidGetPhysicalSize(edidData, &display->physicalWidth, &display->physicalHeight);
    }
    return NULL;
}
