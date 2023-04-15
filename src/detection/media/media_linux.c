#include "fastfetch.h"
#include "detection/media/media.h"
#include "common/thread.h"

#include <string.h>

#define FF_DBUS_MPRIS_PREFIX "org.mpris.MediaPlayer2."
#define FF_DBUS_TIMEOUT_MILLISECONDS 35

#ifdef FF_HAVE_DBUS
#include "common/dbus.h"
#include "common/library.h"
#include "common/parsing.h"

static bool getBusProperties(FFDBusData* data, const char* busName, FFMediaResult* result)
{
    DBusMessage* reply = ffDBusGetProperty(data, busName, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", "Metadata");
    if(reply == NULL)
        return false;

    DBusMessageIter rootIterator;
    if(!data->lib->ffdbus_message_iter_init(reply, &rootIterator))
    {
        data->lib->ffdbus_message_unref(reply);
        return false;
    }

    if(data->lib->ffdbus_message_iter_get_arg_type(&rootIterator) != DBUS_TYPE_VARIANT)
    {
        data->lib->ffdbus_message_unref(reply);
        return false;
    }

    DBusMessageIter variantIterator;
    data->lib->ffdbus_message_iter_recurse(&rootIterator, &variantIterator);
    if(data->lib->ffdbus_message_iter_get_arg_type(&variantIterator) != DBUS_TYPE_ARRAY)
    {
        data->lib->ffdbus_message_unref(reply);
        return false;
    }

    DBusMessageIter arrayIterator;
    data->lib->ffdbus_message_iter_recurse(&variantIterator, &arrayIterator);

    while(true)
    {
        if(data->lib->ffdbus_message_iter_get_arg_type(&arrayIterator) != DBUS_TYPE_DICT_ENTRY)
            FF_DBUS_ITER_CONTINUE(data, &arrayIterator)

        DBusMessageIter dictIterator;
        data->lib->ffdbus_message_iter_recurse(&arrayIterator, &dictIterator);

        if(data->lib->ffdbus_message_iter_get_arg_type(&dictIterator) != DBUS_TYPE_STRING)
            FF_DBUS_ITER_CONTINUE(data, &arrayIterator)

        if(!data->lib->ffdbus_message_iter_has_next(&dictIterator))
            FF_DBUS_ITER_CONTINUE(data, &arrayIterator)

        const char* key;
        data->lib->ffdbus_message_iter_get_basic(&dictIterator, &key);

        data->lib->ffdbus_message_iter_next(&dictIterator);

        if(strcmp(key, "xesam:title") == 0)
            ffDBusGetValue(data, &dictIterator, &result->song);
        else if(strcmp(key, "xesam:album") == 0)
            ffDBusGetValue(data, &dictIterator, &result->album);
        else if(strcmp(key, "xesam:artist") == 0)
            ffDBusGetValue(data, &dictIterator, &result->artist);
        else if(strcmp(key, "xesam:url") == 0)
            ffDBusGetValue(data, &dictIterator, &result->url);

        if(result->song.length > 0 && result->artist.length > 0 && result->album.length > 0 && result->url.length > 0)
            break;

        FF_DBUS_ITER_CONTINUE(data, &arrayIterator)
    }

    data->lib->ffdbus_message_unref(reply);

    if(result->song.length == 0)
    {
        ffStrbufClear(&result->artist);
        ffStrbufClear(&result->album);
        ffStrbufClear(&result->url);
        return false;
    }

    //Set short bus name
    ffStrbufAppendS(&result->playerId, busName + sizeof(FF_DBUS_MPRIS_PREFIX) - 1);

    //We found a song, get the player name
    ffDBusGetPropertyString(data, busName, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2", "Identity", &result->player);
    if(result->player.length == 0)
        ffDBusGetPropertyString(data, busName, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2", "DesktopEntry", &result->player);
    if(result->player.length == 0)
        ffStrbufAppend(&result->player, &result->playerId);

    return true;
}

static void getCustomBus(FFDBusData* data, const FFinstance* instance, FFMediaResult* result)
{
    if(ffStrbufStartsWithS(&instance->config.playerName, FF_DBUS_MPRIS_PREFIX))
    {
        getBusProperties(data, instance->config.playerName.chars, result);
        return;
    }

    FF_STRBUF_AUTO_DESTROY busName = ffStrbufCreateS(FF_DBUS_MPRIS_PREFIX);
    ffStrbufAppend(&busName, &instance->config.playerName);
    getBusProperties(data, busName.chars, result);
}

static void getBestBus(FFDBusData* data, FFMediaResult* result)
{
    if(
        getBusProperties(data, FF_DBUS_MPRIS_PREFIX"spotify", result) ||
        getBusProperties(data, FF_DBUS_MPRIS_PREFIX"vlc", result) ||
        getBusProperties(data, FF_DBUS_MPRIS_PREFIX"plasma-browser-integration", result)
    ) return;

    DBusMessage* reply = ffDBusGetMethodReply(data, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "ListNames");
    if(reply == NULL)
        return;

    DBusMessageIter rootIterator;
    if(!data->lib->ffdbus_message_iter_init(reply, &rootIterator) || data->lib->ffdbus_message_iter_get_arg_type(&rootIterator) != DBUS_TYPE_ARRAY)
        return;

    DBusMessageIter arrayIterator;
    data->lib->ffdbus_message_iter_recurse(&rootIterator, &arrayIterator);

    while(true)
    {
        if(data->lib->ffdbus_message_iter_get_arg_type(&arrayIterator) != DBUS_TYPE_STRING)
            FF_DBUS_ITER_CONTINUE(data, &arrayIterator)

        const char* busName;
        data->lib->ffdbus_message_iter_get_basic(&arrayIterator, &busName);

        if(strncmp(busName, FF_DBUS_MPRIS_PREFIX, sizeof(FF_DBUS_MPRIS_PREFIX) - 1) != 0)
            FF_DBUS_ITER_CONTINUE(data, &arrayIterator)

        if(getBusProperties(data, busName, result))
            break;

        FF_DBUS_ITER_CONTINUE(data, &arrayIterator)
    }

    data->lib->ffdbus_message_unref(reply);
}

static const char* getMedia(const FFinstance* instance, FFMediaResult* result)
{
    FFDBusData data;
    const char* error = ffDBusLoadData(instance, DBUS_BUS_SESSION, &data);
    if(error != NULL)
        return error;

    if(instance->config.playerName.length > 0)
        getCustomBus(&data, instance, result);
    else
        getBestBus(&data, result);

    return NULL;
}

#endif

void ffDetectMediaImpl(const FFinstance* instance, FFMediaResult* media)
{
    #ifdef FF_HAVE_DBUS
        const char* error = getMedia(instance, media);
        ffStrbufAppendS(&media->error, error);
    #else
        FF_UNUSED(instance);
        ffStrbufAppendS(&media->error, "Fastfetch was compiled without DBus support");
    #endif
}
