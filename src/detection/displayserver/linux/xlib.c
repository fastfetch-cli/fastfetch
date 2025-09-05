#include "displayserver_linux.h"

#ifdef FF_HAVE_XRANDR

#include "common/library.h"
#include "common/properties.h"
#include "util/edidHelper.h"
#include "util/stringUtils.h"

#include <X11/extensions/Xrandr.h>
#include <X11/Xlib.h>

typedef struct X11PropertyData
{
    FF_LIBRARY_SYMBOL(XInternAtom)
    FF_LIBRARY_SYMBOL(XGetWindowProperty)
    FF_LIBRARY_SYMBOL(XServerVendor)
    FF_LIBRARY_SYMBOL(XFree)
} X11PropertyData;

static bool x11InitPropertyData(FF_MAYBE_UNUSED void* libraryHandle, X11PropertyData* propertyData)
{
    FF_LIBRARY_LOAD_SYMBOL_PTR(libraryHandle, propertyData, XInternAtom, false)
    FF_LIBRARY_LOAD_SYMBOL_PTR(libraryHandle, propertyData, XGetWindowProperty, false)
    FF_LIBRARY_LOAD_SYMBOL_PTR(libraryHandle, propertyData, XServerVendor, false)
    FF_LIBRARY_LOAD_SYMBOL_PTR(libraryHandle, propertyData, XFree, false)

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

    if(data->ffXGetWindowProperty(display, window, requestAtom, 0, 64, False, AnyPropertyType, &actualType, (int*) &unused, &unused, &unused, &result) != Success)
        return NULL;

    return result;
}

static void x11DetectWMFromEWMH(X11PropertyData* data, Display* display, FFDisplayServerResult* result)
{
    if(result->wmProcessName.length > 0 || ffStrbufCompS(&result->wmProtocolName, FF_WM_PROTOCOL_WAYLAND) == 0)
        return;

    Window* wmWindow = (Window*) x11GetProperty(data, display, DefaultRootWindow(display), "_NET_SUPPORTING_WM_CHECK");
    if(wmWindow == NULL)
        return;

    char* wmName = (char*) x11GetProperty(data, display, *wmWindow, "WM_NAME");
    if(!ffStrSet(wmName))
        wmName = (char*) x11GetProperty(data, display, *wmWindow, "_NET_WM_NAME");

    if(ffStrSet(wmName))
        ffStrbufSetS(&result->wmProcessName, wmName);

    data->ffXFree(wmName);
    data->ffXFree(wmWindow);
}

static void x11FetchServerVendor(X11PropertyData* data, Display* display, FFDisplayServerResult* result)
{
    const char* serverVendor = data->ffXServerVendor(display);
    if (serverVendor && !ffStrEquals(serverVendor, "The X.Org Foundation")) {
        ffStrbufSetS(&result->wmProtocolName, serverVendor);
    }
}

typedef struct XrandrData
{
    FF_LIBRARY_SYMBOL(XInternAtom)
    FF_LIBRARY_SYMBOL(XGetAtomName);
    FF_LIBRARY_SYMBOL(XFree);
    FF_LIBRARY_SYMBOL(XRRGetMonitors)
    FF_LIBRARY_SYMBOL(XRRGetScreenResourcesCurrent)
    FF_LIBRARY_SYMBOL(XRRGetOutputInfo)
    FF_LIBRARY_SYMBOL(XRRGetOutputProperty)
    FF_LIBRARY_SYMBOL(XRRGetCrtcInfo)
    FF_LIBRARY_SYMBOL(XRRFreeCrtcInfo)
    FF_LIBRARY_SYMBOL(XRRFreeOutputInfo)
    FF_LIBRARY_SYMBOL(XRRFreeScreenResources)
    FF_LIBRARY_SYMBOL(XRRFreeMonitors)

    //Init once
    Display* display;
    FFDisplayServerResult* result;
    X11PropertyData* propData;
} XrandrData;

