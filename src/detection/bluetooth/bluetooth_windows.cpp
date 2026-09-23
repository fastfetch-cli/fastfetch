extern "C" {
#include "bluetooth.h"
#include "common/mallocHelper.h"
#include "common/percent.h"
#include "common/strutil.h"
}

#if FF_HAVE_WINRT

    #include "common/windows/winrt.hpp"

    #define INITGUID
    #include <windows.h>
    #include <cfgmgr32.h>
    #include <devpkey.h>

    #include <winrt/Windows.Devices.Bluetooth.h>
    #include <winrt/Windows.Devices.Enumeration.h>

// Same definition as the one in bluetooth_windows.c; both are `selectany` and identical, so the
// linker keeps either. It is not in the SDK headers, hence the hand-written GUID.
/* DEVPROP_TYPE_STRING */
DEFINE_DEVPROPKEY(DEVPKEY_Bluetooth_DeviceAddress, 0x2bd67d8b, 0x8beb, 0x48d5, 0x87, 0xe0, 0x6c, 0xda, 0x34, 0x28, 0x04, 0x0a, 1);

#define GUID_DEVCLASS_BLUETOOTH_STRING L"{e0cbf06c-cd8b-4647-bb8a-263b43f0f974}" // Found in <devguid.h>

using FFBluetoothDeviceInformation = winrt::Windows::Devices::Enumeration::DeviceInformation;
using FFBluetoothDeviceInformationItf = abi_t<winrt::Windows::Devices::Enumeration::IDeviceInformation>;
using FFBluetoothDeviceList = abi_t<winrt::Windows::Foundation::Collections::IVectorView<FFBluetoothDeviceInformation>>;
using FFBluetoothProperties = abi_t<winrt::Windows::Foundation::Collections::IMapView<winrt::hstring, winrt::Windows::Foundation::IInspectable>>;
using FFBluetoothPropertyValue = abi_t<winrt::Windows::Foundation::IPropertyValue>;

// The AEP properties this file asks for. `FindAllAsync` rejects the whole call with
// `0x8002802B` (property key syntax error) if any name is not canonical, so every one of these was
// checked against the fast selector on a machine with paired LE devices; the ones that look
// plausible but do not exist (`...Bluetooth.Cod.Services`, `...Bluetooth.LastConnectedTime`,
// `...Bluetooth.Le.DeviceAddress`) are deliberately absent.
static const wchar_t* const ffBluetoothLeProperties[] = {
    L"System.Devices.Aep.IsConnected",
    L"System.Devices.Aep.SignalStrength",
    L"System.Devices.Aep.DeviceAddress",
    L"System.Devices.Aep.ContainerId",
    L"System.Devices.Aep.Bluetooth.Le.Appearance",
};

// ---------------------------------------------------------------------------------------------
// Reading the AEP property store
// ---------------------------------------------------------------------------------------------

// Every value in `DeviceInformation.Properties()` is boxed, and the one box that can be read back
// without knowing the concrete type is the property-value interface. So: ask the map whether the key
// is there, then ask the box for the type that key is documented to carry.
//
// "The device does not have it" is an ordinary answer rather than a failure -- an endpoint that has
// never been in range has no SignalStrength, and one whose firmware leaves the appearance at its
// default reports 0 instead of omitting the key. The caller owns the returned value.
static FFBluetoothPropertyValue* ffBluetoothLeLookup(FFBluetoothProperties* properties, const wchar_t* key) {
    HSTRING_HEADER header;
    HSTRING name;
    if (FAILED(WindowsCreateStringReference(key, (UINT32) ::wcslen(key), &header, &name))) {
        return nullptr;
    }

    bool present = false; // `Lookup` on a missing key is an error, so ask first
    if (FAILED(properties->HasKey(reinterpret_cast<void*>(name), &present)) || !present) {
        return nullptr;
    }

    FF_AUTO_RELEASE_COM_OBJECT abi_t<winrt::Windows::Foundation::IInspectable>* boxed = nullptr;
    if (FAILED(properties->Lookup(reinterpret_cast<void*>(name), reinterpret_cast<void**>(&boxed))) || !boxed) {
        return nullptr;
    }

    FFBluetoothPropertyValue* value = nullptr;
    if (FAILED(ffQueryInterface<winrt::Windows::Foundation::IPropertyValue>(boxed, &value))) {
        return nullptr;
    }

    return value;
}

