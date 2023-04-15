#include "fastfetch.h"
#include "common/printing.h"
#include "common/parsing.h"
#include "common/bar.h"
#include "detection/swap/swap.h"

#define FF_SWAP_MODULE_NAME "Swap"
#define FF_SWAP_NUM_FORMAT_ARGS 3

void ffPrintSwap(FFinstance* instance)
{
    FFSwapResult storage;
    const char* error = ffDetectSwap(&storage);

    if(error)
    {
        ffPrintError(instance, FF_SWAP_MODULE_NAME, 0, &instance->config.swap, "%s", error);
        return;
    }

    FF_STRBUF_AUTO_DESTROY usedPretty = ffStrbufCreate();
    ffParseSize(storage.bytesUsed, instance->config.binaryPrefixType, &usedPretty);

    FF_STRBUF_AUTO_DESTROY totalPretty = ffStrbufCreate();
    ffParseSize(storage.bytesTotal, instance->config.binaryPrefixType, &totalPretty);

    uint8_t percentage = storage.bytesTotal == 0
        ? 0
        : (uint8_t) (((long double) storage.bytesUsed / (long double) storage.bytesTotal) * 100.0);

    if(instance->config.swap.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_SWAP_MODULE_NAME, 0, &instance->config.swap.key);
        if (storage.bytesTotal == 0)
            puts("Disabled");
        else
        {
            FF_STRBUF_AUTO_DESTROY str = ffStrbufCreate();

            if(instance->config.percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
            {
                ffAppendPercentBar(instance, &str, percentage, 0, 5, 8);
                ffStrbufAppendC(&str, ' ');
            }

            if(!(instance->config.percentType & FF_PERCENTAGE_TYPE_HIDE_OTHERS_BIT))
                ffStrbufAppendF(&str, "%s / %s ", usedPretty.chars, totalPretty.chars);

            if(instance->config.percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
                ffAppendPercentNum(instance, &str, (uint8_t) percentage, 50, 80, str.length > 0);

            ffStrbufTrimRight(&str, ' ');
            ffStrbufPutTo(&str, stdout);
        }
    }
    else
    {
        ffPrintFormat(instance, FF_SWAP_MODULE_NAME, 0, &instance->config.swap, FF_SWAP_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &usedPretty},
            {FF_FORMAT_ARG_TYPE_STRBUF, &totalPretty},
            {FF_FORMAT_ARG_TYPE_UINT8, &percentage},
        });
    }
}
