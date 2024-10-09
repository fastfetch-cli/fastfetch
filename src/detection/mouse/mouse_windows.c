#include "mouse.h"
#include "common/io/io.h"
#include "util/mallocHelper.h"
#include "util/windows/unicode.h"

#include <winternl.h>
#include <windows.h>
#include <hidsdi.h>

const char* ffDetectMouse(FFlist* devices /* List of FFMouseDevice */)
{
    UINT nDevices = 0;
    if (GetRawInputDeviceList(NULL, &nDevices, sizeof(RAWINPUTDEVICELIST)))
        return "GetRawInputDeviceList(NULL) failed";
    if (nDevices == 0)
        return "No HID devices found";
    RAWINPUTDEVICELIST* FF_AUTO_FREE pRawInputDeviceList = (RAWINPUTDEVICELIST*) malloc(sizeof(RAWINPUTDEVICELIST) * nDevices);
    if ((nDevices = GetRawInputDeviceList(pRawInputDeviceList, &nDevices, sizeof(RAWINPUTDEVICELIST))) == (UINT) -1)
        return "GetRawInputDeviceList(pRawInputDeviceList) failed";

    for (UINT i = 0; i < nDevices; ++i)
    {
        if (pRawInputDeviceList[i].dwType != RIM_TYPEMOUSE) continue;

        HANDLE hDevice = pRawInputDeviceList[i].hDevice;

        RID_DEVICE_INFO rdi;
        UINT rdiSize = sizeof(rdi);
        if (GetRawInputDeviceInfoW(hDevice, RIDI_DEVICEINFO, &rdi, &rdiSize) == (UINT) -1)
            continue;

        WCHAR devName[MAX_PATH] = L"";
        UINT nameSize = MAX_PATH;
        if (GetRawInputDeviceInfoW(hDevice, RIDI_DEVICENAME, devName, &nameSize) == (UINT) -1)
            continue;

        FFMouseDevice* device = (FFMouseDevice*) ffListAdd(devices);
        ffStrbufInit(&device->serial);
        ffStrbufInit(&device->name);

        HANDLE FF_AUTO_CLOSE_FD hHidFile = CreateFileW(devName, 0 /* must be 0 instead of GENERIC_READ */, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (hHidFile == INVALID_HANDLE_VALUE)
        {
            ffStrbufSetF(&device->name, "Unknown mouse %04X-%04X", (unsigned) rdi.hid.dwVendorId, (unsigned) rdi.hid.dwProductId);
            continue;
        }

        wchar_t displayName[126];
        if (HidD_GetProductString(hHidFile, displayName, sizeof(displayName)))
        {
            wchar_t manufacturer[126];
            if (HidD_GetManufacturerString(hHidFile, manufacturer, sizeof(manufacturer)))
            {
                ffStrbufSetWS(&device->name, manufacturer);
                FF_STRBUF_AUTO_DESTROY displayNameStr = ffStrbufCreateWS(displayName);
                ffStrbufAppendC(&device->name, ' ');
                ffStrbufAppend(&device->name, &displayNameStr);
            }
            else
            {
                ffStrbufSetWS(&device->name, displayName);
            }
        }

        PHIDP_PREPARSED_DATA preparsedData = NULL;
        if (HidD_GetPreparsedData(hHidFile, &preparsedData))
        {
            HIDP_CAPS caps;
            NTSTATUS capsResult = HidP_GetCaps(preparsedData, &caps);
            HidD_FreePreparsedData(preparsedData);
            if (!NT_SUCCESS(capsResult))
                continue;

            wchar_t serialNumber[127] = L"";
            if (HidD_GetSerialNumberString(hHidFile, serialNumber, sizeof(serialNumber)))
                ffStrbufSetWS(&device->serial, serialNumber);
        }
    }

    return NULL;
}
