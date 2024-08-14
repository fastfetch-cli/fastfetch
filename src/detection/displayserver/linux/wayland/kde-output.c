#ifdef FF_HAVE_WAYLAND

#include "wayland.h"
#include "kde-output-device-v2-client-protocol.h"
#include "kde-output-order-v1-client-protocol.h"
#include "util/edidHelper.h"
#include "util/base64.h"

typedef struct WaylandKdeMode
{
    int32_t width;
    int32_t height;
    int32_t refreshRate;
    struct kde_output_device_mode_v2* pMode;
} WaylandKdeMode;

static void waylandKdeSizeListener(void* data, FF_MAYBE_UNUSED struct kde_output_device_mode_v2 *_, int32_t width, int32_t height)
{
    WaylandKdeMode* mode = (WaylandKdeMode*) data;
    mode->width = width;
    mode->height = height;
}

static void waylandKdeRefreshListener(void* data, FF_MAYBE_UNUSED struct kde_output_device_mode_v2 *_, int32_t rate)
{
    WaylandKdeMode* mode = (WaylandKdeMode*) data;
    mode->refreshRate = rate;
}

static const struct kde_output_device_mode_v2_listener modeListener = {
    .size = waylandKdeSizeListener,
    .refresh = waylandKdeRefreshListener,
    .preferred = (void*) stubListener,
    .removed = (void*) stubListener,
};

static void waylandKdeModeListener(void* data, FF_MAYBE_UNUSED struct kde_output_device_v2* _, struct kde_output_device_mode_v2 *mode)
{
    WaylandDisplay* wldata = (WaylandDisplay*) data;
    if (!wldata->internal) return;

    WaylandKdeMode* newMode = ffListAdd((FFlist*) wldata->internal);
    newMode->pMode = mode;

    // Strangely, the listener is called only in this function, but not in `waylandKdeCurrentModeListener`
    wldata->parent->ffwl_proxy_add_listener((struct wl_proxy *) mode, (void (**)(void)) &modeListener, newMode);
}

