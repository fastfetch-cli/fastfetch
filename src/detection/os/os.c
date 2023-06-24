#include "os.h"
#include "detection/internal.h"

void ffDetectOSImpl(FFOSResult* os);

const FFOSResult* ffDetectOS(void)
{
    FF_DETECTION_INTERNAL_GUARD(FFOSResult,
        ffDetectOSImpl(&result)
    );
}
