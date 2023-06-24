#include "media.h"

void ffDetectMediaImpl(FFMediaResult* media)
{
    ffStrbufAppendS(&media->error, "Not supported on this platform");
}
