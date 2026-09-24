#include "gamepad.h"
#include "common/io.h"
#include "common/mallocHelper.h"
#include "common/time.h"
#include "common/windows/unicode.h"

#include <windows.h>
#include <hidsdi.h>

static const char* detectKnownDeviceName(uint32_t vendorId, uint32_t productId) {
    switch (vendorId) {
        // Nintendo
        case 0x057E: {
            switch (productId) {
                case 0x2006:
                    return "Nintendo Switch Joycon L";
                case 0x2007:
                    return "Nintendo Switch Joycon R";
                case 0x2008:
                    return "Nintendo Switch Joycon L+R";
                case 0x2009:
                    return "Nintendo Switch Pro Controller";
                case 0x200E:
                    return "Nintendo Switch Charging Grip";
                case 0x2017:
                    return "Nintendo Switch SNES Controller";

                case 0x2066:
                    return "Nintendo Switch 2 Joycon R";
                case 0x2067:
                    return "Nintendo Switch 2 Joycon L";
                case 0x2068:
                    return "Nintendo Switch 2 Joycon L+R";
                case 0x2069:
                    return "Nintendo Switch 2 Pro Controller";

                default:
                    return nullptr;
            }
        }

        // Sony
        case 0x054C: {
            switch (productId) {
                case 0x0268:
                    return "Sony DualShock 3 / Six Axis";

                case 0x05C4:
                    return "Sony DualShock 4 Gen1";
                case 0x09CC:
                    return "Sony DualShock 4 Gen2";
                case 0x0BA0:
                    return "Sony DualShock 4 USB receiver";

                case 0x0CE6:
                    return "Sony DualSense";
                case 0x0DF2:
                    return "Sony DualSense Edge";
                case 0x0E5F:
                    return "Sony Access Controller";

                default:
                    return nullptr;
            }
        }

        // Logitech
        case 0x046D: {
            switch (productId) {
                case 0xC216:
                    return "Logitech F310, DirectInput";
                case 0xC218:
                    return "Logitech F510, DirectInput";
                case 0xC219:
                    return "Logitech F710, DirectInput";
                case 0xC21D:
                    return "Logitech F310";
                case 0xC21E:
                    return "Logitech F510";
                case 0xC21F:
                    return "Logitech F710";

                default:
                    return nullptr;
            }
        }

        case 0x045E: // Microsoft Xbox compatible controllers should be handled by Windows without problems
        default:
            return nullptr;
    }
}

// A HID read on Windows hands back as many bytes as the descriptor declares for the largest report,
// so the number of bytes a read returns says nothing about which report actually arrived. The report
// id, the first byte of the buffer, is what identifies it.
static bool readHidReport(HANDLE hHidFile, uint8_t* buffer, uint32_t length, DWORD timeoutMs, DWORD* nBytes) {
    OVERLAPPED overlapped = {};
    DWORD read;
    if (ReadFile(hHidFile, buffer, length, &read, &overlapped)) {
        *nBytes = read;
        return true;
    }
    if (GetOverlappedResultEx(hHidFile, &overlapped, &read, timeoutMs, TRUE)) {
        *nBytes = read;
        return true;
    }

    // A timeout leaves the read pending, and the driver writes into `buffer` and `overlapped` when
    // it eventually completes -- both of which are gone by then: the buffer belongs to the caller
    // and the OVERLAPPED sits on this frame. So the read is cancelled by its own OVERLAPPED rather
    // than by handle (CancelIo would take every other read on the same handle with it), and the
    // cancellation is waited for before returning.
    CancelIoEx(hHidFile, &overlapped);
    GetOverlappedResult(hHidFile, &overlapped, &read, TRUE);
    return false;
}

// The DualShock 4 sends a battery level in its state packet, but only in the extended report it
// switches to when a program asks it to. Until then it sends the plain HID gamepad report, which
// ends after the buttons.
typedef enum FFDs4ReportId: uint8_t {
    FF_DS4_REPORT_ID_USB_STATE = 0x01,
    FF_DS4_REPORT_ID_BLUETOOTH_STATE_FIRST = 0x11,
    FF_DS4_REPORT_ID_BLUETOOTH_STATE_LAST = 0x19,
} FFDs4ReportId;

// Offset of the battery level in the state packet, which is what the DualShock 4 documents and
// what SDL reads: five pad bytes after the six IMU axes.
#define FF_DS4_STATE_PACKET_BATTERY_OFFSET 29

// The plain HID gamepad report is ten bytes long, report id included
#define FF_DS4_NON_EXTENDED_REPORT_LENGTH 10

