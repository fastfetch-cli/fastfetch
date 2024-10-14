#ifdef FF_HAVE_WAYLAND

#include "wayland.h"
#include "wlr-output-management-unstable-v1-client-protocol.h"

static void waylandZwlrTransformListener(void* data, FF_MAYBE_UNUSED struct zwlr_output_head_v1 *zwlr_output_head_v1, int32_t transform)
{
    WaylandDisplay* wldata = (WaylandDisplay*) data;
    wldata->transform = (enum wl_output_transform) transform;
}

static void waylandZwlrScaleListener(void* data, FF_MAYBE_UNUSED struct zwlr_output_head_v1 *zwlr_output_head_v1, wl_fixed_t scale)
{
    WaylandDisplay* wldata = (WaylandDisplay*) data;
    wldata->scale = wl_fixed_to_double(scale);
}

typedef struct WaylandZwlrMode
{
    int32_t width;
    int32_t height;
    int32_t refreshRate;
    struct zwlr_output_mode_v1* pMode;
} WaylandZwlrMode;

static void waylandZwlrSizeListener(void* data, FF_MAYBE_UNUSED struct zwlr_output_mode_v1 *zwlr_output_mode_v1, int32_t width, int32_t height)
{
    WaylandZwlrMode* mode = (WaylandZwlrMode*) data;
    mode->width = width;
    mode->height = height;
}

static void waylandZwlrRefreshListener(void* data, FF_MAYBE_UNUSED struct zwlr_output_mode_v1 *zwlr_output_mode_v1, int32_t rate)
{
    WaylandZwlrMode* mode = (WaylandZwlrMode*) data;
    mode->refreshRate = rate;
}

static const struct zwlr_output_mode_v1_listener modeListener = {
    .size = waylandZwlrSizeListener,
    .refresh = waylandZwlrRefreshListener,
    .preferred = (void*) stubListener,
    .finished = (void*) stubListener,
};

static void waylandZwlrModeListener(void* data, FF_MAYBE_UNUSED struct zwlr_output_head_v1 *zwlr_output_head_v1, struct zwlr_output_mode_v1 *mode)
{
    WaylandDisplay* wldata = (WaylandDisplay*) data;
    WaylandZwlrMode* newMode = ffListAdd((FFlist*) wldata->internal);
    newMode->pMode = mode;

    // Strangely, the listener is called only in this function, but not in `waylandZwlrCurrentModeListener`
    wldata->parent->ffwl_proxy_add_listener((struct wl_proxy *) mode, (void (**)(void)) &modeListener, newMode);
}

static void waylandZwlrCurrentModeListener(void* data, FF_MAYBE_UNUSED struct zwlr_output_head_v1 *zwlr_output_head_v1, struct zwlr_output_mode_v1 *mode)
{
    // waylandZwlrModeListener is always run before this
    WaylandDisplay* wldata = (WaylandDisplay*) data;
    WaylandZwlrMode* current = NULL;
    FF_LIST_FOR_EACH(WaylandZwlrMode, m, *(FFlist*) wldata->internal)
    {
        if (m->pMode == mode)
        {
            current = m;
            break;
        }
    }
    wldata->width = current->width;
    wldata->height = current->height;
    wldata->refreshRate = current->refreshRate;
}

static void waylandZwlrPhysicalSizeListener(void* data, FF_MAYBE_UNUSED struct zwlr_output_head_v1 *zwlr_output_head_v1, int32_t width, int32_t height)
{
    WaylandDisplay* wldata = (WaylandDisplay*) data;
    wldata->physicalWidth = width;
    wldata->physicalHeight = height;
}

static const struct zwlr_output_head_v1_listener headListener = {
    .name = (void*) ffWaylandOutputNameListener,
    .description = (void*) ffWaylandOutputDescriptionListener,
    .physical_size = waylandZwlrPhysicalSizeListener,
    .mode = waylandZwlrModeListener,
    .enabled = (void*) stubListener,
    .current_mode = waylandZwlrCurrentModeListener,
    .position = (void*) stubListener,
    .transform = waylandZwlrTransformListener,
    .scale = waylandZwlrScaleListener,
    .finished = (void*) stubListener,
    .make = (void*) stubListener,
    .model = (void*) stubListener,
    .serial_number = (void*) stubListener,
    .adaptive_sync = (void*) stubListener,
};

static void waylandHandleZwlrHead(void *data, FF_MAYBE_UNUSED struct zwlr_output_manager_v1 *zwlr_output_manager_v1, struct zwlr_output_head_v1 *head)
{
    WaylandData* wldata = data;

    FF_LIST_AUTO_DESTROY modes = ffListCreate(sizeof(WaylandZwlrMode));
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
        .internal = &modes,
    };

    wldata->ffwl_proxy_add_listener((struct wl_proxy*) head, (void(**)(void)) &headListener, &display);
    wldata->ffwl_display_roundtrip(wldata->display);

    if(display.width <= 0 || display.height <= 0)
        return;

    uint32_t rotation = ffWaylandHandleRotation(&display);

    FFDisplayResult* item = ffdsAppendDisplay(wldata->result,
        (uint32_t) display.width,
        (uint32_t) display.height,
        display.refreshRate / 1000.0,
        (uint32_t) (display.width / display.scale + 0.5),
        (uint32_t) (display.height / display.scale + 0.5),
        rotation,
        display.edidName.length
            ? &display.edidName
            : display.description.length && !ffStrbufContain(&display.description, &display.name)
                ? &display.description
                : &display.name,
        display.type,
        false,
        display.id,
        (uint32_t) display.physicalWidth,
        (uint32_t) display.physicalHeight,
        "wayland-zwlr"
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

    // These must be released manually
    FF_LIST_FOR_EACH(WaylandZwlrMode, m, modes)
        wldata->ffwl_proxy_destroy((void*) m->pMode);
    wldata->ffwl_proxy_destroy((void*) head);
}

void ffWaylandHandleZwlrOutput(WaylandData* wldata, struct wl_registry* registry, uint32_t name, uint32_t version)
{
    struct wl_proxy* output = wldata->ffwl_proxy_marshal_constructor_versioned((struct wl_proxy*) registry, WL_REGISTRY_BIND, &zwlr_output_manager_v1_interface, version, name, zwlr_output_manager_v1_interface.name, version, NULL);
    if(output == NULL)
        return;

    const struct zwlr_output_manager_v1_listener outputListener = {
        .head = waylandHandleZwlrHead,
        .done = (void*) stubListener,
        .finished = (void*) stubListener,
    };

    wldata->ffwl_proxy_add_listener(output, (void(**)(void)) &outputListener, wldata);
    wldata->ffwl_display_roundtrip(wldata->display);
    wldata->ffwl_proxy_destroy(output);
}

#endif
