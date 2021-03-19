#include "fastfetch.h"

#include <sys/statvfs.h>

void ffPrintDisk(FFinstance* instance)
{
    struct statvfs fs;
    int ret = statvfs("/", &fs);
    if(ret != 0)
    {
        ffPrintError(instance, "Disk (/)", "statvfs(\"/\", &fs) != 0");
        return;
    }

    uint32_t GB = (1024 * 1024) * 1024;

    uint32_t total     = (fs.f_blocks * fs.f_frsize) / GB;
    uint32_t available = (fs.f_bfree  * fs.f_frsize) / GB;
    uint32_t used = total - available;
    uint8_t percentage = (used / (double) total) * 100.0;

    ffPrintLogoAndKey(instance, "Disk (/)");

    if(ffStrbufIsEmpty(&instance->config.diskFormat))
    {
        printf("%uGB / %uGB (%u%%)\n", used, total, percentage);
    }
    else
    {
        FFstrbuf disk;
        ffStrbufInit(&disk);

        ffParseFormatString(&disk, &instance->config.diskFormat, 3,
            (FFformatarg){FF_FORMAT_ARG_TYPE_UINT, &used},
            (FFformatarg){FF_FORMAT_ARG_TYPE_UINT, &total},
            (FFformatarg){FF_FORMAT_ARG_TYPE_UINT8, &percentage}
        );

        ffStrbufWriteTo(&disk, stdout);
        putchar('\n');
        ffStrbufDestroy(&disk);
    }
}