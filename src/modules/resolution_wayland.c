#include "resolution.h"

#include <string.h>
#include <dlfcn.h>
#include <wayland-client.h>

typedef struct WaylandData
{
    FFinstance* instance;
    void* wayland;
    FFcache cache;
    struct wl_proxy*(*ffwl_proxy_marshal_constructor_versioned)(struct wl_proxy*, uint32_t, const struct wl_interface*, uint32_t, ...);
    int(*ffwl_proxy_add_listener)(struct wl_proxy*, void (**)(void), void *data);
    void(*ffwl_proxy_destroy)(struct wl_proxy*);
    const struct wl_interface* ffwl_output_interface;
    struct wl_output_listener output_listener;
    int8_t moduleCounter;
    int8_t numModules;
} WaylandData;

static void waylandGlobalRemoveListener(void* data, struct wl_registry* wl_registry, uint32_t name){
    UNUSED(data);
    UNUSED(wl_registry);
    UNUSED(name);
}

static void waylandOutputGeometryListener(void* data, struct wl_output* wl_output, int32_t x, int32_t y, int32_t physical_width, int32_t physical_height, int32_t subpixel, const char* make, const char* model, int32_t transform)
{
    UNUSED(data);
    UNUSED(wl_output);
    UNUSED(x);
    UNUSED(y);
    UNUSED(physical_width);
    UNUSED(physical_height);
    UNUSED(subpixel);
    UNUSED(make);
    UNUSED(model);
    UNUSED(transform);
}

static void waylandOutputDoneListener(void* data, struct wl_output* wl_output)
{
    UNUSED(data);
    UNUSED(wl_output);
}

static void waylandOutputScaleListener(void* data, struct wl_output* wl_output, int32_t factor)
{
    UNUSED(data);
    UNUSED(wl_output);
    UNUSED(factor);
}

static int parseRefreshRate(int32_t refreshRate)
{
    if(refreshRate <= 0)
        return 0;

    refreshRate /= 1000; //to Hz

    int remainder = refreshRate % 5;
    if(remainder >= 3)
        refreshRate += (5 - remainder);
    else
        refreshRate -= remainder;

    //All other typicall refresh rates are dividable by 5
    if(refreshRate == 145)
        refreshRate = 144;

    return refreshRate;
}

static void waylandOutputModeListener(void* data, struct wl_output* output, uint32_t flags, int32_t width, int32_t height, int32_t refreshRate)
{
    if(!(flags & WL_OUTPUT_MODE_CURRENT))
        return;

    WaylandData* wldata = (WaylandData*) data;

    ++wldata->moduleCounter;
    ffPrintResolutionValue(wldata->instance, wldata->numModules == 1 ? 0 : wldata->moduleCounter, &wldata->cache, width, height, parseRefreshRate(refreshRate));

    if(wldata->ffwl_proxy_destroy != NULL)
        wldata->ffwl_proxy_destroy((struct wl_proxy*) output);
}

