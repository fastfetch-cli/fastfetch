#include "fastfetch.h"
#include "common/printing.h"

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

    uint32_t used_mem = (total + shared - memfree - buffers - cached - reclaimable) / 1024;
    uint32_t total_mem = total / 1024;
    uint8_t percentage = (uint8_t) ((used_mem / (double) total_mem) * 100);

    if(used_mem == 0 && total_mem == 0 && percentage == 0)
    {
        ffPrintError(instance, FF_MEMORY_MODULE_NAME, 0, &instance->config.memory, "/proc/meminfo could't be parsed");
        return;
    }

    if(instance->config.memory.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_MEMORY_MODULE_NAME, 0, &instance->config.memory.key);
        printf("%uMiB / %uMiB (%u%%)\n", used_mem, total_mem, percentage);
    }
    else
    {
        ffPrintFormat(instance, FF_MEMORY_MODULE_NAME, 0, &instance->config.memory, FF_MEMORY_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_UINT, &used_mem},
            {FF_FORMAT_ARG_TYPE_UINT, &total_mem},
            {FF_FORMAT_ARG_TYPE_UINT8, &percentage}
        });
    }
}
