#include "bluetooth.h"
#include "common/library.h"
#include "util/windows/unicode.h"

#include <windows.h>
#include <bluetoothapis.h>

#pragma GCC diagnostic ignored "-Wpointer-sign"

const char* ffDetectBluetooth(FFlist* devices /* FFBluetoothResult */)
{
    // Actually bluetoothapis.dll, but it's missing on Windows 7
    FF_LIBRARY_LOAD(bluetoothapis, "dlopen bthprops.cpl failed", "bthprops.cpl", 1)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(bluetoothapis, BluetoothFindFirstDevice)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(bluetoothapis, BluetoothFindNextDevice)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(bluetoothapis, BluetoothFindDeviceClose)

    BLUETOOTH_DEVICE_INFO btdi = {
        .dwSize = sizeof(btdi)
    };
    HBLUETOOTH_DEVICE_FIND hFind = ffBluetoothFindFirstDevice(&(BLUETOOTH_DEVICE_SEARCH_PARAMS) {
        .fReturnConnected = TRUE,
        .fReturnRemembered = TRUE,
        .fReturnAuthenticated = TRUE,
        .fReturnUnknown = TRUE,
        .dwSize = sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS)
    }, &btdi);
    if(!hFind)
    {
        return "BluetoothFindFirstDevice() failed";
    }

    do {
        FFBluetoothResult* device = ffListAdd(devices);
        ffStrbufInitWS(&device->name, btdi.szName);
        ffStrbufInitF(&device->address, "%02x:%02x:%02x:%02x:%02x:%02x",
            btdi.Address.rgBytes[0],
            btdi.Address.rgBytes[1],
            btdi.Address.rgBytes[2],
            btdi.Address.rgBytes[3],
            btdi.Address.rgBytes[4],
            btdi.Address.rgBytes[5]);
        ffStrbufInit(&device->type);
        device->battery = 0;
        device->connected = !!btdi.fConnected;

        //https://btprodspecificationrefs.blob.core.windows.net/assigned-numbers/Assigned%20Number%20Types/Assigned%20Numbers.pdf

        if(BitTest(&btdi.ulClassofDevice, 13))
            ffStrbufAppendS(&device->type, "Limited Discoverable Mode, ");
        if(BitTest(&btdi.ulClassofDevice, 14))
            ffStrbufAppendS(&device->type, "LE audio, ");
        if(BitTest(&btdi.ulClassofDevice, 15))
            ffStrbufAppendS(&device->type, "Reserved for future use, ");
        if(BitTest(&btdi.ulClassofDevice, 16))
            ffStrbufAppendS(&device->type, "Positioning, ");
        if(BitTest(&btdi.ulClassofDevice, 17))
            ffStrbufAppendS(&device->type, "Networking, ");
        if(BitTest(&btdi.ulClassofDevice, 18))
            ffStrbufAppendS(&device->type, "Rendering, ");
        if(BitTest(&btdi.ulClassofDevice, 19))
            ffStrbufAppendS(&device->type, "Capturing, ");
        if(BitTest(&btdi.ulClassofDevice, 20))
            ffStrbufAppendS(&device->type, "Object Transfer, ");
        if(BitTest(&btdi.ulClassofDevice, 21))
            ffStrbufAppendS(&device->type, "Audio, ");
        if(BitTest(&btdi.ulClassofDevice, 22))
            ffStrbufAppendS(&device->type, "Telephony, ");
        if(BitTest(&btdi.ulClassofDevice, 23))
            ffStrbufAppendS(&device->type, "Information, ");

        if(device->type.length == 0)
        {
            uint32_t majorDeviceClasses = (btdi.ulClassofDevice >> 8) & ~(UINT32_MAX << 5);
            switch(majorDeviceClasses)
            {
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
        }
        else
        {
            ffStrbufTrimRight(&device->type, ' ');
            ffStrbufTrimRight(&device->type, ',');
        }
    } while (ffBluetoothFindNextDevice(hFind, &btdi));

    ffBluetoothFindDeviceClose(hFind);

    return NULL;
}
