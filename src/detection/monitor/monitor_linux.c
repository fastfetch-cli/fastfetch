#include "monitor.h"

#include "common/io/io.h"
#include "common/library.h"
#include "util/edidHelper.h"
#include "util/stringUtils.h"

#ifdef FF_HAVE_XRANDR

#include <X11/extensions/Xrandr.h>
#include <X11/Xlib.h>

typedef struct XrandrData
{
    FF_LIBRARY_SYMBOL(XInternAtom)
    FF_LIBRARY_SYMBOL(XGetAtomName);
    FF_LIBRARY_SYMBOL(XFree);
    FF_LIBRARY_SYMBOL(XRRGetMonitors)
    FF_LIBRARY_SYMBOL(XRRGetOutputInfo)
    FF_LIBRARY_SYMBOL(XRRGetOutputProperty)
    FF_LIBRARY_SYMBOL(XRRFreeOutputInfo)
    FF_LIBRARY_SYMBOL(XRRFreeMonitors)

    //Init once
    Display* display;

    //Init per screen
    XRRScreenResources* screenResources;
} XrandrData;

static const char* xrandrHandleMonitors(XrandrData* data, Screen* screen, FFlist* results)
{
    int numberOfMonitors;
    XRRMonitorInfo* monitorInfos = data->ffXRRGetMonitors(data->display, RootWindowOfScreen(screen), True, &numberOfMonitors);
    if(monitorInfos == NULL)
        return "XRRGetMonitors() failed";

    for(int i = 0; i < numberOfMonitors; i++)
    {
        XRRMonitorInfo* monitorInfo = &monitorInfos[i];
        for(int i = 0; i < monitorInfo->noutput; i++)
        {
            RROutput output = monitorInfo->outputs[i];
            XRROutputInfo* outputInfo = data->ffXRRGetOutputInfo(data->display, data->screenResources, output);
            if(outputInfo == NULL)
                continue;

            FFMonitorResult* display = (FFMonitorResult*) ffListAdd(results);
            ffStrbufInit(&display->name);
            display->width = 0;
            display->height = 0;
            display->manufactureYear = 0;
            display->manufactureWeek = 0;
            display->serial = 0;
            display->hdrCompatible = false;
            display->refreshRate = 0;

            bool edidOk = false;
            Atom atomEdid = data->ffXInternAtom(data->display, "EDID", true);
            if (atomEdid != None)
            {
                int actual_format = 0;
                unsigned long nitems = 0, bytes_after = 0;
                Atom actual_type = None;
                uint8_t* edidData = NULL;
                if (data->ffXRRGetOutputProperty(data->display, output, atomEdid, 0, 100, false, false, AnyPropertyType, &actual_type, &actual_format, &nitems, &bytes_after, &edidData) == Success)
                {
                    if (nitems > 0 && nitems % 128 == 0)
                    {
                        ffEdidGetName(edidData, &display->name);
                        ffEdidGetPhysicalResolution(edidData, &display->width, &display->height);
                        ffEdidGetPhysicalSize(edidData, &display->physicalWidth, &display->physicalHeight);
                        ffEdidGetSerialAndManufactureDate(edidData, &display->serial, &display->manufactureYear, &display->manufactureWeek);
                        display->hdrCompatible = ffEdidGetHdrCompatible(edidData, (uint32_t) nitems);
                        edidOk = true;
                    }
                }
                if (edidData)
                    data->ffXFree(edidData);
            }

            if (!edidOk)
            {
                ffStrbufSetS(&display->name, data->ffXGetAtomName(data->display, monitorInfo->name));
                display->physicalWidth = (uint32_t) monitorInfo->mwidth;
                display->physicalHeight = (uint32_t) monitorInfo->mheight;
                display->width = (uint32_t) monitorInfo->width;
                display->height = (uint32_t) monitorInfo->height;
            }

            for(int i = 0; i < data->screenResources->nmode; i++)
            {
                bool found = false;
                for (int j = 0; j < outputInfo->nmode; ++j)
                {
                    if (data->screenResources->modes[i].id == outputInfo->modes[j])
                    {
                        found = true;
                        break;
                    }
                }

                if(found)
                {
                    XRRModeInfo* modeInfo = &data->screenResources->modes[i];
                    double refreshRate = (double) modeInfo->dotClock / (double) (modeInfo->hTotal * modeInfo->vTotal);
                    if (edidOk)
                    {
                        if (display->width != modeInfo->width || display->height != modeInfo->height)
                            continue;
                    }
                    else
                    {
                        if (display->width < modeInfo->width || display->height < modeInfo->height)
                        {
                            display->width = (uint32_t) modeInfo->width;
                            display->height = (uint32_t) modeInfo->height;
                            display->refreshRate = refreshRate;
                            continue;
                        }
                        else if (display->width != modeInfo->width || display->height != modeInfo->height)
                            continue;
                    }
                    if (display->refreshRate < refreshRate)
                        display->refreshRate = refreshRate;
                }
            }
            data->ffXRRFreeOutputInfo(outputInfo);
        }
    }

    data->ffXRRFreeMonitors(monitorInfos);
    return NULL;
}

