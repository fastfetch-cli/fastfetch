#include "bluetooth.h"
#include "common/library.h"
#include "common/mallocHelper.h"
#include "common/windows/unicode.h"

#define INITGUID
#include <windows.h>
#include <bluetoothapis.h>
#include <cfgmgr32.h>
#include <devpkey.h>

#pragma GCC diagnostic ignored "-Wpointer-sign"

// https://github.com/wine-mirror/wine/blob/ab6f4584b89f28504b0b277c0b4c723a86b4d6b7/include/ddk/bthguid.h#L4
/* DEVPROP_TYPE_STRING */
DEFINE_DEVPROPKEY(DEVPKEY_Bluetooth_DeviceAddress, 0x2bd67d8b, 0x8beb, 0x48d5, 0x87, 0xe0, 0x6c, 0xda, 0x34, 0x28, 0x04, 0x0a, 1);
/* DEVPROP_TYPE_UINT32 */
DEFINE_DEVPROPKEY(DEVPKEY_Bluetooth_ClassOfDevice, 0x2bd67d8b, 0x8beb, 0x48d5, 0x87, 0xe0, 0x6c, 0xda, 0x34, 0x28, 0x04, 0x0a, 10);
/* DEVPROP_TYPE_FILETIME */
DEFINE_DEVPROPKEY(DEVPKEY_Bluetooth_LastConnectedTime, 0x2bd67d8b, 0x8beb, 0x48d5, 0x87, 0xe0, 0x6c, 0xda, 0x34, 0x28, 0x04, 0x0a, 11);
/* DEVPROP_TYPE_GUID */
DEFINE_DEVPROPKEY(DEVPKEY_Bluetooth_ServiceGUID, 0x2bd67d8b, 0x8beb, 0x48d5, 0x87, 0xe0, 0x6c, 0xda, 0x34, 0x28, 0x04, 0x0a, 2);
/* DEVPROP_TYPE_UINT8 */
DEFINE_DEVPROPKEY(DEVPKEY_Bluetooth_BatteryLevel, 0x104ea319, 0x6ee2, 0x4701, 0xbd, 0x47, 0x8d, 0xdb, 0xf4, 0x25, 0xbb, 0xe5, 2);

// Windows keeps the classic (BR/EDR) and the Low Energy stacks apart, and so does fastfetch: the
// device search below only ever returns BR/EDR devices, and the LE half lives in
// bluetooth_windows.cpp because reaching it means WinRT, which is C++ only. This is the bridge --
// it walks the LE stack, then folds what it finds into the list the classic half already built.
const char* ffBluetoothDetectLe(FFBluetoothOptions* options, FFlist* devices /* FFBluetoothResult */);

// The device tree cannot answer "is this connected". DEVPKEY_DeviceContainer_IsConnected and
// IsPaired are absent from every present devnode, and CM_Get_DevNode_PropertyW answers
// CR_NO_SUCH_VALUE for PKEY_Devices_Aep_IsConnected. The value does exist, but only in the
// Association Endpoint property store, which is reachable through WinRT alone -- see
// ffBluetoothDetectLe() and the note there about the cost of each way in.
#define GUID_DEVCLASS_BLUETOOTH_STRING L"{e0cbf06c-cd8b-4647-bb8a-263b43f0f974}" // Found in <devguid.h>
#define GUID_DEVCLASS_MEDIA_STRING L"{4d36e96c-e325-11ce-bfc1-08002be10318}"     // Found in <devguid.h>

// The device tree spells an address without separators, while the list holds the printed form. The
// case has to be folded because it depends on the enumerator: the BTHENUM nodes are upper case and
// the BTHLE / BTHLEDevice ones lower case, and the list is always upper case.
static bool addressEquals(const wchar_t* plain, const FFstrbuf* address) {
    if (address->length != 17) {
        return false;
    }

    for (uint32_t i = 0; i < 6; ++i) {
        if (toupper((unsigned char) plain[i * 2]) != toupper((unsigned char) address->chars[i * 3]) ||
            toupper((unsigned char) plain[i * 2 + 1]) != toupper((unsigned char) address->chars[i * 3 + 1])) {
            return false;
        }
    }

    return true;
}

