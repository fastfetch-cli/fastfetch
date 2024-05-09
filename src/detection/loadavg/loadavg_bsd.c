#include "detection/loadavg/loadavg.h"

#include <sys/sysctl.h>

const char* ffDetectLoadavg(double result[3])
{
    struct loadavg load;
    size_t size = sizeof(load);
    if (sysctl((int []) { CTL_VM, VM_LOADAVG }, 2, &load, &size, NULL, 0) < 0)
        return "sysctl({CTL_VM, VM_LOADAVG}) failed";
    for (int i = 0; i < 3; i++)
        result[i] = (double) load.ldavg[i] / (double) load.fscale;
    return NULL;
}
