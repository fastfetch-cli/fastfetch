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
    // Local BTH_ADDR, class of defice, and radio name
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
    FF_LIBRARY_LOAD(bluetoothapis, NULL, "dlopen bthprops.cpl failed", "bthprops.cpl", 1)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(bluetoothapis, BluetoothFindFirstRadio)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(bluetoothapis, BluetoothFindNextRadio)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(bluetoothapis, BluetoothFindRadioClose)

    HANDLE hRadio = NULL;
    HBLUETOOTH_DEVICE_FIND hFind = ffBluetoothFindFirstRadio(&(BLUETOOTH_FIND_RADIO_PARAMS) {
        .dwSize = sizeof(BLUETOOTH_FIND_RADIO_PARAMS)
    }, &hRadio);
    if(!hFind)
    {
        if (GetLastError() == ERROR_NO_MORE_ITEMS)
            return "No Bluetooth radios found";
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
        device->hciVersion = blri.hciVersion;
        device->hciRevision = blri.hciRevision;
        device->leSupported = LMP_LE_SUPPORTED(blri.radioInfo.lmpSupportedFeatures);

        switch (blri.radioInfo.mfg)
        {
            case BTH_MFG_ERICSSON: device->vendor = "Ericsson"; break;
            case BTH_MFG_NOKIA: device->vendor = "Nokia"; break;
            case BTH_MFG_INTEL: device->vendor = "Intel"; break;
            case BTH_MFG_IBM: device->vendor = "IBM"; break;
            case BTH_MFG_TOSHIBA: device->vendor = "Toshiba"; break;
            case BTH_MFG_3COM: device->vendor = "3Com"; break;
            case BTH_MFG_MICROSOFT: device->vendor = "Microsoft"; break;
            case BTH_MFG_LUCENT: device->vendor = "Lucent"; break;
            case BTH_MFG_MOTOROLA: device->vendor = "Motorola"; break;
            case BTH_MFG_INFINEON: device->vendor = "Infineon"; break;
            case BTH_MFG_CSR: device->vendor = "CSR"; break;
            case BTH_MFG_SILICONWAVE: device->vendor = "Silicon-Wave"; break;
            case BTH_MFG_DIGIANSWER: device->vendor = "Digi-Answer"; break;
            case BTH_MFG_TI: device->vendor = "Ti"; break;
            case BTH_MFG_PARTHUS: device->vendor = "Parthus"; break;
            case BTH_MFG_BROADCOM: device->vendor = "Broadcom"; break;
            case BTH_MFG_MITEL: device->vendor = "Mitel"; break;
            case BTH_MFG_WIDCOMM: device->vendor = "Widcomm"; break;
            case BTH_MFG_ZEEVO: device->vendor = "Zeevo"; break;
            case BTH_MFG_ATMEL: device->vendor = "Atmel"; break;
            case BTH_MFG_MITSIBUSHI: device->vendor = "Mitsubishi"; break;
            case BTH_MFG_RTX_TELECOM: device->vendor = "RTX Telecom"; break;
            case BTH_MFG_KC_TECHNOLOGY: device->vendor = "KC Technology"; break;
            case BTH_MFG_NEWLOGIC: device->vendor = "Newlogic"; break;
            case BTH_MFG_TRANSILICA: device->vendor = "Transilica"; break;
            case BTH_MFG_ROHDE_SCHWARZ: device->vendor = "Rohde-Schwarz"; break;
            case BTH_MFG_TTPCOM: device->vendor = "TTPCom"; break;
            case BTH_MFG_SIGNIA: device->vendor = "Signia"; break;
            case BTH_MFG_CONEXANT: device->vendor = "Conexant"; break;
            case BTH_MFG_QUALCOMM: device->vendor = "Qualcomm"; break;
            case BTH_MFG_INVENTEL: device->vendor = "Inventel"; break;
            case BTH_MFG_AVM_BERLIN: device->vendor = "AVM Berlin"; break;
            case BTH_MFG_BANDSPEED: device->vendor = "Bandspeed"; break;
            case BTH_MFG_MANSELLA: device->vendor = "Mansella"; break;
            case BTH_MFG_NEC: device->vendor = "NEC"; break;
            case BTH_MFG_WAVEPLUS_TECHNOLOGY_CO: device->vendor = "Waveplus"; break;
            case BTH_MFG_ALCATEL: device->vendor = "Alcatel"; break;
            case BTH_MFG_PHILIPS_SEMICONDUCTOR: device->vendor = "Philips Semiconductors"; break;
            case BTH_MFG_C_TECHNOLOGIES: device->vendor = "C Technologies"; break;
            case BTH_MFG_OPEN_INTERFACE: device->vendor = "Open Interface"; break;
            case BTH_MFG_RF_MICRO_DEVICES: device->vendor = "RF Micro Devices"; break;
            case BTH_MFG_HITACHI: device->vendor = "Hitachi"; break;
            case BTH_MFG_SYMBOL_TECHNOLOGIES: device->vendor = "Symbol Technologies"; break;
            case BTH_MFG_TENOVIS: device->vendor = "Tenovis"; break;
            case BTH_MFG_MACRONIX_INTERNATIONAL: device->vendor = "Macronix International"; break;
            case BTH_MFG_MARVELL: device->vendor = "Marvell"; break;
            case BTH_MFG_APPLE: device->vendor = "Apple"; break;
            case BTH_MFG_NORDIC_SEMICONDUCTORS_ASA: device->vendor = "Nordic Semiconductor ASA"; break;
            case BTH_MFG_ARUBA_NETWORKS: device->vendor = "Aruba Networks"; break;
            case BTH_MFG_INTERNAL_USE: device->vendor = "Internal Use"; break;
            default: device->vendor = "Unknown"; break;
        }
    } while (ffBluetoothFindNextRadio(hFind, &hRadio));

    ffBluetoothFindRadioClose(hFind);

    return NULL;
}