static bool ffBluetoothLeGetBool(FFBluetoothProperties* properties, const wchar_t* key, bool* result) {
    FF_AUTO_RELEASE_COM_OBJECT FFBluetoothPropertyValue* value = ffBluetoothLeLookup(properties, key);
    return value && SUCCEEDED(value->GetBoolean(result));
}

static bool ffBluetoothLeGetInt32(FFBluetoothProperties* properties, const wchar_t* key, int32_t* result) {
    FF_AUTO_RELEASE_COM_OBJECT FFBluetoothPropertyValue* value = ffBluetoothLeLookup(properties, key);
    return value && SUCCEEDED(value->GetInt32(result));
}

static bool ffBluetoothLeGetUInt16(FFBluetoothProperties* properties, const wchar_t* key, uint16_t* result) {
    FF_AUTO_RELEASE_COM_OBJECT FFBluetoothPropertyValue* value = ffBluetoothLeLookup(properties, key);
    return value && SUCCEEDED(value->GetUInt16(result));
}

static bool ffBluetoothLeGetGuid(FFBluetoothProperties* properties, const wchar_t* key, GUID* result) {
    FF_AUTO_RELEASE_COM_OBJECT FFBluetoothPropertyValue* value = ffBluetoothLeLookup(properties, key);
    // `winrt::guid` and `GUID` have the same four members in the same order, and C++/WinRT itself
    // converts between them with exactly this cast.
    return value && SUCCEEDED(value->GetGuid(reinterpret_cast<winrt::guid*>(result)));
}

// The caller owns `result`.
static bool ffBluetoothLeGetString(FFBluetoothProperties* properties, const wchar_t* key, HSTRING* result) {
    FF_AUTO_RELEASE_COM_OBJECT FFBluetoothPropertyValue* value = ffBluetoothLeLookup(properties, key);
    return value && SUCCEEDED(value->GetString(reinterpret_cast<void**>(result)));
}

// ---------------------------------------------------------------------------------------------
// The device tree: which addresses belong to the same physical device
// ---------------------------------------------------------------------------------------------

// A Bluetooth device is one container with one node per stack it speaks, and the two nodes carry
// *different* addresses: the phone is FC:AB:D0:72:40:90 over BR/EDR and 47:E1:D7:21:C7:FC over LE,
// both inside {B3F019F1-...}. The container is therefore the only identity the classic and the LE
// enumerations can be joined on, and the device tree is where the classic side gets it from.
typedef struct FFBluetoothNodeInfo {
    char address[13]; // 12 hex digits + terminator, upper case, no separators
    GUID container;
} FFBluetoothNodeInfo;

static void ffBluetoothCollectNodes(FFlist* nodes /* FFBluetoothNodeInfo */) {
    ULONG idListLength = 0;
    if (CM_Get_Device_ID_List_SizeW(&idListLength, GUID_DEVCLASS_BLUETOOTH_STRING, CM_GETIDLIST_FILTER_CLASS | CM_GETIDLIST_FILTER_PRESENT) != CR_SUCCESS || idListLength == 0) {
        return;
    }

    FF_AUTO_FREE wchar_t* idList = (wchar_t*) malloc((size_t) idListLength * sizeof(wchar_t));
    if (!idList) {
        return;
    }

    if (CM_Get_Device_ID_ListW(GUID_DEVCLASS_BLUETOOTH_STRING, idList, idListLength, CM_GETIDLIST_FILTER_CLASS | CM_GETIDLIST_FILTER_PRESENT) != CR_SUCCESS) {
        return;
    }

    for (const wchar_t* deviceId = idList; *deviceId; deviceId += wcslen(deviceId) + 1) {
        DEVINST devInst = 0;
        if (CM_Locate_DevNodeW(&devInst, (DEVINSTID_W) deviceId, CM_LOCATE_DEVNODE_NORMAL) != CR_SUCCESS) {
            continue;
        }

        FFBluetoothNodeInfo node = {};

        {
            // Both enumerators carry this: BTHENUM nodes spell the address in upper case, BTHLE nodes
            // in lower case, so it is folded here and the comparison later is plain byte equality.
            wchar_t address[13];
            DEVPROPTYPE type = DEVPROP_TYPE_EMPTY;
            ULONG size = sizeof(address);
            if (CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_Bluetooth_DeviceAddress, &type, (PBYTE) address, &size, 0) != CR_SUCCESS || type != DEVPROP_TYPE_STRING || size != sizeof(address)) {
                continue;
            }

            for (size_t i = 0; i < 12; ++i) {
                node.address[i] = (char) toupper((unsigned char) address[i]);
            }
        }

        {
            DEVPROPTYPE type = DEVPROP_TYPE_EMPTY;
            ULONG size = sizeof(node.container);
            if (CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_Device_ContainerId, &type, (PBYTE) &node.container, &size, 0) != CR_SUCCESS || type != DEVPROP_TYPE_GUID || size != sizeof(node.container)) {
                continue;
            }
        }

        *FF_LIST_ADD(FFBluetoothNodeInfo, *nodes) = node;
    }
}

