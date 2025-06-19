#include "brightness.h"
#include "common/io/io.h"

#include <dev/wscons/wsconsio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

const char* ffDetectBrightness(FF_MAYBE_UNUSED FFBrightnessOptions* options, FFlist* result)
{
    char path[] = "/dev/ttyCX";
    for (char i = '0'; i <= '9'; ++i) {
        path[strlen("/dev/ttyC")] = i;

        FF_AUTO_CLOSE_FD int devfd = open(path, O_RDONLY | O_CLOEXEC);

        if (devfd < 0) {
            if (errno == EACCES && i == '0')
                return "Permission denied when opening tty device";
            if (errno == ENOENT)
                break;
            continue;
        }

        struct wsdisplay_param param = {
            .param = WSDISPLAYIO_PARAM_BRIGHTNESS,
        };

        if (ioctl(devfd, WSDISPLAYIO_GETPARAM, &param) < 0)
            continue;

        FFBrightnessResult* brightness = (FFBrightnessResult*) ffListAdd(result);
        ffStrbufInitF(&brightness->name, "ttyC%c", i);

        brightness->max = param.max;
        brightness->min = param.min;
        brightness->current = param.curval;
        brightness->builtin = true;
    }

    return NULL;
}
