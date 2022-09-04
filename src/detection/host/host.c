#include "host.h"
#include "detection/internal.h"

void ffDetectHostImpl(FFHostResult* host);

const FFHostResult* ffDetectHost()
{
    FF_DETECTION_INTERNAL_GUARD(FFHostResult,
        ffDetectHostImpl(&result)
    );
}
