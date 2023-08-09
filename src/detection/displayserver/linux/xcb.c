#include "displayserver_linux.h"
#include "util/mallocHelper.h"

#ifdef FF_HAVE_XCB
#include "common/library.h"
#include <stdlib.h>
#include <string.h>
#include <xcb/xcb.h>

typedef struct XcbPropertyData
{
    FF_LIBRARY_SYMBOL(xcb_intern_atom)
    FF_LIBRARY_SYMBOL(xcb_intern_atom_reply)
    FF_LIBRARY_SYMBOL(xcb_get_property)
    FF_LIBRARY_SYMBOL(xcb_get_property_reply)
    FF_LIBRARY_SYMBOL(xcb_get_property_value)
    FF_LIBRARY_SYMBOL(xcb_get_property_value_length)
    FF_LIBRARY_SYMBOL(xcb_get_atom_name)
    FF_LIBRARY_SYMBOL(xcb_get_atom_name_name)
    FF_LIBRARY_SYMBOL(xcb_get_atom_name_name_length)
    FF_LIBRARY_SYMBOL(xcb_get_atom_name_reply)
} XcbPropertyData;

static bool xcbInitPropertyData(void* libraryHandle, XcbPropertyData* propertyData)
{
    FF_LIBRARY_LOAD_SYMBOL_PTR(libraryHandle, propertyData, xcb_intern_atom, false)
    FF_LIBRARY_LOAD_SYMBOL_PTR(libraryHandle, propertyData, xcb_intern_atom_reply, false)
    FF_LIBRARY_LOAD_SYMBOL_PTR(libraryHandle, propertyData, xcb_get_property, false)
    FF_LIBRARY_LOAD_SYMBOL_PTR(libraryHandle, propertyData, xcb_get_property_reply, false)
    FF_LIBRARY_LOAD_SYMBOL_PTR(libraryHandle, propertyData, xcb_get_property_value, false)
    FF_LIBRARY_LOAD_SYMBOL_PTR(libraryHandle, propertyData, xcb_get_property_value_length, false)
    FF_LIBRARY_LOAD_SYMBOL_PTR(libraryHandle, propertyData, xcb_get_atom_name, false)
    FF_LIBRARY_LOAD_SYMBOL_PTR(libraryHandle, propertyData, xcb_get_atom_name_name, false)
    FF_LIBRARY_LOAD_SYMBOL_PTR(libraryHandle, propertyData, xcb_get_atom_name_name_length, false)
    FF_LIBRARY_LOAD_SYMBOL_PTR(libraryHandle, propertyData, xcb_get_atom_name_reply, false)

    return true;
}

static void* xcbGetProperty(XcbPropertyData* data, xcb_connection_t* connection, xcb_window_t window, const char* request)
{
    xcb_intern_atom_cookie_t requestAtomCookie = data->ffxcb_intern_atom(connection, true, (uint16_t) strlen(request), request);
    xcb_intern_atom_reply_t* requestAtomReply = data->ffxcb_intern_atom_reply(connection, requestAtomCookie, NULL);
    if(requestAtomReply == NULL)
        return NULL;

    xcb_get_property_cookie_t propertyCookie = data->ffxcb_get_property(connection, false, window, requestAtomReply->atom, XCB_ATOM_ANY, 0, 64);

    free(requestAtomReply);

    xcb_get_property_reply_t* propertyReply = data->ffxcb_get_property_reply(connection, propertyCookie, NULL);
    if(propertyReply == NULL)
        return NULL;

    int length = data->ffxcb_get_property_value_length(propertyReply);
    if(length <= 0)
    {
        free(propertyReply);
        return NULL;
    }

    //Why are xcb property strings not null terminated???
    void* replyValue = malloc((size_t) (length + 1));
    memcpy(replyValue, data->ffxcb_get_property_value(propertyReply), (size_t) length);
    ((char*) replyValue)[length] = '\0';

    free(propertyReply);

    return replyValue;
}

