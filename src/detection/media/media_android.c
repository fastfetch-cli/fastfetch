#include "media.h"

void ffDetectMediaImpl(const FFinstance* instance, FFMediaResult* media)
{
    FF_UNUSED(instance);
    ffStrbufAppendS(&media->error, "Media not supported on Android");
}
