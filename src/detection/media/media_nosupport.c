#include "media.h"

void ffDetectMediaImpl(const FFinstance* instance, FFMediaResult* media)
{
    FF_UNUSED(instance);
    ffStrbufAppendS(&media->error, "Not supported on this platform");
}
