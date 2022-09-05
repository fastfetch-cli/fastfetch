#include "fastfetch.h"
#include "common/printing.h"
#include "common/parsing.h"
#include "detection/memory/memory.h"

#define FF_MEMORY_MODULE_NAME "Memory"
#define FF_MEMORY_NUM_FORMAT_ARGS 3

void ffPrintMemory(FFinstance* instance)
{
    const FFMemoryResult* memory = ffDetectMemory();

    if(memory->bytesUsed == 0 && memory->bytesTotal == 0)
    {
        ffPrintError(instance, FF_MEMORY_MODULE_NAME, 0, &instance->config.memory, "Failed to detect memory");
        return;
    }

    FFstrbuf usedPretty;
    ffStrbufInit(&usedPretty);
    ffParseSize(memory->bytesUsed, instance->config.binaryPrefixType, &usedPretty);

    FFstrbuf totalPretty;
    ffStrbufInit(&totalPretty);
    ffParseSize(memory->bytesTotal, instance->config.binaryPrefixType, &totalPretty);

    if(instance->config.memory.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_MEMORY_MODULE_NAME, 0, &instance->config.memory.key);
        printf("%s / %s (%u%%)\n", usedPretty.chars, totalPretty.chars, memory->percentage);
    }
    else
    {
        ffPrintFormat(instance, FF_MEMORY_MODULE_NAME, 0, &instance->config.memory, FF_MEMORY_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &usedPretty},
            {FF_FORMAT_ARG_TYPE_STRBUF, &totalPretty},
            {FF_FORMAT_ARG_TYPE_UINT8, &memory->percentage},
        });
    }

    ffStrbufDestroy(&usedPretty);
    ffStrbufDestroy(&totalPretty);
}
