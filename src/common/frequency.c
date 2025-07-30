#include "frequency.h"

bool ffFreqAppendNum(uint32_t mhz, FFstrbuf* result)
{
    if (mhz == 0)
        return false;

    const FFOptionsDisplay* options = &instance.config.display;
    const char* space = options->freqSpaceBeforeUnit == FF_SPACE_BEFORE_UNIT_NEVER ? "" : " ";
    int8_t ndigits = options->freqNdigits;

    if (ndigits >= 0)
        ffStrbufAppendF(result, "%.*f%sGHz", ndigits, mhz / 1000., space);
    else
        ffStrbufAppendF(result, "%u%sMHz", (unsigned) mhz, space);
    return true;
}
