#include "bluetooth.h"
#include "common/percent.h"
#include "common/strutil.h"

#ifdef FF_HAVE_DBUS
    #include "common/dbus.h"
    #include "common/io.h"

/* Example dbus reply, striped to only the relevant parts:
array [                                                     //root
    dict entry(                                             //object
        object path "/org/bluez/hci0/dev_03_21_8B_91_16_4D"
        array [
           dict entry(                                      //property
              string "org.bluez.Device1"
              array [
                 dict entry(                                //value
                    string "Address"
                    variant string "03:21:8B:91:16:4D"
                 )
                 dict entry(                                //value
                    string "Name"
                    variant string "JBL TUNE160BT"
                 )
                 dict entry(                                //value
                    string "Icon"
                    variant string "audio-headset"
                 )
                 dict entry(                                //value
                    string "Connected"
                    variant boolean true
                 )
                 dict entry(                                //value
                    string "RSSI"
                    variant int16 -63
                 )
              ]
           )
           dict entry(                                      //property
              string "org.bluez.Battery1"
              array [
                 dict entry(                                //value
                    string "Percentage"
                    variant byte 100
                 )
              ]
           )
        ]
    )
]
*/

/* How the two stack bits are inferred here, and why the LE one is mostly unattainable.
 *
 * `org.bluez.Device1` has no "does this device support LE" property, so both bits are read off
 * the only two stack-specific ones it has:
 *
 *   Class      - BR/EDR Class of Device, learned from BR/EDR EIR / inquiry data
 *                (BlueZ src/eir.c, EIR_CLASS_OF_DEV) and persisted in the device file.
 *   Appearance - GATT appearance, learned from LE advertising data only
 *                (BlueZ src/adapter.c -> src/eir.c, EIR_GAP_APPEARANCE, AD type 0x19).
 *
 * For a dual-mode device only the first one ever shows up, for two independent reasons:
 *
 * 1. BlueZ never publishes `Appearance` for a device that has a `Class`. Its getter short
 *    circuits on it, and the property table registers that getter as the `exists` callback,
 *    so the key is absent from `GetManagedObjects` altogether rather than present with a
 *    default value:
 *
 *        static gboolean get_appearance(...)
 *        {
 *            if (dev_property_exists_class(property, data))   // device->class != 0
 *                return FALSE;
 *            if (device->appearance) { ... return TRUE; }
 *            return FALSE;
 *        }
 *
 *    `btd_device_get_icon()` prefers `Class` the same way, which is why `Icon` describes a
 *    classic-seen device by its CoD. BlueZ treats the two as mutually exclusive descriptions
 *    of the device's *category*, not as a record of which radios it speaks. Checked against
 *    BlueZ 5.87, src/device.c.
 *
 * 2. Even without (1) the value is usually unknown: it is only learned if the device
 *    advertised AD type 0x19 while we were scanning (phones rarely do), a zero is never
 *    persisted (`/var/lib/bluetooth/<adapter>/<addr>/info` then has no `Appearance=` line),
 *    and that directory is mode 0700 so it could not be read here anyway.
 *
 * Consequence: a device ever seen over BR/EDR -- every paired phone, since all of them have a
 * CoD -- can only ever be reported as Classic, however LE capable it really is, and no amount
 * of extra data collected on this path can change that. Windows gets it right because it
 * enumerates the two stacks separately (bluetooth_windows.cpp walks the WinRT
 * BluetoothLEDevice endpoints and ORs in the LE bit); BlueZ has no equivalent enumeration --
 * one Device1 object per address, no per-stack capability -- and its own storage only records
 * `SupportedTechnologies=BR/EDR;LE` once a real LE connection or an LE sighting has happened.
 *
 * Signals that look like LE proof but are not, and are deliberately not used here:
 *   - GAP / GATT in `UUIDs` (0x1800 / 0x1801): BlueZ mirrors BR/EDR SDP records advertising
 *     the ATT bearer (L2CAP PSM 0x001F) into the same `attributes` cache it loads GATT
 *     primaries from, so those UUIDs can come from a purely classic SDP browse.
 *   - `AddressType`: a BR/EDR-only record reports "public" too -- `device->bdaddr_type` is
 *     forced to `BDADDR_BREDR` when the record is loaded.
 *   - `ManufacturerData` / `ServiceData`: BlueZ parses them out of BR/EDR EIR as well.
 *
 * The one durable LE-only signal BlueZ does publish, if this is ever revisited:
 *   - `AdvertisingFlags` exists once the device has been seen in an LE advertising report.
 *     It is written only from the `bdaddr_type != BDADDR_BREDR` branch of BlueZ's device-found
 *     handler, starts out invalid (0xff) and is never cleared, so it survives the end of a
 *     discovery session -- but it is not persisted, so a bluetoothd restart loses it again.
 *     `PreferredBearer` would be even better (it exists exactly when both bearers are known),
 *     yet it is flagged experimental and needs `Experimental=true` to be exported at all.
 */