// `address` is the printed form ("AA:BB:CC:DD:EE:FF"); the device tree stores it without separators.
static bool ffBluetoothFindContainer(const FFlist* nodes, const FFstrbuf* address, GUID* container) {
    if (address->length != 17) {
        return false;
    }

    char plain[13];
    for (uint32_t i = 0; i < 6; ++i) {
        plain[i * 2] = (char) toupper((unsigned char) address->chars[i * 3]);
        plain[i * 2 + 1] = (char) toupper((unsigned char) address->chars[i * 3 + 1]);
    }
    plain[12] = '\0';

    FF_LIST_FOR_EACH (FFBluetoothNodeInfo, node, *nodes) {
        if (memcmp(node->address, plain, sizeof(plain)) == 0) {
            *container = node->container;
            return true;
        }
    }

    return false;
}

// ---------------------------------------------------------------------------------------------
// The appearance, and the device it describes
// ---------------------------------------------------------------------------------------------

// The GAP appearance is a 16-bit value whose upper 10 bits are the category. Windows names only the
// first nineteen of them -- `BluetoothLEAppearanceCategories` stops at Cycling (0x12) and has no
// AudioSink -- so these names are the Bluetooth SIG ones, checked against that class where it has an
// entry and against `BluetoothLEAppearance::FromRawValue()` for the rest: a pair of earbuds reports
// 0x0842, which the OS itself decodes as category 0x21, the value used below for Audio Sink.
static const char* ffBluetoothLeAppearanceName(uint16_t appearance) {
    switch (appearance >> 6) {
        case 0x00: return "Unknown";
        case 0x01: return "Phone";
        case 0x02: return "Computer";
        case 0x03: return "Watch";
        case 0x04: return "Clock";
        case 0x05: return "Display";
        case 0x06: return "Remote Control";
        case 0x07: return "Eye Glasses";
        case 0x08: return "Tag";
        case 0x09: return "Keyring";
        case 0x0A: return "Media Player";
        case 0x0B: return "Barcode Scanner";
        case 0x0C: return "Thermometer";
        case 0x0D: return "Heart Rate Sensor";
        case 0x0E: return "Blood Pressure";
        case 0x0F: return "Human Interface Device";
        case 0x10: return "Glucose Meter";
        case 0x11: return "Running Walking Sensor";
        case 0x12: return "Cycling";
        case 0x13: return "Control Device";
        case 0x14: return "Network Device";
        case 0x15: return "Sensor";
        case 0x16: return "Light Fixture";
        case 0x17: return "Fan";
        case 0x18: return "HVAC";
        case 0x19: return "Air Conditioning";
        case 0x1A: return "Humidifier";
        case 0x1B: return "Heating";
        case 0x1C: return "Access Control";
        case 0x1D: return "Motorized Device";
        case 0x1E: return "Power Device";
        case 0x1F: return "Light Source";
        case 0x20: return "Window Covering";
        case 0x21: return "Audio Sink";
        case 0x22: return "Audio Source";
        case 0x23: return "Motorized Vehicle";
        case 0x24: return "Domestic Appliance";
        case 0x25: return "Wearable Audio Device";
        case 0x26: return "Aircraft";
        case 0x27: return "AV Equipment";
        case 0x28: return "Display Equipment";
        case 0x29: return "Hearing Aid";
        case 0x2A: return "Gaming";
        case 0x2B: return "Signage";
        default: return "Unknown";
    }
}

