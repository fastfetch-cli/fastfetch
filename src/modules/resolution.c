#include "fastfetch.h"

#include <string.h>
#include <dlfcn.h>
#include <dirent.h>
#include <X11/extensions/Xrandr.h>
#include <wayland-client.h>

#define FF_RESOLUTION_MODULE_NAME "Resolution"
#define FF_RESOLUTION_NUM_FORMAT_ARGS 3

#define FF_LIBRARY_LOAD(libraryNameUser, libraryNameDefault) dlopen(libraryNameUser.length == 0 ? libraryNameDefault : libraryNameUser.chars, RTLD_LAZY); \
    if(dlerror() != NULL) \
        return false;

#define FF_LIBRARY_LOAD_SYMBOL(library, symbolName) dlsym(library, symbolName); \
    if(dlerror() != NULL) { \
        dlclose(library); \
        return false; \
    }

static void printResolution(FFinstance* instance, uint8_t moduleIndex, FFcache* cache, int width, int height, int refreshRate)
{
    FFstrbuf value;
    ffStrbufInitA(&value, 32);
    ffStrbufAppendF(&value, "%ix%i", width, height);

    if(refreshRate > 0)
        ffStrbufAppendF(&value, " @ %iHz", refreshRate);

    ffPrintAndAppendToCache(instance, FF_RESOLUTION_MODULE_NAME, moduleIndex, &instance->config.resolutionKey, cache, &value, &instance->config.resolutionFormat, FF_RESOLUTION_NUM_FORMAT_ARGS, (FFformatarg[]){
        {FF_FORMAT_ARG_TYPE_INT, &width},
        {FF_FORMAT_ARG_TYPE_INT, &height},
        {FF_FORMAT_ARG_TYPE_INT, &refreshRate}
    });
}

typedef struct DRMData
{
    int width;
    int height;
} DRMData;

static void printResolutionDRMBackend(FFinstance* instance)
{
    const char* drmDirPath = "/sys/class/drm/";

    DIR* dirp = opendir(drmDirPath);
    if(dirp == NULL)
    {
        ffPrintError(instance, FF_RESOLUTION_MODULE_NAME, 0, &instance->config.resolutionKey, &instance->config.resolutionFormat, FF_RESOLUTION_NUM_FORMAT_ARGS, "Couldn't connect to a display server or open %s", drmDirPath);
        return;
    }

    FFstrbuf drmDir;
    ffStrbufInitA(&drmDir, 64);
    ffStrbufAppendS(&drmDir, drmDirPath);

    uint32_t drmDirLength = drmDir.length;

    FFlist modes;
    ffListInitA(&modes, sizeof(DRMData), 4);

    struct dirent* entry;

    while((entry = readdir(dirp)) != NULL)
    {
        ffStrbufAppendS(&drmDir, entry->d_name);
        ffStrbufAppendS(&drmDir, "/modes");

        FILE* modeFile = fopen(drmDir.chars, "r");
        if(modeFile == NULL)
        {
            ffStrbufSubstrBefore(&drmDir, drmDirLength);
            continue;
        }

        DRMData* data = ffListAdd(&modes);
        data->width = 0;
        data->height = 0;
        fscanf(modeFile, "%ix%i", &data->width, &data->height);

        if(data->width == 0 || data->height == 0)
            --modes.length;

        fclose(modeFile);
        ffStrbufSubstrBefore(&drmDir, drmDirLength);
    }

    closedir(dirp);
    ffStrbufDestroy(&drmDir);

    if(modes.length == 0)
    {
        ffPrintError(instance, FF_RESOLUTION_MODULE_NAME, 0, &instance->config.resolutionKey, &instance->config.resolutionFormat, FF_RESOLUTION_NUM_FORMAT_ARGS, "Couldn't connect to a display server or find a resolution in %s", drmDirPath);
        ffListDestroy(&modes);
        return;
    }

    FFcache cache;
    ffCacheOpenWrite(instance, FF_RESOLUTION_MODULE_NAME, &cache);

    for(uint32_t i = 0; i < modes.length; i++)
    {
        DRMData* data = ffListGet(&modes, i);
        printResolution(instance, modes.length == 1 ? 0 : i + 1, &cache, data->width, data->height, 0);
    }

    ffCacheClose(&cache);

    ffListDestroy(&modes);
}

