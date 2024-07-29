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
        uint16_t vendorId;
        if (ffDBusGetUint16(dbus, &dictIter, &vendorId))
            ffStrbufSetStatic(&device->vendor, ffBluetoothRadioGetVendor(vendorId));
    }
    else if(ffStrEquals(deviceProperty, "Version"))
    {
        uint8_t byte;
        if (ffDBusGetByte(dbus, &dictIter, &byte))
            device->lmpVersion = byte;
    }
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

    while(true)
    {
        detectBluetoothValue(dbus, &arrayIter, device);
        FF_DBUS_ITER_CONTINUE(dbus, &arrayIter);
    }
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

    while(true)
    {
        detectBluetoothProperty(dbus, &arrayIter, device);
        FF_DBUS_ITER_CONTINUE(dbus, &arrayIter);
    }

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

    while(true)
    {
        detectBluetoothObject(devices, dbus, &arrayIter);
        FF_DBUS_ITER_CONTINUE(dbus, &arrayIter);
    }
}

static const char* detectBluetooth(FFlist* devices)
{
    FFDBusData dbus;
    const char* error = ffDBusLoadData(DBUS_BUS_SYSTEM, &dbus);
    if(error)
        return error;

    DBusMessage* managedObjects = ffDBusGetMethodReply(&dbus, "org.bluez", "/", "org.freedesktop.DBus.ObjectManager", "GetManagedObjects");
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
        return "Fastfetch was compiled without DBus support";
    #endif
}
