#include "fastfetch.h"
#include "common/printing.h"
#include "common/parsing.h"
#include "detection/memory/memory.h"
#include "detection/swap/swap.h"

#define FF_MEMORY_MODULE_NAME "Memory"
#define FF_SWAP_MODULE_NAME "Swap"

#define FF_MEMORY_NUM_FORMAT_ARGS 3

static uint8_t calculatePercentage(const FFMemoryStorage* storage)
{
    if(storage->error.length != 0 || storage->bytesTotal == 0)
        return 0;
    else
        return (uint8_t) (((long double) storage->bytesUsed / (long double) storage->bytesTotal) * 100.0);
}

static void printMemory(FFinstance* instance, const char* name, const FFModuleArgs* moduleArgs, const FFMemoryStorage* storage)
{
    if(storage->error.length > 0)
    {
        ffPrintError(instance, name, 0, moduleArgs, "%s", storage->error.chars);
        return;
    }

    FFstrbuf usedPretty;
    ffStrbufInit(&usedPretty);
    ffParseSize(storage->bytesUsed, instance->config.binaryPrefixType, &usedPretty);

    FFstrbuf totalPretty;
    ffStrbufInit(&totalPretty);
    ffParseSize(storage->bytesTotal, instance->config.binaryPrefixType, &totalPretty);

    uint8_t percentage = calculatePercentage(storage);

    if(moduleArgs->outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, name, 0, &moduleArgs->key);
        if (storage->bytesTotal == 0)
            puts("Disabled");
        else
            printf("%s / %s (%u%%)\n", usedPretty.chars, totalPretty.chars, percentage);
    }
    else
    {
        ffPrintFormat(instance, name, 0, moduleArgs, FF_MEMORY_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &usedPretty},
            {FF_FORMAT_ARG_TYPE_STRBUF, &totalPretty},
            {FF_FORMAT_ARG_TYPE_UINT8, &percentage},
        });
    }

    ffStrbufDestroy(&usedPretty);
    ffStrbufDestroy(&totalPretty);
}

void ffPrintMemory(FFinstance* instance)
{
    printMemory(instance, FF_MEMORY_MODULE_NAME, &instance->config.memory, ffDetectMemory());
}

void ffPrintSwap(FFinstance* instance)
{
    printMemory(instance, FF_SWAP_MODULE_NAME, &instance->config.swap, ffDetectSwap());
}
