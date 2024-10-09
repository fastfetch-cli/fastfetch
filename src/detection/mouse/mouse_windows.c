#define INITGUID

#include "mouse.h"
#include "common/io/io.h"
#include "util/mallocHelper.h"
#include "util/windows/unicode.h"

#include <winternl.h>
#include <windows.h>
#include <hidsdi.h>
#include <cfgmgr32.h>
#include <devpkey.h>

WINAPI CMAPI CONFIGRET CM_Get_Device_Interface_PropertyW(
  _In_      LPCWSTR          pszDeviceInterface,
  _In_      const DEVPROPKEY *PropertyKey,
  _Out_     DEVPROPTYPE      *PropertyType,
  _Out_     PBYTE            PropertyBuffer,
  _Inout_   PULONG           PropertyBufferSize,
  _In_      ULONG            ulFlags
);

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

        WCHAR devName[MAX_PATH];
        UINT nameSize = MAX_PATH;
        if (GetRawInputDeviceInfoW(hDevice, RIDI_DEVICENAME, devName, &nameSize) == (UINT) -1)
            continue;

        FFMouseDevice* device = (FFMouseDevice*) ffListAdd(devices);
        ffStrbufInit(&device->serial);
        ffStrbufInit(&device->name);

        wchar_t buffer[MAX_PATH];

        HANDLE FF_AUTO_CLOSE_FD hHidFile = CreateFileW(devName, 0 /* must be 0 instead of GENERIC_READ */, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (hHidFile != INVALID_HANDLE_VALUE)
        {
            if (HidD_GetProductString(hHidFile, buffer, (ULONG) sizeof(buffer)))
                ffStrbufSetWS(&device->name, buffer);

            if (HidD_GetSerialNumberString(hHidFile, buffer, sizeof(buffer)))
                ffStrbufSetWS(&device->serial, buffer);
        }

        if (!device->name.length)
        {
            // https://stackoverflow.com/a/64321096/9976392
            DEVPROPTYPE propertyType;
            ULONG propertySize = sizeof(buffer);

            if (CM_Get_Device_Interface_PropertyW(devName, &DEVPKEY_Device_InstanceId, &propertyType, (PBYTE) buffer, &propertySize, 0) == CR_SUCCESS)
            {
                DEVINST devInst;
                if (CM_Locate_DevNodeW(&devInst, buffer, CM_LOCATE_DEVNODE_NORMAL) == CR_SUCCESS)
                {
                    propertySize = sizeof(buffer);
                    if (CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_NAME, &propertyType, (PBYTE) buffer, &propertySize, 0) == CR_SUCCESS)
                        ffStrbufSetWS(&device->name, buffer);
                }
            }
        }

        if (!device->name.length)
            ffStrbufSetF(&device->name, "Unknown device %04X-%04X", (unsigned) rdi.hid.dwVendorId, (unsigned) rdi.hid.dwProductId);
    }

    return NULL;
}
