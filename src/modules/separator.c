#include "fastfetch.h"

void ffPrintSeparator(FFinstance* instance)
{
    const FFTitleResult* result = ffDetectTitle(instance);
    uint32_t titleLength = result->userName.length + 1 +
        (instance->config.titleFQDN ? result->fqdn.length : result->hostname.length);

    ffLogoPrintLine(instance);

    if(instance->config.separatorString.length == 0)
    {
        for(uint32_t i = 0; i < titleLength; i++)
            putchar('-');
    }
    else
    {
        //Write the whole separator as often as it fits fully into titleLength
        for(uint32_t i = 0; i < titleLength / instance->config.separatorString.length; i++)
            ffStrbufWriteTo(&instance->config.separatorString, stdout);

        //Write as much of the separator as needed to fill titleLength
        for(uint32_t i = 0; i < titleLength % instance->config.separatorString.length; i++)
            putchar(instance->config.separatorString.chars[i]);
    }
    putchar('\n');
}
