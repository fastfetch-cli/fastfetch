#include "detection/displayserver/linux/displayserver_linux.h"
#include "util/stringUtils.h"

#include <stdlib.h>
#include <string.h>

#ifdef FF_HAVE_WAYLAND

#include "common/library.h"
#include "common/io/io.h"
#include "common/thread.h"
#include "util/edidHelper.h"
#include <sys/socket.h>
#include <assert.h>
#include <dirent.h>

#include <wayland-client-core.h>

#if WAYLAND_VERSION_MINOR >= 20
    #define WL_HAVE_PROXY_MARSHAL_FLAGS
#endif

static FF_LIBRARY_SYMBOL(wl_display_connect);
static FF_LIBRARY_SYMBOL(wl_display_get_fd);
static FF_LIBRARY_SYMBOL(wl_display_roundtrip);
static FF_LIBRARY_SYMBOL(wl_display_disconnect);
static FF_LIBRARY_SYMBOL(wl_proxy_get_version);
static FF_LIBRARY_SYMBOL(wl_proxy_marshal_constructor); //Only needed with old wayland headers
static FF_LIBRARY_SYMBOL(wl_proxy_marshal_constructor_versioned); //Only needed with old wayland headers
static FF_LIBRARY_SYMBOL(wl_proxy_add_listener);
static FF_LIBRARY_SYMBOL(wl_proxy_destroy);
static const struct wl_interface* ffwl_registry_interface;
static const struct wl_interface* ffwl_output_interface;
static const struct wl_interface* ffwl_compositor_interface;
static const struct wl_interface* ffwl_surface_interface;
static const struct wl_interface* ffwp_fractional_scale_manager_v1_interface;
static const struct wl_interface* ffwp_fractional_scale_v1_interface;

#define wl_display_connect ffwl_display_connect
#define wl_display_get_fd ffwl_display_get_fd
#define wl_display_roundtrip ffwl_display_roundtrip
#define wl_display_disconnect ffwl_display_disconnect
#define wl_proxy_get_version ffwl_proxy_get_version
#define wl_proxy_marshal_constructor ffwl_proxy_marshal_constructor
#define wl_proxy_marshal_constructor_versioned ffwl_proxy_marshal_constructor_versioned
#define wl_proxy_add_listener ffwl_proxy_add_listener
#define wl_proxy_destroy ffwl_proxy_destroy
#define wl_registry_interface (*ffwl_registry_interface)
#define wl_output_interface (*ffwl_output_interface)
#define wl_compositor_interface (*ffwl_compositor_interface)
#define wl_surface_interface (*ffwl_surface_interface)
#define wp_fractional_scale_manager_v1_interface (*ffwp_fractional_scale_manager_v1_interface)
#define wp_fractional_scale_v1_interface (*ffwp_fractional_scale_v1_interface)

#ifdef WL_HAVE_PROXY_MARSHAL_FLAGS
    static FF_LIBRARY_SYMBOL(wl_proxy_marshal_flags);
    #define wl_proxy_marshal_flags ffwl_proxy_marshal_flags
#endif

#include <wayland-client-protocol.h>

#ifdef WL_HAVE_PROXY_MARSHAL_FLAGS
    #include "wayland-fractional-scale-v1.h"
#endif

typedef struct WaylandGlobals
{
    FFlist outputs; //List of struct wl_output*

    struct wl_compositor* compositor;
    struct wl_surface* surface;
    struct wp_fractional_scale_manager_v1* fractionalScaleManager;
    struct wp_fractional_scale_v1* fractionalScale;
} WaylandGlobals;

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

static void transformDisplay(WaylandDisplay* display, FFDisplayServerResult* result, double fractionalScale)
{
    switch(display->transform)
    {
        case WL_OUTPUT_TRANSFORM_90:
        case WL_OUTPUT_TRANSFORM_270:
        case WL_OUTPUT_TRANSFORM_FLIPPED_90:
        case WL_OUTPUT_TRANSFORM_FLIPPED_270: {
            int32_t temp = display->width;
            display->width = display->height;
            display->height = temp;
            break;
        }
        default:
            break;
    }

    uint32_t rotation;
    switch(display->transform)
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

    double scale = display->scale * fractionalScale;

    ffdsAppendDisplay(result,
        (uint32_t) display->width,
        (uint32_t) display->height,
        display->refreshRate / 1000.0,
        (uint32_t) (display->width / scale),
        (uint32_t) (display->height / scale),
        rotation,
        display->edidName.length
            ? &display->edidName
            : display->description.length
                ? &display->description
                : display->vendorAndModelId.length
                    ? &display->vendorAndModelId : &display->name,
        display->type,
        false,
        0
    );
}

static void transformDisplays(FFlist* waylandDisplays, FFDisplayServerResult* result, double fractionalScale)
{
    for(uint32_t i = 0; i < waylandDisplays->length; i++)
        transformDisplay(ffListGet(waylandDisplays, i), result, fractionalScale);
}

