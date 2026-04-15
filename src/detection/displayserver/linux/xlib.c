#include "displayserver_linux.h"

#ifdef FF_HAVE_XRANDR

    #include "common/library.h"
    #include "common/properties.h"
    #include "common/edidHelper.h"
    #include "common/stringUtils.h"

    #include <X11/extensions/Xrandr.h>
    #include <X11/Xlib.h>

typedef struct XrandrData {
    FF_LIBRARY_SYMBOL(XInternAtom)
    FF_LIBRARY_SYMBOL(XGetAtomName)
    FF_LIBRARY_SYMBOL(XGetWindowProperty)
    FF_LIBRARY_SYMBOL(XServerVendor)
    FF_LIBRARY_SYMBOL(XFree)
    FF_LIBRARY_SYMBOL(XRRGetMonitors)
    FF_LIBRARY_SYMBOL(XRRGetScreenResourcesCurrent)
    FF_LIBRARY_SYMBOL(XRRGetOutputInfo)
    FF_LIBRARY_SYMBOL(XRRGetOutputProperty)
    FF_LIBRARY_SYMBOL(XRRGetCrtcInfo)
    FF_LIBRARY_SYMBOL(XRRFreeCrtcInfo)
    FF_LIBRARY_SYMBOL(XRRFreeOutputInfo)
    FF_LIBRARY_SYMBOL(XRRFreeScreenResources)
    FF_LIBRARY_SYMBOL(XRRFreeMonitors)

    // Init once
    Display* display;
    FFDisplayServerResult* result;
} XrandrData;

static unsigned char* x11GetProperty(XrandrData* data, Display* display, Window window, const char* request) {
    Atom requestAtom = data->ffXInternAtom(display, request, False);
    if (requestAtom == None) {
        return NULL;
    }

    Atom actualType;
    unsigned long unused;
    unsigned char* result = NULL;

    if (data->ffXGetWindowProperty(display, window, requestAtom, 0, 64, False, AnyPropertyType, &actualType, (int*) &unused, &unused, &unused, &result) != Success) {
        return NULL;
    }

    return result;
}

static uint8_t* xrandrGetProperty(XrandrData* data, RROutput output, const char* name, uint32_t* bufSize) {
    unsigned long size = 0;
    uint8_t* result = NULL;
    Atom atomEdid = data->ffXInternAtom(data->display, name, true);
    if (atomEdid != None) {
        int actual_format = 0;
        unsigned long bytes_after = 0;
        Atom actual_type = None;
        if (data->ffXRRGetOutputProperty(data->display, output, atomEdid, 0, 100, false, false, AnyPropertyType, &actual_type, &actual_format, &size, &bytes_after, &result) == Success) {
            if (size == 0) {
                data->ffXFree(result);
            } else {
                if (bufSize) {
                    *bufSize = (uint32_t) size;
                }
                return result;
            }
        }
    }

    return NULL;
}

static void x11DetectWMFromEWMH(XrandrData* data, FFDisplayServerResult* result) {
    if (result->wmProcessName.length > 0 || ffStrbufEqualS(&result->wmProtocolName, FF_WM_PROTOCOL_WAYLAND)) {
        return;
    }

    Window* wmWindow = (Window*) x11GetProperty(data, data->display, DefaultRootWindow(data->display), "_NET_SUPPORTING_WM_CHECK");
    if (wmWindow == NULL) {
        return;
    }

    char* wmName = (char*) x11GetProperty(data, data->display, *wmWindow, "WM_NAME");
    if (!ffStrSet(wmName)) {
        wmName = (char*) x11GetProperty(data, data->display, *wmWindow, "_NET_WM_NAME");
    }

    if (ffStrSet(wmName)) {
        ffStrbufSetS(&result->wmProcessName, wmName);
    }

    data->ffXFree(wmName);
    data->ffXFree(wmWindow);
}

static void x11FetchServerVendor(XrandrData* data, FFDisplayServerResult* result) {
    const char* serverVendor = data->ffXServerVendor(data->display);
    if (serverVendor && !ffStrEquals(serverVendor, "The X.Org Foundation")) {
        ffStrbufSetS(&result->wmProtocolName, serverVendor);
    }
}

