#include "bluetooth.h"
#include "util/stringUtils.h"

#ifdef FF_HAVE_DBUS
#include "common/dbus.h"
#include "common/io/io.h"

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

static bool detectBluetoothValue(FFDBusData* dbus, DBusMessageIter* iter, FFBluetoothResult* device)
{
    if(dbus->lib->ffdbus_message_iter_get_arg_type(iter) != DBUS_TYPE_DICT_ENTRY)
        return true;

    DBusMessageIter dictIter;
    dbus->lib->ffdbus_message_iter_recurse(iter, &dictIter);

    if(dbus->lib->ffdbus_message_iter_get_arg_type(&dictIter) != DBUS_TYPE_STRING)
        return true;

    const char* deviceProperty;
    dbus->lib->ffdbus_message_iter_get_basic(&dictIter, &deviceProperty);

    dbus->lib->ffdbus_message_iter_next(&dictIter);

    if(ffStrEquals(deviceProperty, "Address"))
        ffDBusGetString(dbus, &dictIter, &device->address);
    else if(ffStrEquals(deviceProperty, "Name"))
        ffDBusGetString(dbus, &dictIter, &device->name);
    else if(ffStrEquals(deviceProperty, "Icon"))
        ffDBusGetString(dbus, &dictIter, &device->type);
    else if(ffStrEquals(deviceProperty, "Percentage"))
    {
        uint32_t percentage;
        if (ffDBusGetUint(dbus, &dictIter, &percentage))
            device->battery = (uint8_t) percentage;
    }
    else if(ffStrEquals(deviceProperty, "Connected"))
        ffDBusGetBool(dbus, &dictIter, &device->connected);
    else if(ffStrEquals(deviceProperty, "Paired"))
    {
        bool paired = true;
        ffDBusGetBool(dbus, &dictIter, &paired);
        if (!paired) return false;
    }
    return true;
}

static void detectBluetoothProperty(FFDBusData* dbus, DBusMessageIter* iter, FFBluetoothResult* device)
{
    if(dbus->lib->ffdbus_message_iter_get_arg_type(iter) != DBUS_TYPE_DICT_ENTRY)
        return;

    DBusMessageIter dictIter;
    dbus->lib->ffdbus_message_iter_recurse(iter, &dictIter);

    if(dbus->lib->ffdbus_message_iter_get_arg_type(&dictIter) != DBUS_TYPE_STRING)
        return;

    const char* propertyType;
    dbus->lib->ffdbus_message_iter_get_basic(&dictIter, &propertyType);

    if(!ffStrContains(propertyType, ".Device") && !ffStrContains(propertyType, ".Battery"))
        return; // We don't care about other properties

    dbus->lib->ffdbus_message_iter_next(&dictIter);

    if(dbus->lib->ffdbus_message_iter_get_arg_type(&dictIter) != DBUS_TYPE_ARRAY)
        return;

    DBusMessageIter arrayIter;
    dbus->lib->ffdbus_message_iter_recurse(&dictIter, &arrayIter);

    do
    {
        bool shouldContinue = detectBluetoothValue(dbus, &arrayIter, device);
        if (!shouldContinue)
        {
            ffStrbufClear(&device->name);
            break;
        }
    } while (dbus->lib->ffdbus_message_iter_next(&arrayIter));
}

static FFBluetoothResult* detectBluetoothObject(FFlist* devices, FFDBusData* dbus, DBusMessageIter* iter)
{
    if(dbus->lib->ffdbus_message_iter_get_arg_type(iter) != DBUS_TYPE_DICT_ENTRY)
        return NULL;

    DBusMessageIter dictIter;
    dbus->lib->ffdbus_message_iter_recurse(iter, &dictIter);

    if(dbus->lib->ffdbus_message_iter_get_arg_type(&dictIter) != DBUS_TYPE_OBJECT_PATH)
        return NULL;

    const char* objectPath;
    dbus->lib->ffdbus_message_iter_get_basic(&dictIter, &objectPath);

    // We don't want adapter objects
    if(!ffStrContains(objectPath, "/dev_"))
        return NULL;

    dbus->lib->ffdbus_message_iter_next(&dictIter);

    if(dbus->lib->ffdbus_message_iter_get_arg_type(&dictIter) != DBUS_TYPE_ARRAY)
        return NULL;

    DBusMessageIter arrayIter;
    dbus->lib->ffdbus_message_iter_recurse(&dictIter, &arrayIter);

    FFBluetoothResult* device = ffListAdd(devices);
    ffStrbufInit(&device->name);
    ffStrbufInit(&device->address);
    ffStrbufInit(&device->type);
    device->battery = 0;
    device->connected = false;

    do
    {
        detectBluetoothProperty(dbus, &arrayIter, device);
    } while (dbus->lib->ffdbus_message_iter_next(&arrayIter));

    return device;
}

static void detectBluetoothRoot(FFlist* devices, FFDBusData* dbus, DBusMessageIter* iter, int32_t connectedCount)
{
    if(dbus->lib->ffdbus_message_iter_get_arg_type(iter) != DBUS_TYPE_ARRAY)
        return;

    DBusMessageIter arrayIter;
    dbus->lib->ffdbus_message_iter_recurse(iter, &arrayIter);

    do
    {
        FFBluetoothResult* device = detectBluetoothObject(devices, dbus, &arrayIter);

        if (device)
        {
            if(device->name.length == 0 || (connectedCount > 0 && !device->connected))
            {
                ffStrbufDestroy(&device->name);
                ffStrbufDestroy(&device->address);
                ffStrbufDestroy(&device->type);
                --devices->length;
            }

            if (device->connected && --connectedCount == 0)
                break;
        }
    } while (dbus->lib->ffdbus_message_iter_next(&arrayIter));
}

static const char* detectBluetooth(FFlist* devices, int32_t connectedCount)
{
    FFDBusData dbus;
    const char* error = ffDBusLoadData(DBUS_BUS_SYSTEM, &dbus);
    if(error)
        return error;

    DBusMessage* managedObjects = ffDBusGetMethodReply(&dbus, "org.bluez", "/", "org.freedesktop.DBus.ObjectManager", "GetManagedObjects", NULL, NULL);
    if(!managedObjects)
        return "Failed to call GetManagedObjects";

    DBusMessageIter rootIter;
    if(!dbus.lib->ffdbus_message_iter_init(managedObjects, &rootIter))
    {
        dbus.lib->ffdbus_message_unref(managedObjects);
        return "Failed to get root iterator of GetManagedObjects";
    }

    detectBluetoothRoot(devices, &dbus, &rootIter, connectedCount);

    dbus.lib->ffdbus_message_unref(managedObjects);
    return NULL;
}

static uint32_t connectedDevices(void)
{
    FF_AUTO_CLOSE_DIR DIR* dirp = opendir("/sys/class/bluetooth");
    if(dirp == NULL)
        return 0;

    uint32_t result = 0;
    struct dirent* entry;
    while ((entry = readdir(dirp)) != NULL)
    {
        if (strchr(entry->d_name, ':') != NULL)
            ++result;
    }

    return result;
}

#endif

const char* ffDetectBluetooth(FF_MAYBE_UNUSED FFBluetoothOptions* options, FF_MAYBE_UNUSED FFlist* devices /* FFBluetoothResult */)
{
    #ifdef FF_HAVE_DBUS
        int32_t connectedCount = -1;
        if (!options->showDisconnected)
        {
            connectedCount = (int32_t) connectedDevices();
            if (connectedCount == 0)
                return NULL;
        }

        return detectBluetooth(devices, connectedCount);
    #else
        return "Fastfetch was compiled without DBus support";
    #endif
}
