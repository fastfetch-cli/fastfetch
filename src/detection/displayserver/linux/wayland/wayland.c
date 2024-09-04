#include "../displayserver_linux.h"
#include "common/io/io.h"
#include "util/stringUtils.h"

#include <stdlib.h>
#include <string.h>

#ifdef FF_HAVE_WAYLAND

#include <sys/socket.h>

#include "common/properties.h"

#include "wayland.h"
#include "wlr-output-management-unstable-v1-client-protocol.h"
#include "kde-output-device-v2-client-protocol.h"
#include "kde-output-order-v1-client-protocol.h"
#include "xdg-output-unstable-v1-client-protocol.h"

#ifdef __linux__
static bool waylandDetectWM(int fd, FFDisplayServerResult* result)
{
    struct ucred ucred;
    socklen_t len = sizeof(struct ucred);
    if (getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &ucred, &len) == -1 || ucred.pid <= 0)
        return false;

    FF_STRBUF_AUTO_DESTROY procPath = ffStrbufCreate();
    ffStrbufAppendF(&procPath, "/proc/%d/cmdline", ucred.pid); //We check the cmdline for the process name, because it is not trimmed.
    if (!ffReadFileBuffer(procPath.chars, &result->wmProcessName))
        return false;

    // #1135: wl-restart is a special case
    const char* filename = strrchr(result->wmProcessName.chars, '/');
    if (filename)
        filename++;
    else
        filename = result->wmProcessName.chars;

    if (ffStrEquals(filename, "wl-restart"))
        ffStrbufSubstrAfterFirstC(&result->wmProcessName, '\0');

    ffStrbufSubstrBeforeFirstC(&result->wmProcessName, '\0'); //Trim the arguments
    ffStrbufSubstrAfterLastC(&result->wmProcessName, '/'); //Trim the path

    return true;
}
#else
static bool waylandDetectWM(int fd, FFDisplayServerResult* result)
{
    FF_UNUSED(fd, result);
    return false;
}
#endif

