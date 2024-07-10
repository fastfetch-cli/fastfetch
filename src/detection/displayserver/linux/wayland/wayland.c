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
    if (getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &ucred, &len) == -1)
        return false;

    FF_STRBUF_AUTO_DESTROY procPath = ffStrbufCreate();
    ffStrbufAppendF(&procPath, "/proc/%d/cmdline", ucred.pid); //We check the cmdline for the process name, because it is not trimmed.
    ffReadFileBuffer(procPath.chars, &result->wmProcessName);
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

bool detectWayland(FFDisplayServerResult* result)
{
    FF_LIBRARY_LOAD(wayland, &instance.config.library.libWayland, false, "libwayland-client" FF_LIBRARY_EXTENSION, 1)

    FF_LIBRARY_LOAD_SYMBOL(wayland, wl_display_connect, false)
    FF_LIBRARY_LOAD_SYMBOL(wayland, wl_display_get_fd, false)
    FF_LIBRARY_LOAD_SYMBOL(wayland, wl_proxy_marshal_constructor, false)
    FF_LIBRARY_LOAD_SYMBOL(wayland, wl_display_disconnect, false)
    FF_LIBRARY_LOAD_SYMBOL(wayland, wl_registry_interface, false)

    WaylandData data = {};

    FF_LIBRARY_LOAD_SYMBOL_VAR(wayland, data, wl_proxy_marshal_constructor_versioned, false)
    FF_LIBRARY_LOAD_SYMBOL_VAR(wayland, data, wl_proxy_add_listener, false)
    FF_LIBRARY_LOAD_SYMBOL_VAR(wayland, data, wl_proxy_destroy, false)
    FF_LIBRARY_LOAD_SYMBOL_VAR(wayland, data, wl_display_roundtrip, false)
    FF_LIBRARY_LOAD_SYMBOL_VAR(wayland, data, wl_output_interface, false)

    data.display = ffwl_display_connect(NULL);
    if(data.display == NULL)
        return false;

    waylandDetectWM(ffwl_display_get_fd(data.display), result);

    struct wl_proxy* registry = ffwl_proxy_marshal_constructor((struct wl_proxy*) data.display, WL_DISPLAY_GET_REGISTRY, ffwl_registry_interface, NULL);
    if(registry == NULL)
    {
        ffwl_display_disconnect(data.display);
        return false;
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
    ffStrbufSetS(&result->wmProtocolName, FF_WM_PROTOCOL_WAYLAND);
    return true;
}

void ffWaylandOutputNameListener(void* data, FF_MAYBE_UNUSED void* output, const char *name)
{
    WaylandDisplay* display = data;
    if (display->id) return;

    if(ffStrStartsWith(name, "eDP-"))
        display->type = FF_DISPLAY_TYPE_BUILTIN;
    else if(ffStrStartsWith(name, "HDMI-"))
        display->type = FF_DISPLAY_TYPE_EXTERNAL;
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
#endif

void ffdsConnectWayland(FFDisplayServerResult* result)
{
    //Wayland requires this to be set
    if(getenv("XDG_RUNTIME_DIR") == NULL)
        return;

    #ifdef FF_HAVE_WAYLAND
        if(detectWayland(result))
            return;
    #endif

    const char* xdgSessionType = getenv("XDG_SESSION_TYPE");

    //If XDG_SESSION_TYPE is set, and doesn't contain "wayland", we are probably not running in a wayland session.
    if(xdgSessionType != NULL && !ffStrEqualsIgnCase(xdgSessionType, "wayland"))
        return;

    //If XDG_SESSION_TYPE is not set, check if WAYLAND_DISPLAY or WAYLAND_SOCKET is set.
    //If not, there is no indicator for a wayland session
    if(xdgSessionType == NULL && getenv("WAYLAND_DISPLAY") == NULL && getenv("WAYLAND_SOCKET") == NULL)
        return;

    //We are probably running a wayland compositor at this point,
    //but fastfetch was compiled without the required library, or loading the library failed.
    ffStrbufSetS(&result->wmProtocolName, FF_WM_PROTOCOL_WAYLAND);
}
