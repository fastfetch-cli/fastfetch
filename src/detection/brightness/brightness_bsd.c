#include "brightness.h"

#if __has_include(<sys/backlight.h>)

#include "common/io/io.h"

#include <sys/backlight.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <unistd.h>

const char* ffDetectBrightness(FF_MAYBE_UNUSED FFBrightnessOptions* options, FFlist* result)
{
    // https://man.freebsd.org/cgi/man.cgi?query=backlight&sektion=9
    char path[] = "/dev/backlight/backlight0";

    for (char i = '0'; i <= '9'; ++i)
    {
        path[ARRAY_SIZE(path) - 2] = i;

        FF_AUTO_CLOSE_FD int blfd = open(path, O_RDONLY | O_CLOEXEC);
        if (blfd < 0)
            continue;

        struct backlight_props status;
        if(ioctl(blfd, BACKLIGHTGETSTATUS, &status) < 0)
            continue;

        FFBrightnessResult* brightness = (FFBrightnessResult*) ffListAdd(result);
        ffStrbufInit(&brightness->name);

        brightness->max = BACKLIGHTMAXLEVELS;
        brightness->min = 0;
        brightness->current = status.brightness;

        struct backlight_info info;
        if(ioctl(blfd, BACKLIGHTGETINFO, &info) < 0)
            ffStrbufAppendS(&brightness->name, info.name);
        else
            ffStrbufAppendS(&brightness->name, path + strlen("/dev/backlight/"));
    }
    return NULL;
}

#else

const char* ffDetectBrightness(FF_MAYBE_UNUSED FFBrightnessOptions* options, FF_MAYBE_UNUSED FFlist* result)
{
    return "Backlight is supported only on FreeBSD 13 and newer";
}

#endif
