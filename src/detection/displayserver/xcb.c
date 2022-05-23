#include "displayServer.h"

#ifdef FF_HAVE_XCB
#include <xcb/xcb.h>
#include <string.h>

typedef struct XcbPropertyData
{
    FF_LIBRARY_SYMBOL(xcb_intern_atom)
    FF_LIBRARY_SYMBOL(xcb_intern_atom_reply)
    FF_LIBRARY_SYMBOL(xcb_get_property)
    FF_LIBRARY_SYMBOL(xcb_get_property_reply)
    FF_LIBRARY_SYMBOL(xcb_get_property_value)
    FF_LIBRARY_SYMBOL(xcb_get_property_value_length)
} XcbPropertyData;

static bool xcbInitPropertyData(void* libraryHandle, XcbPropertyData* propertyData)
{
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(libraryHandle, propertyData->ffxcb_intern_atom, xcb_intern_atom, false)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(libraryHandle, propertyData->ffxcb_intern_atom_reply, xcb_intern_atom_reply, false)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(libraryHandle, propertyData->ffxcb_get_property, xcb_get_property, false)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(libraryHandle, propertyData->ffxcb_get_property_reply, xcb_get_property_reply, false)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(libraryHandle, propertyData->ffxcb_get_property_value, xcb_get_property_value, false)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(libraryHandle, propertyData->ffxcb_get_property_value_length, xcb_get_property_value_length, false)

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
    if(result->wmProcessName.length > 0)
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

void ffdsConnectXcb(const FFinstance* instance, FFDisplayServerResult* result)
{
    FF_LIBRARY_LOAD(xcb, instance->config.libXcb, , "libxcb.so", 2)
    FF_LIBRARY_LOAD_SYMBOL(xcb, xcb_connect,)
    FF_LIBRARY_LOAD_SYMBOL(xcb, xcb_get_setup,)
    FF_LIBRARY_LOAD_SYMBOL(xcb, xcb_setup_roots_iterator,)
    FF_LIBRARY_LOAD_SYMBOL(xcb, xcb_screen_next,)
    FF_LIBRARY_LOAD_SYMBOL(xcb, xcb_disconnect,)

    XcbPropertyData propertyData;
    bool propertyDataInitialized = xcbInitPropertyData(xcb, &propertyData);

    xcb_connection_t* connection = ffxcb_connect(NULL, NULL);
    if(connection == NULL)
    {
        dlclose(xcb);
        return;
    }

    xcb_screen_iterator_t iterator = ffxcb_setup_roots_iterator(ffxcb_get_setup(connection));

    if(iterator.rem > 0 && propertyDataInitialized)
        xcbDetectWMfromEWMH(&propertyData, connection, iterator.data->root, result);

    while(iterator.rem > 0)
    {
        ffdsAppendResolution(
            result,
            (uint32_t) iterator.data->width_in_pixels,
            (uint32_t) iterator.data->height_in_pixels,
            0
        );
        ffxcb_screen_next(&iterator);
    }

    ffxcb_disconnect(connection);
    dlclose(xcb);

    //If wayland hasn't set this, connection failed for it. So we are running only a X Server, not XWayland.
    if(result->wmProtocolName.length == 0)
        ffStrbufSetS(&result->wmProtocolName, FF_DISPLAYSERVER_PROTOCOL_X11);
}

#else

void ffdsConnectXcb(const FFinstance* instance, FFDisplayServerResult* result)
{
    //Do nothing. There are other implementations coming
    FF_UNUSED(instance, result)
}

#endif

#ifdef FF_HAVE_XCB_RANDR
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

    //init once
    xcb_connection_t* connection;
    FFDisplayServerResult* result;

    //init per screen
    uint32_t defaultRefreshRate;
    xcb_randr_get_screen_resources_reply_t* screenResources;
} XcbRandrData;

