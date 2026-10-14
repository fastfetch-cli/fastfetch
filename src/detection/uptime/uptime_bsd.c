#include "uptime.h"
#include "common/debug.h"
#include "common/time.h"

#include <errno.h>
#include <string.h>
#include <sys/sysctl.h>
#include <sys/time.h>

const char* ffDetectUptime(FFUptimeResult* result) {
#if __NetBSD__
    struct timespec bootTime;
#else
    struct timeval bootTime;
#endif
    size_t bootTimeSize = sizeof(bootTime);
    if (sysctl(
            (int[]) { CTL_KERN, KERN_BOOTTIME }, 2, &bootTime, &bootTimeSize, nullptr, 0) != 0) {
        FF_DEBUG("sysctl({CTL_KERN, KERN_BOOTTIME}) failed: %s", strerror(errno));
        return "sysctl({CTL_KERN, KERN_BOOTTIME}) failed";
    }

#if __NetBSD__
    result->bootTime = (uint64_t) bootTime.tv_sec * 1000 + (uint64_t) bootTime.tv_nsec / 1000000;
#else
    result->bootTime = (uint64_t) bootTime.tv_sec * 1000 + (uint64_t) bootTime.tv_usec / 1000;
#endif
    result->uptime = ffTimeGetNow() - result->bootTime;

    return nullptr;
}
