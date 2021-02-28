#include "fastfetch.h"

#include <sys/statvfs.h>

void ffPrintDisk(FFinstance* instance)
{
    #ifdef FASTFETCH_BUILD_FLASHFETCH
    if(ffPrintCustomValue(instance, "Disk (/)"))
        return;
    #endif // FASTFETCH_BUILD_FLASHFETCH

    struct statvfs fs;
    int ret = statvfs("/", &fs);
    if(ret != 0)
    {
        ffPrintError(instance, "Disk (/)", "statvfs(\"/\", &fs) != 0");
        return;
    }

    const uint32_t GB = (1024 * 1024) * 1024;

    const uint32_t total     = (fs.f_blocks * fs.f_frsize) / GB;
    const uint32_t available = (fs.f_bfree  * fs.f_frsize) / GB;
    const uint32_t used = total - available;
    const uint32_t percentage = (used / (double) total) * 100.0;

    
    ffPrintLogoAndKey(instance, "Disk (/)");
    printf("%uGB / %uGB (%u%%)\n", used, total, percentage);
}