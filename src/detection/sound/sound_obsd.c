#include "sound.h"
#include "util/stringUtils.h"

#include <fcntl.h>
#include <sndio.h>

static void close_hdl(struct sioctl_hdl** phdl)
{
    assert(phdl);
    if (*phdl) sioctl_close(*phdl);
}

enum { MAX_CHANNEL_NUM = 8 };

typedef struct FFSoundDeviceBundle
{
    char name[SIOCTL_DISPLAYMAX];
    double level[MAX_CHANNEL_NUM];
    uint8_t iLevel;
    bool mute[MAX_CHANNEL_NUM];
    uint8_t iMute;
} FFSoundDeviceBundle;

static void enumerate_props(FFSoundDeviceBundle* bundle, struct sioctl_desc* desc, int val)
{
    if (!desc) return;

    if (desc->type == SIOCTL_SEL)
    {
        if (desc->display[0] != '\0' && ffStrEquals(desc->node0.name, "server"))
            ffStrCopy(bundle->name, desc->display, SIOCTL_DISPLAYMAX);
        return;
    }

    if (desc->type != SIOCTL_NUM && desc->type != SIOCTL_SW)
        return;

    if (!ffStrEquals(desc->node0.name, "output"))
        return;

    if (ffStrEquals(desc->func, "level"))
    {
        if (__builtin_expect(bundle->iLevel == MAX_CHANNEL_NUM, false))
            return;
        bundle->level[bundle->iLevel] = (double) val / (double) desc->maxval;
        ++bundle->iLevel;
    }
    else if (ffStrEquals(desc->func, "mute"))
    {
        if (__builtin_expect(bundle->iMute == MAX_CHANNEL_NUM, false))
            return;
        bundle->mute[bundle->iMute] = !!val;
        ++bundle->iMute;
    }
}

const char* ffDetectSound(FFlist* devices)
{
    __attribute__((__cleanup__(close_hdl))) struct sioctl_hdl* hdl = sioctl_open(SIO_DEVANY, SIOCTL_READ, 0);
    if (!hdl) return "sio_open() failed";

    FFSoundDeviceBundle bundle = {};
    if (sioctl_ondesc(hdl, (void*) enumerate_props, &bundle) == 0)
        return "sioctl_ondesc() failed";

    if (bundle.iLevel != bundle.iMute || bundle.iLevel == 0)
        return "Unexpected sioctl_ondesc() result";

    FFSoundDevice* device = ffListAdd(devices);
    ffStrbufInitS(&device->name, bundle.name);
    ffStrbufInitS(&device->identifier, SIO_DEVANY);
    ffStrbufInitStatic(&device->platformApi, "sndio");
    device->active = true;
    device->main = true;
    device->volume = 0;

    double totalLevel = 0;
    for (uint8_t i = 0; i < bundle.iLevel; ++i)
    {
        if (!bundle.mute[i])
            totalLevel += bundle.level[i];
    }
    device->volume = (uint8_t) ((totalLevel * 100 + bundle.iLevel / 2) / bundle.iLevel);

    return NULL;
}