static void xcbDetectWMfromEWMH(XcbPropertyData* data, xcb_connection_t* connection, xcb_window_t rootWindow, FFDisplayServerResult* result)
{
    if(result->wmProcessName.length > 0 || ffStrbufCompS(&result->wmProtocolName, FF_WM_PROTOCOL_WAYLAND) == 0)
        return;

    xcb_window_t* wmWindow = (xcb_window_t*) xcbGetProperty(data, connection, rootWindow, "_NET_SUPPORTING_WM_CHECK");
    if(wmWindow == NULL)
        return;

    char* wmName = (char*) xcbGetProperty(data, connection, *wmWindow, "_NET_WM_NAME");
    if(wmName == NULL)
        wmName = (char*) xcbGetProperty(data, connection, *wmWindow, "WM_NAME");

    free(wmWindow);

    if(wmName == NULL)
        return;

    if(*wmName == '\0')
    {
        free(wmName);
        return;
    }

    ffStrbufSetS(&result->wmProcessName, wmName);

    free(wmName);
}

void ffdsConnectXcb(FFDisplayServerResult* result)
{
    FF_LIBRARY_LOAD(xcb, &instance.config.libXcb, , "libxcb" FF_LIBRARY_EXTENSION, 2)
    FF_LIBRARY_LOAD_SYMBOL(xcb, xcb_connect,)
    FF_LIBRARY_LOAD_SYMBOL(xcb, xcb_get_setup,)
    FF_LIBRARY_LOAD_SYMBOL(xcb, xcb_setup_roots_iterator,)
    FF_LIBRARY_LOAD_SYMBOL(xcb, xcb_screen_next,)
    FF_LIBRARY_LOAD_SYMBOL(xcb, xcb_disconnect,)

    XcbPropertyData propertyData;
    bool propertyDataInitialized = xcbInitPropertyData(xcb, &propertyData);

    xcb_connection_t* connection = ffxcb_connect(NULL, NULL);
    if(connection == NULL)
        return;

    xcb_screen_iterator_t iterator = ffxcb_setup_roots_iterator(ffxcb_get_setup(connection));

    if(iterator.rem > 0 && propertyDataInitialized)
        xcbDetectWMfromEWMH(&propertyData, connection, iterator.data->root, result);

    while(iterator.rem > 0)
    {
        ffdsAppendDisplay(result,
            (uint32_t) iterator.data->width_in_pixels,
            (uint32_t) iterator.data->height_in_pixels,
            0,
            (uint32_t) iterator.data->width_in_pixels,
            (uint32_t) iterator.data->height_in_pixels,
            0,
            NULL,
            FF_DISPLAY_TYPE_UNKNOWN,
            false,
            0
        );
        ffxcb_screen_next(&iterator);
    }

    ffxcb_disconnect(connection);

    //If wayland hasn't set this, connection failed for it. So we are running only a X Server, not XWayland.
    if(result->wmProtocolName.length == 0)
        ffStrbufSetS(&result->wmProtocolName, FF_WM_PROTOCOL_X11);
}

#else

void ffdsConnectXcb(FFDisplayServerResult* result)
{
    //Do nothing. There are other implementations coming
    FF_UNUSED(result)
}

#endif

#ifdef FF_HAVE_XCB_RANDR
#include "util/edidHelper.h"
#include <xcb/randr.h>

