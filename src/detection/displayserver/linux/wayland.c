#include "displayserver_linux.h"
#include "util/stringUtils.h"

#include <stdlib.h>
#include <string.h>

#ifdef FF_HAVE_WAYLAND
#include "common/library.h"
#include "common/io/io.h"
#include "common/thread.h"
#include <wayland-client.h>
#include <sys/socket.h>

typedef struct WaylandData
{
    FFDisplayServerResult* result;
    FF_LIBRARY_SYMBOL(wl_proxy_marshal_constructor_versioned)
    FF_LIBRARY_SYMBOL(wl_proxy_add_listener)
    FF_LIBRARY_SYMBOL(wl_proxy_destroy)
    FF_LIBRARY_SYMBOL(wl_display_roundtrip)
    struct wl_display* display;
    const struct wl_interface* ffwl_output_interface;
} WaylandData;

typedef struct WaylandDisplay
{
    int32_t width;
    int32_t height;
    int32_t refreshRate;
    int32_t scale;
    enum wl_output_transform transform;
    FFDisplayType type;
    FFstrbuf name;
} WaylandDisplay;

#ifndef __FreeBSD__
static void waylandDetectWM(int fd, FFDisplayServerResult* result)
{
    struct ucred ucred;
    socklen_t len = sizeof(struct ucred);
    if (getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &ucred, &len) == -1)
        return;

    FF_STRBUF_AUTO_DESTROY procPath = ffStrbufCreate();
    ffStrbufAppendF(&procPath, "/proc/%d/cmdline", ucred.pid); //We check the cmdline for the process name, because it is not trimmed.
    ffReadFileBuffer(procPath.chars, &result->wmProcessName);
    ffStrbufSubstrBeforeFirstC(&result->wmProcessName, '\0'); //Trim the arguments
    ffStrbufSubstrAfterLastC(&result->wmProcessName, '/'); //Trim the path
}
#else
static void waylandDetectWM(int fd, FFDisplayServerResult* result)
{
    FF_UNUSED(fd, result);
}
#endif

static void stubListener(void* data, ...)
{
    (void) data;
}

static void waylandOutputModeListener(void* data, FF_MAYBE_UNUSED struct wl_output* output, uint32_t flags, int32_t width, int32_t height, int32_t refreshRate)
{
    if(!(flags & WL_OUTPUT_MODE_CURRENT))
        return;

    WaylandDisplay* display = data;
    display->width = width;
    display->height = height;
    display->refreshRate = refreshRate;
}

#ifdef WL_OUTPUT_SCALE_SINCE_VERSION
static void waylandOutputScaleListener(void* data, FF_MAYBE_UNUSED struct wl_output* output, int32_t scale)
{
    WaylandDisplay* display = data;
    display->scale = scale;
}
#endif

static void waylandOutputGeometryListener(void *data,
    FF_MAYBE_UNUSED struct wl_output *output,
    FF_MAYBE_UNUSED int32_t x,
    FF_MAYBE_UNUSED int32_t y,
    FF_MAYBE_UNUSED int32_t physical_width,
    FF_MAYBE_UNUSED int32_t physical_height,
    FF_MAYBE_UNUSED int32_t subpixel,
    FF_MAYBE_UNUSED const char *make,
    FF_MAYBE_UNUSED const char *model,
    int32_t transform)
{
    WaylandDisplay* display = data;
    display->transform = (enum wl_output_transform) transform;
}

#ifdef WL_OUTPUT_NAME_SINCE_VERSION
static void waylandOutputNameListener(void *data, FF_MAYBE_UNUSED struct wl_output *output, const char *name)
{
    WaylandDisplay* display = data;
    if(ffStrStartsWith(name, "eDP-"))
        display->type = FF_DISPLAY_TYPE_BUILTIN;
    else if(ffStrStartsWith(name, "HDMI-") || ffStrStartsWith(name, "DP-"))
        display->type = FF_DISPLAY_TYPE_EXTERNAL;
    ffStrbufAppendS(&display->name, name);
}
#endif

