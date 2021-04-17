#include "resolution.h"

#include <dlfcn.h>
#include <X11/extensions/Xrandr.h>

static Display* xOpenDisplay(FFinstance* instance, void* library, bool printErrors)
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

static void xCloseDisplay(void* library, Display* display)
{
    int(*ffXCloseDisplay)(Display*) = dlsym(library, "XCloseDisplay");
    if(ffXCloseDisplay != NULL)
        ffXCloseDisplay(display);
}

void ffPrintResolutionX11Backend(FFinstance* instance)
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

    Display* display = xOpenDisplay(instance, x11, true);
    if(display == NULL)
        return;

    int screenCount = ScreenCount(display);
    if(screenCount < 1)
    {
        xCloseDisplay(x11, display);
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
        ffPrintResolutionValue(instance, moduleIndex, &cache, WidthOfScreen(screen), HeightOfScreen(screen), 0);
    }

    ffCacheClose(&cache);

    xCloseDisplay(x11, display);
    dlclose(x11);
}

static int xrandrGetCurrentRate(void* xrandr, Display* display)
{
    XRRScreenConfiguration*(*ffXRRGetScreenInfo)(Display*, Window) = dlsym(xrandr, "XRRGetScreenInfo");
    if(ffXRRGetScreenInfo == NULL)
        return 0;

    short(*ffXRRConfigCurrentRate)(XRRScreenConfiguration*) = dlsym(xrandr, "XRRConfigCurrentRate");
    if(ffXRRConfigCurrentRate == NULL)
        return 0;


    XRRScreenConfiguration* xrrscreenconf = ffXRRGetScreenInfo(display, DefaultRootWindow(display));
    if(xrrscreenconf == NULL)
        return 0;

    short currentRate = ffXRRConfigCurrentRate(xrrscreenconf);

    void(*ffXRRFreeScreenConfigInfo)(XRRScreenConfiguration*) = dlsym(xrandr, "XRRFreeScreenConfigInfo");
    if(ffXRRFreeScreenConfigInfo != NULL)
        ffXRRFreeScreenConfigInfo(xrrscreenconf);

    return (int) currentRate;
}

bool ffPrintResolutionXrandrBackend(FFinstance* instance)
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

    Display* display = xOpenDisplay(instance, xrandr, false);
    if(display == NULL)
        return false;

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
        ffPrintResolutionValue(instance, moduleIndex, &cache, monitors[i].width, monitors[i].height, refreshRate);
    }

    ffCacheClose(&cache);

    void(*ffXRRFreeMonitors)(XRRMonitorInfo*) = dlsym(xrandr, "XRRFreeMonitors");
    if(ffXRRFreeMonitors != NULL)
        ffXRRFreeMonitors(monitors);

    xCloseDisplay(xrandr, display);
    dlclose(xrandr);

    return true;
}
