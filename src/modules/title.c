#include "fastfetch.h"
#include "common/printing.h"
#include "detection/title.h"
#include "util/textModifier.h"

static inline void printTitlePart(FFinstance* instance, const FFstrbuf* content)
{
    if(!instance->config.pipe)
    {
        fputs(FASTFETCH_TEXT_MODIFIER_BOLT, stdout);
        ffPrintColor(&instance->config.colorTitle);
    }

    ffStrbufWriteTo(content, stdout);

    if(!instance->config.pipe)
        fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);
}

void ffPrintTitle(FFinstance* instance)
{
    const FFTitleResult* result = ffDetectTitle(instance);

    ffLogoPrintLine(instance);

    printTitlePart(instance, &result->userName);
    putchar('@');
    printTitlePart(instance, instance->config.titleFQDN ? &result->fqdn : &result->hostname);
    putchar('\n');
}
