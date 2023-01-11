#include "brightness.h"
#include "common/io.h"

#include <dirent.h>

const char* ffDetectBrightness(FF_MAYBE_UNUSED FFlist* result)
{
    //https://www.kernel.org/doc/Documentation/ABI/stable/sysfs-class-backlight
    const char* backlightDirPath = "/sys/class/backlight/";

    DIR* dirp = opendir(backlightDirPath);
    if(dirp == NULL)
        return "Failed to open `/sys/class/backlight/`";

    FFstrbuf backlightDir;
    ffStrbufInitA(&backlightDir, 64);
    ffStrbufAppendS(&backlightDir, backlightDirPath);

    uint32_t backlightDirLength = backlightDir.length;

    FFstrbuf buffer;
    ffStrbufInit(&buffer);

    struct dirent* entry;
    while((entry = readdir(dirp)) != NULL)
    {
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        ffStrbufAppendS(&backlightDir, entry->d_name);
        ffStrbufAppendS(&backlightDir, "/actual_brightness");
        if(ffReadFileBuffer(backlightDir.chars, &buffer))
        {
            double actualBrightness = ffStrbufToDouble(&buffer);
            ffStrbufSubstrBefore(&backlightDir, backlightDirLength);
            ffStrbufAppendS(&backlightDir, entry->d_name);
            ffStrbufAppendS(&backlightDir, "/max_brightness");
            if(ffReadFileBuffer(backlightDir.chars, &buffer))
            {
                FFBrightnessResult* display = (FFBrightnessResult*) ffListAdd(result);
                ffStrbufInitS(&display->name, entry->d_name);
                double maxBrightness = ffStrbufToDouble(&buffer);
                display->value = (float) (actualBrightness * 100 / maxBrightness);
            }

            break;
        }
    }

    closedir(dirp);
    ffStrbufDestroy(&backlightDir);
    ffStrbufDestroy(&buffer);

    return NULL;
}
