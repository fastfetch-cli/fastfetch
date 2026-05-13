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

// TODO: use CM API to fetch bluetooth devices instead of BluetoothFindFirstDevice, if we find DEVPKEY_Bluetooth_IsConnected or similar
#define GUID_DEVCLASS_BLUETOOTH_STRING L"{e0cbf06c-cd8b-4647-bb8a-263b43f0f974}" // Found in <devguid.h>
#define GUID_DEVCLASS_MEDIA_STRING L"{4d36e96c-e325-11ce-bfc1-08002be10318}"     // Found in <devguid.h>

static const char* ffBluetoothDetectBattery(FFlist* devices) {
    ULONG idListLength = 0;
    CONFIGRET status = CM_Get_Device_ID_List_SizeW(&idListLength, GUID_DEVCLASS_MEDIA_STRING, CM_GETIDLIST_FILTER_PRESENT);
    if (status != CR_SUCCESS) {
        return "CM_Get_Device_ID_List_SizeW failed";
    }

    if (idListLength == 0) {
        return NULL;
    }

    wchar_t* FF_AUTO_FREE idList = (wchar_t*) malloc((size_t) idListLength * sizeof(wchar_t));
    if (!idList) {
        return "malloc() failed";
    }

    status = CM_Get_Device_ID_ListW(GUID_DEVCLASS_MEDIA_STRING, idList, idListLength, CM_GETIDLIST_FILTER_PRESENT);
    if (status != CR_SUCCESS) {
        return "CM_Get_Device_ID_ListW failed";
    }

    for (const wchar_t* deviceId = idList; *deviceId; deviceId += wcslen(deviceId) + 1) {
        DEVINST devInst = 0;

        // Hands-Free profile service; headsets often expose battery level through this media device node rather than the Bluetooth device node
        // BthHFEnum
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
            if (deviceAddress[0] == bt->address.chars[0] &&
                deviceAddress[1] == bt->address.chars[1] &&
                deviceAddress[2] == bt->address.chars[3] &&
                deviceAddress[3] == bt->address.chars[4] &&
                deviceAddress[4] == bt->address.chars[6] &&
                deviceAddress[5] == bt->address.chars[7] &&
                deviceAddress[6] == bt->address.chars[9] &&
                deviceAddress[7] == bt->address.chars[10] &&
                deviceAddress[8] == bt->address.chars[12] &&
                deviceAddress[9] == bt->address.chars[13] &&
                deviceAddress[10] == bt->address.chars[15] &&
                deviceAddress[11] == bt->address.chars[16]) {
                bt->battery = battery;
                break;
            }
        }
    }

    return NULL;
}

const char* ffDetectBluetooth(FFBluetoothOptions* options, FFlist* devices /* FFBluetoothResult */) {
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
        if (GetLastError() == ERROR_NO_MORE_ITEMS) {
            return NULL;
        }
        return "BluetoothFindFirstDevice() failed";
    }

    do {
        if (!options->showDisconnected && !btdi.fConnected) {
            continue;
        }

        FFBluetoothResult* device = FF_LIST_ADD(FFBluetoothResult, *devices);
        ffStrbufInitWS(&device->name, btdi.szName);
        ffStrbufInitF(&device->address, "%02X:%02X:%02X:%02X:%02X:%02X", btdi.Address.rgBytes[5], btdi.Address.rgBytes[4], btdi.Address.rgBytes[3], btdi.Address.rgBytes[2], btdi.Address.rgBytes[1], btdi.Address.rgBytes[0]);
        ffStrbufInit(&device->type);
        device->battery = 0;
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

    if (devices->length > 0) {
        ffBluetoothDetectBattery(devices);
    }

    return NULL;
}
