#include "fastfetch.h"

// Impl by: https://github.com/sam-barr/paleofetch/blob/b7c58a52c0de39b53c9b5f417889a5886d324bfa/paleofetch.c#L544
void ffPrintMemory(FFstate* state)
{
    ffPrintLogoAndKey(state, "Memory");

    FILE *meminfo = fopen("/proc/meminfo", "r");
    if(meminfo == NULL) {
        printf("[Error opening /proc/meminfo\n");
        return;
    }

    char *line = NULL;
    size_t len;

    uint32_t total, shared, memfree, buffers, cached, reclaimable;

    while (getline(&line, &len, meminfo) != -1) {
        sscanf(line, "MemTotal: %lu", &total);
        sscanf(line, "Shmem: %lu", &shared);
        sscanf(line, "MemFree: %lu", &memfree);
        sscanf(line, "Buffers: %lu", &buffers);
        sscanf(line, "Cached: %lu", &cached);
        sscanf(line, "SReclaimable: %lu", &reclaimable);
    }

    free(line);
    fclose(meminfo);

    uint32_t used_mem = (total + shared - memfree - buffers - cached - reclaimable) / 1024;
    uint32_t total_mem = total / 1024;
    uint8_t percentage = (uint8_t) ((used_mem / (double) total_mem) * 100);

    printf("%luMiB / %luMiB (%u%%)\n", used_mem, total_mem, percentage);
}