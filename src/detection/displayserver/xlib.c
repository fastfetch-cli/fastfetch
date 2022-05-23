#include "displayServer.h"

#ifdef FF_HAVE_X11
#include <X11/Xlib.h>

typedef struct X11PropertyData
{
    FF_LIBRARY_SYMBOL(XInternAtom)
    FF_LIBRARY_SYMBOL(XGetWindowProperty)
} X11PropertyData;

static bool x11InitPropertyData(void* libraryHandle, X11PropertyData* propertyData)
{
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(libraryHandle, propertyData->ffXInternAtom, XInternAtom, false)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(libraryHandle, propertyData->ffXGetWindowProperty, XGetWindowProperty, false)

    return true;
}

static unsigned char* x11GetProperty(X11PropertyData* data, Display* display, Window window, const char* request)
{
    Atom requestAtom = data->ffXInternAtom(display, request, False);
    if(requestAtom == None)
        return NULL;

    Atom actualType;
    unsigned long unused;
    unsigned char* result = NULL;

    data->ffXGetWindowProperty(display, window, requestAtom, 0, 64, False, AnyPropertyType, &actualType, (int*) &unused, &unused, &unused, &result);

    return result;
}

static void x11DetectWMFromEWMH(X11PropertyData* data, Display* display, FFDisplayServerResult* result)
{
    if(result->wmProcessName.length > 0)
        return;

    Window* wmWindow = (Window*) x11GetProperty(data, display, DefaultRootWindow(display), "_NET_SUPPORTING_WM_CHECK");
    if(wmWindow == NULL)
        return;

    const char* wmName = (const char*) x11GetProperty(data, display, *wmWindow, "_NET_WM_NAME");
    if(wmName == NULL)
        wmName = (const char*) x11GetProperty(data, display, *wmWindow, "WM_NAME");

    if(!ffStrSet(wmName))
        return;

    ffStrbufSetS(&result->wmProcessName, wmName);
}

void ffdsConnectXlib(const FFinstance* instance, FFDisplayServerResult* result)
{
    FF_LIBRARY_LOAD(x11, instance->config.libX11, , "libX11.so", 7, "libX11-xcb.so", 2)
    FF_LIBRARY_LOAD_SYMBOL(x11, XOpenDisplay,)
    FF_LIBRARY_LOAD_SYMBOL(x11, XCloseDisplay,)

    X11PropertyData propertyData;
    bool propertyDataInitialized = x11InitPropertyData(x11, &propertyData);

    Display* display = ffXOpenDisplay(x11);
    if(display == NULL)
    {
        dlclose(x11);
        return;
    }

    if(propertyDataInitialized && ScreenCount(display) > 0)
        x11DetectWMFromEWMH(&propertyData, display, result);

    for(int i = 0; i < ScreenCount(display); i++)
    {
        Screen* screen = ScreenOfDisplay(display, i);
        ffdsAppendResolution(
            result,
            (uint32_t) screen->width,
            (uint32_t) screen->height,
            0
        );
    }

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
    FFDisplayServerResult* result;

    //Init per screen
    uint32_t defaultRefreshRate;
    XRRScreenResources* screenResources;
} XrandrData;

static bool xrandrHandleModeInfo(XrandrData* data, XRRModeInfo* modeInfo)
{
    uint32_t refreshRate = ffdsParseRefreshRate((int32_t) (
        modeInfo->dotClock / (modeInfo->hTotal * modeInfo->vTotal)
    ));

    return ffdsAppendResolution(
        data->result,
        (uint32_t) modeInfo->width,
        (uint32_t) modeInfo->height,
        refreshRate == 0 ? data->defaultRefreshRate : refreshRate
    );
}

static bool xrandrHandleMode(XrandrData* data, RRMode mode)
{
    for(int i = 0; i < data->screenResources->nmode; i++)
    {
        if(data->screenResources->modes[i].id == mode)
            return xrandrHandleModeInfo(data, &data->screenResources->modes[i]);
    }
    return false;
}

