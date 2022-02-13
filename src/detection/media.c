#include "fastfetch.h"

#include <string.h>
#include <pthread.h>

#ifdef FF_HAVE_DBUS
#include <dbus/dbus.h>

#define FF_DBUS_MPRIS_PREFIX "org.mpris.MediaPlayer2."

#define FF_DBUS_ITER_CONTINUE(iterator) \
    { \
        if(!data->ffdbus_message_iter_has_next(&iterator)) \
            break; \
        data->ffdbus_message_iter_next(&iterator); \
        continue; \
    }

typedef struct DBusData
{
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
    FF_LIBRARY_SYMBOL(dbus_connection_send_with_reply)
    FF_LIBRARY_SYMBOL(dbus_connection_flush)
    FF_LIBRARY_SYMBOL(dbus_pending_call_block)
    FF_LIBRARY_SYMBOL(dbus_pending_call_steal_reply)
    FF_LIBRARY_SYMBOL(dbus_pending_call_unref)

    DBusConnection *connection;
} DBusData;

static void getString(DBusMessageIter* iter, FFstrbuf* result, DBusData* data)
{
    if(data->ffdbus_message_iter_get_arg_type(iter) != DBUS_TYPE_STRING)
        return;

    const char* value;
    data->ffdbus_message_iter_get_basic(iter, &value);
    ffStrbufAppendS(result, value);
}

static void getArray(DBusMessageIter* iter, FFstrbuf* result, DBusData* data)
{
    if(data->ffdbus_message_iter_get_arg_type(iter) != DBUS_TYPE_ARRAY)
        return;

    DBusMessageIter arrayIter;
    data->ffdbus_message_iter_recurse(iter, &arrayIter);

    while(true)
    {
        getString(&arrayIter, result, data);
        ffStrbufAppendS(result, ", ");

        FF_DBUS_ITER_CONTINUE(arrayIter);
    }

    if(ffStrbufEndsWithS(result, ", "))
        ffStrbufSubstrBefore(result, result->length - 2);
}

