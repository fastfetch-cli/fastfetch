#include "displayServer.h"

#ifdef FF_HAVE_X11
#include <X11/Xlib.h>

static void x11AddScreenAsResult(FFlist* results, Screen* screen, uint32_t refreshRate)
{
    if(WidthOfScreen(screen) == 0 || HeightOfScreen(screen) == 0)
        return;

    FFResolutionResult* result = ffListAdd(results);
    result->width = (uint32_t) WidthOfScreen(screen);
    result->height = (uint32_t) HeightOfScreen(screen);
    result->refreshRate = refreshRate;
}

void ffdsConnectXlib(const FFinstance* instance, FFDisplayServerResult* result)
{
    FF_LIBRARY_LOAD(x11, "libX11.so", instance->config.libX11,)
    FF_LIBRARY_LOAD_SYMBOL(x11, XOpenDisplay,)
    FF_LIBRARY_LOAD_SYMBOL(x11, XCloseDisplay,)

    Display* display = ffXOpenDisplay(x11);
    if(display == NULL)
    {
        dlclose(x11);
        return;
    }

    for(int i = 0; i < ScreenCount(display); i++)
        x11AddScreenAsResult(&result->resolutions, ScreenOfDisplay(display, i), 0);

    ffXCloseDisplay(display);
    dlclose(x11);

    //If wayland hasn't set this, connection failed for it. So we are running only a X Server, not XWayland.
    if(result->wmProtocolName.length == 0)
        ffStrbufSetS(&result->wmProtocolName, FF_DISPLAYSERVER_PROTOCOL_X11);
}

#else

void ffdsConnectXlib(const FFinstance* instance, FFDisplayServerResult* result)
{
    //Do nothing. WM / DE detection will use environment vars to detect as much as possible.
    FF_UNUSED(instance, result);
}

#endif //FF_HAVE_X11

#ifdef FF_HAVE_XRANDR
#include <X11/extensions/Xrandr.h>

typedef struct XrandrData
{
    FF_LIBRARY_SYMBOL(XRRGetScreenInfo)
    FF_LIBRARY_SYMBOL(XRRConfigCurrentConfiguration)
    FF_LIBRARY_SYMBOL(XRRConfigCurrentRate)
    FF_LIBRARY_SYMBOL(XRRGetMonitors)
    FF_LIBRARY_SYMBOL(XRRGetScreenResources)
    FF_LIBRARY_SYMBOL(XRRGetOutputInfo)
    FF_LIBRARY_SYMBOL(XRRGetCrtcInfo)
    FF_LIBRARY_SYMBOL(XRRFreeCrtcInfo)
    FF_LIBRARY_SYMBOL(XRRFreeOutputInfo)
    FF_LIBRARY_SYMBOL(XRRFreeScreenResources)
    FF_LIBRARY_SYMBOL(XRRFreeMonitors)
    FF_LIBRARY_SYMBOL(XRRFreeScreenConfigInfo)

    //Init once
    Display* display;
    FFlist* results;

    //Init per screen
    uint32_t defaultRefreshRate;
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

    FFResolutionResult* result = ffListAdd(data->results);
    result->width = modeInfo->width;
    result->height = modeInfo->height;
    result->refreshRate = ffdsParseRefreshRate((int32_t) (modeInfo->dotClock / (modeInfo->hTotal * modeInfo->vTotal)));

    if(result->refreshRate == 0)
        result->refreshRate = data->defaultRefreshRate;

    data->ffXRRFreeCrtcInfo(crtcInfo);

    return true;
}

static bool xrandrHandleMonitorFallback(XrandrData* data, XRRMonitorInfo* monitorInfo)
{
    if(monitorInfo->width == 0 || monitorInfo->height == 0)
        return false;

    FFResolutionResult* result = ffListAdd(data->results);
    result->width = (uint32_t) monitorInfo->width;
    result->height = (uint32_t) monitorInfo->height;
    result->refreshRate = (uint32_t) data->defaultRefreshRate;
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
        data->defaultRefreshRate = (uint32_t) data->ffXRRConfigCurrentRate(screenConfiguration);
        data->ffXRRFreeScreenConfigInfo(screenConfiguration);
    }
    else
        data->defaultRefreshRate = 0;

    int numberOfMonitors;
    XRRMonitorInfo* monitorInfos = data->ffXRRGetMonitors(data->display, window, True, &numberOfMonitors);
    if(monitorInfos == NULL)
    {
        x11AddScreenAsResult(data->results, screen, data->defaultRefreshRate);
        return;
    }

    data->screenResources = data->ffXRRGetScreenResources(data->display, window);

    bool foundAMonitor;

    if(data->screenResources != NULL)
        foundAMonitor = xrandrLoopMonitors(data, monitorInfos, numberOfMonitors, xrandrHandleMonitor);
    else
        foundAMonitor = xrandrLoopMonitors(data, monitorInfos, numberOfMonitors, xrandrHandleMonitorFallback);

    if(!foundAMonitor)
        x11AddScreenAsResult(data->results, screen, data->defaultRefreshRate);

    data->ffXRRFreeScreenResources(data->screenResources);
    data->ffXRRFreeMonitors(monitorInfos);
}

void ffdsConnectXrandr(const FFinstance* instance, FFDisplayServerResult* result)
{
    FF_LIBRARY_LOAD(xrandr, "libXrandr.so", instance->config.libXrandr,);

    FF_LIBRARY_LOAD_SYMBOL(xrandr, XOpenDisplay,)
    FF_LIBRARY_LOAD_SYMBOL(xrandr, XCloseDisplay,)

    XrandrData data;

    FF_LIBRARY_LOAD_SYMBOL_ADRESS(xrandr, data.ffXRRGetScreenInfo, XRRGetScreenInfo,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(xrandr, data.ffXRRConfigCurrentRate, XRRConfigCurrentRate,);
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(xrandr, data.ffXRRGetMonitors, XRRGetMonitors,);
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(xrandr, data.ffXRRGetScreenResources, XRRGetScreenResources,);
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(xrandr, data.ffXRRGetOutputInfo, XRRGetOutputInfo,);
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(xrandr, data.ffXRRGetCrtcInfo, XRRGetCrtcInfo,);
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(xrandr, data.ffXRRFreeCrtcInfo, XRRFreeCrtcInfo,);
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(xrandr, data.ffXRRFreeOutputInfo, XRRFreeOutputInfo,);
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(xrandr, data.ffXRRFreeScreenResources, XRRFreeScreenResources,);
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(xrandr, data.ffXRRFreeMonitors, XRRFreeMonitors,);
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(xrandr, data.ffXRRFreeScreenConfigInfo, XRRFreeScreenConfigInfo,);

    data.display = ffXOpenDisplay(NULL);
    if(data.display == NULL)
    {
        dlclose(xrandr);
        return;
    }

    data.results = &result->resolutions;

    for(int i = 0; i < ScreenCount(data.display); i++)
        xrandrHandleScreen(&data, ScreenOfDisplay(data.display, i));

    ffXCloseDisplay(data.display);
    dlclose(xrandr);

    //If wayland hasn't set this, connection failed for it. So we are running only a X Server, not XWayland.
    if(result->wmProtocolName.length == 0)
        ffStrbufSetS(&result->wmProtocolName, FF_DISPLAYSERVER_PROTOCOL_X11);
}

#else

void ffdsConnectXrandr(const FFinstance* instance, FFDisplayServerResult* result)
{
    //Do nothing here. There are more x11 implementaions to come.
    FF_UNUSED(instance, result);
}

#endif // FF_HAVE_XRANDR
