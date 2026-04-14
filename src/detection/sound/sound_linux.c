#include "sound.h"

#ifdef FF_HAVE_PULSE
#    include "common/library.h"
#    include <pulse/pulseaudio.h>

struct DetectionInfoBundle {
    FFstrbuf serverName;
    FFstrbuf defaultDeviceId;
    FFlist* result;
    FFSoundOptions* options;
};

static void paSinkInfoCallback(FF_A_UNUSED pa_context* c, const pa_sink_info* i, int eol, void* userdata) {
    if (eol > 0 || !i) {
        return;
    }

    struct DetectionInfoBundle* bundle = userdata;

    bool isMain = ffStrbufEqualS(&bundle->defaultDeviceId, i->name);
    if ((bundle->options->soundType & FF_SOUND_TYPE_MAIN) && !isMain) {
        return;
    }

    bool isActive = i->active_port && i->active_port->available != PA_PORT_AVAILABLE_NO;
    if ((bundle->options->soundType & FF_SOUND_TYPE_ACTIVE) && !isActive) {
        return;
    }

    FFSoundDevice* device = FF_LIST_ADD(FFSoundDevice, *bundle->result);
    ffStrbufInitS(&device->identifier, i->name);
    ffStrbufInitCopy(&device->platformApi, &bundle->serverName);
    ffStrbufTrimRightSpace(&device->identifier);
    ffStrbufInitS(&device->name, i->description);
    ffStrbufTrimRightSpace(&device->name);
    ffStrbufTrimLeft(&device->name, ' ');
    device->volume = i->mute ? 0 : (uint8_t) ((i->volume.values[0] * 100 + PA_VOLUME_NORM / 2 /*round*/) / PA_VOLUME_NORM);
    device->active = isActive;
    device->main = isMain;
}

static void paServerInfoCallback(FF_A_UNUSED pa_context* c, const pa_server_info* i, void* userdata) {
    if (!i) {
        return;
    }

    struct DetectionInfoBundle* bundle = userdata;

    const char* realServer = strstr(i->server_name, "(on ");
    if (realServer) {
        ffStrbufSetS(&bundle->serverName, realServer + strlen("(on "));
        ffStrbufTrimRight(&bundle->serverName, ')');
    } else {
        ffStrbufSetF(&bundle->serverName, "%s %s", i->server_name, i->server_version);
    }

    ffStrbufSetS(&bundle->defaultDeviceId, i->default_sink_name);
}

static const char* detectSound(FFSoundOptions* options, FFlist* devices) {
    FF_LIBRARY_LOAD_MESSAGE(pulse, "libpulse" FF_LIBRARY_EXTENSION, 0)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(pulse, pa_mainloop_new)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(pulse, pa_mainloop_get_api)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(pulse, pa_mainloop_iterate)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(pulse, pa_mainloop_free)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(pulse, pa_context_new)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(pulse, pa_context_connect)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(pulse, pa_context_get_state)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(pulse, pa_context_get_sink_info_list)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(pulse, pa_context_get_server_info)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(pulse, pa_context_unref)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(pulse, pa_operation_get_state)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(pulse, pa_operation_unref)

    pa_mainloop* mainloop = ffpa_mainloop_new();
    if (!mainloop) {
        return "Failed to create pulseaudio mainloop";
    }

    pa_mainloop_api* mainloopApi = ffpa_mainloop_get_api(mainloop);
    if (!mainloopApi) {
        ffpa_mainloop_free(mainloop);
        return "Failed to get pulseaudio mainloop api";
    }

    pa_context* context = ffpa_context_new(mainloopApi, "fastfetch");
    if (!context) {
        ffpa_mainloop_free(mainloop);
        return "Failed to create pulseaudio context";
    }

    if (ffpa_context_connect(context, NULL, PA_CONTEXT_NOFLAGS, NULL) < 0) {
        ffpa_context_unref(context);
        ffpa_mainloop_free(mainloop);
        return "Failed to connect to pulseaudio context";
    }

    pa_context_state_t state;
    while ((state = ffpa_context_get_state(context)) != PA_CONTEXT_READY) {
        if (!PA_CONTEXT_IS_GOOD(state)) {
            ffpa_context_unref(context);
            ffpa_mainloop_free(mainloop);
            return "Failed to get pulseaudio context state";
        }

        ffpa_mainloop_iterate(mainloop, 1, NULL);
    }

    struct DetectionInfoBundle bundle = {
        .serverName = ffStrbufCreate(),
        .defaultDeviceId = ffStrbufCreate(),
        .result = devices,
        .options = options,
    };

    {
        pa_operation* operation = ffpa_context_get_server_info(context, paServerInfoCallback, &bundle);
        while (ffpa_operation_get_state(operation) == PA_OPERATION_RUNNING) {
            ffpa_mainloop_iterate(mainloop, 1, NULL);
        }

        ffpa_operation_unref(operation);
    }

    pa_operation* operation = ffpa_context_get_sink_info_list(context, paSinkInfoCallback, &bundle);
    if (!operation) {
        ffpa_context_unref(context);
        ffpa_mainloop_free(mainloop);
        return "Failed to get pulseaudio sink info list";
    }

    while (ffpa_operation_get_state(operation) == PA_OPERATION_RUNNING) {
        ffpa_mainloop_iterate(mainloop, 1, NULL);
    }

    ffpa_operation_unref(operation);


    ffpa_context_unref(context);
    ffpa_mainloop_free(mainloop);
    return NULL;
}

#endif // FF_HAVE_PULSE

const char* ffDetectSound(FFSoundOptions* options, FFlist* devices) {
#ifdef FF_HAVE_PULSE
    return detectSound(options, devices);
#else
    FF_UNUSED(options, devices);
    return "Fastfetch was built without libpulse support";
#endif
}
