#include "fastfetch.h"

#include "X11/extensions/Xrandr.h"
#include "dlfcn.h"

static short getCurrentRate(FFinstance* instance, Display* display)
{
    void* xrandr;
    if(instance->config.resolutionLibXrandr[0] != '\0')
        xrandr = dlopen(instance->config.resolutionLibXrandr, RTLD_LAZY);
    else
        xrandr = dlopen("libXrandr.so", RTLD_LAZY);
    if(xrandr == NULL)
        return 0;

    Window root = RootWindow(display, 0);

    XRRScreenConfiguration*(*ffXRRGetScreenInfo)(Display*, Window) = dlsym(xrandr, "XRRGetScreenInfo");
    if(ffXRRGetScreenInfo == NULL)
        return 0;

    short(*ffXRRConfigCurrentRate)(XRRScreenConfiguration*) = dlsym(xrandr, "XRRConfigCurrentRate");
    if(ffXRRConfigCurrentRate == NULL)
        return 0;

    XRRScreenConfiguration* xrrscreenconf = ffXRRGetScreenInfo(display, root);
    if(xrrscreenconf == NULL)
        return 0;

    short currentRate = ffXRRConfigCurrentRate(xrrscreenconf);

    void(*ffXRRFreeScreenConfigInfo)(XRRScreenConfiguration*) = dlsym(xrandr, "XRRFreeScreenConfigInfo");
    if(ffXRRFreeScreenConfigInfo != NULL)
        ffXRRFreeScreenConfigInfo(xrrscreenconf);

    dlclose(xrandr);

    return currentRate;
}

void ffPrintResolution(FFinstance* instance)
{
    if(ffPrintCachedValue(instance, "Resolution"))
        return;

    void* x11;
    if(instance->config.resolutionLibX11[0] != '\0')
        x11 = dlopen(instance->config.resolutionLibX11, RTLD_LAZY);
    else
        x11 = dlopen("libX11.so", RTLD_LAZY);
    if(x11 == NULL)
    {
        ffPrintError(instance, "Resolution", "dlopen(\"libX11.so\", RTLD_LAZY) == NULL");
        return;
    }

    Display*(*ffXOpenDisplay)(const char*) = dlsym(x11, "XOpenDisplay");
    if(ffXOpenDisplay == NULL)
    {
        ffPrintError(instance, "Resolution", "dlsym(x11, \"XOpenDisplay\") == NULL");
        return;
    }

    Display* display = ffXOpenDisplay(NULL);
    if(display == NULL)
    {
        ffPrintError(instance, "Resolution", "ffXOpenDisplay(NULL) == NULL");
        return;
    }

    Screen*  screen  = DefaultScreenOfDisplay(display);

    short currentRate = instance->config.resolutionShowRefreshRate ? getCurrentRate(instance, display) : 0;

    dlclose(x11);

    char resolution[1024];

    if(instance->config.resolutionFormat[0] != '\0')
        sprintf(resolution, instance->config.resolutionFormat, screen->width, screen->height, currentRate);
    else if(currentRate == 0)
        sprintf(resolution, "%ix%i", screen->width, screen->height);
    else
        sprintf(resolution, "%ix%i @ %dHz", screen->width, screen->height, currentRate);

    ffPrintAndSaveCachedValue(instance, "Resolution", resolution);
}