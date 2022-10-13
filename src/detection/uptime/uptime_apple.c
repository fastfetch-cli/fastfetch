#include "uptime.h"

#include <time.h>
#include <sys/sysctl.h>

uint64_t ffDetectUptime(const FFinstance* instance)
{
    FF_UNUSED(instance)
    struct timeval bootTime;
    size_t bootTimeSize = sizeof(bootTime);
    if(sysctl(
        (int[]) {CTL_KERN, KERN_BOOTTIME}, 2,
        &bootTime, &bootTimeSize,
        NULL, 0
    ) == 0)
        return (uint64_t) difftime(time(NULL), bootTime.tv_sec);

    return 0;
}
