#include "fastfetch.h"

void ffPrintSeperator(FFinstance* instance)
{
    ffPrintLogoLine(instance);

    for(uint32_t i = 0; i < instance->config.titleLength; i++)
        putchar('-');
    putchar('\n');
}
