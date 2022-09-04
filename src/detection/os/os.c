#include "os.h"
#include "detection/internal.h"

void ffDetectOSImpl(FFOSResult* os, const FFinstance* instance);

const FFOSResult* ffDetectOS(const FFinstance* instance)
{
    FF_DETECTION_INTERNAL_GUARD(FFOSResult,
        ffDetectOSImpl(&result, instance)
    );
}