static bool detectSong(const char* player, FFMediaResult* result, DBusData* data)
{
    DBusMessage* message = data->ffdbus_message_new_method_call(player, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
    if(message == NULL)
        return false;

    DBusMessageIter requestIterator;
    data->ffdbus_message_iter_init_append(message, &requestIterator);

    const char* arg1 = "org.mpris.MediaPlayer2.Player";
    if(!data->ffdbus_message_iter_append_basic(&requestIterator, DBUS_TYPE_STRING, &arg1))
    {
        data->ffdbus_message_unref(message);
        return false;
    }

    const char* arg2 = "Metadata";
    if(!data->ffdbus_message_iter_append_basic(&requestIterator, DBUS_TYPE_STRING, &arg2))
    {
        data->ffdbus_message_unref(message);
        return false;
    }

    DBusPendingCall* pending;
    bool sendSuccessfull = data->ffdbus_connection_send_with_reply(data->connection, message, &pending, DBUS_TIMEOUT_USE_DEFAULT);
    data->ffdbus_message_unref(message);
    if(pending == NULL || !sendSuccessfull)
        return false;

    data->ffdbus_connection_flush(data->connection);
    data->ffdbus_pending_call_block(pending);

    DBusMessage* reply = data->ffdbus_pending_call_steal_reply(pending);
    data->ffdbus_pending_call_unref(pending);
    if(reply == NULL)
        return false;

    DBusMessageIter rootIterator;
    if(!data->ffdbus_message_iter_init(reply, &rootIterator) || data->ffdbus_message_iter_get_arg_type(&rootIterator) != DBUS_TYPE_VARIANT)
        return false;

    DBusMessageIter variantIterator;
    data->ffdbus_message_iter_recurse(&rootIterator, &variantIterator);
    if(data->ffdbus_message_iter_get_arg_type(&variantIterator) != DBUS_TYPE_ARRAY)
        return false;

    DBusMessageIter arrayIterator;
    data->ffdbus_message_iter_recurse(&variantIterator, &arrayIterator);

    while(true)
    {
        if(data->ffdbus_message_iter_get_arg_type(&arrayIterator) != DBUS_TYPE_DICT_ENTRY)
            FF_DBUS_ITER_CONTINUE(arrayIterator)

        DBusMessageIter dictIterator;
        data->ffdbus_message_iter_recurse(&arrayIterator, &dictIterator);

        if(data->ffdbus_message_iter_get_arg_type(&dictIterator) != DBUS_TYPE_STRING)
            FF_DBUS_ITER_CONTINUE(arrayIterator)

        const char* key;
        data->ffdbus_message_iter_get_basic(&dictIterator, &key);

        if(!data->ffdbus_message_iter_has_next(&dictIterator))
            FF_DBUS_ITER_CONTINUE(arrayIterator)

        data->ffdbus_message_iter_next(&dictIterator);

        if(data->ffdbus_message_iter_get_arg_type(&dictIterator) != DBUS_TYPE_VARIANT)
            FF_DBUS_ITER_CONTINUE(arrayIterator)

        DBusMessageIter valueIter;
        data->ffdbus_message_iter_recurse(&dictIterator, &valueIter);

        if(strcmp(key, "xesam:title") == 0)
            getString(&valueIter, &result->song, data);
        else if(strcmp(key, "xesam:album") == 0)
            getString(&valueIter, &result->album, data);
        else if(strcmp(key, "xesam:artist") == 0)
            getArray(&valueIter, &result->artist, data);

        if(result->song.length > 0 && result->artist.length > 0 && result->album.length > 0)
            break;

        FF_DBUS_ITER_CONTINUE(arrayIterator)
    }

    if(result->song.length == 0)
    {
        ffStrbufClear(&result->artist);
        ffStrbufClear(&result->album);
        return false;
    }

    return true;
}

static void getCustomPlayer(FFinstance* instance, FFMediaResult* result, DBusData* data)
{
    if(ffStrbufStartsWithS(&instance->config.playerName, FF_DBUS_MPRIS_PREFIX))
    {
        detectSong(instance->config.playerName.chars, result, data);
        ffStrbufAppendS(&result->player, instance->config.playerName.chars + sizeof(FF_DBUS_MPRIS_PREFIX) - 1);
        return;
    }

    FFstrbuf busName;
    ffStrbufInit(&busName);
    ffStrbufAppendS(&busName, FF_DBUS_MPRIS_PREFIX);
    ffStrbufAppend(&busName, &instance->config.playerName);
    detectSong(busName.chars, result, data);
    ffStrbufDestroy(&busName);

    ffStrbufAppend(&result->player, &instance->config.playerName);
}

static void getBestPlayer(FFMediaResult* result, DBusData* data)
{
    DBusMessage* message = data->ffdbus_message_new_method_call("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "ListNames");
    if(message == NULL)
        return;

    DBusPendingCall* pending;
    bool sendSuccessfull = data->ffdbus_connection_send_with_reply(data->connection, message, &pending, DBUS_TIMEOUT_USE_DEFAULT);
    data->ffdbus_message_unref(message);
    if(pending == NULL || !sendSuccessfull)
        return;

    data->ffdbus_connection_flush(data->connection);
    data->ffdbus_pending_call_block(pending);

    DBusMessage* reply = data->ffdbus_pending_call_steal_reply(pending);
    data->ffdbus_pending_call_unref(pending);
    if(reply == NULL)
        return;

    DBusMessageIter rootIterator;
    if(!data->ffdbus_message_iter_init(reply, &rootIterator) || data->ffdbus_message_iter_get_arg_type(&rootIterator) != DBUS_TYPE_ARRAY)
        return;

    DBusMessageIter arrayIterator;
    data->ffdbus_message_iter_recurse(&rootIterator, &arrayIterator);

    while(true)
    {
        if(data->ffdbus_message_iter_get_arg_type(&arrayIterator) != DBUS_TYPE_STRING)
            FF_DBUS_ITER_CONTINUE(arrayIterator)

        const char* name;
        data->ffdbus_message_iter_get_basic(&arrayIterator, &name);

        if(strncmp(name, FF_DBUS_MPRIS_PREFIX, sizeof(FF_DBUS_MPRIS_PREFIX) - 1) != 0)
            FF_DBUS_ITER_CONTINUE(arrayIterator)

        if(detectSong(name, result, data))
        {
            ffStrbufAppendS(&result->player, name + sizeof(FF_DBUS_MPRIS_PREFIX) - 1);
            break;
        }

        FF_DBUS_ITER_CONTINUE(arrayIterator)
    }
}

static void getMedia(FFinstance* instance, FFMediaResult* result)
{
    DBusData data;

    FF_LIBRARY_LOAD(dbus, instance->config.libDBus, , "libdbus-1.so", "libdbus-1.so.3");
    FF_LIBRARY_LOAD_SYMBOL(dbus, dbus_bus_get,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(dbus, data.ffdbus_message_new_method_call, dbus_message_new_method_call,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(dbus, data.ffdbus_message_iter_init, dbus_message_iter_init,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(dbus, data.ffdbus_message_iter_init_append, dbus_message_iter_init_append,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(dbus, data.ffdbus_message_iter_append_basic, dbus_message_iter_append_basic,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(dbus, data.ffdbus_message_iter_get_arg_type, dbus_message_iter_get_arg_type,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(dbus, data.ffdbus_message_iter_get_basic, dbus_message_iter_get_basic,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(dbus, data.ffdbus_message_iter_recurse, dbus_message_iter_recurse,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(dbus, data.ffdbus_message_iter_has_next, dbus_message_iter_has_next,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(dbus, data.ffdbus_message_iter_next, dbus_message_iter_next,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(dbus, data.ffdbus_message_unref, dbus_message_unref,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(dbus, data.ffdbus_connection_send_with_reply, dbus_connection_send_with_reply,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(dbus, data.ffdbus_connection_flush, dbus_connection_flush,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(dbus, data.ffdbus_pending_call_block, dbus_pending_call_block,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(dbus, data.ffdbus_pending_call_steal_reply, dbus_pending_call_steal_reply,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(dbus, data.ffdbus_pending_call_unref, dbus_pending_call_unref,)

    data.connection = ffdbus_bus_get(DBUS_BUS_SESSION, NULL);
    if(data.connection == NULL)
    {
        dlclose(dbus);
        return;
    }

    if(instance->config.playerName.length > 0)
        getCustomPlayer(instance, result, &data);
    else
        getBestPlayer(result, &data);

    dlclose(dbus);
}

#endif

const FFMediaResult* ffDetectMedia(FFinstance* instance)
{
    static FFMediaResult result;
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static bool init = false;

    pthread_mutex_lock(&mutex);
    if(init)
    {
        pthread_mutex_unlock(&mutex);
        return &result;
    }
    init = true;

    ffStrbufInit(&result.player);
    ffStrbufInit(&result.song);
    ffStrbufInit(&result.artist);
    ffStrbufInit(&result.album);

    #ifdef FF_HAVE_DBUS
        getMedia(instance, &result);
    #endif

    if(instance->config.playerName.length > 0 && result.player.length == 0)
    {
        if(ffStrbufStartsWithS(&instance->config.playerName, FF_DBUS_MPRIS_PREFIX))
            ffStrbufAppendS(&result.player, instance->config.playerName.chars + sizeof(FF_DBUS_MPRIS_PREFIX) - 1);
        else
            ffStrbufAppend(&result.player, &instance->config.playerName);
    }

    pthread_mutex_unlock(&mutex);
    return &result;
}