static void waylandKdeCurrentModeListener(void* data, FF_MAYBE_UNUSED struct kde_output_device_v2 *_, struct kde_output_device_mode_v2 *mode)
{
    // waylandKdeModeListener is always run before this
    WaylandDisplay* wldata = (WaylandDisplay*) data;
    if (!wldata->internal) return;

    WaylandKdeMode* current = NULL;
    FF_LIST_FOR_EACH(WaylandKdeMode, m, *(FFlist*) wldata->internal)
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

static void waylandKdeScaleListener(void* data, FF_MAYBE_UNUSED struct kde_output_device_v2* _, wl_fixed_t scale)
{
    WaylandDisplay* wldata = (WaylandDisplay*) data;
    wldata->scale = wl_fixed_to_double(scale);
}

static void waylandKdeEdidListener(void* data, FF_MAYBE_UNUSED struct kde_output_device_v2* _, const char* raw)
{
    if (!*raw) return;
    WaylandDisplay* wldata = (WaylandDisplay*) data;
    FF_STRBUF_AUTO_DESTROY b64 = ffStrbufCreateStatic(raw);
    FF_STRBUF_AUTO_DESTROY edid = ffBase64DecodeStrbuf(&b64);
    if (edid.length < 128) return;
    ffEdidGetName((const uint8_t*) edid.chars, &wldata->edidName);
}

static void waylandKdeEnabledListener(void* data, FF_MAYBE_UNUSED struct kde_output_device_v2* _, int32_t enabled)
{
    WaylandDisplay* wldata = (WaylandDisplay*) data;
    if (!enabled) wldata->internal = NULL;
}

static void waylandKdeGeometryListener(void *data,
    FF_MAYBE_UNUSED struct kde_output_device_v2 *kde_output_device_v2,
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

static void waylandKdeNameListener(void* data, FF_MAYBE_UNUSED struct kde_output_device_v2* kde_output_device_v2, const char *name)
{
    WaylandDisplay* display = data;
    display->type = ffdsGetDisplayType(name);
    strncpy((char*) &display->id, name, sizeof(display->id));
    ffStrbufAppendS(&display->name, name);
}

static void waylandKdeHdrListener(void *data, FF_MAYBE_UNUSED struct kde_output_device_v2 *kde_output_device_v2, uint32_t hdr_enabled)
{
    WaylandDisplay* display = data;
    display->hdrEnabled = !!hdr_enabled;
}

static struct kde_output_device_v2_listener outputListener = {
    .geometry = waylandKdeGeometryListener,
    .current_mode = waylandKdeCurrentModeListener,
    .mode = waylandKdeModeListener,
    .done = (void*) stubListener,
    .scale = waylandKdeScaleListener,
    .edid = waylandKdeEdidListener,
    .enabled = waylandKdeEnabledListener,
    .uuid = (void*) stubListener,
    .serial_number = (void*) stubListener,
    .eisa_id = (void*) stubListener,
    .capabilities = (void*) stubListener,
    .overscan = (void*) stubListener,
    .vrr_policy = (void*) stubListener,
    .rgb_range = (void*) stubListener,
    .name = waylandKdeNameListener,
    .high_dynamic_range = waylandKdeHdrListener,
    .sdr_brightness = (void*) stubListener,
    .wide_color_gamut = (void*) stubListener,
    .auto_rotate_policy = (void*) stubListener,
    .icc_profile_path = (void*) stubListener,
    .brightness_metadata = (void*) stubListener,
    .brightness_overrides = (void*) stubListener,
    .sdr_gamut_wideness = (void*) stubListener,
    .color_profile_source = (void*) stubListener,
    .brightness = (void*) stubListener,
};

void ffWaylandHandleKdeOutput(WaylandData* wldata, struct wl_registry* registry, uint32_t name, uint32_t version)
{
    struct wl_proxy* output = wldata->ffwl_proxy_marshal_constructor_versioned((struct wl_proxy*) registry, WL_REGISTRY_BIND, &kde_output_device_v2_interface, version, name, kde_output_device_v2_interface.name, version, NULL);
    if(output == NULL)
        return;

    FF_LIST_AUTO_DESTROY modes = ffListCreate(sizeof(WaylandKdeMode));
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

    wldata->ffwl_proxy_add_listener(output, (void(**)(void)) &outputListener, &display);
    wldata->ffwl_display_roundtrip(wldata->display);
    wldata->ffwl_proxy_destroy(output);

    if(display.width <= 0 || display.height <= 0 || !display.internal)
        return;

    uint32_t rotation = ffWaylandHandleRotation(&display);

    FFDisplayResult* item = ffdsAppendDisplay(wldata->result,
        (uint32_t) display.width,
        (uint32_t) display.height,
        display.refreshRate / 1000.0,
        (uint32_t) (display.width / display.scale),
        (uint32_t) (display.height / display.scale),
        rotation,
        display.edidName.length
            ? &display.edidName
            : &display.name,
        display.type,
        false,
        display.id,
        (uint32_t) display.physicalWidth,
        (uint32_t) display.physicalHeight
    );
    if (item)
    {
        item->hdrEnabled = display.hdrEnabled;
    }

    ffStrbufDestroy(&display.description);
    ffStrbufDestroy(&display.name);
    ffStrbufDestroy(&display.edidName);
}


static void waylandKdeOutputOrderListener(void *data, FF_MAYBE_UNUSED struct kde_output_order_v1 *_, const char *output_name)
{
    uint64_t* id = (uint64_t*) data;
    if (*id == 0)
        *id = ffWaylandGenerateIdFromName(output_name);
}

void ffWaylandHandleKdeOutputOrder(WaylandData* wldata, struct wl_registry* registry, uint32_t name, uint32_t version)
{
    struct wl_proxy* output = wldata->ffwl_proxy_marshal_constructor_versioned((struct wl_proxy*) registry, WL_REGISTRY_BIND, &kde_output_order_v1_interface, version, name, kde_output_order_v1_interface.name, version, NULL);
    if(output == NULL)
        return;

    struct kde_output_order_v1_listener orderListener = {
        .output = waylandKdeOutputOrderListener,
        .done = (void*) stubListener,
    };

    wldata->ffwl_proxy_add_listener(output, (void(**)(void)) &orderListener, &wldata->primaryDisplayId);
    wldata->ffwl_display_roundtrip(wldata->display);
    wldata->ffwl_proxy_destroy(output);
}

#endif
