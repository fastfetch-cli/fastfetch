#include "memory.h"
#include "detection/internal.h"

void ffDetectMemoryImpl(FFMemoryResult* memory);

const FFMemoryResult* ffDetectMemory()
{
    FF_DETECTION_INTERNAL_GUARD(FFMemoryResult,
        ffDetectMemoryImpl(&result);

        if(result.bytesTotal > 0)
            result.percentage = (uint8_t) ((result.bytesUsed / (long double) result.bytesTotal) * 100.0);
        else
            result.percentage = 0;
    );
}
