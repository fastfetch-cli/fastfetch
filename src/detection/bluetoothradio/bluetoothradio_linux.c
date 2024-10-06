#include "bluetoothradio.h"
#include "util/stringUtils.h"

#ifdef FF_HAVE_DBUS
#include "common/dbus.h"

/* Example dbus reply, striped to only the relevant parts:
array [                                                     //root
    dict entry(
        object path "/org/bluez/hci0"
        array [
            dict entry(
                string "org.bluez.Adapter1"
                array [
                    dict entry(
                        string "Address"
                        variant string "XX:XX:XX:XX:XX:XX"
                    )
                    dict entry(
                        string "Name"
                        variant string "xxxxxxxx"
                    )
                    dict entry(
                        string "Powered"
                        variant boolean true
                    )
                    dict entry(
                        string "PowerState"
                        variant string "on"
                    )
                    dict entry(
                        string "Manufacturer"
                        variant uint16 2
                    )
                    dict entry(
                        string "Version"
                        variant byte 12
                    )
                ]
            )
        ]
    )
]
*/

static void detectBluetoothValue(FFDBusData* dbus, DBusMessageIter* iter, FFBluetoothRadioResult* device)
{
    if(dbus->lib->ffdbus_message_iter_get_arg_type(iter) != DBUS_TYPE_DICT_ENTRY)
        return;

    DBusMessageIter dictIter;
    dbus->lib->ffdbus_message_iter_recurse(iter, &dictIter);

    if(dbus->lib->ffdbus_message_iter_get_arg_type(&dictIter) != DBUS_TYPE_STRING)
        return;

    const char* deviceProperty;
    dbus->lib->ffdbus_message_iter_get_basic(&dictIter, &deviceProperty);

    dbus->lib->ffdbus_message_iter_next(&dictIter);

    if(ffStrEquals(deviceProperty, "Address"))
        ffDBusGetString(dbus, &dictIter, &device->address);
    else if(ffStrEquals(deviceProperty, "Alias"))
        ffDBusGetString(dbus, &dictIter, &device->name);
    else if(ffStrEquals(deviceProperty, "Manufacturer"))
    {
        uint32_t vendorId;
        if (ffDBusGetUint(dbus, &dictIter, &vendorId))
            ffStrbufSetStatic(&device->vendor, ffBluetoothRadioGetVendor(vendorId));
    }
    else if(ffStrEquals(deviceProperty, "Version"))
        ffDBusGetUint(dbus, &dictIter, (uint32_t*) &device->lmpVersion);
    else if(ffStrEquals(deviceProperty, "Powered"))
        ffDBusGetBool(dbus, &dictIter, &device->enabled);
    else if(ffStrEquals(deviceProperty, "Discoverable"))
        ffDBusGetBool(dbus, &dictIter, &device->discoverable);
    else if(ffStrEquals(deviceProperty, "Pairable"))
        ffDBusGetBool(dbus, &dictIter, &device->connectable);
}

static void detectBluetoothProperty(FFDBusData* dbus, DBusMessageIter* iter, FFBluetoothRadioResult* device)
{
    if(dbus->lib->ffdbus_message_iter_get_arg_type(iter) != DBUS_TYPE_DICT_ENTRY)
        return;

    DBusMessageIter dictIter;
    dbus->lib->ffdbus_message_iter_recurse(iter, &dictIter);

    if(dbus->lib->ffdbus_message_iter_get_arg_type(&dictIter) != DBUS_TYPE_STRING)
        return;

    const char* propertyType;
    dbus->lib->ffdbus_message_iter_get_basic(&dictIter, &propertyType);

    if(!ffStrContains(propertyType, ".Adapter"))
        return; //We don't care about other properties

    dbus->lib->ffdbus_message_iter_next(&dictIter);

    if(dbus->lib->ffdbus_message_iter_get_arg_type(&dictIter) != DBUS_TYPE_ARRAY)
        return;

    DBusMessageIter arrayIter;
    dbus->lib->ffdbus_message_iter_recurse(&dictIter, &arrayIter);

    do
    {
        detectBluetoothValue(dbus, &arrayIter, device);
    } while (dbus->lib->ffdbus_message_iter_next(&arrayIter));
}

