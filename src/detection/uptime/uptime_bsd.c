#include "uptime.h"

#include <time.h>
#include <sys/sysctl.h>
#include <sys/time.h>

const char* ffDetectUptime(uint64_t* result)
{
    struct timeval bootTime;
    size_t bootTimeSize = sizeof(bootTime);
    if(sysctl(
        (int[]) {CTL_KERN, KERN_BOOTTIME}, 2,
        &bootTime, &bootTimeSize,
        NULL, 0
    ) != 0)
        return "sysctl({CTL_KERN, KERN_BOOTTIME}) failed";

    *result = (uint64_t) difftime(time(NULL), bootTime.tv_sec);

    return NULL;
}
