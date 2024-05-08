#include "detection/loadavg/loadavg.h"

#include <stdlib.h>

const char* ffDetectLoadavg(double result[3])
{
    return getloadavg(result, 3) < 0 ? "getloadavg() failed" : NULL;
}