// Reads the battery level out of one input report of a DualShock 4, and returns 0 when the report
// carries none. `report[0]` is the report id and `reportLength` the size of the buffer.
static uint8_t parseDs4Battery(const uint8_t* report, uint32_t reportLength) {
    uint32_t batteryOffset;

    if (report[0] == FF_DS4_REPORT_ID_USB_STATE) {
        // A non-extended report stops after the buttons, so a non-zero byte past that can only come
        // from an extended packet. A wired controller always sends the extended one, a Bluetooth one
        // only once another program has enabled it.
        bool extended = false;
        for (uint32_t i = FF_DS4_NON_EXTENDED_REPORT_LENGTH; i < reportLength; ++i) {
            if (report[i]) {
                extended = true;
                break;
            }
        }
        if (!extended) {
            return 0;
        }
        // On USB the state packet follows the report id directly
        batteryOffset = 1 + FF_DS4_STATE_PACKET_BATTERY_OFFSET;
    } else if (report[0] >= FF_DS4_REPORT_ID_BLUETOOTH_STATE_FIRST && report[0] <= FF_DS4_REPORT_ID_BLUETOOTH_STATE_LAST) {
        // A Bluetooth state packet has two extra bytes in front, the first of which says whether the
        // packet carries HID state at all
        if (reportLength <= 1 || !(report[1] & 0x80)) {
            return 0;
        }
        batteryOffset = 3 + FF_DS4_STATE_PACKET_BATTERY_OFFSET;
    } else {
        return 0;
    }

    if (batteryOffset >= reportLength) {
        return 0;
    }

    uint8_t batteryInfo = report[batteryOffset];
    uint8_t level = batteryInfo & 0x0f;
    if ((batteryInfo & 0x10 /*charging?*/) && level > 11) {
        return 0; // A charging controller reports 11 for a full battery; anything above is not a level
    }

    uint8_t battery = (uint8_t) (level * 10 + 5); // 0..11 maps onto the middle of eleven 10% steps
    return battery > 100 ? 100 : battery;
}

// The DualSense uses one report id over USB and another over Bluetooth, and the state packet follows
// the id directly in the first case and one byte later in the second. The official controller sends
// SDL's PS5StatePacket_t; the shorter PS5StatePacketAlt_t belongs to third-party pads and is not read.
typedef enum FFDualSenseReportId: uint8_t {
    FF_DUALSENSE_REPORT_ID_USB_STATE = 0x01,
    FF_DUALSENSE_REPORT_ID_BLUETOOTH_STATE = 0x31,
} FFDualSenseReportId;

// Offset of the battery level in PS5StatePacket_t, and the length of a whole USB state report
#define FF_DUALSENSE_STATE_PACKET_BATTERY_OFFSET 52
#define FF_DUALSENSE_USB_REPORT_LENGTH 64

static uint8_t parseDualSenseBattery(const uint8_t* report, uint32_t reportLength) {
    uint32_t batteryOffset;

    if (report[0] == FF_DUALSENSE_REPORT_ID_USB_STATE) {
        // Over Bluetooth the controller answers the same id with a short report that stops before the
        // battery, so the USB report length is what tells the two apart
        if (reportLength != FF_DUALSENSE_USB_REPORT_LENGTH) {
            return 0;
        }
        batteryOffset = 1 + FF_DUALSENSE_STATE_PACKET_BATTERY_OFFSET;
    } else if (report[0] == FF_DUALSENSE_REPORT_ID_BLUETOOTH_STATE) {
        // The Bluetooth report has one extra byte in front of the state packet
        batteryOffset = 2 + FF_DUALSENSE_STATE_PACKET_BATTERY_OFFSET;
    } else {
        return 0;
    }

    if (batteryOffset >= reportLength) {
        return 0;
    }

    uint8_t batteryInfo = report[batteryOffset];
    uint32_t status = (batteryInfo >> 4) & 0x0f;
    if (status == 2) {
        return 100; // Charged
    }
    if (status > 1) {
        return 0; // Not a status the controller documents
    }

    uint32_t battery = (batteryInfo & 0x0f) * 10 + 5; // Discharging and charging share one scale
    return (uint8_t) (battery > 100 ? 100 : battery);
}

// A Switch controller interleaves its full state report with subcommand replies, which carry no
// battery at all. SDL skips those replies and waits for the next full report, and so does this,
// inside the same one-second budget a single read of any other controller gets.
typedef enum FFSwitchReportId: uint8_t {
    FF_SWITCH_REPORT_ID_FULL_STATE_FIRST = 0x30,
    FF_SWITCH_REPORT_ID_FULL_STATE_LAST = 0x31,
} FFSwitchReportId;

// The controller state packet starts one byte after the report id, and its second byte carries the
// battery level in its top three bits and the charging flag in bit 4. Anything that is not a full
// state report carries no battery and answers 0.
#define FF_SWITCH_STATE_PACKET_BATTERY_OFFSET 2