typedef struct XcbRandrData
{
    FF_LIBRARY_SYMBOL(xcb_randr_get_screen_resources)
    FF_LIBRARY_SYMBOL(xcb_randr_get_screen_resources_reply)
    FF_LIBRARY_SYMBOL(xcb_randr_get_screen_resources_modes_iterator)
    FF_LIBRARY_SYMBOL(xcb_randr_get_screen_info)
    FF_LIBRARY_SYMBOL(xcb_randr_get_screen_info_reply)
    FF_LIBRARY_SYMBOL(xcb_randr_mode_info_next)
    FF_LIBRARY_SYMBOL(xcb_randr_get_monitors)
    FF_LIBRARY_SYMBOL(xcb_randr_get_monitors_reply)
    FF_LIBRARY_SYMBOL(xcb_randr_get_monitors_monitors_iterator)
    FF_LIBRARY_SYMBOL(xcb_randr_monitor_info_next)
    FF_LIBRARY_SYMBOL(xcb_randr_monitor_info_outputs_length)
    FF_LIBRARY_SYMBOL(xcb_randr_monitor_info_outputs)
    FF_LIBRARY_SYMBOL(xcb_randr_output_next)
    FF_LIBRARY_SYMBOL(xcb_randr_get_output_info)
    FF_LIBRARY_SYMBOL(xcb_randr_get_output_info_reply)
    FF_LIBRARY_SYMBOL(xcb_randr_get_crtc_info)
    FF_LIBRARY_SYMBOL(xcb_randr_get_crtc_info_reply)
    FF_LIBRARY_SYMBOL(xcb_randr_get_output_property)
    FF_LIBRARY_SYMBOL(xcb_randr_get_output_property_reply)
    FF_LIBRARY_SYMBOL(xcb_randr_get_output_property_data)
    FF_LIBRARY_SYMBOL(xcb_randr_get_output_property_data_length)
    FF_LIBRARY_SYMBOL(xcb_intern_atom)
    FF_LIBRARY_SYMBOL(xcb_intern_atom_reply)

    //init once
    xcb_connection_t* connection;
    FFDisplayServerResult* result;
    XcbPropertyData propData;

    //init per screen
    uint32_t defaultRefreshRate;
    uint32_t defaultRotation;
    xcb_randr_get_screen_resources_reply_t* screenResources;
} XcbRandrData;

static bool xcbRandrHandleModeInfo(XcbRandrData* data, xcb_randr_mode_info_t* modeInfo, FFstrbuf* name, uint32_t rotation, bool primary)
{
    double refreshRate = (double) modeInfo->dot_clock / (double) (modeInfo->htotal * modeInfo->vtotal);

    return ffdsAppendDisplay(
        data->result,
        (uint32_t) modeInfo->width,
        (uint32_t) modeInfo->height,
        refreshRate == 0 ? data->defaultRefreshRate : refreshRate,
        (uint32_t) modeInfo->width,
        (uint32_t) modeInfo->height,
        rotation,
        name,
        FF_DISPLAY_TYPE_UNKNOWN,
        primary,
        0
    );
}

static bool xcbRandrHandleMode(XcbRandrData* data, xcb_randr_mode_t mode, FFstrbuf* name, uint32_t rotation, bool primary)
{
    //We do the check here, because we want the best fallback display if this call failed
    if(data->screenResources == NULL)
        return false;

    xcb_randr_mode_info_iterator_t modesIterator = data->ffxcb_randr_get_screen_resources_modes_iterator(data->screenResources);

    while(modesIterator.rem > 0)
    {
        if(modesIterator.data->id == mode)
            return xcbRandrHandleModeInfo(data, modesIterator.data, name, rotation, primary);

        data->ffxcb_randr_mode_info_next(&modesIterator);
    }

    return false;
}

static bool xcbRandrHandleCrtc(XcbRandrData* data, xcb_randr_crtc_t crtc, FFstrbuf* name, bool primary)
{
    xcb_randr_get_crtc_info_cookie_t crtcInfoCookie = data->ffxcb_randr_get_crtc_info(data->connection, crtc, XCB_CURRENT_TIME);
    xcb_randr_get_crtc_info_reply_t* crtcInfoReply = data->ffxcb_randr_get_crtc_info_reply(data->connection, crtcInfoCookie, NULL);
    if(crtcInfoReply == NULL)
        return false;

    uint32_t rotation;
    switch (crtcInfoReply->rotation)
    {
        case XCB_RANDR_ROTATION_ROTATE_90:
            rotation = 90;
            break;
        case XCB_RANDR_ROTATION_ROTATE_180:
            rotation = 180;
            break;
        case XCB_RANDR_ROTATION_ROTATE_270:
            rotation = 270;
            break;
        default:
            rotation = 0;
            break;
    }
    bool res = xcbRandrHandleMode(data, crtcInfoReply->mode, name, rotation, primary);
    res = res ? true : ffdsAppendDisplay(
        data->result,
        (uint32_t) crtcInfoReply->width,
        (uint32_t) crtcInfoReply->height,
        data->defaultRefreshRate,
        (uint32_t) crtcInfoReply->width,
        (uint32_t) crtcInfoReply->height,
        rotation,
        name,
        FF_DISPLAY_TYPE_UNKNOWN,
        primary,
        0
    );

    free(crtcInfoReply);
    return res;
}