// Which entry of `devices` each classic device occupies, and which container it belongs to. The
// index is stored rather than a pointer because appending an LE-only device can move the list.
typedef struct FFBluetoothClassicEntry {
    uint32_t index;
    bool hasContainer;
    GUID container;
} FFBluetoothClassicEntry;

// ---------------------------------------------------------------------------------------------
// The LE stack
// ---------------------------------------------------------------------------------------------

// `devices` already holds everything the classic (BR/EDR) stack knows. This adds what only the LE
// stack knows: devices that never speak BR/EDR at all, and -- for the ones that speak both -- the
// fact that they do, plus the signal strength the LE link is the only source of.
//
// The way in matters. `BluetoothLEDevice::GetDeviceSelectorFromPairingState(true)` as an AQS filter
// answers in 9-22 ms and brings the AEP properties along, whereas enumerating
// `DeviceInformationKind::AssociationEndpoint` returns the same data but costs about 61 seconds per
// call because it scans the radio. The 7.7-second trap is one layer further out: any `Uncached` GATT
// call, which the cache-backed service table hides by returning in a few milliseconds.
extern "C" const char* ffBluetoothDetectLe(FFBluetoothOptions* options, FFlist* devices /* FFBluetoothResult */) {
    const char* error = ffInitCom();
    if (error) {
        return error;
    }

    FF_AUTO_RELEASE_COM_OBJECT abi_t<winrt::Windows::Devices::Bluetooth::IBluetoothLEDeviceStatics2>* ledeviceStatics = nullptr;
    if (FAILED(ffGetActivationFactory<winrt::Windows::Devices::Bluetooth::IBluetoothLEDeviceStatics2>(L"Windows.Devices.Bluetooth.BluetoothLEDevice", &ledeviceStatics)) || !ledeviceStatics) {
        return "winrt: RoGetActivationFactory(BluetoothLEDevice) failed";
    }

    [[gnu::cleanup(ffDeleteHstring)]] HSTRING selector = nullptr;
    if (FAILED(ledeviceStatics->GetDeviceSelectorFromPairingState(true, reinterpret_cast<void**>(&selector))) || !selector) {
        return "winrt: GetDeviceSelectorFromPairingState() failed";
    }

    // Without additional properties the map carries only the eight generic ones, and every AEP key
    // is absent, so the list is not optional.
    FF_AUTO_RELEASE_COM_OBJECT abi_t<winrt::Windows::Foundation::Collections::IIterable<winrt::hstring>>* properties = nullptr;
    if (FAILED(ffWinrtCreateHstringIterable(ffBluetoothLeProperties, (uint32_t) ARRAY_SIZE(ffBluetoothLeProperties), &properties)) || !properties) {
        return "winrt: building the additional-properties list failed";
    }

    FF_AUTO_RELEASE_COM_OBJECT abi_t<winrt::Windows::Devices::Enumeration::IDeviceInformationStatics>* enumerationStatics = nullptr;
    if (FAILED(ffGetActivationFactory<winrt::Windows::Devices::Enumeration::IDeviceInformationStatics>(L"Windows.Devices.Enumeration.DeviceInformation", &enumerationStatics)) || !enumerationStatics) {
        return "winrt: RoGetActivationFactory(DeviceInformation) failed";
    }

    FF_AUTO_RELEASE_COM_OBJECT FFBluetoothDeviceList* found = nullptr;
    HRESULT hr = ffRunAndWait<winrt::Windows::Foundation::Collections::IVectorView<FFBluetoothDeviceInformation>>([&](void** result) {
        return enumerationStatics->FindAllAsyncAqsFilterAndAdditionalProperties(reinterpret_cast<void*>(selector), reinterpret_cast<void*>(properties), result);
    },
        &found);
    if (FAILED(hr) || !found) {
        return "winrt: DeviceInformation::FindAllAsync() failed";
    }

    uint32_t count = 0;
    if (FAILED(found->get_Size(&count)) || count == 0) {
        return nullptr;
    }

    FF_LIST_AUTO_DESTROY nodes = ffListCreate();
    ffBluetoothCollectNodes(&nodes);

    FF_LIST_AUTO_DESTROY classic = ffListCreate();
    {
        uint32_t index = 0;
        FF_LIST_FOR_EACH (FFBluetoothResult, device, *devices) {
            FFBluetoothClassicEntry* entry = FF_LIST_ADD(FFBluetoothClassicEntry, classic);
            entry->index = index++;
            entry->hasContainer = ffBluetoothFindContainer(&nodes, &device->address, &entry->container);
        }
    }

    for (uint32_t i = 0; i < count; ++i) {
        FF_AUTO_RELEASE_COM_OBJECT FFBluetoothDeviceInformationItf* info = nullptr;
        if (FAILED(found->GetAt(i, reinterpret_cast<void**>(&info))) || !info) {
            continue;
        }

        FF_AUTO_RELEASE_COM_OBJECT FFBluetoothProperties* map = nullptr;
        if (FAILED(info->get_Properties(reinterpret_cast<void**>(&map))) || !map) {
            continue;
        }

        GUID container = {};
        if (!ffBluetoothLeGetGuid(map, L"System.Devices.Aep.ContainerId", &container)) {
            continue; // Without a container there is no way to tell whether this is a device we know
        }

        bool connected = false;
        ffBluetoothLeGetBool(map, L"System.Devices.Aep.IsConnected", &connected);

        FFBluetoothResult* target = nullptr;
        FF_LIST_FOR_EACH (FFBluetoothClassicEntry, entry, classic) {
            if (entry->hasContainer && IsEqualGUID(entry->container, container)) {
                target = FF_LIST_GET(FFBluetoothResult, *devices, entry->index);
                break;
            }
        }

        if (!target && !connected && !options->showDisconnected) {
            continue; // An LE-only device nobody asked to see. A known one is already in the list.
        }

        if (!target) {
            target = FF_LIST_ADD(FFBluetoothResult, *devices);
            ffStrbufInit(&target->name);
            ffStrbufInit(&target->address);
            ffStrbufInit(&target->type);
            target->deviceType = FF_BLUETOOTH_DEVICE_TYPE_NONE;
            target->battery = 0;
            target->signalQuality = -DBL_MAX;
            target->connected = false;
        }

        target->deviceType = (FFBluetoothDeviceType) (target->deviceType | FF_BLUETOOTH_DEVICE_TYPE_LE_BIT);

        // The classic half wins wherever it has an answer: its name and address are the ones the
        // Settings app shows, and its type comes from the class of device rather than from an
        // appearance value the firmware may never have set.
        if (target->name.length == 0) {
            [[gnu::cleanup(ffDeleteHstring)]] HSTRING name = nullptr;
            if (SUCCEEDED(info->get_Name(reinterpret_cast<void**>(&name))) && name) {
                ffStrbufSetHstring(&target->name, name);
            }
            ffStrbufTrimSpace(&target->name); // A device whose name is missing is reported as a space
        }
        if (target->name.length == 0) {
            ffStrbufSetStatic(&target->name, "Unknown Device");
        }

        if (target->address.length == 0) {
            [[gnu::cleanup(ffDeleteHstring)]] HSTRING address = nullptr;
            if (ffBluetoothLeGetString(map, L"System.Devices.Aep.DeviceAddress", &address)) {
                ffStrbufSetHstring(&target->address, address);
                ffStrbufUpperCase(&target->address);
            }
        }

        if (target->type.length == 0) {
            uint16_t appearance = 0;
            if (ffBluetoothLeGetUInt16(map, L"System.Devices.Aep.Bluetooth.Le.Appearance", &appearance)) {
                ffStrbufSetStatic(&target->type, ffBluetoothLeAppearanceName(appearance));
            }
        }

        // The signal strength is what the LE stack last saw, and it stays readable while the link is
        // down, so it is reported whenever the endpoint has one. Only the LE side can produce it.
        int32_t signalStrength = 0;
        if (ffBluetoothLeGetInt32(map, L"System.Devices.Aep.SignalStrength", &signalStrength)) {
            target->signalQuality = ffRssiToSignalQuality(signalStrength);
        }

        // A dual-mode device is connected if either of its two links is up, and the two can disagree.
        target->connected |= connected;
    }

    return nullptr;
}

#else

extern "C" const char* ffBluetoothDetectLe(FFBluetoothOptions* options, FFlist* devices /* FFBluetoothResult */) {
    FF_UNUSED(options, devices);
    return nullptr;
}

#endif // FF_HAVE_WINRT
