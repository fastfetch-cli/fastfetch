#include "fastfetch.h"
#include "detection/cpuusage/cpuusage.h"
#include "common/io/io.h"

#include <stdio.h>
#include <inttypes.h>

const char* ffGetCpuUsageInfo(FFlist* cpuTimes)
{
    char buf[PROC_FILE_BUFFSIZ];
    ssize_t nRead = ffReadFileData("/proc/stat", ARRAY_SIZE(buf) - 1, buf);
    if(nRead < 0)
    {
        #ifdef __ANDROID__
        return "Accessing \"/proc/stat\" is restricted on Android O+";
        #else
        return "ffReadFileData(\"/proc/stat\", ARRAY_SIZE(buf) - 1, buf) failed";
        #endif
    }
    buf[nRead] = '\0';

    // Skip first line
    char *start = NULL;
    if((start = strchr(buf, '\n')) == NULL)
        return "skip first line failed";
    ++start;

    uint64_t user = 0, nice = 0, system = 0, idle = 0, iowait = 0, irq = 0, softirq = 0;
    char *token = NULL;
    while ((token = strchr(start, '\n')))
    {
        if(sscanf(start, "cpu%*d%" PRIu64 "%" PRIu64 "%" PRIu64 "%" PRIu64 "%" PRIu64 "%" PRIu64 "%" PRIu64 "%*[^\n]\n", &user, &nice, &system, &idle, &iowait, &irq, &softirq) == 7)
        {
            uint64_t inUse = user + nice + system;
            uint64_t total = inUse + idle + iowait + irq + softirq;

            FFCpuUsageInfo* info = (FFCpuUsageInfo*) ffListAdd(cpuTimes);
            *info = (FFCpuUsageInfo) {
                .inUseAll = inUse,
                .totalAll = total,
            };
        }
        else
            break;
        start = token + 1;
    }

    return NULL;
}
