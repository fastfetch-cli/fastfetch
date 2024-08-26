#include "detection/loadavg/loadavg.h"
#include "common/io/io.h"

#include <sys/sysinfo.h>

const char* ffDetectLoadavg(double result[3])
{
    #ifndef __ANDROID__ // cat: /proc/loadavg: Permission denied

    // Don't use syscall for container compatibility. #620
    char buf[64];
    ssize_t nRead = ffReadFileData("/proc/loadavg", sizeof(buf) - 1, buf);
    if (nRead > 0)
    {
        buf[nRead] = '\0';

        if (sscanf(buf, "%lf%lf%lf", &result[0], &result[1], &result[2]) == 3)
            return NULL;
    }

    #endif

    // getloadavg requires higher ANDROID_API version
    struct sysinfo si;
    if (sysinfo(&si) < 0)
        return "sysinfo() failed";

    for (int i = 0; i < 3; i++)
        result[i] = (double) si.loads[i] / (1 << SI_LOAD_SHIFT);
    return NULL;
}
