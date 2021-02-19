#include "fastfetch.h"

#include <sys/statvfs.h>

void ffPrintDisk(FFstate* state)
{
    struct statvfs fs;
    int ret = statvfs("/", &fs);
    if(ret != 0)
    {
        return;
    }

    const uint32_t GB = (1024 * 1024) * 1024;

    const uint32_t total     = (fs.f_blocks * fs.f_frsize) / GB;
    const uint32_t available = (fs.f_bfree  * fs.f_frsize) / GB;
    const uint32_t used = total - available;
    const uint32_t percentage = (used / (double) total) * 100.0;

    
    ffPrintLogoAndKey(state, "Disk (/)");
    printf("%uGB / %uGB (%u%%)\n", used, total, percentage);
}