static Display* xOpenDisplay(void* library)
{
    Display*(*ffXOpenDisplay)(const char*) = dlsym(library, "XOpenDisplay");
    if(dlerror() != NULL)
        return NULL;

    Display* display = ffXOpenDisplay(NULL);
    if(display == NULL)
        return NULL;

    return display;
}

static void xCloseDisplay(void* library, Display* display)
{
    int(*ffXCloseDisplay)(Display*) = dlsym(library, "XCloseDisplay");
    if(dlerror() != NULL)
        ffXCloseDisplay(display);
}

static bool printResolutionX11Backend(FFinstance* instance)
{
    void* x11 = FF_LIBRARY_LOAD(instance->config.libX11, "libX11.so");

    Display* display = xOpenDisplay(instance);
    if(display == NULL)
    {
        dlclose(x11);
        return false;
    }

    int screenCount = ScreenCount(display);
    if(screenCount < 1)
    {
        xCloseDisplay(x11, display);
        dlclose(x11);
        return false;
    }

    FFcache cache;
    ffCacheOpenWrite(instance, FF_RESOLUTION_MODULE_NAME, &cache);

    for(int i = 0; i < screenCount; i++)
    {
        Screen* screen = ScreenOfDisplay(display, i);
        uint8_t moduleIndex = screenCount == 1 ? 0 : i + 1;
        printResolution(instance, moduleIndex, &cache, WidthOfScreen(screen), HeightOfScreen(screen), 0);
    }

    ffCacheClose(&cache);

    xCloseDisplay(x11, display);
    dlclose(x11);

    return true;
}

static int xrandrGetCurrentRate(void* xrandr, Display* display)
{
    XRRScreenConfiguration*(*ffXRRGetScreenInfo)(Display*, Window) = dlsym(xrandr, "XRRGetScreenInfo");
    if(dlerror() != NULL)
        return 0;

    short(*ffXRRConfigCurrentRate)(XRRScreenConfiguration*) = dlsym(xrandr, "XRRConfigCurrentRate");
    if(dlerror() != NULL)
        return 0;


    XRRScreenConfiguration* xrrscreenconf = ffXRRGetScreenInfo(display, DefaultRootWindow(display));
    if(dlerror() != NULL)
        return 0;

    short currentRate = ffXRRConfigCurrentRate(xrrscreenconf);

    void(*ffXRRFreeScreenConfigInfo)(XRRScreenConfiguration*) = dlsym(xrandr, "XRRFreeScreenConfigInfo");
    if(dlerror() != NULL)
        ffXRRFreeScreenConfigInfo(xrrscreenconf);

    return (int) currentRate;
}

