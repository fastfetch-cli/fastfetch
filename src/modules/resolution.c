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

typedef void* DynamicLibrary;

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

typedef struct ResolutionResult
{
    int width;
    int height;
    int refreshRate;
} ResolutionResult;

static inline void printResolutionResultList(FFinstance* instance, FFlist* results)
{
    FFcache cache;
    ffCacheOpenWrite(instance, FF_RESOLUTION_MODULE_NAME, &cache);

    for(uint32_t i = 0; i < results->length; i++)
    {
        ResolutionResult* result = ffListGet(results, i);
        printResolution(instance, results->length == 1 ? 0 : i + 1, &cache, result->width, result->height, result->refreshRate);
    }

    ffCacheClose(&cache);
}

static int parseRefreshRate(int32_t refreshRate)
{
    if(refreshRate <= 0)
        return 0;

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
    ffListInitA(&modes, sizeof(ResolutionResult), 4);

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

        ResolutionResult* result = ffListAdd(&modes);
        result->width = 0;
        result->height = 0;
        result->refreshRate = 0;

        int scanned = fscanf(modeFile, "%ix%i", &result->width, &result->height);
        if(scanned < 2 || result->width == 0 || result->height == 0)
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

    printResolutionResultList(instance, &modes);

    ffListDestroy(&modes);
}

static void x11AddScreenAsResult(FFlist* results, Screen* screen, int refreshRate)
{
    if(WidthOfScreen(screen) == 0 || HeightOfScreen(screen) == 0)
        return;

    ResolutionResult* result = ffListAdd(results);
    result->width = WidthOfScreen(screen);
    result->height = HeightOfScreen(screen);
    result->refreshRate = refreshRate;
}

static bool printResolutionX11Backend(FFinstance* instance)
{
    DynamicLibrary x11 = FF_LIBRARY_LOAD(instance->config.libX11, "libX11.so");

    Display*(*ffXOpenDisplay)(const char*) = FF_LIBRARY_LOAD_SYMBOL(x11, "XOpenDisplay");
    int(*ffXCloseDisplay)(Display*) = FF_LIBRARY_LOAD_SYMBOL(x11, "XCloseDisplay");

    Display* display = ffXOpenDisplay(x11);
    if(display == NULL)
    {
        dlclose(x11);
        return false;
    }

    FFlist results;
    ffListInitA(&results, sizeof(ResolutionResult), 4);

    for(int i = 0; i < ScreenCount(display); i++)
        x11AddScreenAsResult(&results, ScreenOfDisplay(display, i), 0);

    if(results.length == 0)
    {
        ffListDestroy(&results);
        ffXCloseDisplay(display);
        dlclose(x11);
        return false;
    }

    printResolutionResultList(instance, &results);

    ffListDestroy(&results);
    ffXCloseDisplay(display);
    dlclose(x11);

    return true;
}

typedef struct XrandrData
{
    XRRScreenConfiguration*(*ffXRRGetScreenInfo)(Display* display, Window window);
    short(*ffXRRConfigCurrentRate)(XRRScreenConfiguration* config);
    XRRMonitorInfo*(*ffXRRGetMonitors)(Display* display, Window window, Bool getActive, int* nmonitors);
    XRRScreenResources*(*ffXRRGetScreenResources)(Display* display, Window window);
    XRROutputInfo*(*ffXRRGetOutputInfo)(Display* display, XRRScreenResources* resources, RROutput output);
    XRRCrtcInfo*(*ffXRRGetCrtcInfo)(Display* display, XRRScreenResources* resources, RRCrtc crtc);
    void(*ffXRRFreeCrtcInfo)(XRRCrtcInfo* crtcInfo);
    void(*ffXRRFreeOutputInfo)(XRROutputInfo* outputInfo);
    void(*ffXRRFreeScreenResources)(XRRScreenResources* resources);
    void(*ffXRRFreeMonitors)(XRRMonitorInfo* monitors);
    void(*ffXRRFreeScreenConfigInfo)(XRRScreenConfiguration* config);

    //Init once
    Display* display;
    FFlist results;

    //Init per screen
    int defaultRefreshRate;
    XRRScreenResources* screenResources;
} XrandrData;

