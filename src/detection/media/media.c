#include "media.h"
#include "common/io.h"

void ffDetectMediaImpl(FFMediaResult* media, bool saveCover);

static FFMediaResult result;

static void removeMediaCoverFile(void)
{
    if (result.cover.length > 0)
    {
        ffRemoveFile(result.cover.chars);
        ffStrbufDestroy(&result.cover);
    }
}

const FFMediaResult* ffDetectMedia(bool saveCover)
{
    if (result.error.chars == NULL)
    {
        ffStrbufInit(&result.error);
        ffStrbufInit(&result.playerId);
        ffStrbufInit(&result.player);
        ffStrbufInit(&result.song);
        ffStrbufInit(&result.artist);
        ffStrbufInit(&result.album);
        ffStrbufInit(&result.url);
        ffStrbufInit(&result.status);
        ffStrbufInit(&result.cover);
        result.removeCoverAfterUse = false;
        ffDetectMediaImpl(&result, saveCover);

        if(result.song.length == 0 && result.error.length == 0)
            ffStrbufAppendS(&result.error, "No media found");
        ffStrbufTrimRightSpace(&result.song);
        ffStrbufTrimRightSpace(&result.artist);
        ffStrbufTrimRightSpace(&result.album);
        ffStrbufTrimRightSpace(&result.player);

        if (saveCover && result.removeCoverAfterUse)
            atexit(removeMediaCoverFile);
    }

    return &result;
}
