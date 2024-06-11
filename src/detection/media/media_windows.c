#include "fastfetch.h"
#include "common/library.h"
#include "util/windows/unicode.h"
#include "media.h"

#define FF_MEDIA_WIN_RESULT_BUFLEN 256

typedef struct FFMediaWinResult
{
    wchar_t playerId[FF_MEDIA_WIN_RESULT_BUFLEN];
    wchar_t song[FF_MEDIA_WIN_RESULT_BUFLEN];
    wchar_t artist[FF_MEDIA_WIN_RESULT_BUFLEN];
    wchar_t album[FF_MEDIA_WIN_RESULT_BUFLEN];
    char status[FF_MEDIA_WIN_RESULT_BUFLEN];
    char error[FF_MEDIA_WIN_RESULT_BUFLEN];
} FFMediaWinResult;

bool ffMediaWinDetectMedia(FFMediaWinResult* result);

static const char* getMedia(FFMediaResult* media)
{
    FF_LIBRARY_LOAD(libffdll, NULL, "dlopen fastfetch-dll" FF_LIBRARY_EXTENSION " failed", "fastfetch-dll" FF_LIBRARY_EXTENSION, 0)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libffdll, ffMediaWinDetectMedia)
    libffdll = NULL; // Don't close libffdll or it may crash

    FFMediaWinResult result = {};

    if (!ffffMediaWinDetectMedia(&result))
    {
        ffStrbufAppendS(&media->error, result.error);
        ffStrbufAppendS(&media->error, " (DLL error)");
        return NULL;
    }

    ffStrbufSetWS(&media->playerId, result.playerId);
    ffStrbufSet(&media->player, &media->playerId);
    if (ffStrbufEndsWithIgnCaseS(&media->player, ".exe"))
        ffStrbufSubstrBefore(&media->player, media->player.length - 4);
    else
        ffStrbufSubstrAfterFirstC(&media->player, '!'); // UWP ID
    ffStrbufSetWS(&media->song, result.song);
    ffStrbufSetWS(&media->artist, result.artist);
    ffStrbufSetWS(&media->album, result.album);
    ffStrbufSetS(&media->status, result.status);
    return NULL;
}

void ffDetectMediaImpl(FFMediaResult* media)
{
    const char* error = getMedia(media);
    ffStrbufAppendS(&media->error, error);
}