static XRRModeInfo* xrandrGetModeInfo(XRRScreenResources* screenResources, RRMode mode)
{
    for(int i = 0; i < screenResources->nmode; i++)
    {
        if(screenResources->modes[i].id == mode)
            return &screenResources->modes[i];
    }
    return NULL;
}

static bool xrandrHandleOutputInfo(XrandrData* data, XRROutputInfo* outputInfo)
{
    XRRCrtcInfo* crtcInfo = data->ffXRRGetCrtcInfo(data->display, data->screenResources, outputInfo->crtc);
    if(crtcInfo == NULL)
        return false;

    XRRModeInfo* modeInfo = xrandrGetModeInfo(data->screenResources, crtcInfo->mode);
    if(modeInfo == NULL || modeInfo->width == 0 || modeInfo->height == 0)
    {
        data->ffXRRFreeCrtcInfo(crtcInfo);
        return false;
    }

    ResolutionResult* result = ffListAdd(&data->results);
    result->width = modeInfo->width;
    result->height = modeInfo->height;
    result->refreshRate = parseRefreshRate(modeInfo->dotClock / (modeInfo->hTotal * modeInfo->vTotal));

    if(result->refreshRate == 0)
        result->refreshRate = data->defaultRefreshRate;

    data->ffXRRFreeCrtcInfo(crtcInfo);

    return true;
}

static void xrandrHandleMonitorFallback(XrandrData* data, XRRMonitorInfo* monitorInfo)
{
    if(monitorInfo->width == 0 || monitorInfo->height == 0)
        return;

    ResolutionResult* result = ffListAdd(&data->results);
    result->width = monitorInfo->width;
    result->height = monitorInfo->height;
    result->refreshRate = data->defaultRefreshRate;
}

static void xrandrHandleMonitor(XrandrData* data, XRRMonitorInfo* monitorInfo)
{
    bool foundMode = false;

    for(int i = 0; i < monitorInfo->noutput; i++)
    {
        XRROutputInfo* outputInfo = data->ffXRRGetOutputInfo(data->display, data->screenResources, monitorInfo->outputs[i]);
        if(outputInfo == NULL)
            continue;

        if(xrandrHandleOutputInfo(data, outputInfo))
            foundMode = true;

        data->ffXRRFreeOutputInfo(outputInfo);
    }

    if(!foundMode)
        xrandrHandleMonitorFallback(data, monitorInfo);
}

static void xrandrHandleScreen(XrandrData* data, Screen* screen)
{
    Window window = RootWindowOfScreen(screen);

    XRRScreenConfiguration* screenConfiguration = data->ffXRRGetScreenInfo(data->display, window);
    if(screenConfiguration != NULL)
    {
        data->defaultRefreshRate = (int) data->ffXRRConfigCurrentRate(screenConfiguration);
        data->ffXRRFreeScreenConfigInfo(screenConfiguration);
    }
    else
        data->defaultRefreshRate = 0;

    int numberOfMonitors;
    XRRMonitorInfo* monitorInfos = data->ffXRRGetMonitors(data->display, window, True, &numberOfMonitors);
    if(monitorInfos == NULL)
    {
        x11AddScreenAsResult(&data->results, screen, data->defaultRefreshRate);
        return;
    }

    data->screenResources = data->ffXRRGetScreenResources(data->display, window);

    uint32_t initialResultsLength = data->results.length;

    for(int i = 0; i < numberOfMonitors; i++)
    {
        if(data->screenResources != NULL)
            xrandrHandleMonitor(data, &monitorInfos[i]);
        else
            xrandrHandleMonitorFallback(data, &monitorInfos[i]);
    }

    if(data->results.length == initialResultsLength)
        x11AddScreenAsResult(&data->results, screen, data->defaultRefreshRate);

    data->ffXRRFreeScreenResources(data->screenResources);
    data->ffXRRFreeMonitors(monitorInfos);
}

