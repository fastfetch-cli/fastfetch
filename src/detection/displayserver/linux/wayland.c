#include "displayserver_linux.h"
#include "util/stringUtils.h"

#include <stdlib.h>
#include <string.h>

#ifdef FF_HAVE_WAYLAND
#include "common/library.h"
#include "common/io/io.h"
#include "common/thread.h"
#include "util/edidHelper.h"
#include <wayland-client.h>
#include <sys/socket.h>
#include <assert.h>
#include <dirent.h>

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
    FFstrbuf description;
    FFstrbuf vendorAndModelId;
    FFstrbuf edidName;
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

static void waylandOutputScaleListener(void* data, FF_MAYBE_UNUSED struct wl_output* output, int32_t scale)
{
    WaylandDisplay* display = data;
    display->scale = scale;
}

static void waylandOutputGeometryListener(void *data,
    FF_MAYBE_UNUSED struct wl_output *output,
    FF_MAYBE_UNUSED int32_t x,
    FF_MAYBE_UNUSED int32_t y,
    FF_MAYBE_UNUSED int32_t physical_width,
    FF_MAYBE_UNUSED int32_t physical_height,
    FF_MAYBE_UNUSED int32_t subpixel,
    const char *make,
    const char *model,
    int32_t transform)
{
    WaylandDisplay* display = data;
    display->transform = (enum wl_output_transform) transform;
    if (make && !ffStrEqualsIgnCase(make, "unknown") && model && !ffStrEqualsIgnCase(model, "unknown"))
    {
        ffStrbufAppendS(&display->vendorAndModelId, make);
        ffStrbufAppendS(&display->vendorAndModelId, model);
    }
}

static bool matchDrmConnector(const char* wlName, FFstrbuf* edidName)
{
    // https://wayland.freedesktop.org/docs/html/apa.html#protocol-spec-wl_output-event-name
    // The doc says that "do not assume that the name is a reflection of an underlying DRM connector, X11 connection, etc."
    // However I can't find a better method to get the edid data
    const char* drmDirPath = "/sys/class/drm/";

    DIR* dirp = opendir(drmDirPath);
    if(dirp == NULL)
        return false;

    struct dirent* entry;
    while((entry = readdir(dirp)) != NULL)
    {
        const char* plainName = entry->d_name;
        if (ffStrStartsWith(plainName, "card"))
        {
            const char* tmp = strchr(plainName + strlen("card"), '-');
            if (tmp) plainName = tmp + 1;
        }
        if (ffStrEquals(plainName, wlName))
        {
            ffStrbufAppendF(edidName, "%s%s/edid", drmDirPath, entry->d_name);

            uint8_t edidData[128];
            if(ffReadFileData(edidName->chars, sizeof(edidData), edidData) == sizeof(edidData))
            {
                ffStrbufClear(edidName);
                ffEdidGetName(edidData, edidName);
                closedir(dirp);
                return true;
            }
            break;
        }
    }
    ffStrbufClear(edidName);
    closedir(dirp);
    return false;
}

static void waylandOutputNameListener(void *data, FF_MAYBE_UNUSED struct wl_output *output, const char *name)
{
    WaylandDisplay* display = data;
    if(ffStrStartsWith(name, "eDP-"))
        display->type = FF_DISPLAY_TYPE_BUILTIN;
    else if(ffStrStartsWith(name, "HDMI-"))
        display->type = FF_DISPLAY_TYPE_EXTERNAL;
    matchDrmConnector(name, &display->edidName);
    ffStrbufAppendS(&display->name, name);
}

static void waylandOutputDescriptionListener(void* data, FF_MAYBE_UNUSED struct wl_output* wl_output, const char* description)
{
    WaylandDisplay* display = data;
    while (*description == ' ') ++description;
    if (!ffStrEquals(description, "Unknown Display"))
        ffStrbufAppendS(&display->description, description);
}

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
    ffStrbufInit(&display.description);
    ffStrbufInit(&display.vendorAndModelId);
    ffStrbufInit(&display.edidName);

    // Dirty hack for #477
    // The order of these callbacks MUST follow `struct wl_output_listener`
    void* outputListener[] = {
        waylandOutputGeometryListener, // geometry
        waylandOutputModeListener, // mode
        stubListener, // done
        waylandOutputScaleListener, // scale
        waylandOutputNameListener, // name
        waylandOutputDescriptionListener, // description
    };
    static_assert(
        sizeof(outputListener) >= sizeof(struct wl_output_listener),
        "sizeof(outputListener) is too small. Please report it to fastfetch github issue"
    );

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
        display.edidName.length
            ? &display.edidName
            : display.description.length
                ? &display.description
                : display.vendorAndModelId.length
                    ? &display.vendorAndModelId : &display.name,
        display.type,
        false,
        0
    );

    ffStrbufDestroy(&display.description);
    ffStrbufDestroy(&display.vendorAndModelId);
    ffStrbufDestroy(&display.name);
    ffStrbufDestroy(&display.edidName);

    ffThreadMutexUnlock(&mutex);
}

static void waylandGlobalAddListener(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
{
    WaylandData* wldata = data;

    if(ffStrEquals(interface, wldata->ffwl_output_interface->name))
        waylandOutputHandler(wldata, registry, name, version);
}

bool detectWayland(FFDisplayServerResult* result)
{
    FF_LIBRARY_LOAD(wayland, &instance.config.library.libWayland, false, "libwayland-client" FF_LIBRARY_EXTENSION, 1)

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

void ffdsConnectWayland(FFDisplayServerResult* result)
{
    //Wayland requires this to be set
    if(getenv("XDG_RUNTIME_DIR") == NULL)
        return;

    #ifdef FF_HAVE_WAYLAND
        if(detectWayland(result))
            return;
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