static void waylandOutputHandler(WaylandData* wldata, struct wl_registry* registry, uint32_t name, uint32_t version)
{
    struct wl_proxy* output = wldata->ffwl_proxy_marshal_constructor_versioned((struct wl_proxy*) registry, WL_REGISTRY_BIND, wldata->ffwl_output_interface, version, name, wldata->ffwl_output_interface->name, version, NULL);
    if(output == NULL)
        return;

    WaylandDisplay display = {
        .width = 0,
        .height = 0,
        .refreshRate = 0,
        .scale = 1,
        .transform = WL_OUTPUT_TRANSFORM_NORMAL,
        .type = FF_DISPLAY_TYPE_UNKNOWN,
    };
    ffStrbufInit(&display.name);

    struct wl_output_listener outputListener = {
        .mode = waylandOutputModeListener,
        .geometry = waylandOutputGeometryListener,

        #ifdef WL_OUTPUT_DONE_SINCE_VERSION
            .done = (void*) stubListener,
        #else
            #warning wl_output_listener::done is not supported
        #endif

        #ifdef WL_OUTPUT_SCALE_SINCE_VERSION
            .scale = waylandOutputScaleListener,
        #else
            #warning wl_output_listener::scale is not supported
        #endif

        #ifdef WL_OUTPUT_NAME_SINCE_VERSION
            .name = waylandOutputNameListener,
        #else
            #warning wl_output_listener::name is not supported
        #endif

        #ifdef WL_OUTPUT_DESCRIPTION_SINCE_VERSION
            .description = (void*) stubListener,
        #else
            #warning wl_output_listener::description is not supported
        #endif
    };

    wldata->ffwl_proxy_add_listener(output, (void(**)(void)) &outputListener, &display);
    wldata->ffwl_display_roundtrip(wldata->display);
    wldata->ffwl_proxy_destroy(output);

    if(display.width <= 0 || display.height <= 0)
        return;

    static FFThreadMutex mutex = FF_THREAD_MUTEX_INITIALIZER;
    ffThreadMutexLock(&mutex);

    switch(display.transform)
    {
        case WL_OUTPUT_TRANSFORM_90:
        case WL_OUTPUT_TRANSFORM_270:
        case WL_OUTPUT_TRANSFORM_FLIPPED_90:
        case WL_OUTPUT_TRANSFORM_FLIPPED_270: {
            int32_t temp = display.width;
            display.width = display.height;
            display.height = temp;
            break;
        }
        default:
            break;
    }

    uint32_t rotation;
    switch(display.transform)
    {
        case WL_OUTPUT_TRANSFORM_FLIPPED_90:
        case WL_OUTPUT_TRANSFORM_90:
            rotation = 90;
            break;
        case WL_OUTPUT_TRANSFORM_FLIPPED_180:
        case WL_OUTPUT_TRANSFORM_180:
            rotation = 180;
            break;
        case WL_OUTPUT_TRANSFORM_FLIPPED_270:
        case WL_OUTPUT_TRANSFORM_270:
            rotation = 270;
            break;
        default:
            rotation = 0;
            break;
    }

    ffdsAppendDisplay(wldata->result,
        (uint32_t) display.width,
        (uint32_t) display.height,
        display.refreshRate / 1000.0,
        (uint32_t) (display.width / display.scale),
        (uint32_t) (display.height / display.scale),
        rotation,
        &display.name,
        display.type,
        false
    );

    ffThreadMutexUnlock(&mutex);
}