static bool xrandrHandleCrtc(XrandrData* data, XRROutputInfo* output, FFstrbuf* name, bool primary, FFDisplayType displayType, uint8_t* edidData, uint32_t edidLength, XRRScreenResources* screenResources, uint8_t bitDepth, double scaleFactor)
{
    //We do the check here, because we want the best fallback display if this call failed
    if(screenResources == NULL)
        return false;

    XRRCrtcInfo* crtcInfo = data->ffXRRGetCrtcInfo(data->display, screenResources, output->crtc);
    if(crtcInfo == NULL)
        return false;

    uint32_t rotation;
    switch (crtcInfo->rotation)
    {
        case RR_Rotate_90:
            rotation = 90;
            break;
        case RR_Rotate_180:
            rotation = 180;
            break;
        case RR_Rotate_270:
            rotation = 270;
            break;
        default:
            rotation = 0;
            break;
    }

    XRRModeInfo* currentMode = NULL;
    for(int i = 0; i < screenResources->nmode; i++)
    {
        if(screenResources->modes[i].id == crtcInfo->mode)
        {
            currentMode = &screenResources->modes[i];
            break;
        }
    }

    XRRModeInfo* preferredMode = output->npreferred > 0 ? &screenResources->modes[0] : NULL;

    FFDisplayResult* item = ffdsAppendDisplay(
        data->result,
        (uint32_t) crtcInfo->width,
        (uint32_t) crtcInfo->height,
        currentMode ? (double) currentMode->dotClock / (double) ((uint32_t) currentMode->hTotal * currentMode->vTotal) : 0,
        (uint32_t) (crtcInfo->width / scaleFactor + .5),
        (uint32_t) (crtcInfo->height / scaleFactor + .5),
        preferredMode ? (uint32_t) preferredMode->width : 0,
        preferredMode ? (uint32_t) preferredMode->height : 0,
        preferredMode ? (double) preferredMode->dotClock / (double) ((uint32_t) preferredMode->hTotal * preferredMode->vTotal) : 0,
        rotation,
        name,
        displayType,
        primary,
        0,
        (uint32_t) output->mm_width,
        (uint32_t) output->mm_height,
        "xlib-randr-crtc"
    );

    if (item)
    {
        if (edidLength)
        {
            item->hdrStatus = ffEdidGetHdrCompatible(edidData, edidLength) ? FF_DISPLAY_HDR_STATUS_SUPPORTED : FF_DISPLAY_HDR_STATUS_UNSUPPORTED;
            ffEdidGetSerialAndManufactureDate(edidData, &item->serial, &item->manufactureYear, &item->manufactureWeek);
        }
        item->bitDepth = bitDepth;
    }

    data->ffXRRFreeCrtcInfo(crtcInfo);
    return !!item;
}

static bool xrandrHandleOutput(XrandrData* data, RROutput output, FFstrbuf* name, bool primary, FFDisplayType displayType, XRRScreenResources* screenResources, uint8_t bitDepth, double scaleFactor)
{
    XRROutputInfo* outputInfo = data->ffXRRGetOutputInfo(data->display, screenResources, output);
    if(outputInfo == NULL)
        return false;

    uint8_t* edidData = NULL;
    unsigned long edidLength = 0;
    Atom atomEdid = data->ffXInternAtom(data->display, "EDID", true);
    if (atomEdid != None)
    {
        int actual_format = 0;
        unsigned long bytes_after = 0;
        Atom actual_type = None;
        if (data->ffXRRGetOutputProperty(data->display, output, atomEdid, 0, 100, false, false, AnyPropertyType, &actual_type, &actual_format, &edidLength, &bytes_after, &edidData) == Success)
        {
            if (edidLength >= 128)
            {
                ffStrbufClear(name);
                ffEdidGetName(edidData, name);
            }
            else
                edidLength = 0;
        }
    }

    bool res = xrandrHandleCrtc(data, outputInfo, name, primary, displayType, edidData, (uint32_t) edidLength, screenResources, bitDepth, scaleFactor);

    if (edidData)
        data->ffXFree(edidData);
    data->ffXRRFreeOutputInfo(outputInfo);

    return res;
}

static bool xrandrHandleMonitor(XrandrData* data, XRRMonitorInfo* monitorInfo, XRRScreenResources* screenResources, uint8_t bitDepth, double scaleFactor)
{
    bool foundOutput = false;
    char* xname = data->ffXGetAtomName(data->display, monitorInfo->name);
    FF_STRBUF_AUTO_DESTROY name = ffStrbufCreateS(xname);
    data->ffXFree(xname);
    FFDisplayType displayType = ffdsGetDisplayType(name.chars);
    for(int i = 0; i < monitorInfo->noutput; i++)
    {
        if(xrandrHandleOutput(data, monitorInfo->outputs[i], &name, monitorInfo->primary, displayType, screenResources, bitDepth, scaleFactor))
            foundOutput = true;
    }

    if (foundOutput) return true;

    FFDisplayResult* display = ffdsAppendDisplay(
        data->result,
        (uint32_t) monitorInfo->width,
        (uint32_t) monitorInfo->height,
        0,
        (uint32_t) (monitorInfo->width / scaleFactor + .5),
        (uint32_t) (monitorInfo->height / scaleFactor + .5),
        0, 0, 0,
        0,
        &name,
        displayType,
        !!monitorInfo->primary,
        0,
        (uint32_t) monitorInfo->mwidth,
        (uint32_t) monitorInfo->mheight,
        "xlib-randr-monitor"
    );
    if (display) display->bitDepth = bitDepth;
    return !!display;
}