static const char* detectBattery(FFlist* devices) {
    ULONG idListLength = 0;
    // The class filter is a no-op here, and deliberately so: CM_Get_Device_ID_ListW() only honours the
    // filter string together with CM_GETIDLIST_FILTER_CLASS, so this walks every present node of the
    // tree rather than the media class. That is what finds the level -- the node carrying
    // DEVPKEY_Bluetooth_BatteryLevel is a BTHENUM service node (the Hands-Free profile one), which is
    // not in the media class at all, so narrowing the filter would lose it.
    CONFIGRET status = CM_Get_Device_ID_List_SizeW(&idListLength, GUID_DEVCLASS_MEDIA_STRING, CM_GETIDLIST_FILTER_PRESENT);
    if (status != CR_SUCCESS) {
        return "CM_Get_Device_ID_List_SizeW failed";
    }

    if (idListLength == 0) {
        return nullptr;
    }

    FF_AUTO_FREE wchar_t* idList = (wchar_t*) malloc((size_t) idListLength * sizeof(wchar_t));
    if (!idList) {
        return "malloc() failed";
    }

    status = CM_Get_Device_ID_ListW(GUID_DEVCLASS_MEDIA_STRING, idList, idListLength, CM_GETIDLIST_FILTER_PRESENT);
    if (status != CR_SUCCESS) {
        return "CM_Get_Device_ID_ListW failed";
    }

    for (const wchar_t* deviceId = idList; *deviceId; deviceId += wcslen(deviceId) + 1) {
        DEVINST devInst = 0;

        if (CM_Locate_DevNodeW(&devInst, (DEVINSTID_W) deviceId, CM_LOCATE_DEVNODE_NORMAL) != CR_SUCCESS) {
            continue;
        }

        uint8_t battery = 0;
        {
            DEVPROPTYPE devPropertyType = DEVPROP_TYPE_EMPTY;
            ULONG propertySize = sizeof(battery);
            if (CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_Bluetooth_BatteryLevel, &devPropertyType, (PBYTE) &battery, &propertySize, 0) != CR_SUCCESS || devPropertyType != DEVPROP_TYPE_BYTE || propertySize != sizeof(battery)) {
                continue;
            }
        }

        WCHAR deviceAddress[13]; // 6 bytes in hex + null terminator
        {
            DEVPROPTYPE devPropertyType = DEVPROP_TYPE_EMPTY;
            ULONG propertySize = sizeof(deviceAddress);
            if (CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_Bluetooth_DeviceAddress, &devPropertyType, (PBYTE) deviceAddress, &propertySize, 0) != CR_SUCCESS || devPropertyType != DEVPROP_TYPE_STRING || propertySize != sizeof(deviceAddress)) {
                continue;
            }
        }

        FF_LIST_FOR_EACH (FFBluetoothResult, bt, *devices) {
            if (addressEquals(deviceAddress, &bt->address)) {
                bt->battery = battery;
                break;
            }
        }
    }

    return nullptr;
}