static bool xcbRandrHandleOutput(XcbRandrData* data, xcb_randr_output_t output, FFstrbuf* name, bool primary)
{
    xcb_randr_get_output_info_cookie_t outputInfoCookie = data->ffxcb_randr_get_output_info(data->connection, output, XCB_CURRENT_TIME);
    FF_AUTO_FREE xcb_randr_get_output_info_reply_t* outputInfoReply = data->ffxcb_randr_get_output_info_reply(data->connection, outputInfoCookie, NULL);
    if(outputInfoReply == NULL)
        return false;

    xcb_intern_atom_cookie_t requestAtomCookie = data->ffxcb_intern_atom(data->connection, true, (uint16_t) strlen("EDID"), "EDID");
    FF_AUTO_FREE xcb_intern_atom_reply_t* requestAtomReply = data->ffxcb_intern_atom_reply(data->connection, requestAtomCookie, NULL);

    xcb_randr_get_output_property_cookie_t outputPropertyCookie = data->ffxcb_randr_get_output_property(data->connection, output, requestAtomReply->atom, XCB_GET_PROPERTY_TYPE_ANY, 0, 100, false, false);
    FF_AUTO_FREE xcb_randr_get_output_property_reply_t* outputPropertyReply = data->ffxcb_randr_get_output_property_reply(data->connection, outputPropertyCookie, NULL);
    if(outputPropertyReply)
    {
        if(data->ffxcb_randr_get_output_property_data_length(outputPropertyReply) >= 128)
        {
            ffStrbufClear(name);
            ffEdidGetName(data->ffxcb_randr_get_output_property_data(outputPropertyReply), name);
        }
    }

    bool res = xcbRandrHandleCrtc(data, outputInfoReply->crtc, name, primary);

    return res;
}

static bool xcbRandrHandleMonitor(XcbRandrData* data, xcb_randr_monitor_info_t* monitor)
{
    //for some reasons, we have to construct this our self
    xcb_randr_output_iterator_t outputIterator = {
        .index = 0,
        .data = data->ffxcb_randr_monitor_info_outputs(monitor),
        .rem = data->ffxcb_randr_monitor_info_outputs_length(monitor)
    };

    FF_AUTO_FREE xcb_get_atom_name_reply_t* nameReply = data->propData.ffxcb_get_atom_name_reply(
        data->connection,
        data->propData.ffxcb_get_atom_name(data->connection, monitor->name),
        NULL
    );
    FF_STRBUF_AUTO_DESTROY name = ffStrbufCreateNS(
        (uint32_t) data->propData.ffxcb_get_atom_name_name_length(nameReply),
        data->propData.ffxcb_get_atom_name_name(nameReply)
    );

    bool foundOutput = false;

    while(outputIterator.rem > 0)
    {
        if(xcbRandrHandleOutput(data, *outputIterator.data, &name, monitor->primary))
            foundOutput = true;
        data->ffxcb_randr_output_next(&outputIterator);
    };

    return foundOutput ? true : ffdsAppendDisplay(
        data->result,
        (uint32_t) monitor->width,
        (uint32_t) monitor->height,
        data->defaultRefreshRate,
        (uint32_t) monitor->width,
        (uint32_t) monitor->height,
        data->defaultRotation,
        &name,
        FF_DISPLAY_TYPE_UNKNOWN,
        !!monitor->primary,
        0
    );
}

