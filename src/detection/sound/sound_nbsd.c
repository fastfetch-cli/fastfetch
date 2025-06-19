#include "sound.h"
#include "common/io/io.h"

#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/audioio.h>
#include <sys/ioctl.h>

const char* ffDetectSound(FFlist* devices)
{
    int defaultDev;
    {
        char audiop[12];
        ssize_t plen = readlink("/dev/audio", audiop, ARRAY_SIZE(audiop));
        if (plen < (ssize_t) strlen("audioN"))
            return "readlink(/dev/audio) failed";
        defaultDev = audiop[plen - 1] - '0';
        if (defaultDev < 0 || defaultDev > 9)
            return "Invalid audio device";
    }

    char path[] = "/dev/audio0";

    for (int idev = 0; idev < 9; ++idev)
    {
        path[strlen("/dev/audio")] = (char) ('0' + idev);
        FF_AUTO_CLOSE_FD int fd = open(path, O_RDWR | O_CLOEXEC);
        if (fd < 0) break;

        audio_device_t ad;
        if (ioctl(fd, AUDIO_GETDEV, &ad) < 0)
            continue;

        audio_info_t ai;
        if (ioctl(fd, AUDIO_GETINFO, &ai) < 0)
            continue;

        FFSoundDevice* device = ffListAdd(devices);
        ffStrbufInitS(&device->identifier, path);
        ffStrbufInitS(&device->name, ad.name);
        ffStrbufTrimRightSpace(&device->name);
        ffStrbufInitF(&device->platformApi, "%s", "SunAudio");
        device->volume = (uint8_t) ((ai.play.gain * 100 + AUDIO_MAX_GAIN / 2) / AUDIO_MAX_GAIN);
        device->active = true;
        device->main = defaultDev == idev;
    }

    return NULL;
}