// The classic (BR/EDR) half. `BluetoothFindFirstDevice` only ever answers with BR/EDR devices; the
// Low Energy stack is a separate walk, in bluetooth_windows.cpp, which folds what it finds into the
// list this function builds.
static const char* detectClassic(FFBluetoothOptions* options, FFlist* devices /* FFBluetoothResult */) {
    FF_LIBRARY_LOAD_MESSAGE(bluetoothapis, "bluetoothapis.dll", 1)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(bluetoothapis, BluetoothFindFirstDevice)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(bluetoothapis, BluetoothFindNextDevice)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(bluetoothapis, BluetoothFindDeviceClose)

    BLUETOOTH_DEVICE_INFO btdi = {
        .dwSize = sizeof(btdi)
    };
    HBLUETOOTH_DEVICE_FIND hFind = ffBluetoothFindFirstDevice(&(BLUETOOTH_DEVICE_SEARCH_PARAMS) {
                                                                  .fReturnConnected = TRUE,
                                                                  .fReturnRemembered = options->showDisconnected,
                                                                  .fReturnAuthenticated = options->showDisconnected,
                                                                  .dwSize = sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS) },
        &btdi);
    if (!hFind) {
        if (GetLastError() != ERROR_NO_MORE_ITEMS) {
            return "BluetoothFindFirstDevice() failed";
        }
        // "Nothing found" is not an error here: this search only ever sees BR/EDR devices, and a
        // machine whose only peripheral is a Low Energy one reaches this branch with a working
        // radio. The LE half below is what finds that device, so falling through is the point.
    } else {
        do {
            if (!options->showDisconnected && !btdi.fConnected) {
                continue;
            }

            FFBluetoothResult* device = FF_LIST_ADD(FFBluetoothResult, *devices);
            ffStrbufInitWS(&device->name, btdi.szName);
            ffStrbufInitF(&device->address, "%02X:%02X:%02X:%02X:%02X:%02X", btdi.Address.rgBytes[5], btdi.Address.rgBytes[4], btdi.Address.rgBytes[3], btdi.Address.rgBytes[2], btdi.Address.rgBytes[1], btdi.Address.rgBytes[0]);
            ffStrbufInit(&device->type);
            device->deviceType = FF_BLUETOOTH_DEVICE_TYPE_CLASSIC_BIT;
            device->battery = 0;
            device->signalQuality = -DBL_MAX; // Only the LE stack can report a signal strength
            device->connected = !!btdi.fConnected;

            // https://btprodspecificationrefs.blob.core.windows.net/assigned-numbers/Assigned%20Number%20Types/Assigned%20Numbers.pdf

            if (BitTest(&btdi.ulClassofDevice, 13)) {
                ffStrbufAppendS(&device->type, "Limited Discoverable Mode, ");
            }
            if (BitTest(&btdi.ulClassofDevice, 14)) {
                ffStrbufAppendS(&device->type, "LE audio, ");
            }
            if (BitTest(&btdi.ulClassofDevice, 15)) {
                ffStrbufAppendS(&device->type, "Reserved for future use, ");
            }
            if (BitTest(&btdi.ulClassofDevice, 16)) {
                ffStrbufAppendS(&device->type, "Positioning, ");
            }
            if (BitTest(&btdi.ulClassofDevice, 17)) {
                ffStrbufAppendS(&device->type, "Networking, ");
            }
            if (BitTest(&btdi.ulClassofDevice, 18)) {
                ffStrbufAppendS(&device->type, "Rendering, ");
            }
            if (BitTest(&btdi.ulClassofDevice, 19)) {
                ffStrbufAppendS(&device->type, "Capturing, ");
            }
            if (BitTest(&btdi.ulClassofDevice, 20)) {
                ffStrbufAppendS(&device->type, "Object Transfer, ");
            }
            if (BitTest(&btdi.ulClassofDevice, 21)) {
                ffStrbufAppendS(&device->type, "Audio, ");
            }
            if (BitTest(&btdi.ulClassofDevice, 22)) {
                ffStrbufAppendS(&device->type, "Telephony, ");
            }
            if (BitTest(&btdi.ulClassofDevice, 23)) {
                ffStrbufAppendS(&device->type, "Information, ");
            }

            if (device->type.length == 0) {
                uint32_t majorDeviceClasses = (btdi.ulClassofDevice >> 8) & ~(UINT32_MAX << 5);
                switch (majorDeviceClasses) {
                    case 0b00000:
                        ffStrbufAppendS(&device->type, "Miscellaneous");
                        break;
                    case 0b00001:
                        ffStrbufAppendS(&device->type, "Computer");
                        break;
                    case 0b00010:
                        ffStrbufAppendS(&device->type, "Phone");
                        break;
                    case 0b00011:
                        ffStrbufAppendS(&device->type, "LAN/Network Access point");
                        break;
                    case 0b00100:
                        ffStrbufAppendS(&device->type, "Audio/Video");
                        break;
                    case 0b00101:
                        ffStrbufAppendS(&device->type, "Peripheral");
                        break;
                    case 0b00110:
                        ffStrbufAppendS(&device->type, "Imaging");
                        break;
                    case 0b00111:
                        ffStrbufAppendS(&device->type, "Wearable");
                        break;
                    case 0b01000:
                        ffStrbufAppendS(&device->type, "Toy");
                        break;
                    case 0b01001:
                        ffStrbufAppendS(&device->type, "Health");
                        break;
                    case 0b11111:
                        ffStrbufAppendS(&device->type, "Uncategorized");
                        break;
                    default:
                        ffStrbufAppendS(&device->type, "Unknown");
                        break;
                }
            } else {
                ffStrbufTrimRight(&device->type, ' ');
                ffStrbufTrimRight(&device->type, ',');
            }
        } while (ffBluetoothFindNextDevice(hFind, &btdi));

        ffBluetoothFindDeviceClose(hFind);
    }

    return nullptr;
}

const char* ffDetectBluetooth(FFBluetoothOptions* options, FFlist* devices /* FFBluetoothResult */) {
    // `showType` selects which stacks are walked, not merely which entries are printed. The classic
    // and the Low Energy stack are two different APIs here, so a stack the user switched off is not
    // called at all.
    if (options->showType & FF_BLUETOOTH_DEVICE_TYPE_CLASSIC_BIT) {
        const char* error = detectClassic(options, devices);
        if (error) {
            return error;
        }
    }

    if (options->showType & FF_BLUETOOTH_DEVICE_TYPE_LE_BIT) {
        // A failure here is not fatal: the LE half only adds to the classic list, which stands on
        // its own, and "no LE link on this machine" is the ordinary case rather than an error.
        ffBluetoothDetectLe(options, devices);
    }

    if (devices->length > 0) {
        detectBattery(devices);
    }

    return nullptr;
}
