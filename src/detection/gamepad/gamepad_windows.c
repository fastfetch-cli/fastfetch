#include "gamepad.h"
#include "common/io/io.h"
#include "util/mallocHelper.h"
#include "util/windows/unicode.h"

#include <winternl.h>
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
        ffStrbufInit(&device->name);
        device->battery = 0;

        const char* knownGamepad = detectKnownGamepad(rdi.hid.dwVendorId, rdi.hid.dwProductId);
        if (knownGamepad)
            ffStrbufSetS(&device->name, knownGamepad);
        HANDLE FF_AUTO_CLOSE_FD hHidFile = CreateFileW(devName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (!hHidFile)
        {
            if (!knownGamepad)
                ffStrbufSetF(&device->name, "Unknown gamepad %04X-%04X", (unsigned) rdi.hid.dwVendorId, (unsigned) rdi.hid.dwProductId);
            continue;
        }

        if (!knownGamepad)
        {
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
                ffStrbufSetWS(&device->identifier, serialNumber);
            else if (caps.FeatureReportByteLength >= 6)
            {
                uint8_t* featureBuffer = malloc(caps.FeatureReportByteLength);
                featureBuffer[0] = 18;
                if (HidD_GetFeature(hHidFile, featureBuffer, caps.FeatureReportByteLength))
                    ffStrbufSetF(&device->identifier, "%02X:%02X:%02X:%02X:%02X:%02X", featureBuffer[0], featureBuffer[1], featureBuffer[2], featureBuffer[3], featureBuffer[4], featureBuffer[5]);
            }

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
                FF_AUTO_FREE uint8_t* reportBuffer = malloc(caps.InputReportByteLength);
                ssize_t nBytes = ffReadFDData(hHidFile, caps.InputReportByteLength, reportBuffer);
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
                    if (nBytes > 3 && reportBuffer[0] == 0x30) // Controller must be connected by other programs
                    {
                        uint8_t batteryInfo = reportBuffer[2];
                        device->battery = (uint8_t) (((batteryInfo & 0xE0) >> 4) * 100 / 8);
                        if (device->battery == 0) device->battery = 1;
                        else if (device->battery > 100) device->battery = 100;
                    }
                }
            }
        }
    }

    return NULL;
}