static void detectBluetoothObject(FFlist* devices, FFDBusData* dbus, DBusMessageIter* iter)
{
    if(dbus->lib->ffdbus_message_iter_get_arg_type(iter) != DBUS_TYPE_DICT_ENTRY)
        return;

    DBusMessageIter dictIter;
    dbus->lib->ffdbus_message_iter_recurse(iter, &dictIter);

    if(dbus->lib->ffdbus_message_iter_get_arg_type(&dictIter) != DBUS_TYPE_OBJECT_PATH)
        return;

    const char* objectPath;
    dbus->lib->ffdbus_message_iter_get_basic(&dictIter, &objectPath);

    //We want adapter objects
    if(!ffStrStartsWith(objectPath, "/org/bluez/hci") || ffStrContains(objectPath, "/dev_"))
        return;

    dbus->lib->ffdbus_message_iter_next(&dictIter);

    if(dbus->lib->ffdbus_message_iter_get_arg_type(&dictIter) != DBUS_TYPE_ARRAY)
        return;

    DBusMessageIter arrayIter;
    dbus->lib->ffdbus_message_iter_recurse(&dictIter, &arrayIter);

    FFBluetoothRadioResult* device = ffListAdd(devices);
    ffStrbufInit(&device->name);
    ffStrbufInit(&device->address);
    ffStrbufInitStatic(&device->vendor, "Unknown");
    device->lmpVersion = INT_MIN;
    device->lmpSubversion = INT_MIN;
    device->enabled = false;

    do
    {
        detectBluetoothProperty(dbus, &arrayIter, device);
    } while (dbus->lib->ffdbus_message_iter_next(&arrayIter));

    if(device->name.length == 0)
    {
        ffStrbufDestroy(&device->name);
        ffStrbufDestroy(&device->address);
        --devices->length;
    }
}

static void detectBluetoothRoot(FFlist* devices, FFDBusData* dbus, DBusMessageIter* iter)
{
    if(dbus->lib->ffdbus_message_iter_get_arg_type(iter) != DBUS_TYPE_ARRAY)
        return;

    DBusMessageIter arrayIter;
    dbus->lib->ffdbus_message_iter_recurse(iter, &arrayIter);

    do
    {
        detectBluetoothObject(devices, dbus, &arrayIter);
    } while (dbus->lib->ffdbus_message_iter_next(&arrayIter));
}

static const char* detectBluetooth(FFlist* devices)
{
    FFDBusData dbus;
    const char* error = ffDBusLoadData(DBUS_BUS_SYSTEM, &dbus);
    if(error)
        return error;

    DBusMessage* managedObjects = ffDBusGetMethodReply(&dbus, "org.bluez", "/", "org.freedesktop.DBus.ObjectManager", "GetManagedObjects", NULL);
    if(!managedObjects)
        return "Failed to call GetManagedObjects";

    DBusMessageIter rootIter;
    if(!dbus.lib->ffdbus_message_iter_init(managedObjects, &rootIter))
    {
        dbus.lib->ffdbus_message_unref(managedObjects);
        return "Failed to get root iterator of GetManagedObjects";
    }

    detectBluetoothRoot(devices, &dbus, &rootIter);

    dbus.lib->ffdbus_message_unref(managedObjects);
    return NULL;
}

#endif

const char* ffDetectBluetoothRadio(FFlist* devices /* FFBluetoothRadioResult */)
{
    #ifdef FF_HAVE_DBUS
        return detectBluetooth(devices);
    #else
        FF_UNUSED(devices) 
        return "Fastfetch was compiled without DBus support";
    #endif
}
