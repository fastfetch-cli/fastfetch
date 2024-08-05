#include "camera.h"
#include "common/io/io.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#if FF_HAVE_LINUX_VIDEODEV2
    #include <linux/videodev2.h>
#endif

const char* ffDetectCamera(FFlist* result)
{
#if FF_HAVE_LINUX_VIDEODEV2
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
        switch (fmt.fmt.pix.colorspace)
        {
        case V4L2_COLORSPACE_SMPTE170M: ffStrbufInitStatic(&camera->colorspace, "SMPTE 170M"); break;
        case V4L2_COLORSPACE_SMPTE240M: ffStrbufInitStatic(&camera->colorspace, "SMPTE 240M"); break;
        case V4L2_COLORSPACE_BT878: ffStrbufInitStatic(&camera->colorspace, "BT.808"); break;
        case V4L2_COLORSPACE_470_SYSTEM_M: ffStrbufInitStatic(&camera->colorspace, "NTSC"); break;
        case V4L2_COLORSPACE_470_SYSTEM_BG: ffStrbufInitStatic(&camera->colorspace, "EBU 3213"); break;
        case V4L2_COLORSPACE_JPEG: ffStrbufInitStatic(&camera->colorspace, "JPEG"); break;
        case V4L2_COLORSPACE_REC709:
        case V4L2_COLORSPACE_SRGB: ffStrbufInitStatic(&camera->colorspace, "sRGB"); break;
        case V4L2_COLORSPACE_OPRGB: ffStrbufInitStatic(&camera->colorspace, "Adobe RGB"); break;
        case V4L2_COLORSPACE_BT2020: ffStrbufInitStatic(&camera->colorspace, "BT.2020"); break;
        case V4L2_COLORSPACE_RAW: ffStrbufInitStatic(&camera->colorspace, "RAW"); break;
        case V4L2_COLORSPACE_DCI_P3: ffStrbufInitStatic(&camera->colorspace, "DCI-P3"); break;
        default: ffStrbufInit(&camera->colorspace); break;
        }
        camera->width = fmt.fmt.pix.width;
        camera->height = fmt.fmt.pix.height;
    }

    return NULL;
#else
    FF_UNUSED(result);
    return "Fastfetch was compiled without <linux/videodev2.h>";
#endif
}
