#include "media.h"

void ffDetectMediaImpl(FFMediaResult* media);

const FFMediaResult* ffDetectMedia(void)
{
    static FFMediaResult result;

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
        ffDetectMediaImpl(&result);

        if(result.song.length == 0 && result.error.length == 0)
            ffStrbufAppendS(&result.error, "No media found");
        ffStrbufTrimRightSpace(&result.song);
        ffStrbufTrimRightSpace(&result.artist);
        ffStrbufTrimRightSpace(&result.album);
        ffStrbufTrimRightSpace(&result.player);
    }

    return &result;
}
