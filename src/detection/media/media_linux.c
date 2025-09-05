#include "fastfetch.h"
#include "detection/media/media.h"
#include "util/stringUtils.h"

#include <string.h>

#define FF_DBUS_MPRIS_PREFIX "org.mpris.MediaPlayer2."

#ifdef FF_HAVE_DBUS
#include "common/dbus.h"

#define FF_DBUS_ITER_CONTINUE(dbus, iterator) \
    { \
        if(!(dbus)->lib->ffdbus_message_iter_next(iterator)) \
            break; \
        continue; \
    }

static bool parseMprisMetadata(FFDBusData* data, DBusMessageIter* rootIterator, FFMediaResult* result)
{
    DBusMessageIter arrayIterator;

    if (data->lib->ffdbus_message_iter_get_arg_type(rootIterator) == DBUS_TYPE_VARIANT)
    {
        DBusMessageIter variantIterator;
        data->lib->ffdbus_message_iter_recurse(rootIterator, &variantIterator);
        if(data->lib->ffdbus_message_iter_get_arg_type(&variantIterator) != DBUS_TYPE_ARRAY)
            return false;
        data->lib->ffdbus_message_iter_recurse(&variantIterator, &arrayIterator);
    }
    else
    {
        data->lib->ffdbus_message_iter_recurse(rootIterator, &arrayIterator);
    }

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

        if(!ffStrStartsWith(key, "xesam:"))
            FF_DBUS_ITER_CONTINUE(data, &arrayIterator)

        key += strlen("xesam:");
        if(ffStrEquals(key, "title"))
            ffDBusGetString(data, &dictIterator, &result->song);
        else if(ffStrEquals(key, "album"))
            ffDBusGetString(data, &dictIterator, &result->album);
        else if(ffStrEquals(key, "artist"))
            ffDBusGetString(data, &dictIterator, &result->artist);
        else if(ffStrEquals(key, "url"))
            ffDBusGetString(data, &dictIterator, &result->url);

        if(result->song.length > 0 && result->artist.length > 0 && result->album.length > 0 && result->url.length > 0)
            break;

        FF_DBUS_ITER_CONTINUE(data, &arrayIterator)
    }

    return true;
}

