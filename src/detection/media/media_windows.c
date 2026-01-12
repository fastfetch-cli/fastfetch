#include "common/library.h"
#include "common/windows/unicode.h"
#include "media.h"
#include "media_windows.dll.h"

static const char* getMedia(FFMediaResult* media, bool saveCover)
{
    FF_LIBRARY_LOAD_MESSAGE(libffwinrt, "libffwinrt" FF_LIBRARY_EXTENSION, 0)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libffwinrt, ffWinrtDetectMedia)
    libffwinrt = NULL; // Don't close libffwinrt or it may crash

    FFWinrtMediaResult result = {};

    const char* error = ffffWinrtDetectMedia(&result, saveCover);
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
    ffStrbufSetWS(&media->cover, result.cover);
    ffStrbufSetStatic(&media->status, result.status);
    if (media->cover.length > 0)
        media->removeCoverAfterUse = true;
    return NULL;
}

void ffDetectMediaImpl(FFMediaResult* media, bool saveCover)
{
    const char* error = getMedia(media, saveCover);
    ffStrbufAppendS(&media->error, error);
}
