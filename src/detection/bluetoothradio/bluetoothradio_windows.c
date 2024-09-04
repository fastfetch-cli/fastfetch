#include "bluetoothradio.h"
#include "common/library.h"
#include "common/io/io.h"
#include "util/windows/unicode.h"

#include <windows.h>
#include <bluetoothapis.h>
#include <winioctl.h>

// #include <bthioctl.h>

#define BTH_IOCTL_BASE              0
#define BTH_CTL(id)                 CTL_CODE(FILE_DEVICE_BLUETOOTH, (id), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_BTH_GET_LOCAL_INFO    BTH_CTL(BTH_IOCTL_BASE+0x00)
#define LMP_LE_SUPPORTED(x)         ((x >> 38) & 1)

typedef struct _BTH_RADIO_INFO
{
    // Supported LMP features of the radio.  Use LMP_XXX() to extract
    // the desired bits.
    ULONGLONG lmpSupportedFeatures;

    // Manufacturer ID (possibly BTH_MFG_XXX)
    USHORT mfg;

    // LMP subversion
    USHORT lmpSubversion;

    // LMP version
    UCHAR lmpVersion;
} __attribute__((__packed__)) BTH_RADIO_INFO;

typedef struct _BTH_LOCAL_RADIO_INFO
{
    // Local BTH_ADDR, class of device, and radio name
    BTH_DEVICE_INFO         localInfo;

    // Combo of LOCAL_RADIO_XXX values
    ULONG flags;

    // HCI revision, see core spec
    USHORT hciRevision;

    // HCI version, see core spec
    UCHAR hciVersion;

    // More information about the local radio (LMP, MFG)
    BTH_RADIO_INFO radioInfo;
} __attribute__((__packed__)) BTH_LOCAL_RADIO_INFO;
static_assert(sizeof(BTH_LOCAL_RADIO_INFO) == 292, "BTH_LOCAL_RADIO_INFO should be 292 bytes");

#pragma GCC diagnostic ignored "-Wpointer-sign"

const char* ffDetectBluetoothRadio(FFlist* devices /* FFBluetoothRadioResult */)
{
    // Actually bluetoothapis.dll, but it's missing on Windows 7
    FF_LIBRARY_LOAD(bluetoothapis, "dlopen bthprops.cpl failed", "bthprops.cpl", 1)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(bluetoothapis, BluetoothFindFirstRadio)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(bluetoothapis, BluetoothFindNextRadio)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(bluetoothapis, BluetoothFindRadioClose)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(bluetoothapis, BluetoothIsConnectable)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(bluetoothapis, BluetoothIsDiscoverable)

    HANDLE hRadio = NULL;
    HBLUETOOTH_DEVICE_FIND hFind = ffBluetoothFindFirstRadio(&(BLUETOOTH_FIND_RADIO_PARAMS) {
        .dwSize = sizeof(BLUETOOTH_FIND_RADIO_PARAMS)
    }, &hRadio);
    if(!hFind)
    {
        if (GetLastError() == ERROR_NO_MORE_ITEMS)
            return "No Bluetooth radios found or service disabled";
        else
            return "BluetoothFindFirstRadio() failed";
    }

    do {
        BTH_LOCAL_RADIO_INFO blri;
        DWORD returned;
        if (!DeviceIoControl(hRadio, IOCTL_BTH_GET_LOCAL_INFO, NULL, 0, &blri, sizeof(blri), &returned, NULL))
            continue;

        FFBluetoothRadioResult* device = ffListAdd(devices);
        ffStrbufInitS(&device->name, blri.localInfo.name);

        BLUETOOTH_ADDRESS_STRUCT addr = { .ullLong = blri.localInfo.address };
        ffStrbufInitF(&device->address, "%02x:%02x:%02x:%02x:%02x:%02x",
            addr.rgBytes[0],
            addr.rgBytes[1],
            addr.rgBytes[2],
            addr.rgBytes[3],
            addr.rgBytes[4],
            addr.rgBytes[5]);

        device->lmpVersion = blri.radioInfo.lmpVersion;
        device->lmpSubversion = blri.radioInfo.lmpSubversion;
        ffStrbufInitStatic(&device->vendor, ffBluetoothRadioGetVendor(blri.radioInfo.mfg));
        device->enabled = true;
        device->connectable = ffBluetoothIsConnectable(hRadio);
        device->discoverable = ffBluetoothIsDiscoverable(hRadio);
    } while (ffBluetoothFindNextRadio(hFind, &hRadio));

    ffBluetoothFindRadioClose(hFind);

    return NULL;
}