static void waylandGlobalAddListener(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
{
    WaylandData* wldata = data;

    if((wldata->protocolType == FF_WAYLAND_PROTOCOL_TYPE_NONE || wldata->protocolType == FF_WAYLAND_PROTOCOL_TYPE_GLOBAL) && ffStrEquals(interface, wldata->ffwl_output_interface->name))
    {
        wldata->protocolType = FF_WAYLAND_PROTOCOL_TYPE_GLOBAL;
        ffWaylandHandleGlobalOutput(wldata, registry, name, version);
    }
    else if((wldata->protocolType == FF_WAYLAND_PROTOCOL_TYPE_NONE || wldata->protocolType == FF_WAYLAND_PROTOCOL_TYPE_ZWLR) && ffStrEquals(interface, zwlr_output_manager_v1_interface.name))
    {
        wldata->protocolType = FF_WAYLAND_PROTOCOL_TYPE_ZWLR;
        ffWaylandHandleZwlrOutput(wldata, registry, name, version);
    }
    else if((wldata->protocolType == FF_WAYLAND_PROTOCOL_TYPE_NONE || wldata->protocolType == FF_WAYLAND_PROTOCOL_TYPE_KDE) && ffStrEquals(interface, kde_output_device_v2_interface.name))
    {
        wldata->protocolType = FF_WAYLAND_PROTOCOL_TYPE_KDE;
        ffWaylandHandleKdeOutput(wldata, registry, name, version);
    }
    else if(ffStrEquals(interface, kde_output_order_v1_interface.name))
    {
        ffWaylandHandleKdeOutputOrder(wldata, registry, name, version);
    }
    else if((wldata->protocolType == FF_WAYLAND_PROTOCOL_TYPE_GLOBAL || wldata->protocolType == FF_WAYLAND_PROTOCOL_TYPE_NONE) && ffStrEquals(interface, zxdg_output_manager_v1_interface.name))
    {
        ffWaylandHandleZxdgOutput(wldata, registry, name, version);
    }
}

void ffWaylandOutputNameListener(void* data, FF_MAYBE_UNUSED void* output, const char *name)
{
    WaylandDisplay* display = data;
    if (display->id) return;

    display->type = ffdsGetDisplayType(name);
    if (!display->edidName.length)
        ffdsMatchDrmConnector(name, &display->edidName);
    display->id = ffWaylandGenerateIdFromName(name);
    ffStrbufAppendS(&display->name, name);
}

void ffWaylandOutputDescriptionListener(void* data, FF_MAYBE_UNUSED void* output, const char* description)
{
    WaylandDisplay* display = data;
    if (display->description.length) return;

    while (*description == ' ') ++description;
    if (!ffStrEquals(description, "Unknown Display") && !ffStrContains(description, "(null)"))
        ffStrbufAppendS(&display->description, description);
}

uint32_t ffWaylandHandleRotation(WaylandDisplay* display)
{
    uint32_t rotation;
    switch(display->transform)
    {
        case WL_OUTPUT_TRANSFORM_FLIPPED_90:
        case WL_OUTPUT_TRANSFORM_90:
            rotation = 90;
            break;
        case WL_OUTPUT_TRANSFORM_FLIPPED_180:
        case WL_OUTPUT_TRANSFORM_180:
            rotation = 180;
            break;
        case WL_OUTPUT_TRANSFORM_FLIPPED_270:
        case WL_OUTPUT_TRANSFORM_270:
            rotation = 270;
            break;
        default:
            rotation = 0;
            break;
    }

    switch(rotation)
    {
        case 90:
        case 270: {
            int32_t temp = display->width;
            display->width = display->height;
            display->height = temp;

            temp = display->physicalWidth;
            display->physicalWidth = display->physicalHeight;
            display->physicalHeight = temp;
            break;
        }
        default:
            break;
    }
    return rotation;
}

const char* ffdsConnectWayland(FFDisplayServerResult* result)
{
    if (getenv("XDG_RUNTIME_DIR") == NULL)
        return "Wayland requires $XDG_RUNTIME_DIR being set";

    FF_LIBRARY_LOAD(wayland, false, "libwayland-client" FF_LIBRARY_EXTENSION, 1)

    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(wayland, wl_display_connect)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(wayland, wl_display_get_fd)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(wayland, wl_proxy_marshal_constructor)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(wayland, wl_display_disconnect)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(wayland, wl_registry_interface)

    WaylandData data = {};

    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(wayland, data, wl_proxy_marshal_constructor_versioned)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(wayland, data, wl_proxy_add_listener)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(wayland, data, wl_proxy_destroy)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(wayland, data, wl_display_roundtrip)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(wayland, data, wl_output_interface)

    data.display = ffwl_display_connect(NULL);
    if(data.display == NULL)
        return "wl_display_connect returned NULL";

    waylandDetectWM(ffwl_display_get_fd(data.display), result);

    struct wl_proxy* registry = ffwl_proxy_marshal_constructor((struct wl_proxy*) data.display, WL_DISPLAY_GET_REGISTRY, ffwl_registry_interface, NULL);
    if(registry == NULL)
    {
        ffwl_display_disconnect(data.display);
        return "wl_display_get_registry returned NULL";
    }

    data.result = result;

    struct wl_registry_listener registry_listener = {
        .global = waylandGlobalAddListener,
        .global_remove = (void*) stubListener
    };

    data.ffwl_proxy_add_listener(registry, (void(**)(void)) &registry_listener, &data);
    data.ffwl_display_roundtrip(data.display);

    if (data.zxdgOutputManager)
        data.ffwl_proxy_destroy(data.zxdgOutputManager);

    data.ffwl_proxy_destroy(registry);
    ffwl_display_disconnect(data.display);

    if(data.primaryDisplayId == 0 && result->wmProcessName.length > 0)
    {
        const char* fileName = ffStrbufEqualS(&result->wmProcessName, "gnome-shell")
            ? "monitors.xml"
            : ffStrbufEqualS(&result->wmProcessName, "cinnamon")
                ? "cinnamon-monitors.xml"
                : NULL;
        if (fileName)
        {
            FF_STRBUF_AUTO_DESTROY monitorsXml = ffStrbufCreate();
            FF_LIST_FOR_EACH(FFstrbuf, basePath, instance.state.platform.configDirs)
            {
                char path[1024];
                snprintf(path, sizeof(path) - 1, "%s%s", basePath->chars, fileName);
                if (ffReadFileBuffer(path, &monitorsXml))
                    break;
            }
            if (monitorsXml.length)
            {
                // <monitors version="2">
                // <configuration>
                //     <logicalmonitor>
                //     <x>0</x>
                //     <y>0</y>
                //     <scale>1.7489879131317139</scale>
                //     <primary>yes</primary>
                //     <monitor>
                //         <monitorspec>
                //         <connector>Virtual-1</connector>
                //         <vendor>unknown</vendor>
                //         <product>unknown</product>
                //         <serial>unknown</serial>
                //         </monitorspec>
                //         <mode>
                //         <width>3456</width>
                //         <height>2160</height>
                //         <rate>60.000068664550781</rate>
                //         </mode>
                //     </monitor>
                //     </logicalmonitor>
                // </configuration>
                // </monitors>
                uint32_t start = ffStrbufFirstIndexS(&monitorsXml, "<primary>yes</primary>");
                if (start < monitorsXml.length)
                {
                    start = ffStrbufNextIndexS(&monitorsXml, start, "<connector>");
                    if (start < monitorsXml.length)
                    {
                        uint32_t end = ffStrbufNextIndexS(&monitorsXml, start, "</connector>");
                        if (end < monitorsXml.length)
                        {
                            ffStrbufSubstrBefore(&monitorsXml, end);
                            const char* name = monitorsXml.chars + start + strlen("<connector>");
                            data.primaryDisplayId = ffWaylandGenerateIdFromName(name);
                        }
                    }
                }
            }
        }
    }

    if(data.primaryDisplayId)
    {
        FF_LIST_FOR_EACH(FFDisplayResult, d, data.result->displays)
        {
            if(d->id == data.primaryDisplayId)
            {
                d->primary = true;
                break;
            }
        }
    }

    //We successfully connected to wayland and detected the display.
    //So we can set set the session type to wayland.
    //This is used as an indicator that we are running wayland by the x11 backends.
    ffStrbufSetStatic(&result->wmProtocolName, FF_WM_PROTOCOL_WAYLAND);
    return NULL;
}

#else

const char* ffdsConnectWayland(FF_MAYBE_UNUSED FFDisplayServerResult* result)
{
    return "Fastfetch was compiled without Wayland support";
}

#endif
