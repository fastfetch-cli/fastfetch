#include "fastfetch.h"

#include <dlfcn.h>
#include <X11/extensions/Xrandr.h>

#define FF_RESOLUTION_MODULE_NAME "Resolution"
#define FF_RESOLUTION_NUM_FORMAT_ARGS 3

static void printValue(FFinstance* instance, uint8_t moduleIndex, FFcache* cache, int width, int height, int refreshRate)
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

static Display* openDisplay(FFinstance* instance, void* library, bool printErrors)
{
    Display*(*ffXOpenDisplay)(const char*) = dlsym(library, "XOpenDisplay");
    if(ffXOpenDisplay == NULL)
    {
        dlclose(library);
        if(printErrors)
            ffPrintError(instance, FF_RESOLUTION_MODULE_NAME, 0, &instance->config.resolutionKey, &instance->config.resolutionFormat, FF_RESOLUTION_NUM_FORMAT_ARGS, "dlsym(library, \"XOpenDisplay\") == NULL");
        return NULL;
    }

    Display* display = ffXOpenDisplay(NULL);
    if(display == NULL)
    {
        dlclose(library);
        if(printErrors)
            ffPrintError(instance, FF_RESOLUTION_MODULE_NAME, 0, &instance->config.resolutionKey, &instance->config.resolutionFormat, FF_RESOLUTION_NUM_FORMAT_ARGS, "ffXOpenDisplay(NULL) == NULL");
        return NULL;
    }

    return display;
}

static void closeDisplay(void* library, Display* display)
{
    int(*ffXCloseDisplay)(Display*) = dlsym(library, "XCloseDisplay");
    if(ffXCloseDisplay != NULL)
        ffXCloseDisplay(display);
}

static void printResolutionX11Backend(FFinstance* instance)
{
    void* x11;
    if(instance->config.libX11.length == 0)
        x11 = dlopen("libX11.so", RTLD_LAZY);
    else
        x11 = dlopen(instance->config.libX11.chars, RTLD_LAZY);

    if(x11 == NULL)
    {
        ffPrintError(instance, FF_RESOLUTION_MODULE_NAME, 0, &instance->config.resolutionKey, &instance->config.resolutionFormat, FF_RESOLUTION_NUM_FORMAT_ARGS, "dlopen(\"libX11.so\", RTLD_LAZY) == NULL");
        return;
    }

    Display* display = openDisplay(instance, x11, true);
    if(display == NULL)
        return;

    int screenCount = ScreenCount(display);
    if(screenCount < 1)
    {
        closeDisplay(x11, display);
        dlclose(x11);
        ffPrintError(instance, FF_RESOLUTION_MODULE_NAME, 0, &instance->config.resolutionKey, &instance->config.resolutionFormat, FF_RESOLUTION_NUM_FORMAT_ARGS, "ScreenCount(display) < 1: %i", screenCount);
        return;
    }

    FFcache cache;
    ffCacheOpenWrite(instance, FF_RESOLUTION_MODULE_NAME, &cache);

    for(int i = 0; i < screenCount; i++)
    {
        Screen* screen = ScreenOfDisplay(display, i);
        uint8_t moduleIndex = screenCount == 1 ? 0 : i + 1;
        printValue(instance, moduleIndex, &cache, WidthOfScreen(screen), HeightOfScreen(screen), 0);
    }

    ffCacheClose(&cache);

    closeDisplay(x11, display);
    dlclose(x11);
}

static int getCurrentRate(void* xrandr, Display* display)
{
    XRRScreenConfiguration*(*ffXRRGetScreenInfo)(Display*, Window) = dlsym(xrandr, "XRRGetScreenInfo");
    if(ffXRRGetScreenInfo == NULL)
        return 0;

    short(*ffXRRConfigCurrentRate)(XRRScreenConfiguration*) = dlsym(xrandr, "XRRConfigCurrentRate");
    if(ffXRRConfigCurrentRate == NULL)
        return 0;

    XRRMonitorInfo*(*ffXRRGetMonitors)(Display*, Window, Bool, int*) = dlsym(xrandr, "XRRGetMonitors");
    if(ffXRRGetMonitors == NULL)
        return 0;

    void(*ffXRRFreeMonitors)(XRRMonitorInfo*) = dlsym(xrandr, "XRRFreeMonitors");
    if(ffXRRFreeMonitors == NULL)
        return 0;

    XRRScreenConfiguration* xrrscreenconf = ffXRRGetScreenInfo(display, RootWindow(display, 0));
    if(xrrscreenconf == NULL)
        return 0;

    short currentRate = ffXRRConfigCurrentRate(xrrscreenconf);

    void(*ffXRRFreeScreenConfigInfo)(XRRScreenConfiguration*) = dlsym(xrandr, "XRRFreeScreenConfigInfo");
    if(ffXRRFreeScreenConfigInfo != NULL)
        ffXRRFreeScreenConfigInfo(xrrscreenconf);

    return (int) currentRate;
}

static bool printResolutionXrandrBackend(FFinstance* instance)
{
    void* xrandr;
    if(instance->config.libXrandr.length == 0)
        xrandr = dlopen("libXrandr.so", RTLD_LAZY);
    else
        xrandr = dlopen(instance->config.libXrandr.chars, RTLD_LAZY);

    if(xrandr == NULL)
        return false;

    XRRMonitorInfo*(*ffXRRGetMonitors)(Display*, Window, Bool, int*) = dlsym(xrandr, "XRRGetMonitors");
    if(ffXRRGetMonitors == NULL)
    {
        dlclose(xrandr);
        return false;
    }

    Display* display = openDisplay(instance, xrandr, false);
    if(display == NULL)
        return false;

    int numberOfMonitors;
    XRRMonitorInfo* monitors = ffXRRGetMonitors(display, RootWindow(display, 0), False, &numberOfMonitors);
    if(monitors == NULL)
    {
        closeDisplay(xrandr, display);
        dlclose(xrandr);
        return false;
    }

    if(numberOfMonitors < 1)
    {
        closeDisplay(xrandr, display);
        dlclose(xrandr);
        return false;
    }

    int refreshRate = getCurrentRate(xrandr, display);

    FFcache cache;
    ffCacheOpenWrite(instance, FF_RESOLUTION_MODULE_NAME, &cache);

    for(int i = 0; i < numberOfMonitors; i++)
    {
        uint8_t moduleIndex = numberOfMonitors == 1 ? 0 : i + 1;
        printValue(instance, moduleIndex, &cache, monitors[i].width, monitors[i].height, refreshRate);
    }

    ffCacheClose(&cache);

    void(*ffXRRFreeMonitors)(XRRMonitorInfo*) = dlsym(xrandr, "XRRFreeMonitors");
    if(ffXRRFreeMonitors != NULL)
        ffXRRFreeMonitors(monitors);

    dlclose(xrandr);

    return true;
}

void ffPrintResolution(FFinstance* instance)
{
    if(ffPrintFromCache(instance, FF_RESOLUTION_MODULE_NAME, &instance->config.resolutionKey, &instance->config.resolutionFormat, FF_RESOLUTION_NUM_FORMAT_ARGS))
        return;

    if(printResolutionXrandrBackend(instance))
        return;

    printResolutionX11Backend(instance);
}