static bool xcbRandrHandleModeInfo(XcbRandrData* data, xcb_randr_mode_info_t* modeInfo)
{
    uint32_t refreshRate = ffdsParseRefreshRate((int32_t) (
        modeInfo->dot_clock / (uint32_t) (modeInfo->htotal * modeInfo->vtotal)
    ));

    return ffdsAppendResolution(
        data->result,
        (uint32_t) modeInfo->width,
        (uint32_t) modeInfo->height,
        refreshRate == 0 ? data->defaultRefreshRate : refreshRate
    );
}

static bool xcbRandrHandleMode(XcbRandrData* data, xcb_randr_mode_t mode)
{
    //We do the check here, because we want the best fallback resolution if this call failed
    if(data->screenResources == NULL)
        return false;

    xcb_randr_mode_info_iterator_t modesIterator = data->ffxcb_randr_get_screen_resources_modes_iterator(data->screenResources);

    while(modesIterator.rem > 0)
    {
        if(modesIterator.data->id == mode)
            return xcbRandrHandleModeInfo(data, modesIterator.data);

        data->ffxcb_randr_mode_info_next(&modesIterator);
    }

    return false;
}

static bool xcbRandrHandleCrtc(XcbRandrData* data, xcb_randr_crtc_t crtc)
{
    xcb_randr_get_crtc_info_cookie_t crtcInfoCookie = data->ffxcb_randr_get_crtc_info(data->connection, crtc, XCB_CURRENT_TIME);
    xcb_randr_get_crtc_info_reply_t* crtcInfoReply = data->ffxcb_randr_get_crtc_info_reply(data->connection, crtcInfoCookie, NULL);
    if(crtcInfoReply == NULL)
        return false;

    bool res = xcbRandrHandleMode(data, crtcInfoReply->mode);
    res = res ? true : ffdsAppendResolution(
        data->result,
        (uint32_t) crtcInfoReply->width,
        (uint32_t) crtcInfoReply->height,
        data->defaultRefreshRate
    );

    free(crtcInfoReply);
    return res;
}

static bool xcbRandrHandleOutput(XcbRandrData* data, xcb_randr_output_t output)
{
    xcb_randr_get_output_info_cookie_t outputInfoCookie = data->ffxcb_randr_get_output_info(data->connection, output, XCB_CURRENT_TIME);
    xcb_randr_get_output_info_reply_t* outputInfoReply = data->ffxcb_randr_get_output_info_reply(data->connection, outputInfoCookie, NULL);
    if(outputInfoReply == NULL)
        return false;

    bool res = xcbRandrHandleCrtc(data, outputInfoReply->crtc);

    free(outputInfoReply);

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

    bool foundOutput = false;

    while(outputIterator.rem > 0)
    {
        if(xcbRandrHandleOutput(data, *outputIterator.data))
            foundOutput = true;
        data->ffxcb_randr_output_next(&outputIterator);
    };

    return foundOutput ? true : ffdsAppendResolution(
        data->result,
        (uint32_t) monitor->width,
        (uint32_t) monitor->height,
        data->defaultRefreshRate
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
        free(screenInfoReply);
    }
    else
        data->defaultRefreshRate = 0;

    //Init screen resources. They are used to iterate over all modes. xcbRandrHandleMode checks for " == NULL", to fail as late as possible.
    xcb_randr_get_screen_resources_cookie_t screenResourcesCookie = data->ffxcb_randr_get_screen_resources(data->connection, screen->root);
    data->screenResources = data->ffxcb_randr_get_screen_resources_reply(data->connection, screenResourcesCookie, NULL);

    //With all the initialisation done, start the detection
    bool ret = xcbRandrHandleMonitors(data, screen);

    free(data->screenResources);

    if(ret)
        return;

    //If detetction failed, fallback to screen = monitor, like in the libxcb.so implementation
    ffdsAppendResolution(
        data->result,
        (uint32_t) screen->width_in_pixels,
        (uint32_t) screen->height_in_pixels,
        data->defaultRefreshRate
    );
}