static bool printResolutionXrandrBackend(FFinstance* instance)
{
    DynamicLibrary xrandr = FF_LIBRARY_LOAD(instance->config.libXrandr, "libXrandr.so");

    Display*(*ffXOpenDisplay)(const char*) = FF_LIBRARY_LOAD_SYMBOL(xrandr, "XOpenDisplay");
    int(*ffXCloseDisplay)(Display*) = FF_LIBRARY_LOAD_SYMBOL(xrandr, "XCloseDisplay");

    XrandrData data;

    data.ffXRRGetScreenInfo = FF_LIBRARY_LOAD_SYMBOL(xrandr, "XRRGetScreenInfo");
    data.ffXRRConfigCurrentRate = FF_LIBRARY_LOAD_SYMBOL(xrandr, "XRRConfigCurrentRate");
    data.ffXRRGetMonitors = FF_LIBRARY_LOAD_SYMBOL(xrandr, "XRRGetMonitors");
    data.ffXRRGetScreenResources = FF_LIBRARY_LOAD_SYMBOL(xrandr, "XRRGetScreenResources");
    data.ffXRRGetOutputInfo = FF_LIBRARY_LOAD_SYMBOL(xrandr, "XRRGetOutputInfo");
    data.ffXRRGetCrtcInfo = FF_LIBRARY_LOAD_SYMBOL(xrandr, "XRRGetCrtcInfo");
    data.ffXRRFreeCrtcInfo = FF_LIBRARY_LOAD_SYMBOL(xrandr, "XRRFreeCrtcInfo");
    data.ffXRRFreeOutputInfo = FF_LIBRARY_LOAD_SYMBOL(xrandr, "XRRFreeOutputInfo");
    data.ffXRRFreeScreenResources = FF_LIBRARY_LOAD_SYMBOL(xrandr, "XRRFreeScreenResources");
    data.ffXRRFreeMonitors = FF_LIBRARY_LOAD_SYMBOL(xrandr, "XRRFreeMonitors");
    data.ffXRRFreeScreenConfigInfo = FF_LIBRARY_LOAD_SYMBOL(xrandr, "XRRFreeScreenConfigInfo");

    data.display = ffXOpenDisplay(NULL);
    if(data.display == NULL)
    {
        dlclose(xrandr);
        return false;
    }

    ffListInitA(&data.results, sizeof(ResolutionResult), 4);

    for(int i = 0; i < ScreenCount(data.display); i++)
        xrandrHandleScreen(&data, ScreenOfDisplay(data.display, i));

    if(data.results.length == 0)
    {
        ffListDestroy(&data.results);
        ffXCloseDisplay(data.display);
        dlclose(xrandr);
        return false;
    }

    printResolutionResultList(instance, &data.results);

    ffListDestroy(&data.results);
    ffXCloseDisplay(data.display);
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

static void waylandOutputModeListener(void* data, struct wl_output* output, uint32_t flags, int32_t width, int32_t height, int32_t refreshRate)
{
    if(!(flags & WL_OUTPUT_MODE_CURRENT))
        return;

    WaylandData* wldata = (WaylandData*) data;

    ++wldata->moduleCounter;
    printResolution(wldata->instance, wldata->numModules == 1 ? 0 : wldata->moduleCounter, &wldata->cache, width, height, parseRefreshRate(refreshRate / 1000));

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

    DynamicLibrary wayland = FF_LIBRARY_LOAD(instance->config.libWayland, "libwayland-client.so");

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
        printResolutionWaylandBackend(instance) ||
        printResolutionXrandrBackend(instance) ||
        printResolutionX11Backend(instance)
    ) return;

    printResolutionDRMBackend(instance);
}

#undef FF_LIBRARY_LOAD
#undef FF_LIBRARY_LOAD_SYMBOL