static void waylandGlobalAddListener(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
{
    if(strcmp(interface, "wl_output") == 0)
    {

        WaylandData* wldata = (WaylandData*) data;

        struct wl_output* output = (struct wl_output*) wldata->ffwl_proxy_marshal_constructor_versioned((struct wl_proxy *) registry, WL_REGISTRY_BIND, wldata->ffwl_output_interface, version, name, wldata->ffwl_output_interface->name, version, NULL);
        if(output == NULL)
            return;

        ++wldata->numModules;
        wldata->ffwl_proxy_add_listener((struct wl_proxy*) output, (void(**)(void)) &wldata->output_listener, data);
    }
}

static inline void waylandDisplayDisconnect(void* wayland, struct wl_display* display)
{
    void(*ffwl_display_disconnect)(struct wl_display*) = dlsym(wayland, "wl_display_disconnect");
    if(ffwl_display_disconnect != NULL)
        ffwl_display_disconnect(display);
}

bool ffPrintResolutionWaylandBackend(FFinstance* instance)
{
    const char* sessionType = getenv("XDG_SESSION_TYPE");
    if(sessionType != NULL && strcasecmp(sessionType, "wayland") != 0)
        return false;

    void* wayland = dlopen(instance->config.libWayland.length == 0 ? "libwayland-client.so" : instance->config.libWayland.chars, RTLD_LAZY);
    if(wayland == NULL)
        return false;

    WaylandData data;
    data.instance = instance;
    data.wayland = wayland;
    data.moduleCounter = 0;
    data.numModules = 0;

    struct wl_display*(*ffwl_display_connect)(const char*) = dlsym(wayland, "wl_display_connect");
    if(ffwl_display_connect == NULL)
    {
        dlclose(wayland);
        return false;
    }

    int(*ffwl_display_dispatch)(struct wl_display*) = dlsym(wayland, "wl_display_dispatch");
    if(ffwl_display_dispatch == NULL)
    {
        dlclose(wayland);
        return false;
    }

    void(*ffwl_display_roundtrip)(struct wl_display*) = dlsym(wayland, "wl_display_roundtrip");
    if(ffwl_display_roundtrip == NULL)
    {
        dlclose(wayland);
        return false;
    }

    struct wl_proxy*(*ffwl_proxy_marshal_constructor)(struct wl_proxy*, uint32_t, const struct wl_interface*, ...) = dlsym(wayland, "wl_proxy_marshal_constructor");
    if(ffwl_proxy_marshal_constructor == NULL)
    {
        dlclose(wayland);
        return false;
    }

    data.ffwl_proxy_marshal_constructor_versioned = dlsym(wayland, "wl_proxy_marshal_constructor_versioned");
    if(data.ffwl_proxy_marshal_constructor_versioned == NULL)
    {
        dlclose(wayland);
        return false;
    }

    data.ffwl_proxy_add_listener = dlsym(wayland, "wl_proxy_add_listener");
    if(data.ffwl_proxy_add_listener == NULL)
    {
        dlclose(wayland);
        return false;
    }

    const struct wl_interface* ffwl_registry_interface = dlsym(wayland, "wl_registry_interface");
    if(ffwl_registry_interface == NULL)
    {
        dlclose(wayland);
        return false;
    }

    data.ffwl_output_interface = dlsym(wayland, "wl_output_interface");
    if(data.ffwl_output_interface == NULL)
    {
        dlclose(wayland);
        return false;
    }

    data.ffwl_proxy_destroy = dlsym(wayland, "wl_proxy_destroy");
    //We check for NULL before each call because this is just used for cleanup and not actually needed

    struct wl_display* display = ffwl_display_connect(NULL);
    if(display == NULL)
    {
        dlclose(wayland);
        return false;
    }

    struct wl_registry* registry = (struct wl_registry*) ffwl_proxy_marshal_constructor((struct wl_proxy*) display, WL_DISPLAY_GET_REGISTRY, ffwl_registry_interface, NULL);
    if(registry == NULL)
    {
        waylandDisplayDisconnect(wayland, display);
        dlclose(wayland);
        return false;
    }

    struct wl_registry_listener regestry_listener;
    regestry_listener.global = waylandGlobalAddListener;
    regestry_listener.global_remove = waylandGlobalRemoveListener;

    data.output_listener.geometry = waylandOutputGeometryListener;
    data.output_listener.mode = waylandOutputModeListener;
    data.output_listener.done = waylandOutputDoneListener;
    data.output_listener.scale = waylandOutputScaleListener;

    ffCacheOpenWrite(instance, FF_RESOLUTION_MODULE_NAME, &data.cache);

    data.ffwl_proxy_add_listener((struct wl_proxy*) registry, (void(**)(void)) &regestry_listener, &data);
    ffwl_display_dispatch(display);
    ffwl_display_roundtrip(display);

    ffCacheClose(&data.cache);

    if(data.ffwl_proxy_destroy != NULL)
        data.ffwl_proxy_destroy((struct wl_proxy*) registry);

    waylandDisplayDisconnect(wayland, display);
    dlclose(wayland);

    return data.numModules > 0;
}
