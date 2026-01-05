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

    if (counter == 0)
        ffStrbufAppendUInt(result, bytes);
    else
        ffStrbufAppendDouble(result, size, (int8_t) options->sizeNdigits, true);
    if (options->sizeSpaceBeforeUnit != FF_SPACE_BEFORE_UNIT_NEVER)
        ffStrbufAppendC(result, ' ');
    ffStrbufAppendS(result, prefixes[counter]);
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