static uint8_t parseSwitchBattery(const uint8_t* report, uint32_t reportLength) {
    if (report[0] < FF_SWITCH_REPORT_ID_FULL_STATE_FIRST || report[0] > FF_SWITCH_REPORT_ID_FULL_STATE_LAST) {
        return 0;
    }
    if (reportLength <= FF_SWITCH_STATE_PACKET_BATTERY_OFFSET) {
        return 0;
    }

    uint8_t batteryInfo = report[FF_SWITCH_STATE_PACKET_BATTERY_OFFSET];
    uint32_t battery = ((batteryInfo & 0xE0) >> 4) * 100 / 8; // A level of 8 is a full battery
    if (battery == 0) {
        return 1; // Keep a drained controller apart from the "unknown" 0 the module uses
    }
    return (uint8_t) (battery > 100 ? 100 : battery);
}

// The longest run of reports without a full state report measured on a Pro Controller was eleven
#define FF_SWITCH_BATTERY_MAX_READS 16

static uint8_t detectSwitchBattery(HANDLE hHidFile, uint8_t* reportBuffer, uint32_t reportLength) {
    const double deadline = ffTimeGetTick() + (double) FF_IO_TERM_RESP_WAIT_MS;

    for (uint32_t i = 0; i < FF_SWITCH_BATTERY_MAX_READS; ++i) {
        double remaining = deadline - ffTimeGetTick();
        if (remaining < 1.) {
            break;
        }

        DWORD nBytes;
        if (!readHidReport(hHidFile, reportBuffer, reportLength, (DWORD) remaining, &nBytes)) {
            break;
        }

        uint8_t battery = parseSwitchBattery(reportBuffer, reportLength);
        if (battery) {
            return battery;
        }
    }

    return 0;
}

// Which of the battery layouts a controller uses, if any. A controller that is not listed here keeps
// the module's "unknown" 0 without a single read being issued.
typedef enum FFGamepadBatteryKind: uint8_t {
    FF_GAMEPAD_BATTERY_NONE,
    FF_GAMEPAD_BATTERY_DS4,
    FF_GAMEPAD_BATTERY_DUALSENSE,
    FF_GAMEPAD_BATTERY_SWITCH,
} FFGamepadBatteryKind;

static FFGamepadBatteryKind detectBatteryKind(uint32_t vendorId, uint32_t productId) {
    switch (vendorId) {
        case 0x054C: // Sony
            switch (productId) {
                case 0x0CE6: // DualSense
                case 0x0DF2: // DualSense Edge
                case 0x0E5F: // Access Controller
                    return FF_GAMEPAD_BATTERY_DUALSENSE;

                case 0x05C4: // DualShock 4 Gen1
                case 0x09CC: // DualShock 4 Gen2
                case 0x0BA0: // DualShock 4 USB receiver
                    return FF_GAMEPAD_BATTERY_DS4;

                default:
                    return FF_GAMEPAD_BATTERY_NONE;
            }

        // The Nintendo list is deliberately shorter than the name table above. The Charging Grip
        // (0x200E) is a rail that charges Joy-Con and has no battery of its own, and the SNES
        // online controller (0x2017) and the Switch 2 controllers (0x2066-0x2069) are ones SDL --
        // which is what this parsing follows -- reports no battery for: `SDL_hidapi_switch2.c` has
        // no power reporting at all, and the SNES entry is commented out of `controller_list.h`.
        // Reading the Switch state report from them would be a guess, so they keep the module's
        // "unknown".
        case 0x057E: // Nintendo
            switch (productId) {
                case 0x2006: // Joy-Con L
                case 0x2007: // Joy-Con R
                case 0x2008: // Joy-Con pair
                case 0x2009: // Pro Controller
                    return FF_GAMEPAD_BATTERY_SWITCH;

                default:
                    return FF_GAMEPAD_BATTERY_NONE;
            }

        default:
            return FF_GAMEPAD_BATTERY_NONE;
    }
}

