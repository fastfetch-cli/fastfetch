#include "size.h"

#include <inttypes.h>

static void appendNum(FFstrbuf* result, uint64_t bytes, uint32_t base, const char** prefixes)
{
    const FFOptionsDisplay* options = &instance.config.display;
    double size = (double) bytes;
    uint8_t counter = 0;

    while(size >= base && counter < options->sizeMaxPrefix && prefixes[counter + 1])
    {
        size /= base;
        counter++;
    }

    const char* space = options->sizeSpaceBeforeUnit == FF_SPACE_BEFORE_UNIT_NEVER ? "" : " ";
    if(counter == 0)
        ffStrbufAppendF(result, "%" PRIu64 "%s%s", bytes, space, prefixes[0]);
    else
        ffStrbufAppendF(result, "%.*f%s%s", options->sizeNdigits, size, space, prefixes[counter]);
}

void ffSizeAppendNum(uint64_t bytes, FFstrbuf* result)
{
    const FFOptionsDisplay* options = &instance.config.display;
    switch (options->sizeBinaryPrefix)
    {
        case FF_SIZE_BINARY_PREFIX_TYPE_IEC:
            appendNum(result, bytes, 1024, (const char*[]) {"B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB", "ZiB", "YiB", NULL});
            break;
        case FF_SIZE_BINARY_PREFIX_TYPE_SI:
            appendNum(result, bytes, 1000, (const char*[]) {"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB", NULL});
            break;
        case FF_SIZE_BINARY_PREFIX_TYPE_JEDEC:
            appendNum(result, bytes, 1024, (const char*[]) {"B", "KB", "MB", "GB", "TB", NULL});
            break;
        default:
            appendNum(result, bytes, 1024, (const char*[]) {"B", NULL});
            break;
    }
}
