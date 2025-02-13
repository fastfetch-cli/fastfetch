#include "fastfetch.h"
#include "battery.h"
#include "common/io/io.h"
#include "util/stringUtils.h"

void parseBattery(FFstrbuf* content, FFlist* results)
{
    char* line = NULL;
    size_t n = 0;
    while (ffStrbufGetline(&line, &n, content))
    {

    }
}

const char* ffDetectBattery(FFBatteryOptions* options, FFlist* results)
{
    FF_AUTO_CLOSE_DIR DIR* dir = opendir("/dev/power/acpi_battery/");
    if (!dir) return "opendir(/dev/power/acpi_battery) failed";

    FF_STRBUF_AUTO_DESTROY content = ffStrbufCreate();
    struct dirent* entry;
    while ((entry = readdir(dir)))
    {
        if (entry->d_name[0] == '.') continue;
        if (!ffReadFileBufferRelative(dirfd(dir), entry->d_name, &content)) continue;
        parseBattery(&content, results);
    }

    return "To be supported";
}
