#include "media.h"
#include "detection/internal.h"

void ffDetectMediaImpl(const FFinstance* instance, FFMediaResult* media);

const FFMediaResult* ffDetectMedia(const FFinstance* instance)
{
    FF_DETECTION_INTERNAL_GUARD(FFMediaResult,
        ffStrbufInitA(&result.error, 0);
        ffStrbufInit(&result.busNameShort);
        ffStrbufInit(&result.player);
        ffStrbufInit(&result.song);
        ffStrbufInit(&result.artist);
        ffStrbufInit(&result.album);
        ffStrbufInit(&result.url);

        ffDetectMediaImpl(instance, &result);

        if(result.song.length == 0 && result.error.length == 0)
            ffStrbufAppendS(&result.error, "No media found");
    );
}
