#include "fastfetch.h"
#include "common/printing.h"
#include "util/textModifier.h"

void ffPrintColors(FFinstance* instance)
{
    if(instance->config.pipe)
        return;

    ffLogoPrintLine(instance);

    // 4%d: Set the background color
    // 3%d: Set the foreground color
    for(uint8_t i = 0; i < 8; i++)
        printf("\033[4%d;3%dm███", i, i);

    puts(FASTFETCH_TEXT_MODIFIER_RESET);

    ffLogoPrintLine(instance);

    // 1: Set everything to bolt. This causes normal colors on some systems to be bright.
    // 4%d: Set the backgound to the not bright color
    // 3%d: Set the foreground to the not bright color
    // 10%d: Set the background to the bright color
    // 9%d: Set the foreground to the bright color
    for(uint8_t i = 0; i < 8; i++)
        printf("\033[1;4%d;3%d;10%d;9%dm███", i, i, i, i);

    puts(FASTFETCH_TEXT_MODIFIER_RESET);
}
