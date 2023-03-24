#include "gamepad.h"
#include "common/io/io.h"
#include "util/mallocHelper.h"
#include "util/windows/unicode.h"

#include <windows.h>
#include <hidsdi.h>

static const char* detectKnownGamepad(uint32_t vendorId, uint32_t productId)
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
                case 0x2009: return "Nintendo Switch Pro";
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
                case 0x0268: return "Sony Playstation 3 Controller";

                case 0x05C4: return "Sony DualShock 4 Gen1";
                case 0x09CC: return "Sony DualShock 4 Gen2";
                case 0x0BA0: return "Sony PS4 Controller USB receiver";

                case 0x0CE6: return "Sony DualSense";

                default: return NULL;
            }
        }

        // Microsoft
        case 0x045E:
        {
            switch (productId)
            {
                case 0x02E0: return "Microsoft X-Box One S pad (Wireless)";
                case 0x02FD: return "Microsoft X-Box One S pad (Wireless, 2016 FW)";
                case 0x0B05: return "Microsoft X-Box One Elite Series 2 pad (Wireless)";
                case 0x0B13: return "Microsoft X-Box Series X (Wireless)";

                case 0x028E: return "Microsoft XBox 360";
                case 0x028F: return "Microsoft XBox 360 v2";
                case 0x02A1: return "Microsoft XBox 360";
                case 0x0291: return "Microsoft XBox 360 USB receiver";
                case 0x02A0: return "Microsoft XBox 360 Big Button IR";
                case 0x02DD: return "Microsoft XBox One";
                case 0xB326: return "Microsoft XBox One Firmware 2015";
                case 0x02E3: return "Microsoft XBox One Elite";
                case 0x02FF: return "Microsoft XBox One Elite";
                case 0x02EA: return "Microsoft XBox One S";

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

        default: return NULL;
    }
}

const char* ffDetectGamepad(FF_MAYBE_UNUSED const FFinstance* instance, FFlist* devices /* List of FFGamepadDevice */)
{
    UINT nDevices = 0;
    if (GetRawInputDeviceList(NULL, &nDevices, sizeof(RAWINPUTDEVICELIST)))
        return "GetRawInputDeviceList(NULL) failed";
    if (nDevices == 0)
        return "No HID devices found";
    RAWINPUTDEVICELIST* FF_AUTO_FREE pRawInputDeviceList = (RAWINPUTDEVICELIST*) malloc(sizeof(RAWINPUTDEVICELIST) * nDevices);
    if ((nDevices = GetRawInputDeviceList(pRawInputDeviceList, &nDevices, sizeof(RAWINPUTDEVICELIST))) == (UINT) -1)
        return "GetRawInputDeviceList(pRawInputDeviceList) failed";

    for (UINT i = 0; i < nDevices; ++i) {
        if (pRawInputDeviceList[i].dwType != 2) continue;

        HANDLE hDevice = pRawInputDeviceList[i].hDevice;

        RID_DEVICE_INFO rdi;
        UINT rdiSize = sizeof(rdi);
        if (GetRawInputDeviceInfoW(hDevice, RIDI_DEVICEINFO, &rdi, &rdiSize) == (UINT) -1)
            continue;

        if (rdi.hid.usUsagePage != 1 || rdi.hid.usUsage != 5) // Gamepad
            continue;

        WCHAR devName[MAX_PATH] = L"";
        UINT nameSize = MAX_PATH;
        if (GetRawInputDeviceInfoW(hDevice, RIDI_DEVICENAME, devName, &nameSize) == (UINT) -1)
            continue;

        FFGamepadDevice* device = (FFGamepadDevice*) ffListAdd(devices);
        ffStrbufInit(&device->identifier);
        ffStrbufSetWS(&device->identifier, devName);
        ffStrbufInit(&device->name);

        const char* knownGamepad = detectKnownGamepad(rdi.hid.dwVendorId, rdi.hid.dwProductId);
        if (knownGamepad)
            ffStrbufSetS(&device->name, knownGamepad);
        else
        {
            wchar_t displayName[126];
            HANDLE FF_AUTO_CLOSE_FD hHidFile = CreateFileW(devName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
            if (hHidFile && HidD_GetProductString(hHidFile, displayName, sizeof(wchar_t) * 126))
                ffStrbufSetWS(&device->name, displayName);
            else
                ffStrbufSetF(&device->name, "Unknown gamepad %4X-%4X", rdi.hid.dwVendorId, rdi.hid.dwProductId);
        }
    }

    return NULL;
}
