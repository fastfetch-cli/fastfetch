#include "detection/loadavg/loadavg.h"

#include <sys/loadavg.h>

const char* ffDetectLoadavg(double result[3])
{
    return getloadavg(result, 3) == 3 ? NULL : "getloadavg() failed";
}
