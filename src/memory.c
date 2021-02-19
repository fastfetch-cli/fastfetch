#include "fastfetch.h"

// Impl by: https://github.com/sam-barr/paleofetch/blob/b7c58a52c0de39b53c9b5f417889a5886d324bfa/paleofetch.c#L544
void ffPrintMemory(FFstate* state)
{
    FILE* meminfo = fopen("/proc/meminfo", "r");
    if(meminfo == NULL) {
        if(state->showErrors)
            ffPrintError(state, "Memory", "fopen(\"/proc/meminfo\", \"r\") == NULL");
        return;
    }

    char* line = NULL;
    size_t len;

    uint32_t total, shared, memfree, buffers, cached, reclaimable;

    while (getline(&line, &len, meminfo) != -1) {
        sscanf(line, "MemTotal: %u", &total);
        sscanf(line, "Shmem: %u", &shared);
        sscanf(line, "MemFree: %u", &memfree);
        sscanf(line, "Buffers: %u", &buffers);
        sscanf(line, "Cached: %u", &cached);
        sscanf(line, "SReclaimable: %u", &reclaimable);
    }

    free(line);
    fclose(meminfo);

    uint32_t used_mem = (total + shared - memfree - buffers - cached - reclaimable) / 1024;
    uint32_t total_mem = total / 1024;
    uint8_t percentage = (uint8_t) ((used_mem / (double) total_mem) * 100);

    ffPrintLogoAndKey(state, "Memory");
    printf("%uMiB / %uMiB (%u%%)\n", used_mem, total_mem, percentage);
}