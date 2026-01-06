#include "duration.h"

void ffDurationAppendNum(uint64_t totalSeconds, FFstrbuf* result)
{
    const FFOptionsDisplay* options = &instance.config.display;

    bool spaceBeforeUnit = options->durationSpaceBeforeUnit != FF_SPACE_BEFORE_UNIT_NEVER;

    if (totalSeconds < 60)
    {
        ffStrbufAppendUInt(result, totalSeconds);
        if (spaceBeforeUnit) ffStrbufAppendC(result, ' ');
        ffStrbufAppendS(result, options->durationAbbreviation ? "sec" : "second");
        if (totalSeconds != 1)
            ffStrbufAppendC(result, 's');
        return;
    }

    uint32_t seconds = (uint32_t) (totalSeconds % 60);
    totalSeconds /= 60;
    if (seconds >= 30)
        totalSeconds++;

    uint32_t minutes = (uint32_t) (totalSeconds % 60);
    totalSeconds /= 60;
    uint32_t hours = (uint32_t) (totalSeconds % 24);
    totalSeconds /= 24;
    uint32_t days = (uint32_t) totalSeconds;

    if (days > 0)
    {
        ffStrbufAppendUInt(result, days);
        if (spaceBeforeUnit) ffStrbufAppendC(result, ' ');
        if (options->durationAbbreviation)
        {
            ffStrbufAppendC(result, 'd');

            if (hours > 0 || minutes > 0)
                ffStrbufAppendC(result, ' ');
        }
        else
        {
            ffStrbufAppendS(result, days == 1 ? "day" : "days");

            if (days >= 100)
                ffStrbufAppendS(result, "(!)");

            if (hours > 0 || minutes > 0)
                ffStrbufAppendS(result, ", ");
        }
    }

    if (hours > 0)
    {
        ffStrbufAppendUInt(result, hours);
        if (spaceBeforeUnit) ffStrbufAppendC(result, ' ');
        if (options->durationAbbreviation)
        {
            ffStrbufAppendC(result, 'h');

            if (minutes > 0)
                ffStrbufAppendC(result, ' ');
        }
        else
        {
            ffStrbufAppendS(result, hours == 1 ? "hour" : "hours");

            if (minutes > 0)
                ffStrbufAppendS(result, ", ");
        }
    }

    if (minutes > 0)
    {
        ffStrbufAppendUInt(result, minutes);
        if (spaceBeforeUnit) ffStrbufAppendC(result, ' ');
        if (options->durationAbbreviation)
            ffStrbufAppendC(result, 'm');
        else
            ffStrbufAppendS(result, minutes == 1 ? "min" : "mins");
    }
}
