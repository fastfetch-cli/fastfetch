#ifdef FF_HAVE_WAYLAND

#include "wayland.h"
#include "util/stringUtils.h"

static void waylandOutputModeListener(void* data, FF_MAYBE_UNUSED struct wl_output* output, uint32_t flags, int32_t width, int32_t height, int32_t refreshRate)
{
    if(!(flags & WL_OUTPUT_MODE_CURRENT))
        return;

    WaylandDisplay* display = data;
    display->width = width;
    display->height = height;
    display->refreshRate = refreshRate;
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
    FF_MAYBE_UNUSED int32_t physical_width,
    FF_MAYBE_UNUSED int32_t physical_height,
    FF_MAYBE_UNUSED int32_t subpixel,
    FF_MAYBE_UNUSED const char *make,
    FF_MAYBE_UNUSED const char *model,
    int32_t transform)
{
    WaylandDisplay* display = data;
    display->transform = (enum wl_output_transform) transform;
}

void ffWaylandHandleGlobalOutput(WaylandData* wldata, struct wl_registry* registry, uint32_t name, uint32_t version)
{
    struct wl_proxy* output = wldata->ffwl_proxy_marshal_constructor_versioned((struct wl_proxy*) registry, WL_REGISTRY_BIND, wldata->ffwl_output_interface, version, name, wldata->ffwl_output_interface->name, version, NULL);
    if(output == NULL)
        return;

    WaylandDisplay display = {
        .parent = wldata,
        .width = 0,
        .height = 0,
        .refreshRate = 0,
        .scale = 1,
        .transform = WL_OUTPUT_TRANSFORM_NORMAL,
        .type = FF_DISPLAY_TYPE_UNKNOWN,
        .name = ffStrbufCreate(),
        .description = ffStrbufCreate(),
        .edidName = ffStrbufCreate(),
    };

    // Dirty hack for #477
    // The order of these callbacks MUST follow `struct wl_output_listener`
    void* outputListener[] = {
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

    wldata->ffwl_proxy_add_listener(output, (void(**)(void)) &outputListener, &display);
    wldata->ffwl_display_roundtrip(wldata->display);
    wldata->ffwl_proxy_destroy(output);

    if(display.width <= 0 || display.height <= 0)
        return;

    uint32_t rotation;
    switch(display.transform)
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
            int32_t temp = display.width;
            display.width = display.height;
            display.height = temp;
            break;
        }
        default:
            break;
    }

    ffdsAppendDisplay(wldata->result,
        (uint32_t) display.width,
        (uint32_t) display.height,
        display.refreshRate / 1000.0,
        (uint32_t) (display.width / display.scale),
        (uint32_t) (display.height / display.scale),
        rotation,
        display.edidName.length
            ? &display.edidName
            // Try ignoring `eDP-1-unknown`, where `unknown` is localized
            : display.description.length && !(ffStrbufStartsWithS(&display.description, display.name.chars) && display.description.chars[display.name.length] == '-')
                ? &display.description
                : &display.name,
        display.type,
        false,
        0
    );

    ffStrbufDestroy(&display.description);
    ffStrbufDestroy(&display.name);
    ffStrbufDestroy(&display.edidName);
}

#endif
