#include "dbus.h"

#ifdef FF_HAVE_DBUS

#include "common/thread.h"
#include "util/stringUtils.h"

static bool loadLibSymbols(FFDBusLibrary* lib)
{
    FF_LIBRARY_LOAD(dbus, false, "libdbus-1" FF_LIBRARY_EXTENSION, 4);
    FF_LIBRARY_LOAD_SYMBOL_PTR(dbus, lib, dbus_bus_get, false)
    FF_LIBRARY_LOAD_SYMBOL_PTR(dbus, lib, dbus_message_new_method_call, false)
    FF_LIBRARY_LOAD_SYMBOL_PTR(dbus, lib, dbus_message_append_args, false)
    FF_LIBRARY_LOAD_SYMBOL_PTR(dbus, lib, dbus_message_iter_init, false)
    FF_LIBRARY_LOAD_SYMBOL_PTR(dbus, lib, dbus_message_iter_get_arg_type, false)
    FF_LIBRARY_LOAD_SYMBOL_PTR(dbus, lib, dbus_message_iter_get_basic, false)
    FF_LIBRARY_LOAD_SYMBOL_PTR(dbus, lib, dbus_message_iter_recurse, false)
    FF_LIBRARY_LOAD_SYMBOL_PTR(dbus, lib, dbus_message_iter_has_next, false)
    FF_LIBRARY_LOAD_SYMBOL_PTR(dbus, lib, dbus_message_iter_next, false)
    FF_LIBRARY_LOAD_SYMBOL_PTR(dbus, lib, dbus_message_unref, false)
    FF_LIBRARY_LOAD_SYMBOL_PTR(dbus, lib, dbus_connection_send_with_reply_and_block, false)
    dbus = NULL; // don't auto dlclose
    return true;
}

static const FFDBusLibrary* loadLib(void)
{
    static FFDBusLibrary lib;
    static bool loaded = false;
    static bool loadSuccess = false;

    if(!loaded)
    {
        loaded = true;
        loadSuccess = loadLibSymbols(&lib);
    }

    return loadSuccess ? &lib : NULL;
}

const char* ffDBusLoadData(DBusBusType busType, FFDBusData* data)
{
    data->lib = loadLib();
    if(data->lib == NULL)
        return "Failed to load DBus library";

    data->connection = data->lib->ffdbus_bus_get(busType, NULL);
    if(data->connection == NULL)
        return "Failed to connect to DBus";

    return NULL;
}

bool ffDBusGetString(FFDBusData* dbus, DBusMessageIter* iter, FFstrbuf* result)
{
    int argType = dbus->lib->ffdbus_message_iter_get_arg_type(iter);

    if(argType == DBUS_TYPE_STRING || argType == DBUS_TYPE_OBJECT_PATH)
    {
        const char* value = NULL;
        dbus->lib->ffdbus_message_iter_get_basic(iter, &value);

        if(!ffStrSet(value))
            return false;

        ffStrbufAppendS(result, value);
        return true;
    }

    if (argType == DBUS_TYPE_BYTE)
    {
        uint8_t value;
        dbus->lib->ffdbus_message_iter_get_basic(iter, &value);
        ffStrbufAppendC(result, (char) value);
        return false; // Don't append a comma
    }

    if(argType != DBUS_TYPE_VARIANT && argType != DBUS_TYPE_ARRAY)
        return false;

    DBusMessageIter subIter;
    dbus->lib->ffdbus_message_iter_recurse(iter, &subIter);

    if(argType == DBUS_TYPE_VARIANT)
        return ffDBusGetString(dbus, &subIter, result);

    //At this point we have an array

    bool foundAValue = false;

    while(true)
    {
        if(ffDBusGetString(dbus, &subIter, result))
        {
            foundAValue = true;
            ffStrbufAppendS(result, ", ");
        }

        if(!dbus->lib->ffdbus_message_iter_next(&subIter))
            break;
        else
            continue;
    }

    if(foundAValue)
        ffStrbufSubstrBefore(result, result->length - 2);

    return foundAValue;
}