const char* ffDetectGamepad(FFlist* devices /* List of FFGamepadDevice */) {
    UINT nDevices = 0;
    if (GetRawInputDeviceList(nullptr, &nDevices, sizeof(RAWINPUTDEVICELIST))) {
        return "GetRawInputDeviceList(nullptr) failed";
    }
    if (nDevices == 0) {
        return "No HID devices found";
    }
    FF_AUTO_FREE RAWINPUTDEVICELIST* pRawInputDeviceList = (RAWINPUTDEVICELIST*) malloc(sizeof(RAWINPUTDEVICELIST) * nDevices);
    if ((nDevices = GetRawInputDeviceList(pRawInputDeviceList, &nDevices, sizeof(RAWINPUTDEVICELIST))) == (UINT) -1) {
        return "GetRawInputDeviceList(pRawInputDeviceList) failed";
    }

    for (UINT i = 0; i < nDevices; ++i) {
        if (pRawInputDeviceList[i].dwType != RIM_TYPEHID) {
            continue;
        }

        HANDLE hDevice = pRawInputDeviceList[i].hDevice;

        RID_DEVICE_INFO rdi;
        UINT rdiSize = sizeof(rdi);
        if (GetRawInputDeviceInfoW(hDevice, RIDI_DEVICEINFO, &rdi, &rdiSize) == (UINT) -1) {
            continue;
        }

        if (rdi.hid.usUsagePage != 1 || (rdi.hid.usUsage != 4 /*Joystick*/ && rdi.hid.usUsage != 5 /*Gamepad*/)) {
            continue;
        }

        WCHAR devName[MAX_PATH] = L"";
        UINT nameSize = MAX_PATH;
        if (GetRawInputDeviceInfoW(hDevice, RIDI_DEVICENAME, devName, &nameSize) == (UINT) -1) {
            continue;
        }

        FFGamepadDevice* device = FF_LIST_ADD(FFGamepadDevice, *devices);
        ffStrbufInit(&device->serial);
        ffStrbufInit(&device->name);
        device->battery = 0;

        const char* knownGamepad = detectKnownDeviceName(rdi.hid.dwVendorId, rdi.hid.dwProductId);
        if (knownGamepad) {
            ffStrbufSetS(&device->name, knownGamepad);
        }
        FF_AUTO_CLOSE_FD HANDLE hHidFile = CreateFileW(devName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
        if (hHidFile == INVALID_HANDLE_VALUE) {
            if (!knownGamepad) {
                ffStrbufSetF(&device->name, "Unknown gamepad %04X-%04X", (unsigned) rdi.hid.dwVendorId, (unsigned) rdi.hid.dwProductId);
            }
            continue;
        }

        if (!knownGamepad) {
            wchar_t displayName[126];
            if (HidD_GetProductString(hHidFile, displayName, sizeof(displayName) /*in bytes*/)) {
                wchar_t manufacturer[126];
                if (HidD_GetManufacturerString(hHidFile, manufacturer, sizeof(manufacturer) /*in bytes*/)) {
                    ffStrbufSetWS(&device->name, manufacturer);
                    FF_STRBUF_AUTO_DESTROY displayNameStr = ffStrbufCreateWS(displayName);
                    ffStrbufAppendC(&device->name, ' ');
                    ffStrbufAppend(&device->name, &displayNameStr);
                } else {
                    ffStrbufSetWS(&device->name, displayName);
                }
            }
        }

        wchar_t serialNumber[127] = L"";
        if (HidD_GetSerialNumberString(hHidFile, serialNumber, sizeof(serialNumber) /*in bytes*/)) {
            ffStrbufSetWS(&device->serial, serialNumber);
        }

        PHIDP_PREPARSED_DATA preparsedData = nullptr;
        if (HidD_GetPreparsedData(hHidFile, &preparsedData)) {
            HIDP_CAPS caps;
            NTSTATUS capsResult = HidP_GetCaps(preparsedData, &caps);
            HidD_FreePreparsedData(preparsedData);
            if (!NT_SUCCESS(capsResult)) {
                continue;
            }

            // Sony and Nintendo controllers report a battery level only in a report a program has to
            // ask the controller for, so a controller that nothing else has opened reports none.
            FFGamepadBatteryKind batteryKind = detectBatteryKind(rdi.hid.dwVendorId, rdi.hid.dwProductId);
            if (batteryKind != FF_GAMEPAD_BATTERY_NONE) {
                FF_AUTO_FREE uint8_t* reportBuffer = malloc(caps.InputReportByteLength);
                DWORD nBytes;
                switch (batteryKind) {
                    case FF_GAMEPAD_BATTERY_DS4:
                        if (readHidReport(hHidFile, reportBuffer, caps.InputReportByteLength, FF_IO_TERM_RESP_WAIT_MS, &nBytes)) {
                            device->battery = parseDs4Battery(reportBuffer, caps.InputReportByteLength);
                        }
                        break;
                    case FF_GAMEPAD_BATTERY_DUALSENSE:
                        if (readHidReport(hHidFile, reportBuffer, caps.InputReportByteLength, FF_IO_TERM_RESP_WAIT_MS, &nBytes)) {
                            device->battery = parseDualSenseBattery(reportBuffer, caps.InputReportByteLength);
                        }
                        break;
                    case FF_GAMEPAD_BATTERY_SWITCH:
                        device->battery = detectSwitchBattery(hHidFile, reportBuffer, caps.InputReportByteLength);
                        break;
                    case FF_GAMEPAD_BATTERY_NONE:
                        break;
                }
            }
        }
    }

    return nullptr;
}
