#include "frequency.h"

bool ffFreqAppendNum(uint32_t mhz, FFstrbuf* result)
{
    if (mhz == 0)
        return false;

    const FFOptionsDisplay* options = &instance.config.display;
    bool spaceBeforeUnit = options->freqSpaceBeforeUnit != FF_SPACE_BEFORE_UNIT_NEVER;
    int8_t ndigits = options->freqNdigits;

    if (ndigits >= 0)
    {
        ffStrbufAppendDouble(result, mhz / 1000., ndigits, true);
        if (spaceBeforeUnit) ffStrbufAppendC(result, ' ');
        ffStrbufAppendS(result, "GHz");
    }
    else
    {
        ffStrbufAppendUInt(result, mhz);
        if (spaceBeforeUnit) ffStrbufAppendC(result, ' ');
        ffStrbufAppendS(result, "MHz");
    }
    return true;
}
