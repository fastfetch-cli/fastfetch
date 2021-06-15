#include "fastfetch.h"

void ffPrintSeparator(FFinstance* instance)
{
    const FFTitleResult* result = ffDetectTitle(instance);
    uint32_t titleLength = result->userName.length + 1 + result->hostname.length;

    ffPrintLogoLine(instance);

    for(uint32_t i = 0; i < titleLength; i++)
        putchar('-');
    putchar('\n');
}
