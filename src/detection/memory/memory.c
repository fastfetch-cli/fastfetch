#include "memory.h"
#include "detection/internal.h"

void ffDetectMemoryImpl(FFMemoryStorage* memory);

const FFMemoryStorage* ffDetectMemory()
{
    FF_DETECTION_INTERNAL_GUARD(FFMemoryStorage,
        ffStrbufInit(&result.error);

        ffDetectMemoryImpl(&result);
    );
}
