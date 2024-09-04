#include "sound.h"

#ifdef FF_HAVE_PULSE
#include <common/library.h>
#include <pulse/pulseaudio.h>

static void paSinkInfoCallback(pa_context *c, const pa_sink_info *i, int eol, void *userdata)
{
    FF_UNUSED(c);

    if(eol > 0 || !i)
        return;

    FFSoundDevice* device = ffListAdd(userdata);
    ffStrbufInitS(&device->identifier, i->name);
    ffStrbufTrimRightSpace(&device->identifier);
    ffStrbufInitS(&device->name, i->description);
    ffStrbufTrimRightSpace(&device->name);
    ffStrbufTrimLeft(&device->name, ' ');
    device->volume = i->mute ? 0 : (uint8_t) (i->volume.values[0] * 100 / PA_VOLUME_NORM);
    device->active = i->active_port && i->active_port->available != PA_PORT_AVAILABLE_NO;
    device->main = false;
}

static void paServerInfoCallback(pa_context *c, const pa_server_info *i, void *userdata)
{
    FF_UNUSED(c);

    if(!i)
        return;

    FF_LIST_FOR_EACH(FFSoundDevice, device, *(FFlist*)userdata)
    {
        if(ffStrbufEqualS(&device->identifier, i->default_sink_name))
        {
            device->main = true;
            break;
        }
    }
}

static const char* detectSound(FFlist* devices)
{
    FF_LIBRARY_LOAD(pulse, "Failed to load libpulse" FF_LIBRARY_EXTENSION, "libpulse" FF_LIBRARY_EXTENSION, 0)
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
    if(!mainloop)
        return "Failed to create pulseaudio mainloop";

    pa_mainloop_api* mainloopApi = ffpa_mainloop_get_api(mainloop);
    if(!mainloopApi)
    {
        ffpa_mainloop_free(mainloop);
        return "Failed to get pulseaudio mainloop api";
    }

    pa_context* context = ffpa_context_new(mainloopApi, "fastfetch");
    if(!context)
    {
        ffpa_mainloop_free(mainloop);
        return "Failed to create pulseaudio context";
    }

    if(ffpa_context_connect(context, NULL, PA_CONTEXT_NOFLAGS, NULL) < 0)
    {
        ffpa_context_unref(context);
        ffpa_mainloop_free(mainloop);
        return "Failed to connect to pulseaudio context";
    }

    pa_context_state_t state;
    while((state = ffpa_context_get_state(context)) != PA_CONTEXT_READY)
    {
        if(!PA_CONTEXT_IS_GOOD(state))
        {
            ffpa_context_unref(context);
            ffpa_mainloop_free(mainloop);
            return "Failed to get pulseaudio context state";
        }

        ffpa_mainloop_iterate(mainloop, 1, NULL);
    }

    pa_operation* operation = ffpa_context_get_sink_info_list(context, paSinkInfoCallback, devices);
    if(!operation)
    {
        ffpa_context_unref(context);
        ffpa_mainloop_free(mainloop);
        return "Failed to get pulseaudio sink info list";
    }

    while(ffpa_operation_get_state(operation) == PA_OPERATION_RUNNING)
        ffpa_mainloop_iterate(mainloop, 1, NULL);

    ffpa_operation_unref(operation);

    operation = ffpa_context_get_server_info(context, paServerInfoCallback, devices);
    if(operation)
    {
        while(ffpa_operation_get_state(operation) == PA_OPERATION_RUNNING)
            ffpa_mainloop_iterate(mainloop, 1, NULL);

        ffpa_operation_unref(operation);
    }

    ffpa_context_unref(context);
    ffpa_mainloop_free(mainloop);
    return NULL;
}

#endif // FF_HAVE_PULSE

const char* ffDetectSound(FFlist* devices)
{
    #ifdef FF_HAVE_PULSE
        return detectSound(devices);
    #else
        FF_UNUSED(devices);
        return "Fastfetch was built without libpulse support";
    #endif
}
