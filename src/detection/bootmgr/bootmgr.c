#include "efi.h"

static inline uint8_t evBits(uint16_t val, uint8_t mask, uint8_t shift)
{
    return (uint8_t) ((val & (mask << shift)) >> shift);
}

void ffEfiUcs2ToUtf8(const uint16_t *const chars, FFstrbuf* result)
{
    for (uint32_t i = 0; chars[i]; i++)
    {
        if (chars[i] <= 0x007f)
            ffStrbufAppendC(result, (char) chars[i]);
        else if (chars[i] > 0x007f && chars[i] <= 0x07ff)
        {
            ffStrbufAppendC(result, (char) (0xc0 | evBits(chars[i], 0x1f, 6)));
            ffStrbufAppendC(result, (char) (0x80 | evBits(chars[i], 0x3f, 0)));
        }
        else
        {
            ffStrbufAppendC(result, (char) (0xe0 | evBits(chars[i], 0xf, 12)));
            ffStrbufAppendC(result, (char) (0x80 | evBits(chars[i], 0x3f, 6)));
            ffStrbufAppendC(result, (char) (0x80 | evBits(chars[i], 0x3f, 0)));
        }
    }
}