bool ffDBusGetBool(FFDBusData* dbus, DBusMessageIter* iter, bool* result)
{
    int argType = dbus->lib->ffdbus_message_iter_get_arg_type(iter);

    if(argType == DBUS_TYPE_BOOLEAN)
    {
        dbus_bool_t value = 0;
        dbus->lib->ffdbus_message_iter_get_basic(iter, &value);
        *result = value != 0;
        return true;
    }

    if(argType != DBUS_TYPE_VARIANT)
        return false;

    DBusMessageIter subIter;
    dbus->lib->ffdbus_message_iter_recurse(iter, &subIter);
    return ffDBusGetBool(dbus, &subIter, result);
}

bool ffDBusGetUint(FFDBusData* dbus, DBusMessageIter* iter, uint32_t* result)
{
    int argType = dbus->lib->ffdbus_message_iter_get_arg_type(iter);

    if(argType == DBUS_TYPE_BYTE)
    {
        uint8_t value = 0;
        dbus->lib->ffdbus_message_iter_get_basic(iter, &value);
        *result = value;
        return true;
    }

    if(argType == DBUS_TYPE_UINT16)
    {
        uint16_t value = 0;
        dbus->lib->ffdbus_message_iter_get_basic(iter, &value);
        *result = value;
        return true;
    }

    if(argType == DBUS_TYPE_UINT32)
    {
        dbus->lib->ffdbus_message_iter_get_basic(iter, result);
        return true;
    }

    if(argType != DBUS_TYPE_VARIANT)
        return false;

    DBusMessageIter subIter;
    dbus->lib->ffdbus_message_iter_recurse(iter, &subIter);
    return ffDBusGetUint(dbus, &subIter, result);
}

DBusMessage* ffDBusGetMethodReply(FFDBusData* dbus, const char* busName, const char* objectPath, const char* interface, const char* method, const char* arg)
{
    DBusMessage* message = dbus->lib->ffdbus_message_new_method_call(busName, objectPath, interface, method);
    if(message == NULL)
        return NULL;

    if (arg)
        dbus->lib->ffdbus_message_append_args(message, DBUS_TYPE_STRING, &arg, DBUS_TYPE_INVALID);

    DBusMessage* reply = dbus->lib->ffdbus_connection_send_with_reply_and_block(dbus->connection, message, FF_DBUS_TIMEOUT_MILLISECONDS, NULL);

    dbus->lib->ffdbus_message_unref(message);

    return reply;
}

DBusMessage* ffDBusGetProperty(FFDBusData* dbus, const char* busName, const char* objectPath, const char* interface, const char* property)
{
    DBusMessage* message = dbus->lib->ffdbus_message_new_method_call(busName, objectPath, "org.freedesktop.DBus.Properties", "Get");
    if(message == NULL)
        return NULL;

    dbus->lib->ffdbus_message_append_args(message,
        DBUS_TYPE_STRING, &interface,
        DBUS_TYPE_STRING, &property,
        DBUS_TYPE_INVALID);

    DBusMessage* reply = dbus->lib->ffdbus_connection_send_with_reply_and_block(dbus->connection, message, FF_DBUS_TIMEOUT_MILLISECONDS, NULL);

    dbus->lib->ffdbus_message_unref(message);

    return reply;
}

bool ffDBusGetPropertyString(FFDBusData* dbus, const char* busName, const char* objectPath, const char* interface, const char* property, FFstrbuf* result)
{
    DBusMessage* reply = ffDBusGetProperty(dbus, busName, objectPath, interface, property);
    if(reply == NULL)
        return false;

    DBusMessageIter rootIterator;
    if(!dbus->lib->ffdbus_message_iter_init(reply, &rootIterator))
    {
        dbus->lib->ffdbus_message_unref(reply);
        return false;
    }

    bool ret = ffDBusGetString(dbus, &rootIterator, result);

    dbus->lib->ffdbus_message_unref(reply);

    return ret;
}

bool ffDBusGetPropertyUint(FFDBusData* dbus, const char* busName, const char* objectPath, const char* interface, const char* property, uint32_t* result)
{
    DBusMessage* reply = ffDBusGetProperty(dbus, busName, objectPath, interface, property);
    if(reply == NULL)
        return false;

    DBusMessageIter rootIterator;
    if(!dbus->lib->ffdbus_message_iter_init(reply, &rootIterator))
    {
        dbus->lib->ffdbus_message_unref(reply);
        return false;
    }

    bool ret = ffDBusGetUint(dbus, &rootIterator, result);

    dbus->lib->ffdbus_message_unref(reply);

    return ret;
}

#endif //FF_HAVE_DBUS
