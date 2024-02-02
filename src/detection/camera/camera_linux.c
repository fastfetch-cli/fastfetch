#include "camera.h"
#include "common/io/io.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#if __has_include(<linux/videodev2.h>)
    #include <linux/videodev2.h>
#endif

const char* ffDetectCamera(FFlist* result)
{
#if __has_include(<linux/videodev2.h>)
    char path[] = "/dev/videoN";

    for (uint32_t i = 0; i <= 9; ++i)
    {
        path[sizeof(path) - 2] = (char) (i + '0');
        FF_AUTO_CLOSE_FD int fd = open(path, O_RDONLY);
        if (fd < 0)
            break;

        struct v4l2_capability cap = {};
        if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0 || !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
            continue;

        struct v4l2_format fmt = { .type = V4L2_BUF_TYPE_VIDEO_CAPTURE };
        if (ioctl(fd, VIDIOC_G_FMT, &fmt) < 0)
            continue;

        FFCameraResult* camera = (FFCameraResult*) ffListAdd(result);
        ffStrbufInitS(&camera->name, (const char*) cap.card);
        ffStrbufInit(&camera->vendor);
        ffStrbufInitS(&camera->id, (const char*) cap.bus_info);
        camera->width = camera->height = 0;

        camera->width = fmt.fmt.pix.width;
        camera->height = fmt.fmt.pix.height;
    }

    return NULL;
#else
    return "Fastfetch was compiled without <linux/videodev2.h>";
#endif
}
