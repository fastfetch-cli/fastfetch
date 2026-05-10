#include "media.h"

void ffDetectMediaImpl(FFMediaResult* media, bool saveCover) {
    ffStrbufAppendS(&media->error, "Not supported on this platform");
}
