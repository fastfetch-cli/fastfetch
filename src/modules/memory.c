#include "fastfetch.h"
#include "common/printing.h"
#include "common/parsing.h"

#include <stdlib.h>

#define FF_MEMORY_MODULE_NAME "Memory"
#define FF_MEMORY_NUM_FORMAT_ARGS 3

// Impl inspired by: https://github.com/sam-barr/paleofetch/blob/b7c58a52c0de39b53c9b5f417889a5886d324bfa/paleofetch.c#L544
void ffPrintMemory(FFinstance* instance)
{
    FILE* meminfo = fopen("/proc/meminfo", "r");
    if(meminfo == NULL) {
        ffPrintError(instance, FF_MEMORY_MODULE_NAME, 0, &instance->config.memory, "fopen(\"""/proc/meminfo\", \"r\") == NULL");
        return;
    }

    char* line = NULL;
    size_t len = 0;

    uint32_t total = 0,
             shared = 0,
             memfree = 0,
             buffers = 0,
             cached = 0,
             reclaimable = 0;

    while (getline(&line, &len, meminfo) != -1) {
        sscanf(line, "MemTotal: %u", &total);
        sscanf(line, "Shmem: %u", &shared);
        sscanf(line, "MemFree: %u", &memfree);
        sscanf(line, "Buffers: %u", &buffers);
        sscanf(line, "Cached: %u", &cached);
        sscanf(line, "SReclaimable: %u", &reclaimable);
    }

    if(line != NULL)
        free(line);

    fclose(meminfo);

    uint32_t used = total + shared - memfree - buffers - cached - reclaimable;
    uint8_t percentage = (uint8_t) ((used / (double) total) * 100);

    if(used == 0 && total == 0)
    {
        ffPrintError(instance, FF_MEMORY_MODULE_NAME, 0, &instance->config.memory, "/proc/meminfo could't be parsed");
        return;
    }

    FFstrbuf usedPretty;
    ffStrbufInit(&usedPretty);
    ffParseSize(used * (uint64_t) 1024, instance->config.binaryPrefixType, &usedPretty);

    FFstrbuf totalPretty;
    ffStrbufInit(&totalPretty);
    ffParseSize(total * (uint64_t) 1024, instance->config.binaryPrefixType, &totalPretty);

    if(instance->config.memory.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_MEMORY_MODULE_NAME, 0, &instance->config.memory.key);
        printf("%s / %s (%u%%)\n", usedPretty.chars, totalPretty.chars, percentage);
    }
    else
    {
        ffPrintFormat(instance, FF_MEMORY_MODULE_NAME, 0, &instance->config.memory, FF_MEMORY_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &usedPretty},
            {FF_FORMAT_ARG_TYPE_STRBUF, &totalPretty},
            {FF_FORMAT_ARG_TYPE_UINT8, &percentage},
        });
    }

    ffStrbufDestroy(&usedPretty);
    ffStrbufDestroy(&totalPretty);
}