static void destroyDisplays(FFlist* waylandDisplays)
{
    for(uint32_t i = 0; i < waylandDisplays->length; i++)
    {
        WaylandDisplay* display = ffListGet(waylandDisplays, i);
        ffStrbufDestroy(&display->name);
        ffStrbufDestroy(&display->description);
        ffStrbufDestroy(&display->vendorAndModelId);
        ffStrbufDestroy(&display->edidName);
    }
    ffListDestroy(waylandDisplays);
}

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
    int32_t transform
) {
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

static void addOutputListener(FFlist* displays, struct wl_output* output)
{
    // Dirty hack for #477
    // The order of these callbacks MUST follow `struct wl_output_listener`
    static void* outputListener[] = {
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

    if(output == NULL)
        return;

    WaylandDisplay* display = ffListAdd(displays);
    display->width = 0;
    display->height = 0;
    display->refreshRate = 0;
    display->scale = 1;
    display->transform = WL_OUTPUT_TRANSFORM_NORMAL;
    display->type = FF_DISPLAY_TYPE_UNKNOWN;
    ffStrbufInit(&display->name);
    ffStrbufInit(&display->description);
    ffStrbufInit(&display->vendorAndModelId);
    ffStrbufInit(&display->edidName);

    wl_output_add_listener(output, (const struct wl_output_listener*) outputListener, display);
}

static void addOutputListeners(FFlist* outputs, FFlist* displays)
{
    for(uint32_t i = 0; i < outputs->length; i++)
        addOutputListener(displays, *(struct wl_output**)ffListGet(outputs, i));
}

#ifdef WL_HAVE_PROXY_MARSHAL_FLAGS

static void waylandFractionalScaleListener(void *data, FF_MAYBE_UNUSED struct wp_fractional_scale_v1 *wp_fractional_scale_v1, uint32_t scale)
{
    uint32_t* preferredScale = data;
    *preferredScale = scale;
}

static void addFractionalScaleListener(WaylandGlobals* globals, uint32_t* preferredScale)
{
    if(!globals->fractionalScaleManager || !globals->compositor)
        return;

    globals->surface = wl_compositor_create_surface(globals->compositor);
    if(!globals->surface)
        return;

    globals->fractionalScale = wp_fractional_scale_manager_v1_get_fractional_scale(globals->fractionalScaleManager, globals->surface);
    if(!globals->fractionalScale)
        return;

    static const struct wp_fractional_scale_v1_listener fractionalScaleListener = {
        .preferred_scale = waylandFractionalScaleListener
    };

    wp_fractional_scale_v1_add_listener(globals->fractionalScale, &fractionalScaleListener, preferredScale);
}

#else
static void addFractionalScaleListener(WaylandGlobals* globals, uint32_t* preferredScale)
{
    FF_UNUSED(globals, preferredScale);
}
#endif

static void waylandGlobalAddListener(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
{
    WaylandGlobals* globals = data;

    if(ffStrEquals(interface, wl_output_interface.name))
        *(void**) ffListAdd(&globals->outputs) = wl_registry_bind(registry, name, &wl_output_interface, version);
    else if(ffStrEquals(interface, wl_compositor_interface.name))
        globals->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, version);
    else if(ffStrEquals(interface, wp_fractional_scale_manager_v1_interface.name))
        globals->fractionalScaleManager = wl_registry_bind(registry, name, &wp_fractional_scale_manager_v1_interface, version);
}

static void addRegistryListener(struct wl_registry* registry, WaylandGlobals* globals)
{
    static struct wl_registry_listener registry_listener = {
        .global = waylandGlobalAddListener,
        .global_remove = (void*) stubListener
    };

    wl_registry_add_listener(registry, &registry_listener, globals);
}

static void destroyGlobals(WaylandGlobals* globals)
{
    #ifdef WL_HAVE_PROXY_MARSHAL_FLAGS
        if(globals->fractionalScale) wp_fractional_scale_v1_destroy(globals->fractionalScale);
        if(globals->fractionalScaleManager) wp_fractional_scale_manager_v1_destroy(globals->fractionalScaleManager);
    #endif

    if(globals->surface) wl_surface_destroy(globals->surface);
    if(globals->compositor) wl_compositor_destroy(globals->compositor);

    for(uint32_t i = 0; i < globals->outputs.length; i++)
        wl_output_destroy(*(struct wl_output**)ffListGet(&globals->outputs, i));
    ffListDestroy(&globals->outputs);
}

static void createFractionalScaleInterfaces()
{
    //This is modeled after the wayland-scanner generated code.

    static const struct wl_interface *fractional_scale_v1_types[] = {
        NULL,
        NULL,  // wp_fractional_scale_v1_interface
        NULL  // wl_surface_interface
    };

    static const struct wl_message wp_fractional_scale_manager_v1_requests[] = {
        { "destroy", "", fractional_scale_v1_types + 0 },
        { "get_fractional_scale", "no", fractional_scale_v1_types + 1 },
    };

    static const struct wl_interface actual_wp_fractional_scale_manager_v1_interface = {
        "wp_fractional_scale_manager_v1", 1,
        2, wp_fractional_scale_manager_v1_requests,
        0, NULL,
    };

    static const struct wl_message wp_fractional_scale_v1_requests[] = {
        { "destroy", "", fractional_scale_v1_types + 0 },
    };

    static const struct wl_message wp_fractional_scale_v1_events[] = {
        { "preferred_scale", "u", fractional_scale_v1_types + 0 },
    };

    static const struct wl_interface actual_wp_fractional_scale_v1_interface = {
        "wp_fractional_scale_v1", 1,
        1, wp_fractional_scale_v1_requests,
        1, wp_fractional_scale_v1_events,
    };

    fractional_scale_v1_types[1] = &wp_fractional_scale_v1_interface;
    fractional_scale_v1_types[2] = &wl_surface_interface;
    ffwp_fractional_scale_manager_v1_interface = &actual_wp_fractional_scale_manager_v1_interface;
    ffwp_fractional_scale_v1_interface = &actual_wp_fractional_scale_v1_interface;
}

static bool detectWaylandImpl(FFDisplayServerResult* result)
{
    struct wl_display* display = wl_display_connect(NULL);
    if(!display)
        return false;

    //We successfully connected to wayland, so we can set set the session type to wayland.
    //This is used as an indicator that we are running wayland by the x11 backends.
    ffStrbufSetS(&result->wmProtocolName, FF_WM_PROTOCOL_WAYLAND);

    waylandDetectWM(wl_display_get_fd(display), result);

    createFractionalScaleInterfaces();

    struct wl_registry* registry = wl_display_get_registry(display);
    if(!registry)
    {
        wl_display_disconnect(display);
        return false;
    }

    WaylandGlobals globals = {0};
    ffListInit(&globals.outputs, sizeof(struct wl_output*));
    addRegistryListener(registry, &globals);

    wl_display_roundtrip(display);

    FFlist waylandDisplays;
    ffListInit(&waylandDisplays, sizeof(WaylandDisplay));
    addOutputListeners(&globals.outputs, &waylandDisplays);

    uint32_t preferredScale = 120; //The sent scale is the numerator of a fraction with a denominator of 120
    addFractionalScaleListener(&globals, &preferredScale);

    wl_display_roundtrip(display);

    double fractionalScale = preferredScale / 120.0;
    transformDisplays(&waylandDisplays, result, fractionalScale);

    destroyDisplays(&waylandDisplays);
    destroyGlobals(&globals);
    wl_registry_destroy(registry);
    wl_display_disconnect(display);

    return true;
}

bool detectWayland(FFDisplayServerResult* result)
{
    FF_LIBRARY_LOAD(wayland, &instance.config.libWayland, false, "libwayland-client" FF_LIBRARY_EXTENSION, 1);
    FF_LIBRARY_LOAD_SYMBOL_STR(wayland, ffwl_display_connect, "wl_display_connect", false);
    FF_LIBRARY_LOAD_SYMBOL_STR(wayland, ffwl_display_get_fd, "wl_display_get_fd", false);
    FF_LIBRARY_LOAD_SYMBOL_STR(wayland, ffwl_display_roundtrip, "wl_display_roundtrip", false);
    FF_LIBRARY_LOAD_SYMBOL_STR(wayland, ffwl_display_disconnect, "wl_display_disconnect", false);
    FF_LIBRARY_LOAD_SYMBOL_STR(wayland, ffwl_proxy_get_version, "wl_proxy_get_version", false);
    FF_LIBRARY_LOAD_SYMBOL_STR(wayland, ffwl_proxy_marshal_constructor, "wl_proxy_marshal_constructor", false);
    FF_LIBRARY_LOAD_SYMBOL_STR(wayland, ffwl_proxy_marshal_constructor_versioned, "wl_proxy_marshal_constructor_versioned", false);
    FF_LIBRARY_LOAD_SYMBOL_STR(wayland, ffwl_proxy_add_listener, "wl_proxy_add_listener", false);
    FF_LIBRARY_LOAD_SYMBOL_STR(wayland, ffwl_proxy_destroy, "wl_proxy_destroy", false);
    FF_LIBRARY_LOAD_SYMBOL_STR(wayland, ffwl_registry_interface, "wl_registry_interface", false);
    FF_LIBRARY_LOAD_SYMBOL_STR(wayland, ffwl_output_interface, "wl_output_interface", false);
    FF_LIBRARY_LOAD_SYMBOL_STR(wayland, ffwl_compositor_interface, "wl_compositor_interface", false);
    FF_LIBRARY_LOAD_SYMBOL_STR(wayland, ffwl_surface_interface, "wl_surface_interface", false);

    #ifdef WL_HAVE_PROXY_MARSHAL_FLAGS
        FF_LIBRARY_LOAD_SYMBOL_STR(wayland, ffwl_proxy_marshal_flags, "wl_proxy_marshal_flags", false);
    #endif

    bool success = detectWaylandImpl(result);

    dlclose(wayland);
    return success;
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
