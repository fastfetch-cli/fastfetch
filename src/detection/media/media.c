#include "media.h"
#include "detection/internal.h"

void ffDetectMediaImpl(FFMediaResult* media);

const FFMediaResult* ffDetectMedia(void)
{
    FF_DETECTION_INTERNAL_GUARD(FFMediaResult,
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
    );
}
