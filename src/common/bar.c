#include "bar.h"

// green, yellow, red: print the color on nth (0~9) block
// set its value == 10 means the color will not be printed
void ffAppendPercentBar(FFinstance* instance, FFstrbuf* buffer, uint8_t percent, uint8_t green, uint8_t yellow, uint8_t red)
{
    assert(green <= 10 && yellow <= 10 && red <= 10);

    // [ 0%,  5%) prints 0 blocks
    // [ 5%, 15%) prints 1 block;
    // ...
    // [85%, 95%) prints 9 blocks;
    // [95%,100%] prints 10 blocks
    percent = (percent + 5) / 10;
    assert(percent <= 10);

    if(!instance->config.pipe)
        ffStrbufAppendS(buffer, "\033[97m[ ");
    else
        ffStrbufAppendS(buffer, "[ ");

    for (uint8_t i = 0; i < percent; ++i)
    {
        if(!instance->config.pipe)
        {
            if (i == green)
                ffStrbufAppendS(buffer, "\033[32m");
            else if (i == yellow)
                ffStrbufAppendS(buffer, "\033[93m");
            else if (i == red)
                ffStrbufAppendS(buffer, "\033[91m");
        }
        ffStrbufAppendS(buffer, "■");
    }

    if (percent < 10)
    {
        if(!instance->config.pipe)
            ffStrbufAppendS(buffer, "\033[97m");
        for (uint8_t i = percent; i < 10; ++i)
            ffStrbufAppendS(buffer, "-");
    }

    if(!instance->config.pipe)
        ffStrbufAppendS(buffer, "\033[97m ]" FASTFETCH_TEXT_MODIFIER_RESET);
    else
        ffStrbufAppendS(buffer, " ]");
}
