#include "brightness.h"
#include "common/io/io.h"

#include <dirent.h>
#include <ctype.h>
#include <limits.h>

const char* ffDetectBrightness(FF_MAYBE_UNUSED FFlist* result)
{
    //https://www.kernel.org/doc/Documentation/ABI/stable/sysfs-class-backlight
    const char* backlightDirPath = "/sys/class/backlight/";

    DIR* dirp = opendir(backlightDirPath);
    if(dirp == NULL)
        return "Failed to open `/sys/class/backlight/`";

    FF_STRBUF_AUTO_DESTROY backlightDir;
    ffStrbufInitA(&backlightDir, 64);
    ffStrbufAppendS(&backlightDir, backlightDirPath);

    uint32_t backlightDirLength = backlightDir.length;

    FF_STRBUF_AUTO_DESTROY buffer;
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
                ffStrbufSubstrBeforeLastC(&backlightDir, '/');
                ffStrbufAppendS(&backlightDir, "/device");
                ffStrbufInitA(&display->name, PATH_MAX + 1);
                if(realpath(backlightDir.chars, display->name.chars))
                {
                    ffStrbufRecalculateLength(&display->name);
                    ffStrbufSubstrAfterLastC(&display->name, '/');
                    if(ffStrbufStartsWithS(&display->name, "card") && isdigit(display->name.chars[4]))
                        ffStrbufSubstrAfterFirstC(&display->name, '-');
                }
                else
                    ffStrbufInitS(&display->name, entry->d_name);
                double maxBrightness = ffStrbufToDouble(&buffer);
                display->value = (float) (actualBrightness * 100 / maxBrightness);
            }
        }
        ffStrbufSubstrBefore(&backlightDir, backlightDirLength);
    }

    closedir(dirp);

    return NULL;
}
