#include "brightness.h"
#include "common/io/io.h"

#include <dev/wscons/wsconsio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

const char* ffDetectBrightness(FF_MAYBE_UNUSED FFBrightnessOptions* options, FFlist* result)
{
    FF_AUTO_CLOSE_FD int devfd = open("/dev/ttyC0", O_RDONLY);

    if (devfd < 0) return "open(dev/ttyC0, O_RDONLY) failed";

    struct wsdisplay_param param = {
        .param = WSDISPLAYIO_PARAM_BRIGHTNESS,
    };

    if (ioctl(devfd, WSDISPLAYIO_GETPARAM, &param) < 0)
        return "ioctl(WSDISPLAYIO_GETPARAM) failed";

    FFBrightnessResult* brightness = (FFBrightnessResult*) ffListAdd(result);
    ffStrbufInitStatic(&brightness->name, "wsdisplay");

    brightness->max = param.max;
    brightness->min = param.min;
    brightness->current = param.curval;

    return NULL;
}