static bool xcbRandrHandleMonitors(XcbRandrData* data, xcb_screen_t* screen)
{
    xcb_randr_get_monitors_cookie_t monitorsCookie = data->ffxcb_randr_get_monitors(data->connection, screen->root, true);
    xcb_randr_get_monitors_reply_t* monitorsReply = data->ffxcb_randr_get_monitors_reply(data->connection, monitorsCookie, NULL);
    if(monitorsReply == NULL)
        return false;

    xcb_randr_monitor_info_iterator_t monitorInfoIterator = data->ffxcb_randr_get_monitors_monitors_iterator(monitorsReply);

    bool foundMonitor = false;

    while(monitorInfoIterator.rem > 0)
    {
        if(xcbRandrHandleMonitor(data, monitorInfoIterator.data))
            foundMonitor = true;
        data->ffxcb_randr_monitor_info_next(&monitorInfoIterator);
    }

    free(monitorsReply);

    return foundMonitor;
}

static void xcbRandrHandleScreen(XcbRandrData* data, xcb_screen_t* screen)
{
    //Init screen info. This is used to get the default refresh rate. If this fails, default refresh rate is simply 0.
    xcb_randr_get_screen_info_cookie_t screenInfoCookie = data->ffxcb_randr_get_screen_info(data->connection, screen->root);
    xcb_randr_get_screen_info_reply_t* screenInfoReply = data->ffxcb_randr_get_screen_info_reply(data->connection, screenInfoCookie, NULL);

    if(screenInfoReply != NULL)
    {
        data->defaultRefreshRate = screenInfoReply->rate;
        switch (screenInfoReply->rotation)
        {
            case XCB_RANDR_ROTATION_ROTATE_90:
                data->defaultRotation = 90;
                break;
            case XCB_RANDR_ROTATION_ROTATE_180:
                data->defaultRotation = 180;
                break;
            case XCB_RANDR_ROTATION_ROTATE_270:
                data->defaultRotation = 270;
                break;
            default:
                data->defaultRotation = 0;
                break;
        }
        free(screenInfoReply);
    }
    else
    {
        data->defaultRefreshRate = 0;
        data->defaultRotation = 0;
    }
    //Init screen resources. They are used to iterate over all modes. xcbRandrHandleMode checks for " == NULL", to fail as late as possible.
    xcb_randr_get_screen_resources_cookie_t screenResourcesCookie = data->ffxcb_randr_get_screen_resources(data->connection, screen->root);
    data->screenResources = data->ffxcb_randr_get_screen_resources_reply(data->connection, screenResourcesCookie, NULL);

    //With all the initialisation done, start the detection
    bool ret = xcbRandrHandleMonitors(data, screen);

    free(data->screenResources);

    if(ret)
        return;

    //If detetction failed, fallback to screen = monitor, like in the libxcb.so implementation
    ffdsAppendDisplay(
        data->result,
        (uint32_t) screen->width_in_pixels,
        (uint32_t) screen->height_in_pixels,
        data->defaultRefreshRate,
        (uint32_t) screen->width_in_pixels,
        (uint32_t) screen->height_in_pixels,
        data->defaultRotation,
        NULL,
        FF_DISPLAY_TYPE_UNKNOWN,
        false,
        0
    );
}