static bool printResolutionXrandrBackend(FFinstance* instance)
{
    void* xrandr = FF_LIBRARY_LOAD(instance->config.libXrandr, "libXrandr.so");

    XRRMonitorInfo*(*ffXRRGetMonitors)(Display*, Window, Bool, int*) = FF_LIBRARY_LOAD_SYMBOL(xrandr, "XRRGetMonitors");

    Display* display = xOpenDisplay(instance);
    if(display == NULL)
    {
        dlclose(xrandr);
        return false;
    }

    int numberOfMonitors;
    XRRMonitorInfo* monitors = ffXRRGetMonitors(display, DefaultRootWindow(display), False, &numberOfMonitors);
    if(monitors == NULL)
    {
        xCloseDisplay(xrandr, display);
        dlclose(xrandr);
        return false;
    }

    if(numberOfMonitors < 1)
    {
        xCloseDisplay(xrandr, display);
        dlclose(xrandr);
        return false;
    }

    int refreshRate = xrandrGetCurrentRate(xrandr, display);

    FFcache cache;
    ffCacheOpenWrite(instance, FF_RESOLUTION_MODULE_NAME, &cache);

    for(int i = 0; i < numberOfMonitors; i++)
    {
        uint8_t moduleIndex = numberOfMonitors == 1 ? 0 : i + 1;
        printResolution(instance, moduleIndex, &cache, monitors[i].width, monitors[i].height, refreshRate);
    }

    ffCacheClose(&cache);

    void(*ffXRRFreeMonitors)(XRRMonitorInfo*) = dlsym(xrandr, "XRRFreeMonitors");
    if(dlerror() != NULL)
        ffXRRFreeMonitors(monitors);

    xCloseDisplay(xrandr, display);
    dlclose(xrandr);

    return true;
}

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
    printResolution(wldata->instance, wldata->numModules == 1 ? 0 : wldata->moduleCounter, &wldata->cache, width, height, parseRefreshRate(refreshRate));

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
    if(dlerror() != NULL)
        ffwl_display_disconnect(display);
}

static bool printResolutionWaylandBackend(FFinstance* instance)
{
    const char* sessionType = getenv("XDG_SESSION_TYPE");
    if(sessionType != NULL && strcasecmp(sessionType, "wayland") != 0)
        return false;

    void* wayland = FF_LIBRARY_LOAD(instance->config.libWayland, "libwayland-client.so");

    WaylandData data;
    data.instance = instance;
    data.wayland = wayland;
    data.moduleCounter = 0;
    data.numModules = 0;

    struct wl_display*(*ffwl_display_connect)(const char*) = FF_LIBRARY_LOAD_SYMBOL(wayland, "wl_display_connect");
    int(*ffwl_display_dispatch)(struct wl_display*) = FF_LIBRARY_LOAD_SYMBOL(wayland, "wl_display_dispatch");
    void(*ffwl_display_roundtrip)(struct wl_display*) = FF_LIBRARY_LOAD_SYMBOL(wayland, "wl_display_roundtrip");
    struct wl_proxy*(*ffwl_proxy_marshal_constructor)(struct wl_proxy*, uint32_t, const struct wl_interface*, ...) = FF_LIBRARY_LOAD_SYMBOL(wayland, "wl_proxy_marshal_constructor");
    data.ffwl_proxy_marshal_constructor_versioned = FF_LIBRARY_LOAD_SYMBOL(wayland, "wl_proxy_marshal_constructor_versioned");
    data.ffwl_proxy_add_listener = FF_LIBRARY_LOAD_SYMBOL(wayland, "wl_proxy_add_listener");
    const struct wl_interface* ffwl_registry_interface = FF_LIBRARY_LOAD_SYMBOL(wayland, "wl_registry_interface");
    data.ffwl_output_interface = FF_LIBRARY_LOAD_SYMBOL(wayland, "wl_output_interface");

    data.ffwl_proxy_destroy = dlsym(wayland, "wl_proxy_destroy");
    //We check for NULL before each call because this is just used for cleanup and not actually needed
    if(dlerror() != NULL)
        data.ffwl_proxy_destroy = NULL;

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

void ffPrintResolution(FFinstance* instance)
{
    if(
        ffPrintFromCache(instance, FF_RESOLUTION_MODULE_NAME, &instance->config.resolutionKey, &instance->config.resolutionFormat, FF_RESOLUTION_NUM_FORMAT_ARGS) ||
        printResolutionWaylandBackend(instance) ||
        printResolutionXrandrBackend(instance) ||
        printResolutionX11Backend(instance)
    ) return;

    printResolutionDRMBackend(instance);
}

#undef FF_LIBRARY_LOAD
#undef FF_LIBRARY_LOAD_SYMBOL