static bool detectBluetoothValue(FFDBusData* dbus, DBusMessageIter* iter, FFBluetoothResult* device) {
    if (dbus->lib->ffdbus_message_iter_get_arg_type(iter) != DBUS_TYPE_DICT_ENTRY) {
        return true;
    }

    DBusMessageIter dictIter;
    dbus->lib->ffdbus_message_iter_recurse(iter, &dictIter);

    if (dbus->lib->ffdbus_message_iter_get_arg_type(&dictIter) != DBUS_TYPE_STRING) {
        return true;
    }

    const char* deviceProperty;
    dbus->lib->ffdbus_message_iter_get_basic(&dictIter, &deviceProperty);

    dbus->lib->ffdbus_message_iter_next(&dictIter);

    if (ffStrEquals(deviceProperty, "Address")) {
        ffDBusGetString(dbus, &dictIter, &device->address);
    } else if (ffStrEquals(deviceProperty, "Name")) {
        ffDBusGetString(dbus, &dictIter, &device->name);
    } else if (ffStrEquals(deviceProperty, "Icon")) {
        // BlueZ derives this from the Class of Device whenever there is one and only falls back
        // to the LE appearance otherwise, so `type` describes a classic-seen device by its CoD.
        ffDBusGetString(dbus, &dictIter, &device->type);
    } else if (ffStrEquals(deviceProperty, "Class")) {
        // The BR/EDR Class of Device. Published iff BlueZ ever learned a CoD, which comes from
        // BR/EDR EIR / inquiry data, so this means "seen over BR/EDR" -- not "classic only".
        // Its mere presence is also what hides `Appearance` in the branch below.
        device->deviceType |= FF_BLUETOOTH_DEVICE_TYPE_CLASSIC_BIT;
    } else if (ffStrEquals(deviceProperty, "Appearance")) {
        // The LE-only GATT appearance. Absence does *not* mean "not LE": BlueZ omits this
        // property entirely for every device that has a `Class` (see the block comment above),
        // and knows the value only if the device advertised AD type 0x19 while we were scanning.
        // The two properties are mutually exclusive in BlueZ's view, so a dual-mode phone that
        // was ever seen classically reports Classic only, whatever it is capable of.
        device->deviceType |= FF_BLUETOOTH_DEVICE_TYPE_LE_BIT;
    } else if (ffStrEquals(deviceProperty, "Percentage")) {
        uint64_t percentage;
        if (ffDBusGetUint(dbus, &dictIter, &percentage)) {
            device->battery = (uint8_t) percentage;
        }
    } else if (ffStrEquals(deviceProperty, "Connected")) {
        ffDBusGetBool(dbus, &dictIter, &device->connected);
    } else if (ffStrEquals(deviceProperty, "RSSI")) {
        // `int16` on the wire, in dBm. Written only from discovery reports and invalidated when
        // the discovery session ends, so it says nothing about the connection state: a connected
        // device that has not been scanned lately reports no RSSI at all.
        int64_t rssi;
        if (ffDBusGetInt(dbus, &dictIter, &rssi)) {
            device->signalQuality = ffRssiToSignalQuality((int) rssi);
        }
    } else if (ffStrEquals(deviceProperty, "Paired")) {
        bool paired = true;
        ffDBusGetBool(dbus, &dictIter, &paired);
        if (!paired) {
            return false;
        }
    }
    return true;
}

static void detectBluetoothProperty(FFDBusData* dbus, DBusMessageIter* iter, FFBluetoothResult* device) {
    if (dbus->lib->ffdbus_message_iter_get_arg_type(iter) != DBUS_TYPE_DICT_ENTRY) {
        return;
    }

    DBusMessageIter dictIter;
    dbus->lib->ffdbus_message_iter_recurse(iter, &dictIter);

    if (dbus->lib->ffdbus_message_iter_get_arg_type(&dictIter) != DBUS_TYPE_STRING) {
        return;
    }

    const char* propertyType;
    dbus->lib->ffdbus_message_iter_get_basic(&dictIter, &propertyType);

    if (!ffStrContains(propertyType, ".Device") && !ffStrContains(propertyType, ".Battery")) {
        return; // We don't care about other properties
    }

    dbus->lib->ffdbus_message_iter_next(&dictIter);

    if (dbus->lib->ffdbus_message_iter_get_arg_type(&dictIter) != DBUS_TYPE_ARRAY) {
        return;
    }

    DBusMessageIter arrayIter;
    dbus->lib->ffdbus_message_iter_recurse(&dictIter, &arrayIter);

    do {
        bool shouldContinue = detectBluetoothValue(dbus, &arrayIter, device);
        if (!shouldContinue) {
            ffStrbufClear(&device->name);
            break;
        }
    } while (dbus->lib->ffdbus_message_iter_next(&arrayIter));
}