static void waylandGlobalAddListener(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
{
    WaylandData* wldata = data;

    if(ffStrEquals(interface, wldata->ffwl_output_interface->name))
        waylandOutputHandler(wldata, registry, name, version);
}

bool detectWayland(const FFinstance* instance, FFDisplayServerResult* result)
{
    FF_LIBRARY_LOAD(wayland, &instance->config.libWayland, false, "libwayland-client" FF_LIBRARY_EXTENSION, 1)

    FF_LIBRARY_LOAD_SYMBOL(wayland, wl_display_connect, false)
    FF_LIBRARY_LOAD_SYMBOL(wayland, wl_display_get_fd, false)
    FF_LIBRARY_LOAD_SYMBOL(wayland, wl_display_dispatch, false)
    FF_LIBRARY_LOAD_SYMBOL(wayland, wl_proxy_marshal_constructor, false)
    FF_LIBRARY_LOAD_SYMBOL(wayland, wl_display_disconnect, false)
    FF_LIBRARY_LOAD_SYMBOL(wayland, wl_registry_interface, false)

    WaylandData data;

    FF_LIBRARY_LOAD_SYMBOL_VAR(wayland, data, wl_proxy_marshal_constructor_versioned, false)
    FF_LIBRARY_LOAD_SYMBOL_VAR(wayland, data, wl_proxy_add_listener, false)
    FF_LIBRARY_LOAD_SYMBOL_VAR(wayland, data, wl_proxy_destroy, false)
    FF_LIBRARY_LOAD_SYMBOL_VAR(wayland, data, wl_display_roundtrip, false)
    FF_LIBRARY_LOAD_SYMBOL_VAR(wayland, data, wl_output_interface, false)

    data.display = ffwl_display_connect(NULL);
    if(data.display == NULL)
        return false;

    waylandDetectWM(ffwl_display_get_fd(data.display), result);

    struct wl_proxy* registry = ffwl_proxy_marshal_constructor((struct wl_proxy*) data.display, WL_DISPLAY_GET_REGISTRY, ffwl_registry_interface, NULL);
    if(registry == NULL)
    {
        ffwl_display_disconnect(data.display);
        return false;
    }

    data.result = result;

    struct wl_registry_listener registry_listener = {
        .global = waylandGlobalAddListener,
        .global_remove = (void*) stubListener
    };

    data.ffwl_proxy_add_listener(registry, (void(**)(void)) &registry_listener, &data);
    ffwl_display_dispatch(data.display);
    data.ffwl_display_roundtrip(data.display);

    data.ffwl_proxy_destroy(registry);
    ffwl_display_disconnect(data.display);

    //We successfully connected to wayland and detected the display.
    //So we can set set the session type to wayland.
    //This is used as an indicator that we are running wayland by the x11 backends.
    ffStrbufSetS(&result->wmProtocolName, FF_WM_PROTOCOL_WAYLAND);
    return true;
}
#endif

void ffdsConnectWayland(const FFinstance* instance, FFDisplayServerResult* result)
{
    //Wayland requires this to be set
    if(getenv("XDG_RUNTIME_DIR") == NULL)
        return;

    #ifdef FF_HAVE_WAYLAND
        if(detectWayland(instance, result))
            return;
    #else
        FF_UNUSED(instance);
    #endif

    const char* xdgSessionType = getenv("XDG_SESSION_TYPE");

    //If XDG_SESSION_TYPE is set, and doesn't contain "wayland", we are probably not running in a wayland session.
    if(xdgSessionType != NULL && strcasecmp(xdgSessionType, "wayland") != 0)
        return;

    //If XDG_SESSION_TYPE is not set, check if WAYLAND_DISPLAY or WAYLAND_SOCKET is set.
    //If not, there is no indicator for a wayland session
    if(xdgSessionType == NULL && getenv("WAYLAND_DISPLAY") == NULL && getenv("WAYLAND_SOCKET") == NULL)
        return;

    //We are probably running a wayland compositor at this point,
    //but fastfetch was compiled without the required library, or loading the library failed.
    ffStrbufSetS(&result->wmProtocolName, FF_WM_PROTOCOL_WAYLAND);
}