static bool xrandrHandleCrtc(XrandrData* data, RRCrtc crtc)
{
    //We do the check here, because we want the best fallback resolution if this call failed
    if(data->screenResources == NULL)
        return false;

    XRRCrtcInfo* crtcInfo = data->ffXRRGetCrtcInfo(data->display, data->screenResources, crtc);
    if(crtcInfo == NULL)
        return false;

    bool res = xrandrHandleMode(data, crtcInfo->mode);
    res = res ? true : ffdsAppendResolution(
        data->result,
        (uint32_t) crtcInfo->width,
        (uint32_t) crtcInfo->height,
        data->defaultRefreshRate
    );

    data->ffXRRFreeCrtcInfo(crtcInfo);
    return res;
}

static bool xrandrHandleOutput(XrandrData* data, RROutput output)
{
    XRROutputInfo* outputInfo = data->ffXRRGetOutputInfo(data->display, data->screenResources, output);
    if(outputInfo == NULL)
        return false;

    bool res = xrandrHandleCrtc(data, outputInfo->crtc);

    data->ffXRRFreeOutputInfo(outputInfo);

    return res;
}

static bool xrandrHandleMonitor(XrandrData* data, XRRMonitorInfo* monitorInfo)
{
    bool foundOutput = false;

    for(int i = 0; i < monitorInfo->noutput; i++)
    {
        if(xrandrHandleOutput(data, monitorInfo->outputs[i]))
            foundOutput = true;
    }

    return foundOutput ? true : ffdsAppendResolution(
        data->result,
        (uint32_t) monitorInfo->width,
        (uint32_t) monitorInfo->height,
        data->defaultRefreshRate
    );
}

static bool xrandrHandleMonitors(XrandrData* data, Screen* screen)
{
    int numberOfMonitors;
    XRRMonitorInfo* monitorInfos = data->ffXRRGetMonitors(data->display, RootWindowOfScreen(screen), True, &numberOfMonitors);
    if(monitorInfos == NULL)
        return false;

    bool foundAMonitor;

    for(int i = 0; i < numberOfMonitors; i++)
    {
        if(xrandrHandleMonitor(data, &monitorInfos[i]))
            foundAMonitor = true;
    }

    data->ffXRRFreeMonitors(monitorInfos);

    return foundAMonitor;
}

static void xrandrHandleScreen(XrandrData* data, Screen* screen)
{
    //Init screen configuration. This is used to get the default refresh rate. If this fails, default refresh rate is simply 0.
    XRRScreenConfiguration* screenConfiguration = data->ffXRRGetScreenInfo(data->display, RootWindowOfScreen(screen));

    if(screenConfiguration != NULL)
    {
        data->defaultRefreshRate = (uint32_t) data->ffXRRConfigCurrentRate(screenConfiguration);
        data->ffXRRFreeScreenConfigInfo(screenConfiguration);
    }
    else
        data->defaultRefreshRate = 0;

    //Init screen resources
    data->screenResources = data->ffXRRGetScreenResources(data->display, RootWindowOfScreen(screen));

    bool ret = xrandrHandleMonitors(data, screen);

    data->ffXRRFreeScreenResources(data->screenResources);

    if(ret)
        return;

    //Fallback to screen
    ffdsAppendResolution(
        data->result,
        (uint32_t) WidthOfScreen(screen),
        (uint32_t) HeightOfScreen(screen),
        data->defaultRefreshRate
    );
}

void ffdsConnectXrandr(const FFinstance* instance, FFDisplayServerResult* result)
{
    FF_LIBRARY_LOAD(xrandr, instance->config.libXrandr, , "libXrandr.so", 3)

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

    X11PropertyData propertyData;
    bool propertyDataInitialized = x11InitPropertyData(xrandr, &propertyData);

    data.display = ffXOpenDisplay(NULL);
    if(data.display == NULL)
    {
        dlclose(xrandr);
        return;
    }

    if(propertyDataInitialized && ScreenCount(data.display) > 0)
        x11DetectWMFromEWMH(&propertyData, data.display, result);

    data.result = result;

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
