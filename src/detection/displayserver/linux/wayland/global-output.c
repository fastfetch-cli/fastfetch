#ifdef FF_HAVE_WAYLAND

#include "wayland.h"
#include "util/stringUtils.h"
#include "xdg-output-unstable-v1-client-protocol.h"

static void waylandOutputModeListener(void* data, FF_MAYBE_UNUSED struct wl_output* output, uint32_t flags, int32_t width, int32_t height, int32_t refreshRate)
{
    WaylandDisplay* display = data;

    if (flags & WL_OUTPUT_MODE_CURRENT)
    {
        display->width = width;
        display->height = height;
        display->refreshRate = refreshRate;
    }
    if (flags & WL_OUTPUT_MODE_PREFERRED)
    {
        display->preferredWidth = width;
        display->preferredHeight = height;
        display->preferredRefreshRate = refreshRate;
    }
}

static void waylandOutputScaleListener(void* data, FF_MAYBE_UNUSED struct wl_output* output, int32_t scale)
{
    WaylandDisplay* display = data;
    display->scale = scale;
}

static void waylandOutputGeometryListener(void *data,
    FF_MAYBE_UNUSED struct wl_output *output,
    FF_MAYBE_UNUSED int32_t x,
    FF_MAYBE_UNUSED int32_t y,
    int32_t physical_width,
    int32_t physical_height,
    FF_MAYBE_UNUSED int32_t subpixel,
    FF_MAYBE_UNUSED const char *make,
    FF_MAYBE_UNUSED const char *model,
    int32_t transform)
{
    WaylandDisplay* display = data;
    display->physicalWidth = physical_width;
    display->physicalHeight = physical_height;
    display->transform = (enum wl_output_transform) transform;
}

static void handleXdgLogicalSize(void *data, FF_MAYBE_UNUSED struct zxdg_output_v1 *_, int32_t width, FF_MAYBE_UNUSED int32_t height)
{
    WaylandDisplay* display = data;
    // Seems the values are only useful when ractional scale is enabled
    if (width < display->width)
    {
        display->scale = (double) display->width / width;
    }
}

// Dirty hack for #477
// The order of these callbacks MUST follow `struct wl_output_listener`
static void* outputListener[] = {
    waylandOutputGeometryListener, // geometry
    waylandOutputModeListener, // mode
    stubListener, // done
    waylandOutputScaleListener, // scale
    ffWaylandOutputNameListener, // name
    ffWaylandOutputDescriptionListener, // description
};
static_assert(
    sizeof(outputListener) >= sizeof(struct wl_output_listener),
    "sizeof(outputListener) is too small. Please report it to fastfetch github issue"
);

static struct zxdg_output_v1_listener zxdgOutputListener = {
    .logical_position = (void*) stubListener,
    .logical_size = handleXdgLogicalSize,
    .done = (void*) stubListener,
    .name = (void*) ffWaylandOutputNameListener,
    .description = (void*) ffWaylandOutputDescriptionListener,
};

const char* ffWaylandHandleGlobalOutput(WaylandData* wldata, struct wl_registry* registry, uint32_t name, uint32_t version)
{
    struct wl_proxy* output = wldata->ffwl_proxy_marshal_constructor_versioned((struct wl_proxy*) registry, WL_REGISTRY_BIND, wldata->ffwl_output_interface, version, name, wldata->ffwl_output_interface->name, version, NULL);
    if(output == NULL)
        return "Failed to create wl_output";

    WaylandDisplay display = {
        .parent = wldata,
        .scale = 1,
        .transform = WL_OUTPUT_TRANSFORM_NORMAL,
        .type = FF_DISPLAY_TYPE_UNKNOWN,
        .name = ffStrbufCreate(),
        .description = ffStrbufCreate(),
        .edidName = ffStrbufCreate(),
    };

    if (wldata->ffwl_proxy_add_listener(output, (void(**)(void)) &outputListener, &display) < 0)
    {
        wldata->ffwl_proxy_destroy(output);
        return "Failed to add listener to wl_output";
    }
    if (wldata->ffwl_display_roundtrip(wldata->display) < 0)
    {
        wldata->ffwl_proxy_destroy(output);
        return "Failed to roundtrip wl_output";
    }

    if (wldata->zxdgOutputManager)
    {
        struct wl_proxy* zxdgOutput = wldata->ffwl_proxy_marshal_constructor_versioned(wldata->zxdgOutputManager, ZXDG_OUTPUT_MANAGER_V1_GET_XDG_OUTPUT, &zxdg_output_v1_interface, version, NULL, output);

        if (zxdgOutput)
        {
            wldata->ffwl_proxy_add_listener(zxdgOutput, (void(**)(void)) &zxdgOutputListener, &display);
            wldata->ffwl_display_roundtrip(wldata->display);
            wldata->ffwl_proxy_destroy(zxdgOutput);
        }
    }

    wldata->ffwl_proxy_destroy(output);

    if(display.width <= 0 || display.height <= 0)
        return "Failed to get display information from wl_output";

    uint32_t rotation = ffWaylandHandleRotation(&display);

    FFDisplayResult* item = ffdsAppendDisplay(wldata->result,
        (uint32_t) display.width,
        (uint32_t) display.height,
        display.refreshRate / 1000.0,
        (uint32_t) (display.width / display.scale + .5),
        (uint32_t) (display.height / display.scale + .5),
        (uint32_t) display.preferredWidth,
        (uint32_t) display.preferredHeight,
        display.preferredRefreshRate / 1000.0,
        rotation,
        display.edidName.length
            ? &display.edidName
            // Try ignoring `eDP-1-unknown`, where `unknown` is localized
            : display.description.length && !ffStrbufContain(&display.description, &display.name)
                ? &display.description
                : &display.name,
        display.type,
        false,
        display.id,
        (uint32_t) display.physicalWidth,
        (uint32_t) display.physicalHeight,
        "wayland-global"
    );
    if (item)
    {
        if (display.hdrSupported)
            item->hdrStatus = FF_DISPLAY_HDR_STATUS_SUPPORTED;
        else if (display.hdrInfoAvailable)
            item->hdrStatus = FF_DISPLAY_HDR_STATUS_UNSUPPORTED;
        else
            item->hdrStatus = FF_DISPLAY_HDR_STATUS_UNKNOWN;

        item->manufactureYear = display.myear;
        item->manufactureWeek = display.mweek;
        item->serial = display.serial;
    }

    ffStrbufDestroy(&display.description);
    ffStrbufDestroy(&display.name);
    ffStrbufDestroy(&display.edidName);

    return NULL;
}

const char* ffWaylandHandleZxdgOutput(WaylandData* wldata, struct wl_registry* registry, uint32_t name, uint32_t version)
{
    struct wl_proxy* manager = wldata->ffwl_proxy_marshal_constructor_versioned((struct wl_proxy*) registry, WL_REGISTRY_BIND, &zxdg_output_manager_v1_interface, version, name, zxdg_output_manager_v1_interface.name, version, NULL);
    if(manager == NULL)
        return "Failed to create zxdg_output_manager_v1";

    wldata->zxdgOutputManager = manager;

    return NULL;
}

#endif
