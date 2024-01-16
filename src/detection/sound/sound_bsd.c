#include "sound.h"

#include <mixer.h>

const char* ffDetectSound(FFlist* devices)
{
    int nmixers = mixer_get_nmixers();
    if (nmixers == 0) return "No mixers found";

    if (__builtin_expect(nmixers > 9, false)) nmixers = 9;

    char path[] = "/dev/mixer0";

    for (int idev = 0; idev < nmixers; ++idev)
    {
        path[strlen("/dev/mixer")] = (char) ('0' + idev);
        struct mixer* m = mixer_open(path);
        if (!m) continue;

        if (m->mode & MIX_MODE_PLAY)
        {
            struct mix_dev* dev = mixer_get_dev_byname(m, "vol");
            if (dev)
            {
                FFSoundDevice* device = ffListAdd(devices);
                ffStrbufInitS(&device->identifier, path);
                ffStrbufInitF(&device->name, "%s %s", m->ci.longname, m->ci.hw_info);
                device->volume = MIX_ISMUTE(m, dev->devno) ? 0 : (uint8_t) MIX_VOLDENORM((dev->vol.left + dev->vol.right) / 2);
                device->active = true;
                device->main = !!m->f_default;
            }
        }

        mixer_close(m);
    }

    return NULL;
}
