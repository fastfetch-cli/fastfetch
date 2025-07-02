#include "sound.h"
#include "common/io/io.h"
#include "util/stringUtils.h"

#include <fcntl.h>
#include <unistd.h>
#if __has_include(<sys/soundcard.h>)
    #include <sys/soundcard.h>
#else
    // Strangely, they don't provide this file on default installation
    #include "audio_oss_sunos.h"
#endif

const char* ffDetectSound(FFlist* devices)
{
    int defaultDev;
    {
        char mixerp[12];
        ssize_t plen = readlink("/dev/audio", mixerp, ARRAY_SIZE(mixerp));
        if (plen < 6)
            return "readlink(/dev/audio) failed";
        defaultDev = mixerp[plen - 1] - '0';
        if (defaultDev < 0 || defaultDev > 9)
            return "Invalid mixer device";
    }

    char path[] = "/dev/mixer0";

    FF_STRBUF_AUTO_DESTROY sndstat = ffStrbufCreate();

    struct oss_sysinfo info = { .nummixers = 9 };

    // The implementation is very different from *BSD's. They call it OSS4
    for (int idev = 0; idev < info.nummixers; ++idev)
    {
        path[strlen("/dev/mixer")] = (char) ('0' + idev);
        FF_AUTO_CLOSE_FD int fd = open(path, O_RDWR | O_CLOEXEC);
        if (fd < 0) break;

        if (idev == 0)
        {
            if (ioctl(fd, SNDCTL_SYSINFO, &info) != 0)
                return "ioctl(SNDCTL_SYSINFO) failed";
            if (ffAppendFDBuffer(fd, &sndstat))
                ffStrbufSubstrAfterFirstS(&sndstat, "\nMixers:");
        }

        struct oss_mixerinfo mi = {};
        if (ioctl(fd, SNDCTL_MIXERINFO, &mi) < 0)
            continue;

        int volume = -1;
        for (int iext = 0; iext < mi.nrext; ++iext)
        {
            struct oss_mixext me = { .dev = mi.dev, .ctrl = iext };
            if (ioctl(fd, SNDCTL_MIX_EXTINFO, &me) < 0)
                continue;
            if (me.flags & MIXF_PCMVOL)
            {
                struct oss_mixer_value mv = { .dev = mi.dev, .ctrl = iext, .timestamp = me.timestamp };
                if (ioctl(fd, SNDCTL_MIX_READ, &mv) >= 0)
                {
                    mv.value -= me.minvalue;
                    me.maxvalue -= me.minvalue;
                    volume = (uint8_t) ((mv.value * 100 + me.maxvalue / 2) / me.maxvalue);
                }
                break;
            }
        }
        if (volume == -1) continue;

        FFSoundDevice* device = ffListAdd(devices);
        ffStrbufInitS(&device->identifier, path);
        char buf[16];
        int bufLen = snprintf(buf, ARRAY_SIZE(buf), "\n%d: ", mi.dev);
        assert(bufLen > 3);
        const char* pLine = memmem(sndstat.chars, sndstat.length, buf, (size_t) bufLen);
        if (pLine)
        {
            pLine += bufLen;
            const char* pEnd = strchr(pLine, '\n');
            if (!pEnd) pEnd = sndstat.chars + sndstat.length;
            ffStrbufInitNS(&device->name, (uint32_t) (pEnd - pLine), pLine);
        }
        else
            ffStrbufInitS(&device->name, mi.name);
        ffStrbufTrimRightSpace(&device->name);
        ffStrbufInitF(&device->platformApi, "%s %s", info.product, info.version);
        device->volume = (uint8_t) volume;
        device->active = !!mi.enabled;
        device->main = defaultDev == idev;
    }

    return NULL;
}
