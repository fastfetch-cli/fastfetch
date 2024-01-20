#include "common/color.h"
#include "common/percent.h"
#include "util/textModifier.h"

void ffAppendPercentBar(FFstrbuf* buffer, double percent, uint8_t green, uint8_t yellow, uint8_t red)
{
    assert(green <= 100 && yellow <= 100 && red <= 100);

    const FFOptionsDisplay* options = &instance.config.display;

    uint32_t blocksPercent = (uint32_t) (percent / 100.0 * options->barWidth + 0.5);
    uint32_t blocksGreen = (uint32_t) (green / 100.0 * options->barWidth + 0.5);
    uint32_t blocksYellow = (uint32_t) (yellow / 100.0 * options->barWidth + 0.5);
    uint32_t blocksRed = (uint32_t) (red / 100.0 * options->barWidth + 0.5);
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
            if (i == blocksGreen)
                ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_GREEN "m");
            else if (i == blocksYellow)
                ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_LIGHT_YELLOW "m");
            else if (i == blocksRed)
                ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_LIGHT_RED "m");
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

// if (green < yellow)
// [0, green]: print green
// (green, yellow]: print yellow
// (yellow, 100]: print red
//
// if (green > yellow)
// [green, 100]: print green
// [yellow, green): print yellow
// [0, yellow): PRINT RED
void ffAppendPercentNum(FFstrbuf* buffer, double percent, uint8_t green, uint8_t yellow, bool parentheses)
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
        else if(green < yellow)
        {
            if (percent <= green)
                ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_GREEN "m");
            else if (percent <= yellow)
                ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_LIGHT_YELLOW "m");
            else
                ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_LIGHT_RED "m");
        }
        else
        {
            if (percent >= green)
                ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_GREEN "m");
            else if (percent >= yellow)
                ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_LIGHT_YELLOW "m");
            else
                ffStrbufAppendS(buffer, "\e[" FF_COLOR_FG_LIGHT_RED "m");
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
