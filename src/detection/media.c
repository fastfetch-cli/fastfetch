#include "fastfetch.h"

#include <ctype.h>
#include <string.h>
#include <pthread.h>

#define FF_DBUS_MPRIS_PREFIX "org.mpris.MediaPlayer2."

#ifdef FF_HAVE_DBUS
#include <dbus/dbus.h>

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

static bool getValue(DBusMessageIter* iter, FFstrbuf* result, DBusData* data)
{
    int argType = data->ffdbus_message_iter_get_arg_type(iter);

    if(argType == DBUS_TYPE_STRING)
    {
        const char* value = NULL;
        data->ffdbus_message_iter_get_basic(iter, &value);

        if(!ffStrSet(value))
            return false;

        ffStrbufAppendS(result, value);
        return true;
    }

    if(argType != DBUS_TYPE_VARIANT && argType != DBUS_TYPE_ARRAY)
        return false;

    DBusMessageIter subIter;
    data->ffdbus_message_iter_recurse(iter, &subIter);

    if(argType == DBUS_TYPE_VARIANT)
        return getValue(&subIter, result, data);

    //At this point we have an array

    bool foundAValue = false;

    while(true)
    {
        if((foundAValue = getValue(&subIter, result, data)))
            ffStrbufAppendS(result, ", ");

        FF_DBUS_ITER_CONTINUE(subIter);
    }

    if(foundAValue)
        ffStrbufSubstrBefore(result, result->length - 2);

    return foundAValue;
}

static DBusMessage* getReply(DBusMessage* request, DBusData* data)
{
    DBusPendingCall* pendingCall = NULL;
    dbus_bool_t succesfull = data->ffdbus_connection_send_with_reply(data->connection, request, &pendingCall, DBUS_TIMEOUT_USE_DEFAULT);
    data->ffdbus_message_unref(request);
    if(!succesfull || pendingCall == NULL)
        return NULL;

    data->ffdbus_connection_flush(data->connection);
    data->ffdbus_pending_call_block(pendingCall);

    DBusMessage* reply = data->ffdbus_pending_call_steal_reply(pendingCall);
    data->ffdbus_pending_call_unref(pendingCall);
    return reply;
}

static DBusMessage* getProperty(const char* busName, const char* objectPath, const char* interface, const char* property, DBusData* data)
{
    DBusMessage* message = data->ffdbus_message_new_method_call(busName, objectPath, "org.freedesktop.DBus.Properties", "Get");
    if(message == NULL)
        return NULL;

    DBusMessageIter requestIterator;
    data->ffdbus_message_iter_init_append(message, &requestIterator);

    if(!data->ffdbus_message_iter_append_basic(&requestIterator, DBUS_TYPE_STRING, &interface))
    {
        data->ffdbus_message_unref(message);
        return NULL;
    }

    if(!data->ffdbus_message_iter_append_basic(&requestIterator, DBUS_TYPE_STRING, &property))
    {
        data->ffdbus_message_unref(message);
        return NULL;
    }

    return getReply(message, data);
}

static void getPropertyString(const char* busName, const char* objectPath, const char* interface, const char* property, FFstrbuf* result, DBusData* data)
{
    DBusMessage* reply = getProperty(busName, objectPath, interface, property, data);
    if(reply == NULL)
        return;

    DBusMessageIter rootIterator;
    if(!data->ffdbus_message_iter_init(reply, &rootIterator))
    {
        data->ffdbus_message_unref(reply);
        return;
    }

    getValue(&rootIterator, result, data);

    data->ffdbus_message_unref(reply);
}

