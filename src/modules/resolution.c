#include "fastfetch.h"

#include <string.h>
#include <dlfcn.h>
#include <dirent.h>
#include <pthread.h>
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

typedef struct ResolutionResult
{
    int width;
    int height;
    int refreshRate;
} ResolutionResult;

static bool printResolutionResultList(FFinstance* instance, FFlist* results)
{
    for(uint32_t i = 0; i < results->length; i++)
    {
        ResolutionResult* result = ffListGet(results, i);
        uint8_t moduleIndex = results->length == 1 ? 0 : i + 1;

        if(instance->config.resolutionFormat.length == 0)
        {
            ffPrintLogoAndKey(instance, FF_RESOLUTION_MODULE_NAME, moduleIndex, &instance->config.resolutionKey);
            printf("%ix%i", result->width, result->height);

            if(result->refreshRate > 0)
                printf(" @ %iHz", result->refreshRate);

            putchar('\n');
        }
        else
        {
            ffPrintFormatString(instance, FF_RESOLUTION_MODULE_NAME, moduleIndex, &instance->config.resolutionKey, &instance->config.resolutionFormat, NULL, FF_RESOLUTION_NUM_FORMAT_ARGS, (FFformatarg[]) {
                {FF_FORMAT_ARG_TYPE_INT, &result->width},
                {FF_FORMAT_ARG_TYPE_INT, &result->height},
                {FF_FORMAT_ARG_TYPE_INT, &result->refreshRate}
            });
        }
    }

    bool res = results->length > 0;
    ffListDestroy(results);
    return res;
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

    ffXCloseDisplay(display);
    dlclose(x11);

    return printResolutionResultList(instance, &results);
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

static bool xrandrHandleMonitorFallback(XrandrData* data, XRRMonitorInfo* monitorInfo)
{
    if(monitorInfo->width == 0 || monitorInfo->height == 0)
        return false;

    ResolutionResult* result = ffListAdd(&data->results);
    result->width = monitorInfo->width;
    result->height = monitorInfo->height;
    result->refreshRate = data->defaultRefreshRate;
    return true;
}

static bool xrandrHandleMonitor(XrandrData* data, XRRMonitorInfo* monitorInfo)
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
        return xrandrHandleMonitorFallback(data, monitorInfo);

    return true;
}

static inline bool xrandrLoopMonitors(XrandrData* data, XRRMonitorInfo* monitorInfos, int numberOfMonitors, bool(*monitorFunc)(XrandrData* data, XRRMonitorInfo* monitorInfo))
{
    bool foundAMonitor = false;

    for(int i = 0; i < numberOfMonitors; i++)
    {
        if(monitorFunc(data, &monitorInfos[i]))
            foundAMonitor = true;
    }

    return foundAMonitor;
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

    bool foundAMonitor;

    if(data->screenResources != NULL)
        foundAMonitor = xrandrLoopMonitors(data, monitorInfos, numberOfMonitors, xrandrHandleMonitor);
    else
        foundAMonitor = xrandrLoopMonitors(data, monitorInfos, numberOfMonitors, xrandrHandleMonitorFallback);

    if(!foundAMonitor)
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

    ffXCloseDisplay(data.display);
    dlclose(xrandr);

    return printResolutionResultList(instance, &data.results);
}

typedef struct WaylandData
{
    FFinstance* instance;
    FFlist results;
    struct wl_proxy*(*ffwl_proxy_marshal_constructor_versioned)(struct wl_proxy*, uint32_t, const struct wl_interface*, uint32_t, ...);
    int(*ffwl_proxy_add_listener)(struct wl_proxy*, void (**)(void), void *data);
    void(*ffwl_proxy_destroy)(struct wl_proxy*);
    const struct wl_interface* ffwl_output_interface;
    struct wl_output_listener output_listener;
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

    wldata->ffwl_proxy_destroy((struct wl_proxy*) output);

    if(width == 0 || height == 0)
        return;

    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&mutex);

    ResolutionResult* result = ffListAdd(&wldata->results);

    pthread_mutex_unlock(&mutex);

    result->width = (int) width;
    result->height = (int) height;
    result->refreshRate = parseRefreshRate(refreshRate / 1000);
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

static bool printResolutionWaylandBackend(FFinstance* instance)
{
    const char* sessionType = getenv("XDG_SESSION_TYPE");
    if(sessionType != NULL && strcasecmp(sessionType, "wayland") != 0)
        return false;

    DynamicLibrary wayland = FF_LIBRARY_LOAD(instance->config.libWayland, "libwayland-client.so");

    struct wl_display*(*ffwl_display_connect)(const char*) = FF_LIBRARY_LOAD_SYMBOL(wayland, "wl_display_connect");
    int(*ffwl_display_dispatch)(struct wl_display*) = FF_LIBRARY_LOAD_SYMBOL(wayland, "wl_display_dispatch");
    void(*ffwl_display_roundtrip)(struct wl_display*) = FF_LIBRARY_LOAD_SYMBOL(wayland, "wl_display_roundtrip");
    struct wl_proxy*(*ffwl_proxy_marshal_constructor)(struct wl_proxy*, uint32_t, const struct wl_interface*, ...) = FF_LIBRARY_LOAD_SYMBOL(wayland, "wl_proxy_marshal_constructor");
    const struct wl_interface* ffwl_registry_interface = FF_LIBRARY_LOAD_SYMBOL(wayland, "wl_registry_interface");
    void(*ffwl_display_disconnect)(struct wl_display*) = FF_LIBRARY_LOAD_SYMBOL(wayland, "wl_display_disconnect");

    WaylandData data;

    data.ffwl_proxy_marshal_constructor_versioned = FF_LIBRARY_LOAD_SYMBOL(wayland, "wl_proxy_marshal_constructor_versioned");
    data.ffwl_proxy_add_listener = FF_LIBRARY_LOAD_SYMBOL(wayland, "wl_proxy_add_listener");
    data.ffwl_output_interface = FF_LIBRARY_LOAD_SYMBOL(wayland, "wl_output_interface");
    data.ffwl_proxy_destroy = FF_LIBRARY_LOAD_SYMBOL(wayland, "wl_proxy_destroy");

    struct wl_display* display = ffwl_display_connect(NULL);
    if(display == NULL)
    {
        dlclose(wayland);
        return false;
    }

    struct wl_registry* registry = (struct wl_registry*) ffwl_proxy_marshal_constructor((struct wl_proxy*) display, WL_DISPLAY_GET_REGISTRY, ffwl_registry_interface, NULL);
    if(registry == NULL)
    {
        ffwl_display_disconnect(display);
        dlclose(wayland);
        return false;
    }

    data.instance = instance;
    ffListInitA(&data.results, sizeof(ResolutionResult), 4);

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

    return printResolutionResultList(instance, &data.results);
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