static FFBluetoothResult* detectBluetoothObject(FFlist* devices, FFDBusData* dbus, DBusMessageIter* iter) {
    if (dbus->lib->ffdbus_message_iter_get_arg_type(iter) != DBUS_TYPE_DICT_ENTRY) {
        return nullptr;
    }

    DBusMessageIter dictIter;
    dbus->lib->ffdbus_message_iter_recurse(iter, &dictIter);

    if (dbus->lib->ffdbus_message_iter_get_arg_type(&dictIter) != DBUS_TYPE_OBJECT_PATH) {
        return nullptr;
    }

    const char* objectPath;
    dbus->lib->ffdbus_message_iter_get_basic(&dictIter, &objectPath);

    // We don't want adapter objects
    if (!ffStrContains(objectPath, "/dev_")) {
        return nullptr;
    }

    dbus->lib->ffdbus_message_iter_next(&dictIter);

    if (dbus->lib->ffdbus_message_iter_get_arg_type(&dictIter) != DBUS_TYPE_ARRAY) {
        return nullptr;
    }

    DBusMessageIter arrayIter;
    dbus->lib->ffdbus_message_iter_recurse(&dictIter, &arrayIter);

    FFBluetoothResult* device = FF_LIST_ADD(FFBluetoothResult, *devices);
    ffStrbufInit(&device->name);
    ffStrbufInit(&device->address);
    ffStrbufInit(&device->type);
    device->deviceType = FF_BLUETOOTH_DEVICE_TYPE_NONE;
    device->battery = 0;
    device->signalQuality = -DBL_MAX;
    device->connected = false;

    do {
        detectBluetoothProperty(dbus, &arrayIter, device);
    } while (dbus->lib->ffdbus_message_iter_next(&arrayIter));

    return device;
}

static void detectBluetoothRoot(FFBluetoothOptions* options, FFlist* devices, FFDBusData* dbus, DBusMessageIter* iter, int32_t connectedCount) {
    if (dbus->lib->ffdbus_message_iter_get_arg_type(iter) != DBUS_TYPE_ARRAY) {
        return;
    }

    DBusMessageIter arrayIter;
    dbus->lib->ffdbus_message_iter_recurse(iter, &arrayIter);

    do {
        FFBluetoothResult* device = detectBluetoothObject(devices, dbus, &arrayIter);

        if (device) {
            // `showType` selects which stacks are reported. Unlike the other backends there is no
            // per-stack call to skip here: both bits are read off this one walk of Device1, so the
            // filter lands on the result instead. That filter is necessarily one sided -- see the
            // block comment above detectBluetoothValue(): `Class` and `Appearance` are mutually
            // exclusive in BlueZ, so a dual-mode device that was ever seen classically carries only
            // the classic bit, and `showType: le` drops it even though it does speak LE. A device
            // whose stack could not be determined at all is kept -- BlueZ promises neither property,
            // and dropping those would lose devices the default configuration reports.
            bool deviceTypeWanted = device->deviceType == FF_BLUETOOTH_DEVICE_TYPE_NONE || (device->deviceType & options->showType);

            if (!deviceTypeWanted || (!options->showDisconnected && !device->connected)) {
                ffStrbufDestroy(&device->name);
                ffStrbufDestroy(&device->address);
                ffStrbufDestroy(&device->type);
                --devices->length;
                continue;
            }

            if (device->name.length == 0) {
                ffStrbufSetStatic(&device->name, "Unknown Device");
            }

            if (device->connected && --connectedCount == 0) {
                break;
            }
        }
    } while (dbus->lib->ffdbus_message_iter_next(&arrayIter));
}

static const char* detectBluetooth(FFBluetoothOptions* options, FFlist* devices, int32_t connectedCount) {
    FF_DBUS_AUTO_DESTROY_DATA FFDBusData dbus = {};
    const char* error = ffDBusLoadData(DBUS_BUS_SYSTEM, &dbus);
    if (error) {
        return error;
    }

    DBusMessage* managedObjects = ffDBusGetMethodReply(&dbus, "org.bluez", "/", "org.freedesktop.DBus.ObjectManager", "GetManagedObjects", nullptr, nullptr);
    if (!managedObjects) {
        return "Failed to call GetManagedObjects";
    }

    DBusMessageIter rootIter;
    if (!dbus.lib->ffdbus_message_iter_init(managedObjects, &rootIter)) {
        dbus.lib->ffdbus_message_unref(managedObjects);
        return "Failed to get root iterator of GetManagedObjects";
    }

    detectBluetoothRoot(options, devices, &dbus, &rootIter, connectedCount);

    dbus.lib->ffdbus_message_unref(managedObjects);
    return nullptr;
}

static uint32_t connectedDevices(void) {
    FF_AUTO_CLOSE_DIR DIR* dirp = opendir("/sys/class/bluetooth");
    if (dirp == nullptr) {
        return 0;
    }

    uint32_t result = 0;
    struct dirent* entry;
    while ((entry = readdir(dirp)) != nullptr) {
        if (strchr(entry->d_name, ':') != nullptr) {
            ++result;
        }
    }

    return result;
}

#endif

const char* ffDetectBluetooth(FFBluetoothOptions* options, FFlist* devices /* FFBluetoothResult */) {
#ifdef FF_HAVE_DBUS
    int32_t connectedCount = -1;
    if (!options->showDisconnected) {
        connectedCount = (int32_t) connectedDevices();
        if (connectedCount == 0) {
            return nullptr;
        }
    }

    return detectBluetooth(options, devices, connectedCount);
#else
    FF_UNUSED(options, devices);
    return "Fastfetch was compiled without DBus support";
#endif
}