void ffdsConnectXcbRandr(const FFinstance* instance, FFDisplayServerResult* result)
{
    FF_LIBRARY_LOAD(xcbRandr, instance->config.libXcbRandr, , "libxcb-randr.so", 1)
    FF_LIBRARY_LOAD_SYMBOL(xcbRandr, xcb_connect,)
    FF_LIBRARY_LOAD_SYMBOL(xcbRandr, xcb_get_setup,)
    FF_LIBRARY_LOAD_SYMBOL(xcbRandr, xcb_setup_roots_iterator,)
    FF_LIBRARY_LOAD_SYMBOL(xcbRandr, xcb_screen_next,)
    FF_LIBRARY_LOAD_SYMBOL(xcbRandr, xcb_disconnect,)

    XcbRandrData data;

    FF_LIBRARY_LOAD_SYMBOL_ADRESS(xcbRandr, data.ffxcb_randr_get_screen_resources, xcb_randr_get_screen_resources,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(xcbRandr, data.ffxcb_randr_get_screen_resources_reply, xcb_randr_get_screen_resources_reply,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(xcbRandr, data.ffxcb_randr_get_screen_resources_modes_iterator, xcb_randr_get_screen_resources_modes_iterator,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(xcbRandr, data.ffxcb_randr_get_screen_info, xcb_randr_get_screen_info,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(xcbRandr, data.ffxcb_randr_get_screen_info_reply, xcb_randr_get_screen_info_reply,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(xcbRandr, data.ffxcb_randr_mode_info_next, xcb_randr_mode_info_next,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(xcbRandr, data.ffxcb_randr_get_monitors, xcb_randr_get_monitors,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(xcbRandr, data.ffxcb_randr_get_monitors_reply, xcb_randr_get_monitors_reply,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(xcbRandr, data.ffxcb_randr_get_monitors_monitors_iterator, xcb_randr_get_monitors_monitors_iterator,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(xcbRandr, data.ffxcb_randr_monitor_info_next, xcb_randr_monitor_info_next,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(xcbRandr, data.ffxcb_randr_monitor_info_outputs_length, xcb_randr_monitor_info_outputs_length,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(xcbRandr, data.ffxcb_randr_monitor_info_outputs, xcb_randr_monitor_info_outputs,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(xcbRandr, data.ffxcb_randr_output_next, xcb_randr_output_next,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(xcbRandr, data.ffxcb_randr_get_output_info, xcb_randr_get_output_info,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(xcbRandr, data.ffxcb_randr_get_output_info_reply, xcb_randr_get_output_info_reply,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(xcbRandr, data.ffxcb_randr_get_crtc_info, xcb_randr_get_crtc_info,)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(xcbRandr, data.ffxcb_randr_get_crtc_info_reply, xcb_randr_get_crtc_info_reply,)

    XcbPropertyData propertyData;
    bool propertyDataInitialized = xcbInitPropertyData(xcbRandr, &propertyData);

    data.connection = ffxcb_connect(NULL, NULL);
    if(data.connection == NULL)
    {
        dlclose(xcbRandr);
        return;
    }

    data.result = result;

    xcb_screen_iterator_t iterator = ffxcb_setup_roots_iterator(ffxcb_get_setup(data.connection));

    if(iterator.rem > 0 && propertyDataInitialized)
        xcbDetectWMfromEWMH(&propertyData, data.connection, iterator.data->root, result);

    while(iterator.rem > 0)
    {
        xcbRandrHandleScreen(&data, iterator.data);
        ffxcb_screen_next(&iterator);
    }

    ffxcb_disconnect(data.connection);
    dlclose(xcbRandr);

    //If wayland hasn't set this, connection failed for it. So we are running only a X Server, not XWayland.
    if(result->wmProtocolName.length == 0)
        ffStrbufSetS(&result->wmProtocolName, FF_DISPLAYSERVER_PROTOCOL_X11);
}

#else

void ffdsConnectXcbRandr(const FFinstance* instance, FFDisplayServerResult* result)
{
    //Do nothing. There are other implementations coming
    FF_UNUSED(instance, result)
}

#endif
