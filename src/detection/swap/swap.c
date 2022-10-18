#include "swap.h"
#include "detection/internal.h"

void ffDetectSwapImpl(FFMemoryStorage* swap);

const FFMemoryStorage* ffDetectSwap()
{
    FF_DETECTION_INTERNAL_GUARD(FFMemoryStorage,
        ffStrbufInit(&result.error);

        ffDetectSwapImpl(&result);
    );
}