static bool getBusProperties(const char* busName, FFMediaResult* result, DBusData* data)
{
    DBusMessage* reply = getProperty(busName, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", "Metadata", data);
    if(reply == NULL)
        return false;

    DBusMessageIter rootIterator;
    if(!data->ffdbus_message_iter_init(reply, &rootIterator))
    {
        data->ffdbus_message_unref(reply);
        return false;
    }

    if(data->ffdbus_message_iter_get_arg_type(&rootIterator) != DBUS_TYPE_VARIANT)
    {
        data->ffdbus_message_unref(reply);
        return false;
    }

    DBusMessageIter variantIterator;
    data->ffdbus_message_iter_recurse(&rootIterator, &variantIterator);
    if(data->ffdbus_message_iter_get_arg_type(&variantIterator) != DBUS_TYPE_ARRAY)
    {
        data->ffdbus_message_unref(reply);
        return false;
    }

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

        if(!data->ffdbus_message_iter_has_next(&dictIterator))
            FF_DBUS_ITER_CONTINUE(arrayIterator)

        const char* key;
        data->ffdbus_message_iter_get_basic(&dictIterator, &key);

        data->ffdbus_message_iter_next(&dictIterator);

        if(strcmp(key, "xesam:title") == 0)
            getValue(&dictIterator, &result->song, data);
        else if(strcmp(key, "xesam:album") == 0)
            getValue(&dictIterator, &result->album, data);
        else if(strcmp(key, "xesam:artist") == 0)
            getValue(&dictIterator, &result->artist, data);
        else if(strcmp(key, "xesam:url") == 0)
            getValue(&dictIterator, &result->url, data);

        if(result->song.length > 0 && result->artist.length > 0 && result->album.length > 0)
            break;

        FF_DBUS_ITER_CONTINUE(arrayIterator)
    }

    data->ffdbus_message_unref(reply);

    if(result->song.length == 0)
    {
        ffStrbufClear(&result->artist);
        ffStrbufClear(&result->album);
        return false;
    }

    //We found a song, get the player name

    getPropertyString(busName, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2", "Identity", &result->player, data);

    if(result->player.length == 0)
        getPropertyString(busName, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2", "DesktopEntry", &result->player, data);

    return true;
}

static void getCustomBus(FFinstance* instance, FFMediaResult* result, DBusData* data)
{
    if(ffStrbufStartsWithS(&instance->config.playerName, FF_DBUS_MPRIS_PREFIX))
    {
        ffStrbufAppendS(&result->busNameShort, instance->config.playerName.chars + sizeof(FF_DBUS_MPRIS_PREFIX) - 1);
        getBusProperties(instance->config.playerName.chars, result, data);
        return;
    }

    ffStrbufAppend(&result->busNameShort, &instance->config.playerName);

    FFstrbuf busName;
    ffStrbufInit(&busName);
    ffStrbufAppendS(&busName, FF_DBUS_MPRIS_PREFIX);
    ffStrbufAppend(&busName, &instance->config.playerName);
    getBusProperties(busName.chars, result, data);
    ffStrbufDestroy(&busName);
}

static void getBestBus(FFMediaResult* result, DBusData* data)
{
    DBusMessage* message = data->ffdbus_message_new_method_call("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "ListNames");
    if(message == NULL)
        return;

    DBusMessage* reply = getReply(message, data);
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

        const char* busName;
        data->ffdbus_message_iter_get_basic(&arrayIterator, &busName);

        if(strncmp(busName, FF_DBUS_MPRIS_PREFIX, sizeof(FF_DBUS_MPRIS_PREFIX) - 1) != 0)
            FF_DBUS_ITER_CONTINUE(arrayIterator)

        if(getBusProperties(busName, result, data))
        {
            ffStrbufAppendS(&result->busNameShort, busName + sizeof(FF_DBUS_MPRIS_PREFIX) - 1);
            break;
        }

        FF_DBUS_ITER_CONTINUE(arrayIterator)
    }

    data->ffdbus_message_unref(reply);
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
        getCustomBus(instance, result, &data);
    else
        getBestBus(result, &data);

    dlclose(dbus);

    //If we are on a website, prepend the website name
    if(ffStrbufStartsWithS(&result->url, "https://www."))
        ffStrbufAppendS(&result->playerPretty, result->url.chars + 12);
    else if(ffStrbufStartsWithS(&result->url, "http://www."))
        ffStrbufAppendS(&result->playerPretty, result->url.chars + 11);
    else if(ffStrbufStartsWithS(&result->url, "https://"))
        ffStrbufAppendS(&result->playerPretty, result->url.chars + 8);
    else if(ffStrbufStartsWithS(&result->url, "http://"))
        ffStrbufAppendS(&result->playerPretty, result->url.chars + 7);

    //If we found a website name, make it more pretty
    if(result->playerPretty.length > 0)
    {
        ffStrbufSubstrBeforeFirstC(&result->playerPretty, '/'); //Remove the path
        ffStrbufSubstrBeforeLastC(&result->playerPretty, '.'); //Remove the TLD
    }

    //Check again for length, as we may have removed everything.
    //If we don't have subdomains, it is usually more pretty to capitalize the first letter.
    if(result->playerPretty.length > 0 && ffStrbufFirstIndexC(&result->playerPretty, '.') == result->playerPretty.length)
        result->playerPretty.chars[0] = (char) toupper(result->playerPretty.chars[0]);
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

    ffStrbufInit(&result.busNameShort);
    ffStrbufInit(&result.player);
    ffStrbufInit(&result.playerPretty);
    ffStrbufInit(&result.song);
    ffStrbufInit(&result.artist);
    ffStrbufInit(&result.album);
    ffStrbufInit(&result.url);

    #ifdef FF_HAVE_DBUS
        getMedia(instance, &result);
    #endif

    //Set busNameShort if a custom player was given, but loading dbus failed
    if(instance->config.playerName.length > 0 && result.busNameShort.length == 0)
    {
        if(ffStrbufStartsWithS(&instance->config.playerName, FF_DBUS_MPRIS_PREFIX))
            ffStrbufAppendS(&result.busNameShort, instance->config.playerName.chars + sizeof(FF_DBUS_MPRIS_PREFIX) - 1);
        else
            ffStrbufAppend(&result.busNameShort, &instance->config.playerName);
    }

    //Set player to busNameShort, if detection failed
    if(result.player.length == 0)
        ffStrbufAppend(&result.player, &result.busNameShort);

    bool hasCustomPrettyName = result.playerPretty.length > 0;

    if(hasCustomPrettyName)
        ffStrbufAppendS(&result.playerPretty, " (");

    ffStrbufAppend(&result.playerPretty, &result.player);

    if(hasCustomPrettyName)
        ffStrbufAppendC(&result.playerPretty, ')');

    pthread_mutex_unlock(&mutex);
    return &result;
}