static bool xrandrHandleCrtc(XrandrData* data, XRROutputInfo* output, FFstrbuf* name, bool primary, FFDisplayType displayType, uint8_t* edidData, uint32_t edidLength, XRRScreenResources* screenResources, uint8_t bitDepth, uint32_t dpi, bool randrEmulation) {
    // We do the check here, because we want the best fallback display if this call failed
    if (screenResources == NULL) {
        return false;
    }

    XRRCrtcInfo* crtcInfo = data->ffXRRGetCrtcInfo(data->display, screenResources, output->crtc);
    if (crtcInfo == NULL) {
        return false;
    }

    uint32_t rotation;
    switch (crtcInfo->rotation) {
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
    for (int i = 0; i < screenResources->nmode; i++) {
        if (screenResources->modes[i].id == crtcInfo->mode) {
            currentMode = &screenResources->modes[i];
            break;
        }
    }

    XRRModeInfo* preferredMode = output->npreferred > 0 ? &screenResources->modes[0] : NULL;

    FFDisplayResult* item = ffdsAppendDisplay(
        data->result,
        (uint32_t) (currentMode ? currentMode->width : crtcInfo->width),
        (uint32_t) (currentMode ? currentMode->height : crtcInfo->height),
        currentMode ? (double) currentMode->dotClock / (double) ((uint32_t) currentMode->hTotal * currentMode->vTotal) : 0,
        dpi,
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
        randrEmulation
            ? (currentMode ? "xlib-randr-emu-mode" : "xlib-randr-emu-crtc")
            : (currentMode ? "xlib-randr-mode" : "xlib-randr-crtc"));

    if (item) {
        if (edidLength) {
            item->hdrStatus = ffEdidGetHdrCompatible(edidData, edidLength) ? FF_DISPLAY_HDR_STATUS_SUPPORTED : FF_DISPLAY_HDR_STATUS_UNSUPPORTED;
            ffEdidGetSerialAndManufactureDate(edidData, &item->serial, &item->manufactureYear, &item->manufactureWeek);
        }
        item->bitDepth = bitDepth;
        if ((rotation == 90 || rotation == 180) && !randrEmulation) {
            // In XWayland mode, width / height has been swapped out of box
            uint32_t tmp = item->width;
            item->width = item->height;
            item->height = tmp;
        }
    }

    data->ffXRRFreeCrtcInfo(crtcInfo);
    return !!item;
}

static bool xrandrHandleOutput(XrandrData* data, RROutput output, FFstrbuf* name, bool primary, FFDisplayType displayType, XRRScreenResources* screenResources, uint8_t bitDepth, uint32_t dpi) {
    XRROutputInfo* outputInfo = data->ffXRRGetOutputInfo(data->display, screenResources, output);
    if (outputInfo == NULL) {
        return false;
    }

    uint32_t edidLength = 0;
    uint8_t* edidData = xrandrGetProperty(data, output, RR_PROPERTY_RANDR_EDID, &edidLength);

    if (edidLength >= 128) {
        ffStrbufClear(name);
        ffEdidGetName(edidData, name);
    } else {
        edidLength = 0;
    }

    uint8_t* randrEmulation = xrandrGetProperty(data, output, "RANDR Emulation", NULL);

    bool res = xrandrHandleCrtc(data, outputInfo, name, primary, displayType, edidData, edidLength, screenResources, bitDepth, dpi, randrEmulation ? !!randrEmulation[0] : false);

    if (edidData) {
        data->ffXFree(edidData);
    }
    if (randrEmulation) {
        data->ffXFree(randrEmulation);
    }
    data->ffXRRFreeOutputInfo(outputInfo);

    return res;
}

static bool xrandrHandleMonitor(XrandrData* data, XRRMonitorInfo* monitorInfo, XRRScreenResources* screenResources, uint8_t bitDepth, uint32_t dpi) {
    bool foundOutput = false;
    char* xname = data->ffXGetAtomName(data->display, monitorInfo->name);
    FF_STRBUF_AUTO_DESTROY name = ffStrbufCreateS(xname);
    data->ffXFree(xname);
    FFDisplayType displayType = ffdsGetDisplayType(name.chars);
    for (int i = 0; i < monitorInfo->noutput; i++) {
        if (xrandrHandleOutput(data, monitorInfo->outputs[i], &name, monitorInfo->primary, displayType, screenResources, bitDepth, dpi)) {
            foundOutput = true;
        }
    }

    if (foundOutput) {
        return true;
    }

    FFDisplayResult* display = ffdsAppendDisplay(
        data->result,
        (uint32_t) monitorInfo->width,
        (uint32_t) monitorInfo->height,
        0,
        dpi,
        0,
        0,
        0,
        0,
        &name,
        displayType,
        !!monitorInfo->primary,
        0,
        (uint32_t) monitorInfo->mwidth,
        (uint32_t) monitorInfo->mheight,
        "xlib-randr-monitor");
    if (display) {
        display->bitDepth = bitDepth;
    }
    return !!display;
}

static bool xrandrHandleMonitors(XrandrData* data, Screen* screen) {
    int numberOfMonitors;
    XRRMonitorInfo* monitorInfos = data->ffXRRGetMonitors(data->display, RootWindowOfScreen(screen), True, &numberOfMonitors);
    if (monitorInfos == NULL) {
        return false;
    }

    XRRScreenResources* screenResources = data->ffXRRGetScreenResourcesCurrent(data->display, RootWindowOfScreen(screen));

    uint32_t dpi = 1;
    char* resourceManager = (char*) x11GetProperty(data, data->display, screen->root, "RESOURCE_MANAGER");
    if (resourceManager) {
        FF_STRBUF_AUTO_DESTROY dpiStr = ffStrbufCreate();
        if (ffParsePropLines(resourceManager, "Xft.dpi:", &dpiStr)) {
            dpi = (uint32_t) ffStrbufToUInt(&dpiStr, 96);
        }
        data->ffXFree(resourceManager);
    }
    uint8_t bitDepth = (uint8_t) (screen->root_depth / 3);

    bool foundAMonitor = false;

    for (int i = 0; i < numberOfMonitors; i++) {
        if (xrandrHandleMonitor(data, &monitorInfos[i], screenResources, bitDepth, dpi)) {
            foundAMonitor = true;
        }
    }

    data->ffXRRFreeMonitors(monitorInfos);
    data->ffXRRFreeScreenResources(screenResources);

    return foundAMonitor;
}

static void xrandrHandleScreen(XrandrData* data, Screen* screen) {
    if (xrandrHandleMonitors(data, screen)) {
        return;
    }

    // Fallback to screen
    ffdsAppendDisplay(
        data->result,
        (uint32_t) WidthOfScreen(screen),
        (uint32_t) HeightOfScreen(screen),
        0,
        0,
        0,
        0,
        0,
        0,
        NULL,
        FF_DISPLAY_TYPE_UNKNOWN,
        false,
        RootWindowOfScreen(screen),
        (uint32_t) WidthMMOfScreen(screen),
        (uint32_t) HeightMMOfScreen(screen),
        "xlib-randr-screen");
}

const char* ffdsConnectXrandr(FFDisplayServerResult* result) {
    FF_LIBRARY_LOAD_MESSAGE(xrandr, "libXrandr" FF_LIBRARY_EXTENSION, 3)

    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(xrandr, XOpenDisplay)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(xrandr, XCloseDisplay)

    XrandrData data;

    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(xrandr, data, XInternAtom);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(xrandr, data, XGetAtomName);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(xrandr, data, XGetWindowProperty);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(xrandr, data, XServerVendor);
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

    data.display = ffXOpenDisplay(NULL);
    if (data.display == NULL) {
        return "XOpenDisplay() failed";
    }

    if (ScreenCount(data.display) > 0) {
        x11DetectWMFromEWMH(&data, result);
        x11FetchServerVendor(&data, result);
    }

    data.result = result;

    for (int i = 0; i < ScreenCount(data.display); i++) {
        xrandrHandleScreen(&data, ScreenOfDisplay(data.display, i));
    }

    ffXCloseDisplay(data.display);

    // If wayland hasn't set this, connection failed for it. So we are running only a X Server, not XWayland.
    if (result->wmProtocolName.length == 0) {
        ffStrbufSetS(&result->wmProtocolName, FF_WM_PROTOCOL_X11);
    }

    return NULL;
}

#else

const char* ffdsConnectXrandr(FFDisplayServerResult* result) {
    // Do nothing here. There are more x11 implementations to come.
    FF_UNUSED(result);
    return "Fastfetch was compiled without libXrandr support";
}

#endif // FF_HAVE_XRANDR