static bool xrandrHandleMonitors(XrandrData* data, Screen* screen)
{
    int numberOfMonitors;
    XRRMonitorInfo* monitorInfos = data->ffXRRGetMonitors(data->display, RootWindowOfScreen(screen), True, &numberOfMonitors);
    if(monitorInfos == NULL)
        return false;

    XRRScreenResources* screenResources = data->ffXRRGetScreenResourcesCurrent(data->display, RootWindowOfScreen(screen));

    double scaleFactor = 1;
    char* resourceManager = (char*) x11GetProperty(data->propData, data->display, screen->root, "RESOURCE_MANAGER");
    if (resourceManager)
    {
        FF_STRBUF_AUTO_DESTROY dpi = ffStrbufCreate();
        if (ffParsePropLines(resourceManager, "Xft.dpi:", &dpi))
            scaleFactor = ffStrbufToDouble(&dpi, 96) / 96;
        data->ffXFree(resourceManager);
    }
    uint8_t bitDepth = (uint8_t) (screen->root_depth / 3);

    bool foundAMonitor = false;

    for(int i = 0; i < numberOfMonitors; i++)
    {
        if(xrandrHandleMonitor(data, &monitorInfos[i], screenResources, bitDepth, scaleFactor))
            foundAMonitor = true;
    }

    data->ffXRRFreeMonitors(monitorInfos);
    data->ffXRRFreeScreenResources(screenResources);

    return foundAMonitor;
}

static void xrandrHandleScreen(XrandrData* data, Screen* screen)
{
    if(xrandrHandleMonitors(data, screen))
        return;

    //Fallback to screen
    ffdsAppendDisplay(
        data->result,
        (uint32_t) WidthOfScreen(screen),
        (uint32_t) HeightOfScreen(screen),
        0,
        (uint32_t) WidthOfScreen(screen),
        (uint32_t) HeightOfScreen(screen),
        0, 0, 0,
        0,
        NULL,
        FF_DISPLAY_TYPE_UNKNOWN,
        false,
        RootWindowOfScreen(screen),
        (uint32_t) WidthMMOfScreen(screen),
        (uint32_t) HeightMMOfScreen(screen),
        "xlib-randr-screen"
    );
}

const char* ffdsConnectXrandr(FFDisplayServerResult* result)
{
    FF_LIBRARY_LOAD(xrandr, "dlopen libXrandr failed", "libXrandr" FF_LIBRARY_EXTENSION, 3)

    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(xrandr, XOpenDisplay)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(xrandr, XCloseDisplay)

    XrandrData data;

    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(xrandr, data, XInternAtom);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(xrandr, data, XGetAtomName);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(xrandr, data, XFree);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(xrandr, data, XRRGetMonitors);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(xrandr, data, XRRGetScreenResourcesCurrent);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(xrandr, data, XRRGetOutputInfo);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(xrandr, data, XRRGetOutputProperty);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(xrandr, data, XRRGetCrtcInfo);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(xrandr, data, XRRFreeCrtcInfo);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(xrandr, data, XRRFreeOutputInfo);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(xrandr, data, XRRFreeScreenResources);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(xrandr, data, XRRFreeMonitors);

    X11PropertyData propertyData;
    bool propertyDataInitialized = x11InitPropertyData(xrandr, &propertyData);
    data.propData = &propertyData;

    data.display = ffXOpenDisplay(NULL);
    if(data.display == NULL)
        return "XOpenDisplay() failed";

    if(propertyDataInitialized && ScreenCount(data.display) > 0) {
        x11DetectWMFromEWMH(&propertyData, data.display, result);
        x11FetchServerVendor(&propertyData, data.display, result);
    }

    data.result = result;

    for(int i = 0; i < ScreenCount(data.display); i++)
        xrandrHandleScreen(&data, ScreenOfDisplay(data.display, i));

    ffXCloseDisplay(data.display);

    //If wayland hasn't set this, connection failed for it. So we are running only a X Server, not XWayland.
    if(result->wmProtocolName.length == 0)
        ffStrbufSetS(&result->wmProtocolName, FF_WM_PROTOCOL_X11);

    return NULL;
}

#else

const char* ffdsConnectXrandr(FFDisplayServerResult* result)
{
    //Do nothing here. There are more x11 implementations to come.
    FF_UNUSED(result);
    return "Fastfetch was compiled without libXrandr support";
}

#endif // FF_HAVE_XRANDR
