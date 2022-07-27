#define _GNU_SOURCE //required for struct ucred

#include "displayServer.h"

#include <stdlib.h>
#include <string.h>

#ifdef FF_HAVE_WAYLAND
#include "common/library.h"
#include "common/io.h"
#include <pthread.h>
#include <wayland-client.h>
#include <sys/socket.h>

typedef struct WaylandData
{
    FFlist* results;
    FF_LIBRARY_SYMBOL(wl_proxy_marshal_constructor_versioned)
    FF_LIBRARY_SYMBOL(wl_proxy_add_listener)
    const struct wl_interface* ffwl_output_interface;
} WaylandData;

static void waylandDetectWM(int fd, FFDisplayServerResult* result)
{
    struct ucred ucred;
    socklen_t len = sizeof(struct ucred);
    if (getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &ucred, &len) == -1)
        return;

    FFstrbuf procPath;
    ffStrbufInit(&procPath);
    ffStrbufAppendF(&procPath, "/proc/%d/cmdline", ucred.pid); //We check the cmdline for the process name, because it is not trimmed.
    ffReadFileBuffer(procPath.chars, &result->wmProcessName);
    ffStrbufSubstrBeforeFirstC(&result->wmProcessName, '\0'); //Trim the arguments
    ffStrbufSubstrAfterLastC(&result->wmProcessName, '/'); //Trim the path
    ffStrbufDestroy(&procPath);
}

static void stubListener(void* data, ...)
{
    (void) data;
}

static void waylandOutputModeListener(void* data, struct wl_output* output, uint32_t flags, int32_t width, int32_t height, int32_t refreshRate)
{
    FF_UNUSED(output);

    WaylandData* wldata = data;

    if(!(flags & WL_OUTPUT_MODE_CURRENT) || width <= 0 || height <= 0)
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
    WaylandData* wldata = data;

    if(strcmp(interface, wldata->ffwl_output_interface->name) == 0)
    {
        struct wl_proxy* output = wldata->ffwl_proxy_marshal_constructor_versioned((struct wl_proxy*) registry, WL_REGISTRY_BIND, wldata->ffwl_output_interface, version, name, wldata->ffwl_output_interface->name, version, NULL);
        if(output == NULL)
            return;

        //Needs to be static, because the scope of the function will already be lost when calling the listener
        static struct wl_output_listener outputListener = {
            .mode = waylandOutputModeListener,
            .geometry = (void*) stubListener,

            //https://lists.freedesktop.org/archives/wayland-devel/2013-July/010278.html
            #if WAYLAND_VERSION_MINOR >= 2
                .done = (void*) stubListener,
                .scale = (void*) stubListener,
            #endif

            //https://lists.freedesktop.org/archives/wayland-devel/2021-December/042064.html
            #if WAYLAND_VERSION_MINOR >= 20
                .name = (void*) stubListener,
                .description = (void*) stubListener,
            #endif
        };

        wldata->ffwl_proxy_add_listener(output, (void(**)(void)) &outputListener, data);
    }
}

bool detectWayland(const FFinstance* instance, FFDisplayServerResult* result)
{
    FF_LIBRARY_LOAD(wayland, instance->config.libWayland, false, "libwayland-client.so", 1)

    FF_LIBRARY_LOAD_SYMBOL(wayland, wl_display_connect, false)
    FF_LIBRARY_LOAD_SYMBOL(wayland, wl_display_get_fd, false)
    FF_LIBRARY_LOAD_SYMBOL(wayland, wl_display_dispatch, false)
    FF_LIBRARY_LOAD_SYMBOL(wayland, wl_display_roundtrip, false)
    FF_LIBRARY_LOAD_SYMBOL(wayland, wl_proxy_marshal_constructor, false)
    FF_LIBRARY_LOAD_SYMBOL(wayland, wl_display_disconnect, false)
    FF_LIBRARY_LOAD_SYMBOL(wayland, wl_registry_interface, false)

    WaylandData data;

    FF_LIBRARY_LOAD_SYMBOL_ADRESS(wayland, data.ffwl_proxy_marshal_constructor_versioned, wl_proxy_marshal_constructor_versioned, false)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(wayland, data.ffwl_proxy_add_listener, wl_proxy_add_listener, false)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(wayland, data.ffwl_output_interface, wl_output_interface, false)

    struct wl_display* display = ffwl_display_connect(NULL);
    if(display == NULL)
    {
        dlclose(wayland);
        return false;
    }

    waylandDetectWM(ffwl_display_get_fd(display), result);

    struct wl_proxy* registry = ffwl_proxy_marshal_constructor((struct wl_proxy*) display, WL_DISPLAY_GET_REGISTRY, ffwl_registry_interface, NULL);
    if(registry == NULL)
    {
        ffwl_display_disconnect(display);
        dlclose(wayland);
        return false;
    }

    data.results = &result->resolutions;

    struct wl_registry_listener registry_listener = {
        .global = waylandGlobalAddListener,
        .global_remove = (void*) stubListener
    };

    data.ffwl_proxy_add_listener(registry, (void(**)(void)) &registry_listener, &data);
    ffwl_display_dispatch(display);
    ffwl_display_roundtrip(display);

    ffwl_display_disconnect(display); //This will also destroy our wl_registry and wl_output proxies
    dlclose(wayland);

    //We successfully connected to wayland and detected the resolution.
    //So we can set set the session type to wayland.
    //This is used as an indicator that we are running wayland by the x11 backends.
    ffStrbufSetS(&result->wmProtocolName, FF_DISPLAYSERVER_PROTOCOL_WAYLAND);
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
    ffStrbufSetS(&result->wmProtocolName, FF_DISPLAYSERVER_PROTOCOL_WAYLAND);
}
