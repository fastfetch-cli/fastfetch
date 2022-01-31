#include "displayServer.h"
#include <string.h>

#ifdef FF_HAVE_WAYLAND
#include <wayland-client.h>
#include <pthread.h>

typedef struct WaylandData
{
    const FFinstance* instance;
    FFlist* results;
    FF_LIBRARY_SYMBOL(wl_proxy_marshal_constructor_versioned)
    FF_LIBRARY_SYMBOL(wl_proxy_add_listener)
    FF_LIBRARY_SYMBOL(wl_proxy_destroy)
    const struct wl_interface* ffwl_output_interface;
    struct wl_output_listener output_listener;
} WaylandData;

static void waylandGlobalRemoveListener(void* data, struct wl_registry* wl_registry, uint32_t name){
    FF_UNUSED(data, wl_registry, name);
}

static void waylandOutputGeometryListener(void* data, struct wl_output* wl_output, int32_t x, int32_t y, int32_t physical_width, int32_t physical_height, int32_t subpixel, const char* make, const char* model, int32_t transform)
{
    FF_UNUSED(data, wl_output, x, y, physical_width, physical_height, subpixel, make, model, transform);
}

static void waylandOutputDoneListener(void* data, struct wl_output* wl_output)
{
    FF_UNUSED(data, wl_output);
}

static void waylandOutputScaleListener(void* data, struct wl_output* wl_output, int32_t factor)
{
    FF_UNUSED(data, wl_output, factor);
}

static void waylandOutputModeListener(void* data, struct wl_output* output, uint32_t flags, int32_t width, int32_t height, int32_t refreshRate)
{
    if(!(flags & WL_OUTPUT_MODE_CURRENT))
        return;

    WaylandData* wldata = (WaylandData*) data;

    wldata->ffwl_proxy_destroy((struct wl_proxy*) output);

    if(width <= 0 || height <= 0)
        return;

    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&mutex);

    FFResolutionResult* result = ffListAdd(wldata->results);

    pthread_mutex_unlock(&mutex);

    result->width = (uint32_t) width;
    result->height = (uint32_t) height;
    result->refreshRate = ffdsParseRefreshRate(refreshRate / 1000);
}

static void waylandGlobalAddListener(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
{
    if(strcmp(interface, "wl_output") == 0)
    {
        WaylandData* wldata = (WaylandData*) data;

        struct wl_output* output = (struct wl_output*) wldata->ffwl_proxy_marshal_constructor_versioned((struct wl_proxy *) registry, WL_REGISTRY_BIND, wldata->ffwl_output_interface, version, name, wldata->ffwl_output_interface->name, version, NULL);
        if(output == NULL)
            return;

        wldata->ffwl_proxy_add_listener((struct wl_proxy*) output, (void(**)(void)) &wldata->output_listener, data);
    }
}

void ffdsConnectWayland(const FFinstance* instance, FFDisplayServerResult* result)
{
    if(getenv("XDG_RUNTIME_DIR") == NULL)
        return;

    FF_LIBRARY_LOAD(wayland, instance->config.libWayland, , "libwayland-client.so", "libwayland-client.so.0")

    FF_LIBRARY_LOAD_SYMBOL(wayland, wl_display_connect,)
    FF_LIBRARY_LOAD_SYMBOL(wayland, wl_display_dispatch,)
    FF_LIBRARY_LOAD_SYMBOL(wayland, wl_display_roundtrip,)
    FF_LIBRARY_LOAD_SYMBOL(wayland, wl_proxy_marshal_constructor,)
    FF_LIBRARY_LOAD_SYMBOL(wayland, wl_display_disconnect,)
    FF_LIBRARY_LOAD_SYMBOL(wayland, wl_registry_interface,)

    WaylandData data;

    FF_LIBRARY_LOAD_SYMBOL_ADRESS(wayland, data.ffwl_proxy_marshal_constructor_versioned, wl_proxy_marshal_constructor_versioned,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(wayland, data.ffwl_proxy_add_listener, wl_proxy_add_listener,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(wayland, data.ffwl_output_interface, wl_output_interface,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(wayland, data.ffwl_proxy_destroy, wl_proxy_destroy,)

    struct wl_display* display = ffwl_display_connect(NULL);
    if(display == NULL)
    {
        dlclose(wayland);
        return;
    }

    struct wl_registry* registry = (struct wl_registry*) ffwl_proxy_marshal_constructor((struct wl_proxy*) display, WL_DISPLAY_GET_REGISTRY, ffwl_registry_interface, NULL);
    if(registry == NULL)
    {
        ffwl_display_disconnect(display);
        dlclose(wayland);
        return;
    }

    data.instance = instance;
    data.results = &result->resolutions;

    struct wl_registry_listener regestry_listener;
    regestry_listener.global = waylandGlobalAddListener;
    regestry_listener.global_remove = waylandGlobalRemoveListener;

    data.output_listener.geometry = waylandOutputGeometryListener;
    data.output_listener.mode = waylandOutputModeListener;
    data.output_listener.done = waylandOutputDoneListener;
    data.output_listener.scale = waylandOutputScaleListener;

    data.ffwl_proxy_add_listener((struct wl_proxy*) registry, (void(**)(void)) &regestry_listener, &data);
    ffwl_display_dispatch(display);
    ffwl_display_roundtrip(display);

    data.ffwl_proxy_destroy((struct wl_proxy*) registry);
    ffwl_display_disconnect(display);
    dlclose(wayland);

    //We successfully connected to wayland and detected the resolution.
    //So we can set set the session type to wayland.
    //This is used as an indicator that we are running wayland by the x11 backends.
    ffStrbufSetS(&result->wmProtocolName, FF_DISPLAYSERVER_PROTOCOL_WAYLAND);
}

#else

void ffdsConnectWayland(const FFinstance* instance, FFDisplayServerResult* result)
{
    FF_UNUSED(instance);

    //We try to detect wayland already here, so the x11 methods only to resolution detection.

    //Wayland requires this to be set
    if(getenv("XDG_RUNTIME_DIR") == NULL)
        return;

    const char* xdgSessionType = getenv("XDG_SESSION_TYPE");

    //If XDG_SESSION_TYPE is set, and doesn't contain "wayland", we are probably not running in a wayland session.
    if(xdgSessionType != NULL && strcasecmp(xdgSessionType, "wayland") != 0)
        return;

    //If XDG_SESSION_TYPE is not set, check if WAYLAND_DISPLAY or WAYLAND_SOCKET is set.
    //If not, there is no indicator for a wayland session
    if(xdgSessionType == NULL && getenv("WAYLAND_DISPLAY") == NULL && getenv("WAYLAND_SOCKET") == NULL)
        return;

    //We are probably running a wayland compositor at this point, but fastfetch was compiled without the required library.
    //Set the protocol name and use DRM for resolution detection.
    ffStrbufSetS(&result->wmProtocolName, FF_DISPLAYSERVER_PROTOCOL_WAYLAND);
}

#endif