void ffdsConnectXcbRandr(FFDisplayServerResult* result)
{
    FF_LIBRARY_LOAD(xcbRandr, &instance.config.libXcbRandr, , "libxcb-randr" FF_LIBRARY_EXTENSION, 1)
    FF_LIBRARY_LOAD_SYMBOL(xcbRandr, xcb_connect,)
    FF_LIBRARY_LOAD_SYMBOL(xcbRandr, xcb_get_setup,)
    FF_LIBRARY_LOAD_SYMBOL(xcbRandr, xcb_setup_roots_iterator,)
    FF_LIBRARY_LOAD_SYMBOL(xcbRandr, xcb_screen_next,)
    FF_LIBRARY_LOAD_SYMBOL(xcbRandr, xcb_disconnect,)

    XcbRandrData data;

    FF_LIBRARY_LOAD_SYMBOL_VAR(xcbRandr, data, xcb_intern_atom,)
    FF_LIBRARY_LOAD_SYMBOL_VAR(xcbRandr, data, xcb_intern_atom_reply,)
    FF_LIBRARY_LOAD_SYMBOL_VAR(xcbRandr, data, xcb_randr_get_screen_resources,)
    FF_LIBRARY_LOAD_SYMBOL_VAR(xcbRandr, data, xcb_randr_get_screen_resources_reply,)
    FF_LIBRARY_LOAD_SYMBOL_VAR(xcbRandr, data, xcb_randr_get_screen_resources_modes_iterator,)
    FF_LIBRARY_LOAD_SYMBOL_VAR(xcbRandr, data, xcb_randr_get_screen_info,)
    FF_LIBRARY_LOAD_SYMBOL_VAR(xcbRandr, data, xcb_randr_get_screen_info_reply,)
    FF_LIBRARY_LOAD_SYMBOL_VAR(xcbRandr, data, xcb_randr_mode_info_next,)
    FF_LIBRARY_LOAD_SYMBOL_VAR(xcbRandr, data, xcb_randr_get_monitors,)
    FF_LIBRARY_LOAD_SYMBOL_VAR(xcbRandr, data, xcb_randr_get_monitors_reply,)
    FF_LIBRARY_LOAD_SYMBOL_VAR(xcbRandr, data, xcb_randr_get_monitors_monitors_iterator,)
    FF_LIBRARY_LOAD_SYMBOL_VAR(xcbRandr, data, xcb_randr_monitor_info_next,)
    FF_LIBRARY_LOAD_SYMBOL_VAR(xcbRandr, data, xcb_randr_monitor_info_outputs_length,)
    FF_LIBRARY_LOAD_SYMBOL_VAR(xcbRandr, data, xcb_randr_monitor_info_outputs,)
    FF_LIBRARY_LOAD_SYMBOL_VAR(xcbRandr, data, xcb_randr_output_next,)
    FF_LIBRARY_LOAD_SYMBOL_VAR(xcbRandr, data, xcb_randr_get_output_info,)
    FF_LIBRARY_LOAD_SYMBOL_VAR(xcbRandr, data, xcb_randr_get_output_info_reply,)
    FF_LIBRARY_LOAD_SYMBOL_VAR(xcbRandr, data, xcb_randr_get_output_property,)
    FF_LIBRARY_LOAD_SYMBOL_VAR(xcbRandr, data, xcb_randr_get_output_property_reply,)
    FF_LIBRARY_LOAD_SYMBOL_VAR(xcbRandr, data, xcb_randr_get_output_property_data,)
    FF_LIBRARY_LOAD_SYMBOL_VAR(xcbRandr, data, xcb_randr_get_output_property_data_length,)
    FF_LIBRARY_LOAD_SYMBOL_VAR(xcbRandr, data, xcb_randr_get_crtc_info,)
    FF_LIBRARY_LOAD_SYMBOL_VAR(xcbRandr, data, xcb_randr_get_crtc_info_reply,)

    bool propertyDataInitialized = xcbInitPropertyData(xcbRandr, &data.propData);

    data.connection = ffxcb_connect(NULL, NULL);
    if(data.connection == NULL)
        return;

    data.result = result;

    xcb_screen_iterator_t iterator = ffxcb_setup_roots_iterator(ffxcb_get_setup(data.connection));

    if(iterator.rem > 0 && propertyDataInitialized)
        xcbDetectWMfromEWMH(&data.propData, data.connection, iterator.data->root, result);

    while(iterator.rem > 0)
    {
        xcbRandrHandleScreen(&data, iterator.data);
        ffxcb_screen_next(&iterator);
    }

    ffxcb_disconnect(data.connection);

    //If wayland hasn't set this, connection failed for it. So we are running only a X Server, not XWayland.
    if(result->wmProtocolName.length == 0)
        ffStrbufSetS(&result->wmProtocolName, FF_WM_PROTOCOL_X11);
}

#else

void ffdsConnectXcbRandr(FFDisplayServerResult* result)
{
    //Do nothing. There are other implementations coming
    FF_UNUSED(result)
}

#endif