static const char* detectByXrandr(FFlist* results)
{
    FF_LIBRARY_LOAD(xrandr, "dlopen libXrandr" FF_LIBRARY_EXTENSION " failed", "libXrandr" FF_LIBRARY_EXTENSION, 3)

    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(xrandr, XOpenDisplay)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(xrandr, XCloseDisplay)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(xrandr, XRRGetScreenResourcesCurrent);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(xrandr, XRRFreeScreenResources);

    XrandrData data;

    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(xrandr, data, XInternAtom);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(xrandr, data, XGetAtomName);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(xrandr, data, XFree);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(xrandr, data, XRRGetMonitors);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(xrandr, data, XRRGetOutputInfo);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(xrandr, data, XRRGetOutputProperty);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(xrandr, data, XRRFreeOutputInfo);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(xrandr, data, XRRFreeMonitors);

    data.display = ffXOpenDisplay(NULL);
    if(data.display == NULL)
        return "XOpenDisplay() failed";

    for(int i = 0; i < ScreenCount(data.display); i++)
    {
        Screen* screen = ScreenOfDisplay(data.display, i);
        data.screenResources = ffXRRGetScreenResourcesCurrent(data.display, RootWindowOfScreen(screen));
        xrandrHandleMonitors(&data, screen, results);
        ffXRRFreeScreenResources(data.screenResources);
    }

    ffXCloseDisplay(data.display);

    return NULL;
}
#endif // FF_HAVE_XRANDR

#ifdef __linux__
FF_MAYBE_UNUSED static const char* detectByDrm(FFlist* results)
{
    const char* drmDirPath = "/sys/class/drm/";

    DIR* dirp = opendir(drmDirPath);
    if(dirp == NULL)
        return "opendir(drmDirPath) == NULL";

    FF_STRBUF_AUTO_DESTROY drmDir = ffStrbufCreateA(64);
    ffStrbufAppendS(&drmDir, drmDirPath);

    uint32_t drmDirLength = drmDir.length;

    struct dirent* entry;
    while((entry = readdir(dirp)) != NULL)
    {
        if(ffStrEquals(entry->d_name, ".") || ffStrEquals(entry->d_name, ".."))
            continue;

        ffStrbufAppendS(&drmDir, entry->d_name);
        uint32_t drmDirWithDnameLength = drmDir.length;

        ffStrbufAppendS(&drmDir, "/status");
        char status = 'd'; // disconnected
        ffReadFileData(drmDir.chars, sizeof(status), &status);
        if (status != 'c') // connected
        {
            ffStrbufSubstrBefore(&drmDir, drmDirLength);
            continue;
        }

        ffStrbufSubstrBefore(&drmDir, drmDirWithDnameLength);
        ffStrbufAppendS(&drmDir, "/edid");

        uint8_t edidData[512];
        ssize_t edidLength = ffReadFileData(drmDir.chars, sizeof(edidData), edidData);
        if(edidLength <= 0 || edidLength % 128 != 0)
        {
            ffStrbufSubstrBefore(&drmDir, drmDirLength);
            continue;
        }

        uint32_t width, height;
        ffEdidGetPhysicalResolution(edidData, &width, &height);
        if (width != 0 && height != 0)
        {
            FFMonitorResult* display = (FFMonitorResult*) ffListAdd(results);
            display->width = width;
            display->height = height;
            ffStrbufInit(&display->name);
            ffEdidGetName(edidData, &display->name);
            ffEdidGetPhysicalSize(edidData, &display->physicalWidth, &display->physicalHeight);
            ffEdidGetSerialAndManufactureDate(edidData, &display->serial, &display->manufactureYear, &display->manufactureWeek);
            display->hdrCompatible = ffEdidGetHdrCompatible(edidData, (uint32_t) edidLength);
            display->refreshRate = 0;
        }

        ffStrbufSubstrBefore(&drmDir, drmDirLength);
    }

    closedir(dirp);
    return NULL;
}
#endif // __linux__

const char* ffDetectMonitor(FFlist* results)
{
    const char* error = "Fastfetch was compiled without xrandr support";

    #ifdef FF_HAVE_XRANDR
        error = detectByXrandr(results);
        if (!error) return NULL;
    #endif

    #if defined(__linux__)
        error = detectByDrm(results);
    #endif

    return error;
}
