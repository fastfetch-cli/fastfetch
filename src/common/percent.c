#include "fastfetch.h"
#include "common/percent.h"
#include "common/color.h"
#include "util/textModifier.h"

void ffPercentAppendBar(FFstrbuf* buffer, double percent, uint8_t green, uint8_t yellow)
{
    assert(green <= 100 && yellow <= 100);

    const FFOptionsDisplay* options = &instance.config.display;

    uint32_t blocksPercent = (uint32_t) (percent / 100.0 * options->barWidth + 0.5);
    assert(blocksPercent <= options->barWidth);

    if(options->barBorder)
    {
        if(!options->pipe)
            ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_LIGHT_WHITE "m[ ");
        else
            ffStrbufAppendS(buffer, "[ ");
    }

    for (uint32_t i = 0; i < blocksPercent; ++i)
    {
        if(!options->pipe)
        {
            uint32_t section1Begin = (uint32_t) ((green <= yellow ? green : yellow) / 100.0 * options->barWidth + 0.5);
            uint32_t section2Begin = (uint32_t) ((green > yellow ? green : yellow) / 100.0 * options->barWidth + 0.5);
            if (i == section2Begin)
                ffStrbufAppendF(buffer, "\e[%sm", (green > yellow ? FF_COLOR_FG_LIGHT_GREEN : FF_COLOR_FG_LIGHT_RED));
            else if (i == section1Begin)
                ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_LIGHT_YELLOW "m");
            else if (i == 0)
                ffStrbufAppendF(buffer, "\e[%sm", (green <= yellow ? FF_COLOR_FG_GREEN : FF_COLOR_FG_LIGHT_RED));
        }
        ffStrbufAppend(buffer, &options->barCharElapsed);
    }

    if (blocksPercent < options->barWidth)
    {
        if(!options->pipe)
            ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_LIGHT_WHITE "m");
        for (uint32_t i = blocksPercent; i < options->barWidth; ++i)
            ffStrbufAppend(buffer, &options->barCharTotal);
    }

    if(options->barBorder)
    {
        if(!options->pipe)
            ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_LIGHT_WHITE "m ]");
        else
            ffStrbufAppendS(buffer, " ]");
    }

    if(!options->pipe)
        ffStrbufAppendS(buffer, FASTFETCH_TEXT_MODIFIER_RESET);
}

void ffPercentAppendNum(FFstrbuf* buffer, double percent, uint8_t green, uint8_t yellow, bool parentheses)
{
    assert(green <= 100 && yellow <= 100);

    const FFOptionsDisplay* options = &instance.config.display;

    bool colored = !!(options->percentType & FF_PERCENTAGE_TYPE_NUM_COLOR_BIT);

    if (parentheses)
        ffStrbufAppendC(buffer, '(');

    if (colored && !options->pipe)
    {
        if(percent != percent)
            ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_LIGHT_BLACK "m");
        else if(green <= yellow)
        {
            if (percent > yellow)
                ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_LIGHT_RED "m");
            else if (percent > green)
                ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_LIGHT_YELLOW "m");
            else
                ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_GREEN "m");

        }
        else
        {
            if (percent < yellow)
                ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_LIGHT_RED "m");
            else if (percent < green)
                ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_LIGHT_YELLOW "m");
            else
                ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_GREEN "m");
        }
    }
    ffStrbufAppendF(buffer, "%.*f%%", options->percentNdigits, percent);

    if (colored && !options->pipe)
    {
        ffStrbufAppendS(buffer, FASTFETCH_TEXT_MODIFIER_RESET);
    }

    if (parentheses)
        ffStrbufAppendC(buffer, ')');
}
