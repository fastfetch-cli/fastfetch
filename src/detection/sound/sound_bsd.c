#include "sound.h"
#include "common/io/io.h"
#include "common/sysctl.h"

#include <fcntl.h>
#include <sys/soundcard.h>

const char* ffDetectSound(FFlist* devices)
{
    char path[] = "/dev/mixer0";
    int defaultDev = ffSysctlGetInt("hw.snd.default_unit", -1);

    for (int idev = 0; idev <= 9; ++idev)
    {
        path[strlen("/dev/mixer")] = (char) ('0' + idev);
        FF_AUTO_CLOSE_FD int fd = open(path, O_RDWR);
        if (fd < 0) break;

        uint32_t devmask = 0;
        if (ioctl(fd, SOUND_MIXER_READ_DEVMASK, &devmask) < 0)
            continue;
        if (!((1 << SOUND_MIXER_VOLUME) & devmask))
            continue;

        uint32_t mutemask = 0;
        if (ioctl(fd, SOUND_MIXER_READ_MUTE, &mutemask) < 0)
            continue;

        struct oss_card_info ci = { .card = idev };
        if (ioctl(fd, SNDCTL_CARDINFO, &ci) < 0)
            continue;

        uint32_t volume;
        if (ioctl(fd, MIXER_READ(SOUND_MIXER_VOLUME), &volume) < 0)
            continue;

        FFSoundDevice* device = ffListAdd(devices);
        ffStrbufInitS(&device->identifier, path);
        ffStrbufInitF(&device->name, "%s %s", ci.longname, ci.hw_info);
        device->volume = (1 << SOUND_MIXER_VOLUME) & mutemask
            ? 0
            : ((uint8_t) volume /*left*/ + (uint8_t) (volume >> 8) /*right*/) / 2;
        device->active = true;
        device->main = defaultDev == idev;
    }

    return NULL;
}
