#include "common/library.h"
#include "util/windows/unicode.h"
#include "media.h"
#include "media_windows.dll.h"

static const char* getMedia(FFMediaResult* media)
{
    FF_LIBRARY_LOAD(libffwinrt, "dlopen libffwinrt" FF_LIBRARY_EXTENSION " failed", "libffwinrt" FF_LIBRARY_EXTENSION, 0)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libffwinrt, ffWinrtDetectMedia)
    libffwinrt = NULL; // Don't close libffwinrt or it may crash

    FFWinrtMediaResult result = {};

    const char* error = ffffWinrtDetectMedia(&result);
    if (error)
    {
        ffStrbufSetStatic(&media->error, error);
        return NULL;
    }

    ffStrbufSetWS(&media->playerId, result.playerId);
    if (result.playerName[0])
    {
        ffStrbufSetWS(&media->player, result.playerName);
    }
    else
    {
        ffStrbufSet(&media->player, &media->playerId);
        if (ffStrbufEndsWithIgnCaseS(&media->player, ".exe"))
            ffStrbufSubstrBefore(&media->player, media->player.length - 4);
    }
    ffStrbufSetWS(&media->song, result.song);
    ffStrbufSetWS(&media->artist, result.artist);
    ffStrbufSetWS(&media->album, result.album);
    ffStrbufSetStatic(&media->status, result.status);
    return NULL;
}

void ffDetectMediaImpl(FFMediaResult* media)
{
    const char* error = getMedia(media);
    ffStrbufAppendS(&media->error, error);
}
