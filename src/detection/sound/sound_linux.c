#include "sound.h"

#ifdef FF_HAVE_ALSA

#include "common/library.h"
#include "common/io.h"

#include <alsa/asoundlib.h>

static const char* doDetectSound(const FFinstance* instance, FFlist* devices)
{
    FF_LIBRARY_LOAD(asound, &instance->config.libAlsa, "dlopen asound failed", "libasound" FF_LIBRARY_EXTENSION, 2);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(asound, snd_mixer_open);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(asound, snd_mixer_attach);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(asound, snd_mixer_selem_register);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(asound, snd_mixer_load);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(asound, snd_mixer_first_elem);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(asound, snd_mixer_elem_next);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(asound, snd_mixer_selem_is_active);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(asound, snd_mixer_selem_get_playback_volume_range);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(asound, snd_mixer_selem_get_playback_volume);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(asound, snd_mixer_selem_get_name);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(asound, snd_mixer_close);

    snd_mixer_t *mixer;
    if (ffsnd_mixer_open(&mixer, 0) < 0)
        return "snd_mixer_open() failed";

    if (ffsnd_mixer_attach(mixer, "default") < 0)
    {
        ffsnd_mixer_close(mixer);
        return "snd_mixer_attach(\"default\") failed";
    }

    if (ffsnd_mixer_selem_register(mixer, NULL, NULL) < 0)
    {
        ffsnd_mixer_close(mixer);
        return "snd_mixer_selem_register() failed";
    }

    if (ffsnd_mixer_load(mixer) < 0)
    {
        ffsnd_mixer_close(mixer);
        return "snd_mixer_load() failed";
    }

    for (snd_mixer_elem_t* elem = ffsnd_mixer_first_elem(mixer); elem; elem = ffsnd_mixer_elem_next(elem))
    {
        if (!ffsnd_mixer_selem_is_active(elem))
            continue;

        long min, max;
        if (ffsnd_mixer_selem_get_playback_volume_range(elem, &min, &max) < 0)
            continue;

        long volume;
        if (ffsnd_mixer_selem_get_playback_volume(elem, 0, &volume) < 0)
            continue;

        const char* name = ffsnd_mixer_selem_get_name(elem);

        FFSoundDevice* device = (FFSoundDevice*) ffListAdd(devices);
        device->main = strcmp(name, "Master") == 0;
        device->volume = (uint8_t) ((double)(volume - min) * 100.0 / (double)(max - min) + 0.5);
        ffStrbufInitS(&device->name, name);
        ffStrbufInit(&device->manufacturer);
    }

    ffsnd_mixer_close(mixer);
    return NULL;
}

#endif // FF_HAVE_ALSA

const char* ffDetectSound(FF_MAYBE_UNUSED const FFinstance* instance, FF_MAYBE_UNUSED FFlist* devices /* List of FFSoundDevice */)
{
    #ifdef FF_HAVE_ALSA

    ffSuppressIO(true);
    const char* error = doDetectSound(instance, devices);
    ffSuppressIO(false);
    return error;

    #else

    return "Fastfetch was built without alsa support";

    #endif
}
