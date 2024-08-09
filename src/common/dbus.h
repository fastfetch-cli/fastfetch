#pragma once

#ifdef FF_HAVE_DBUS
#include <dbus/dbus.h>

#include "util/FFstrbuf.h"
#include "common/library.h"

#define FF_DBUS_TIMEOUT_MILLISECONDS 100

typedef struct FFDBusLibrary
{
    FF_LIBRARY_SYMBOL(dbus_bus_get)
    FF_LIBRARY_SYMBOL(dbus_message_new_method_call)
    FF_LIBRARY_SYMBOL(dbus_message_append_args)
    FF_LIBRARY_SYMBOL(dbus_message_iter_init)
    FF_LIBRARY_SYMBOL(dbus_message_iter_get_arg_type)
    FF_LIBRARY_SYMBOL(dbus_message_iter_get_basic)
    FF_LIBRARY_SYMBOL(dbus_message_iter_recurse)
    FF_LIBRARY_SYMBOL(dbus_message_iter_has_next)
    FF_LIBRARY_SYMBOL(dbus_message_iter_next)
    FF_LIBRARY_SYMBOL(dbus_message_unref)
    FF_LIBRARY_SYMBOL(dbus_connection_send_with_reply_and_block)
} FFDBusLibrary;

typedef struct FFDBusData
{
    const FFDBusLibrary* lib;
    DBusConnection* connection;
} FFDBusData;

const char* ffDBusLoadData(DBusBusType busType, FFDBusData* data); //Returns an error message or NULL on success
bool ffDBusGetString(FFDBusData* dbus, DBusMessageIter* iter, FFstrbuf* result);
bool ffDBusGetBool(FFDBusData* dbus, DBusMessageIter* iter, bool* result);
bool ffDBusGetUint(FFDBusData* dbus, DBusMessageIter* iter, uint32_t* result);
DBusMessage* ffDBusGetMethodReply(FFDBusData* dbus, const char* busName, const char* objectPath, const char* interface, const char* method, const char* arg);
DBusMessage* ffDBusGetProperty(FFDBusData* dbus, const char* busName, const char* objectPath, const char* interface, const char* property);
bool ffDBusGetPropertyString(FFDBusData* dbus, const char* busName, const char* objectPath, const char* interface, const char* property, FFstrbuf* result);
bool ffDBusGetPropertyUint(FFDBusData* dbus, const char* busName, const char* objectPath, const char* interface, const char* property, uint32_t* result);

#endif // FF_HAVE_DBUS
