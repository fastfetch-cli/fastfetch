#pragma once

#ifndef FF_INCLUDED_common_dbus
#define FF_INCLUDED_common_dbus

#ifdef FF_HAVE_DBUS
#include <dbus/dbus.h>

#include "util/FFstrbuf.h"
#include "common/library.h"

#define FF_DBUS_TIMEOUT_MILLISECONDS 35

#define FF_DBUS_ITER_CONTINUE(dbus, iterator) \
    { \
        if(!(dbus)->lib->ffdbus_message_iter_has_next(iterator)) \
            break; \
        (dbus)->lib->ffdbus_message_iter_next(iterator); \
        continue; \
    }

typedef struct FFDBusLibrary
{
    FF_LIBRARY_SYMBOL(dbus_bus_get)
    FF_LIBRARY_SYMBOL(dbus_message_new_method_call)
    FF_LIBRARY_SYMBOL(dbus_message_iter_init)
    FF_LIBRARY_SYMBOL(dbus_message_iter_init_append)
    FF_LIBRARY_SYMBOL(dbus_message_iter_append_basic)
    FF_LIBRARY_SYMBOL(dbus_message_iter_get_arg_type)
    FF_LIBRARY_SYMBOL(dbus_message_iter_get_basic)
    FF_LIBRARY_SYMBOL(dbus_message_iter_recurse)
    FF_LIBRARY_SYMBOL(dbus_message_iter_has_next)
    FF_LIBRARY_SYMBOL(dbus_message_iter_next)
    FF_LIBRARY_SYMBOL(dbus_message_unref)
    FF_LIBRARY_SYMBOL(dbus_connection_send_with_reply_and_block)
    FF_LIBRARY_SYMBOL(dbus_connection_flush)
    FF_LIBRARY_SYMBOL(dbus_pending_call_block)
    FF_LIBRARY_SYMBOL(dbus_pending_call_steal_reply)
    FF_LIBRARY_SYMBOL(dbus_pending_call_unref)
} FFDBusLibrary;

typedef struct FFDBusData
{
    const FFDBusLibrary* lib;
    DBusConnection* connection;
} FFDBusData;

const char* ffDBusLoadData(const FFinstance* instance, DBusBusType busType, FFDBusData* data); //Returns an error message or NULL on success
bool ffDBusGetValue(FFDBusData* dbus, DBusMessageIter* iter, FFstrbuf* result);
bool ffDBusGetBool(FFDBusData* dbus, DBusMessageIter* iter, bool* result);
bool ffDBusGetByte(FFDBusData* dbus, DBusMessageIter* iter, uint8_t* result);
DBusMessage* ffDBusGetMethodReply(FFDBusData* dbus, const char* busName, const char* objectPath, const char* interface, const char* method);
DBusMessage* ffDBusGetProperty(FFDBusData* dbus, const char* busName, const char* objectPath, const char* interface, const char* property);
void ffDBusGetPropertyString(FFDBusData* dbus, const char* busName, const char* objectPath, const char* interface, const char* property, FFstrbuf* result);

#endif // FF_HAVE_DBUS
#endif // FF_INCLUDED_common_dbus