static bool getBusProperties(FFDBusData* data, const char* busName, FFMediaResult* result)
{
    // Get all properties at once to reduce the number of IPCs
    DBusMessage* reply = ffDBusGetAllProperties(data, busName, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player");
    if(reply == NULL)
        return false;

    DBusMessageIter rootIterator;
    if(!data->lib->ffdbus_message_iter_init(reply, &rootIterator) &&
        data->lib->ffdbus_message_iter_get_arg_type(&rootIterator) != DBUS_TYPE_ARRAY)
    {
        data->lib->ffdbus_message_unref(reply);
        return false;
    }

    DBusMessageIter arrayIterator;
    data->lib->ffdbus_message_iter_recurse(&rootIterator, &arrayIterator);

    while(true)
    {
        if(data->lib->ffdbus_message_iter_get_arg_type(&arrayIterator) != DBUS_TYPE_DICT_ENTRY)
            FF_DBUS_ITER_CONTINUE(data, &arrayIterator)

        DBusMessageIter dictIterator;
        data->lib->ffdbus_message_iter_recurse(&arrayIterator, &dictIterator);

        const char* key;
        data->lib->ffdbus_message_iter_get_basic(&dictIterator, &key);

        data->lib->ffdbus_message_iter_next(&dictIterator);

        if(ffStrEquals(key, "Metadata"))
            parseMprisMetadata(data, &dictIterator, result);
        else if(ffStrEquals(key, "PlaybackStatus"))
            ffDBusGetString(data, &dictIterator, &result->status);

        FF_DBUS_ITER_CONTINUE(data, &arrayIterator)
    }

    if(result->song.length == 0)
    {
        if(result->url.length)
        {
            const char* fileName = memrchr(result->url.chars, '/', result->url.length);
            assert(fileName);
            ++fileName;
            ffStrbufEnsureFixedLengthFree(&result->song, result->url.length - (uint32_t) (fileName - result->url.chars));
            for(; *fileName && *fileName != '?'; ++fileName)
            {
                if (*fileName != '%')
                {
                    ffStrbufAppendC(&result->song, *fileName);
                }
                else
                {
                    if (fileName[1] == 0 || fileName[2] == 0)
                        break;
                    char str[] = { fileName[1], fileName[2], 0 };
                    ffStrbufAppendC(&result->song, (char) strtoul(str, NULL, 16));
                    fileName += 2;
                }
            }
        }
        else
        {
            ffStrbufClear(&result->artist);
            ffStrbufClear(&result->album);
            ffStrbufClear(&result->url);
            ffStrbufClear(&result->status);
            return false;
        }
    }

    //Set short bus name
    ffStrbufAppendS(&result->playerId, busName + sizeof(FF_DBUS_MPRIS_PREFIX) - 1);

    //We found a song, get the player name
    if (ffStrbufStartsWithS(&result->playerId, "musikcube.instance"))
    {
        // dbus calls are EXTREMELY slow on musikcube, so we set the player name manually
        ffStrbufSetStatic(&result->player, "musikcube");
    }
    else
    {
        ffDBusGetPropertyString(data, busName, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2", "Identity", &result->player);
        if(result->player.length == 0)
            ffDBusGetPropertyString(data, busName, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2", "DesktopEntry", &result->player);
        if(result->player.length == 0)
            ffStrbufAppend(&result->player, &result->playerId);
    }

    data->lib->ffdbus_message_unref(reply);

    return true;
}

static void getCustomBus(FFDBusData* data, const FFstrbuf* playerName, FFMediaResult* result)
{
    if(ffStrbufStartsWithS(playerName, FF_DBUS_MPRIS_PREFIX))
    {
        getBusProperties(data, playerName->chars, result);
        return;
    }

    FF_STRBUF_AUTO_DESTROY busName = ffStrbufCreateS(FF_DBUS_MPRIS_PREFIX);
    ffStrbufAppend(&busName, playerName);
    getBusProperties(data, busName.chars, result);
}

static void getBestBus(FFDBusData* data, FFMediaResult* result)
{
    if(
        getBusProperties(data, FF_DBUS_MPRIS_PREFIX "spotify", result) ||
        getBusProperties(data, FF_DBUS_MPRIS_PREFIX "vlc", result) ||
        getBusProperties(data, FF_DBUS_MPRIS_PREFIX "plasma-browser-integration", result)
    ) return;

    DBusMessage* reply = ffDBusGetMethodReply(data, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "ListNames", NULL, NULL);
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

        if(!ffStrStartsWith(busName, FF_DBUS_MPRIS_PREFIX))
            FF_DBUS_ITER_CONTINUE(data, &arrayIterator)

        if(getBusProperties(data, busName, result))
            break;

        FF_DBUS_ITER_CONTINUE(data, &arrayIterator)
    }

    data->lib->ffdbus_message_unref(reply);
}

static const char* getMedia(FFMediaResult* result)
{
    FFDBusData data;
    const char* error = ffDBusLoadData(DBUS_BUS_SESSION, &data);
    if(error != NULL)
        return error;

    // FIXME: This is shared for both player and media module.
    // However it uses an option in one specific module
    if(instance.config.general.playerName.length > 0)
        getCustomBus(&data, &instance.config.general.playerName, result);
    else
        getBestBus(&data, result);

    return NULL;
}

#endif

void ffDetectMediaImpl(FFMediaResult* media)
{
    #ifdef FF_HAVE_DBUS
        const char* error = getMedia(media);
        ffStrbufAppendS(&media->error, error);
    #else
        ffStrbufAppendS(&media->error, "Fastfetch was compiled without DBus support");
    #endif
}
