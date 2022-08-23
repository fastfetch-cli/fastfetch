#include "fastfetch.h"
#include "common/printing.h"

#include <stdlib.h>

#define FF_MEMORY_MODULE_NAME "Memory"
#define FF_MEMORY_NUM_FORMAT_ARGS 7

static void printValue(uint32_t kib, double mib, double gib)
{
    if(gib >= 1.0)
        printf("%.2f GiB", gib);
    else if(mib >= 1.0)
        printf("%.2f MiB", mib);
    else
        printf("%u KiB", kib);
}

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

    double usedMiB = used / 1024.0;
    double totalMiB = total / 1024.0;

    double usedGiB = usedMiB / 1024.0;
    double totalGiB = totalMiB / 1024.0;

    if(used == 0 && total == 0)
    {
        ffPrintError(instance, FF_MEMORY_MODULE_NAME, 0, &instance->config.memory, "/proc/meminfo could't be parsed");
        return;
    }

    if(instance->config.memory.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_MEMORY_MODULE_NAME, 0, &instance->config.memory.key);
        printValue(used, usedMiB, usedGiB);
        fputs(" / ", stdout);
        printValue(total, totalMiB, totalGiB);
        printf(" (%u%%)\n", percentage);
    }
    else
    {
        ffPrintFormat(instance, FF_MEMORY_MODULE_NAME, 0, &instance->config.memory, FF_MEMORY_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_UINT8, &percentage},
            {FF_FORMAT_ARG_TYPE_UINT, &total},
            {FF_FORMAT_ARG_TYPE_UINT, &used},
            {FF_FORMAT_ARG_TYPE_DOUBLE, &usedMiB},
            {FF_FORMAT_ARG_TYPE_DOUBLE, &totalMiB},
            {FF_FORMAT_ARG_TYPE_DOUBLE, &usedGiB},
            {FF_FORMAT_ARG_TYPE_DOUBLE, &totalGiB}
        });
    }
}
