#include "memory.h"
#include "detection/internal.h"

void ffDetectMemoryImpl(FFMemoryResult* memory);

static void calculatePercentage(FFMemoryStorage* storage)
{
    if(storage->error.length != 0)
        return;

    if(storage->bytesTotal == 0)
        storage->percentage = 0;
    else
        storage->percentage = (uint8_t) ((storage->bytesUsed / (long double) storage->bytesTotal) * 100.0);
}

const FFMemoryResult* ffDetectMemory()
{
    FF_DETECTION_INTERNAL_GUARD(FFMemoryResult,
        ffStrbufInitA(&result.ram.error, 0);
        ffStrbufInitA(&result.swap.error, 0);

        ffDetectMemoryImpl(&result);
        calculatePercentage(&result.ram);
        calculatePercentage(&result.swap);
    );
}
