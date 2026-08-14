#include "detection/loadavg/loadavg.h"

#if __has_include(<sys/loadavg.h>)
    #include <sys/loadavg.h>
#else
    #include <stdlib.h>
#endif

const char* ffDetectLoadavg(double result[3]) {
    return getloadavg(result, 3) == 3 ? nullptr : "getloadavg() failed";
}
