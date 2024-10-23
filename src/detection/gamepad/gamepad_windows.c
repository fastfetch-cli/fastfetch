#include "gamepad.h"
#include "common/io/io.h"
#include "util/mallocHelper.h"
#include "util/windows/unicode.h"

#include <winternl.h>
#include <windows.h>
#include <hidsdi.h>

static const char* detectKnownDeviceName(uint32_t vendorId, uint32_t productId)
{
    switch (vendorId)
    {
        // Nintendo
        case 0x057E:
        {
            switch (productId)
            {
                case 0x2006: return "Nintendo Switch Joycon L";
                case 0x2007: return "Nintendo Switch Joycon R";
                case 0x2009: return "Nintendo Switch Pro Controller";
                case 0x200E: return "Nintendo Switch Charging Grip";
                case 0x2017: return "Nintendo Switch SNES Controller";

                default: return NULL;
            }
        }

        // Sony
        case 0x054C:
        {
            switch (productId)
            {
                case 0x0268: return "Sony DualShock 3 / Six Axis";

                case 0x05C4: return "Sony DualShock 4 Gen1";
                case 0x09CC: return "Sony DualShock 4 Gen2";
                case 0x0BA0: return "Sony DualShock 4 USB receiver";

                case 0x0CE6: return "Sony DualSense";
                case 0x0DF2: return "Sony DualSense Edge";

                default: return NULL;
            }
        }

        // Logitech
        case 0x046D:
        {
            switch (productId)
            {
                case 0xC216: return "Logitech F310, DirectInput";
                case 0xC218: return "Logitech F510, DirectInput";
                case 0xC219: return "Logitech F710, DirectInput";
                case 0xC21D: return "Logitech F310";
                case 0xC21E: return "Logitech F510";
                case 0xC21F: return "Logitech F710";

                default: return NULL;
            }
        }

        case 0x045E: // Microsoft Xbox compatible controllers should be handled by Windows without problems
        default: return NULL;
    }
}

const char* ffDetectGamepad(FFlist* devices /* List of FFGamepadDevice */)
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
        if (pRawInputDeviceList[i].dwType != RIM_TYPEHID) continue;

        HANDLE hDevice = pRawInputDeviceList[i].hDevice;

        RID_DEVICE_INFO rdi;
        UINT rdiSize = sizeof(rdi);
        if (GetRawInputDeviceInfoW(hDevice, RIDI_DEVICEINFO, &rdi, &rdiSize) == (UINT) -1)
            continue;

        if (rdi.hid.usUsagePage != 1 || (rdi.hid.usUsage != 4/*Joystick*/ && rdi.hid.usUsage != 5/*Gamepad*/))
            continue;

        WCHAR devName[MAX_PATH] = L"";
        UINT nameSize = MAX_PATH;
        if (GetRawInputDeviceInfoW(hDevice, RIDI_DEVICENAME, devName, &nameSize) == (UINT) -1)
            continue;

        FFGamepadDevice* device = (FFGamepadDevice*) ffListAdd(devices);
        ffStrbufInit(&device->serial);
        ffStrbufInit(&device->name);
        device->battery = 0;

        const char* knownGamepad = detectKnownDeviceName(rdi.hid.dwVendorId, rdi.hid.dwProductId);
        if (knownGamepad)
            ffStrbufSetS(&device->name, knownGamepad);
        HANDLE FF_AUTO_CLOSE_FD hHidFile = CreateFileW(devName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
        if (hHidFile == INVALID_HANDLE_VALUE)
        {
            if (!knownGamepad)
                ffStrbufSetF(&device->name, "Unknown gamepad %04X-%04X", (unsigned) rdi.hid.dwVendorId, (unsigned) rdi.hid.dwProductId);
            continue;
        }

        if (!knownGamepad)
        {
            wchar_t displayName[126];
            if (HidD_GetProductString(hHidFile, displayName, sizeof(displayName) /*in bytes*/))
            {
                wchar_t manufacturer[126];
                if (HidD_GetManufacturerString(hHidFile, manufacturer, sizeof(manufacturer) /*in bytes*/))
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
        }

        wchar_t serialNumber[127] = L"";
        if (HidD_GetSerialNumberString(hHidFile, serialNumber, sizeof(serialNumber) /*in bytes*/))
            ffStrbufSetWS(&device->serial, serialNumber);

        PHIDP_PREPARSED_DATA preparsedData = NULL;
        if (HidD_GetPreparsedData(hHidFile, &preparsedData))
        {
            HIDP_CAPS caps;
            NTSTATUS capsResult = HidP_GetCaps(preparsedData, &caps);
            HidD_FreePreparsedData(preparsedData);
            if (!NT_SUCCESS(capsResult))
                continue;

            if (
                (rdi.hid.dwVendorId == 0x054C && (
                    rdi.hid.dwProductId == 0x05C4 || // PS4 Gen1
                    rdi.hid.dwProductId == 0x09CC // PS4 Gen2
                )) ||
                (rdi.hid.dwVendorId == 0x057E && (
                    rdi.hid.dwProductId == 0x2009 // NS Pro
                ))
            )
            {
                // Controller must be connected by other programs
                FF_AUTO_FREE uint8_t* reportBuffer = malloc(caps.InputReportByteLength);
                OVERLAPPED overlapped = { };
                DWORD nBytes;
                if (ReadFile(hHidFile, reportBuffer, caps.InputReportByteLength, &nBytes, &overlapped) ||
                    (WaitForSingleObject(hHidFile, FF_IO_TERM_RESP_WAIT_MS) == WAIT_OBJECT_0 && GetOverlappedResult(hHidFile, &overlapped, &nBytes, FALSE)))
                {
                    if (rdi.hid.dwVendorId == 0x054C)
                    {
                        if (nBytes > 31)
                        {
                            uint8_t batteryInfo = reportBuffer[caps.InputReportByteLength == 64 /*USB?*/ ? 30 : 32];
                            device->battery = (uint8_t) ((batteryInfo & 0x0f) * 100 / (batteryInfo & 0x10 /*charging?*/ ? 11 /*BATTERY_MAX_USB*/ : 8 /*BATTERY_MAX*/));
                            if (device->battery > 100) device->battery = 100;
                        }
                    }
                    else
                    {
                        if (nBytes > 3 && reportBuffer[0] == 0x30)
                        {
                            uint8_t batteryInfo = reportBuffer[2];
                            device->battery = (uint8_t) (((batteryInfo & 0xE0) >> 4) * 100 / 8);
                            if (device->battery == 0) device->battery = 1;
                            else if (device->battery > 100) device->battery = 100;
                        }
                    }
                }
                else
                    CancelIo(hHidFile);
            }
        }
    }

    return NULL;
}
