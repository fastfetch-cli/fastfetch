#include "bluetoothradio.h"
#include "util/stringUtils.h"

#ifdef FF_HAVE_DBUS
#include "common/dbus.h"
#include "common/io/io.h"

/* Example dbus reply:
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
*/

static const char* detectBluetoothProperty(FFBluetoothRadioResult* device, FFDBusData* dbus, DBusMessageIter* iter)
{
    if(dbus->lib->ffdbus_message_iter_get_arg_type(iter) != DBUS_TYPE_DICT_ENTRY)
        return "Expected dict entry";

    DBusMessageIter dictIter;
    dbus->lib->ffdbus_message_iter_recurse(iter, &dictIter);

    if(dbus->lib->ffdbus_message_iter_get_arg_type(&dictIter) != DBUS_TYPE_STRING)
        return "Expected dict entry key to be a string";

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

    return NULL;
}

static const char* detectBluetoothRoot(FFBluetoothRadioResult* device, const char* hciName, FFDBusData* dbus)
{
    char objPath[300];
    snprintf(objPath, sizeof(objPath), "/org/bluez/%s", hciName);

    DBusMessage* properties = ffDBusGetMethodReply(dbus, "org.bluez", objPath, "org.freedesktop.DBus.Properties", "GetAll", "org.bluez.Adapter1", NULL);
    if(!properties)
        return "Failed to call org.freedesktop.DBus.Properties.GetAll";

    DBusMessageIter rootIter;
    if(!dbus->lib->ffdbus_message_iter_init(properties, &rootIter))
    {
        dbus->lib->ffdbus_message_unref(properties);
        return "Failed to get root iterator of org.freedesktop.DBus.Properties.GetAll";
    }

    if(dbus->lib->ffdbus_message_iter_get_arg_type(&rootIter) != DBUS_TYPE_ARRAY)
    {
        dbus->lib->ffdbus_message_unref(properties);
        return "Expected array";
    }

    DBusMessageIter arrayIter;
    dbus->lib->ffdbus_message_iter_recurse(&rootIter, &arrayIter);

    do
    {
        detectBluetoothProperty(device, dbus, &arrayIter);
    } while (dbus->lib->ffdbus_message_iter_next(&arrayIter));

    dbus->lib->ffdbus_message_unref(properties);
    return NULL;
}

static const char* detectBluetooth(FFlist* devices)
{
    FF_AUTO_CLOSE_DIR DIR* dirp = opendir("/sys/class/bluetooth");
    if(dirp == NULL)
        return "Failed to open /sys/class/bluetooth";

    FFDBusData dbus;
    const char* error = ffDBusLoadData(DBUS_BUS_SYSTEM, &dbus);
    if(error)
        return error;

    struct dirent* entry;
    while ((entry = readdir(dirp)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;

        if (strchr(entry->d_name, ':') != NULL) // ignore connected devices
            continue;

        FFBluetoothRadioResult* device = ffListAdd(devices);
        ffStrbufInit(&device->name);
        ffStrbufInit(&device->address);
        ffStrbufInitStatic(&device->vendor, "Unknown");
        device->lmpVersion = INT_MIN;
        device->lmpSubversion = INT_MIN;
        device->enabled = false;
        detectBluetoothRoot(device, entry->d_name, &dbus);
    }
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
