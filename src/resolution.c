#include "fastfetch.h"

#include "X11/extensions/Xrandr.h"
#include "dlfcn.h"

short getCurrentRate(Display* display)
{
    void* xrandr = dlopen("libXrandr.so", RTLD_LAZY);
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

void ffPrintResolution(FFstate* state)
{
    if(ffPrintCachedValue(state, "Resolution"))
        return;

    void* x11 = dlopen("libX11.so", RTLD_LAZY);
    if(x11 == NULL)
        return;

    Display*(*ffXOpenDisplay)(const char*) = dlsym(x11, "XOpenDisplay");
    if(ffXOpenDisplay == NULL)
        return;

    Display* display = ffXOpenDisplay(NULL);
    if(display == NULL)
        return;

    Screen*  screen  = DefaultScreenOfDisplay(display);

    short currentRate = getCurrentRate(display);

    dlclose(x11);

    char resolution[1024];

    if(currentRate == 0)
        sprintf(resolution, "%ix%i", screen->width, screen->height);
    else
        sprintf(resolution, "%ix%i @ %dHz", screen->width, screen->height, currentRate);

    ffPrintAndSaveCachedValue(state, "Resolution", resolution);
}