#include "fastfetch.h"
#include "detection/cpuusage/cpuusage.h"

const char* ffGetCpuUsageInfo([[maybe_unused]] FFlist* cpuTimes) {
    return "Not support on this platform";
}
