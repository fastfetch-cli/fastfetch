#include "duration.h"

void ffDurationAppendNum(uint64_t totalSeconds, FFstrbuf* result)
{
    const FFOptionsDisplay* options = &instance.config.display;

    const char* space = instance.config.display.durationSpaceBeforeUnit != FF_SPACE_BEFORE_UNIT_NEVER ? " " : "";

    if(totalSeconds < 60)
    {
        ffStrbufAppendF(result, options->durationAbbreviation ? "%u%ssec" : "%u%ssecond", (unsigned) totalSeconds, space);
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

    if(days > 0)
    {
        if(options->durationAbbreviation)
        {
            ffStrbufAppendF(result, "%u%sd", days, space);

            if(hours > 0 || minutes > 0)
                ffStrbufAppendC(result, ' ');
        }
        else
        {
            ffStrbufAppendF(result, "%u%sday", days, space);

            if(days > 1)
                ffStrbufAppendC(result, 's');

            if(days >= 100)
                ffStrbufAppendS(result, "(!)");

            if(hours > 0 || minutes > 0)
                ffStrbufAppendS(result, ", ");
        }
    }

    if(hours > 0)
    {
        if(options->durationAbbreviation)
        {
            ffStrbufAppendF(result, "%u%sh", hours, space);

            if (minutes > 0)
                ffStrbufAppendC(result, ' ');
        }
        else
        {
            ffStrbufAppendF(result, "%u%shour", hours, space);

            if(hours > 1)
                ffStrbufAppendC(result, 's');

            if(minutes > 0)
                ffStrbufAppendS(result, ", ");
        }
    }

    if(minutes > 0)
    {
        if(options->durationAbbreviation)
        {
            ffStrbufAppendF(result, "%u%sm", minutes, space);
        }
        else
        {
            ffStrbufAppendF(result, "%u%smin", minutes, space);

            if(minutes > 1)
                ffStrbufAppendC(result, 's');
        }
    }
}
